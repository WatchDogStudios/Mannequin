/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Linq;
using T3Foundation.Context;
using T3Foundation.Documents;
using T3Foundation.Services.DI;

namespace T3Foundation.Workflow
{
  /// <summary>
  /// Default implementation of <see cref="IT3WorkflowManager"/>.
  /// Routes Context types to Document types and manages the open document set.
  /// </summary>
  public class T3WorkflowManager : IT3WorkflowManager
  {
    private readonly List<WorkflowRegistration> _registrations = new();
    private readonly List<IT3Document> _openDocuments = new();
    private IT3Document? _activeDocument;

    public IT3Document? ActiveDocument => _activeDocument;
    public IReadOnlyList<IT3Document> OpenDocuments => _openDocuments;

    public event Action<IT3Document>? DocumentOpened;
    public event Action<IT3Document>? DocumentClosed;
    public event Action<IT3Document?>? ActiveDocumentChanged;

    // ─────────────────── Registration ───────────────────

    public void RegisterWorkflow<TContext, TDocument>()
      where TContext : IT3Context
      where TDocument : IT3Document
    {
      RegisterWorkflow(typeof(TContext), typeof(TDocument));
    }

    public void RegisterWorkflow(Type contextType, Type documentType)
    {
      RegisterWorkflow(contextType, documentType, predicate: null);
    }

    public void RegisterWorkflow(Type contextType, Type documentType, Func<IT3Context, bool>? predicate)
    {
      _registrations.Add(new WorkflowRegistration(contextType, documentType, predicate));
      T3Core.Log($"Workflow registered: {contextType.Name} -> {documentType.Name}", T3LogLevel.Debug);
    }

    // ─────────────────── Open / Close ───────────────────

    public IT3Document? OpenContext(IT3Context context)
    {
      var contextType = context.GetType();

      // Find matching registration (check predicates, most specific type first)
      var registration = _registrations
        .Where(r => r.ContextType.IsAssignableFrom(contextType))
        .Where(r => r.Predicate == null || r.Predicate(context))
        .OrderByDescending(r => GetTypeDepth(r.ContextType, contextType))
        .FirstOrDefault();

      if (registration == null)
      {
        T3Core.Log($"No workflow registered for context type '{contextType.Name}'.", T3LogLevel.Warning);
        return null;
      }

      // Resolve the Document
      IT3Document? document;
      try
      {
        document = T3ServiceCollection.Provider.GetService(registration.DocumentType) as IT3Document;
        document ??= Activator.CreateInstance(registration.DocumentType) as IT3Document;
      }
      catch (Exception ex)
      {
        T3Core.Log($"Failed to create document '{registration.DocumentType.Name}': {ex.Message}", T3LogLevel.Error);
        return null;
      }

      if (document == null)
      {
        T3Core.Log($"Failed to create document '{registration.DocumentType.Name}'.", T3LogLevel.Error);
        return null;
      }

      // Configure the Document with the Context
      document.Configure(context);
      _openDocuments.Add(document);

      T3Core.Log($"Opened document '{document.DocumentName}' for context '{context.ContextName}'.", T3LogLevel.Info);
      DocumentOpened?.Invoke(document);

      // Activate the new document
      ActivateDocument(document);

      return document;
    }

    public void ActivateDocument(IT3Document document)
    {
      if (_activeDocument == document)
        return;

      _activeDocument?.OnDeactivated();
      _activeDocument = document;
      _activeDocument?.OnActivated();

      ActiveDocumentChanged?.Invoke(_activeDocument);
    }

    public bool CloseDocument(IT3Document document)
    {
      if (!document.CanClose())
        return false;

      document.Close();
      _openDocuments.Remove(document);

      // If the closed document was active, activate the last open one
      if (_activeDocument == document)
      {
        _activeDocument = _openDocuments.LastOrDefault();
        _activeDocument?.OnActivated();
        ActiveDocumentChanged?.Invoke(_activeDocument);
      }

      T3Core.Log($"Closed document '{document.DocumentName}'.", T3LogLevel.Info);
      DocumentClosed?.Invoke(document);
      return true;
    }

    public void CloseAll()
    {
      // Close in reverse order
      for (int i = _openDocuments.Count - 1; i >= 0; i--)
        CloseDocument(_openDocuments[i]);
    }

    // ─────────────────── Helpers ───────────────────

    /// <summary>
    /// Get how many levels of inheritance between baseType and derivedType.
    /// Higher = more specific match (prefer derived type registrations).
    /// </summary>
    private static int GetTypeDepth(Type baseType, Type derivedType)
    {
      int depth = 0;
      var current = derivedType;
      while (current != null && current != baseType)
      {
        depth++;
        current = current.BaseType;
      }
      return depth;
    }

    private class WorkflowRegistration
    {
      public Type ContextType { get; }
      public Type DocumentType { get; }
      public Func<IT3Context, bool>? Predicate { get; }

      public WorkflowRegistration(Type contextType, Type documentType, Func<IT3Context, bool>? predicate)
      {
        ContextType = contextType;
        DocumentType = documentType;
        Predicate = predicate;
      }
    }
  }
}
