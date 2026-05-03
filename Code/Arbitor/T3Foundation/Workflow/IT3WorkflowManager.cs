/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using T3Foundation.Context;
using T3Foundation.Documents;

namespace T3Foundation.Workflow
{
  /// <summary>
  /// Context-sensitive workflow routing.
  ///
  /// When a user selects data (an asset, a level, a script), the Workflow Manager
  /// determines which Document type to open based on the Context type, creates it,
  /// configures it with the context, and notifies the host window to build the tab layout.
  ///
  /// Based on Workflow-Driven Tool Design
  /// (See Remedy's Talk: https://www.youtube.com/watch?v=kAfb0yx07Po)
  /// </summary>
  public interface IT3WorkflowManager
  {
    /// <summary>
    /// Register a mapping: "when a Context of type TContext is opened, use Document of type TDocument."
    /// </summary>
    void RegisterWorkflow<TContext, TDocument>()
      where TContext : IT3Context
      where TDocument : IT3Document;

    /// <summary>
    /// Register a mapping using runtime types.
    /// </summary>
    void RegisterWorkflow(Type contextType, Type documentType);

    /// <summary>
    /// Register a conditional mapping with a predicate.
    /// The predicate is evaluated to decide if this Document applies to a given Context instance.
    /// </summary>
    void RegisterWorkflow(Type contextType, Type documentType, Func<IT3Context, bool> predicate);

    /// <summary>
    /// Open a Context: find the matching Document type, create it, configure it, and fire events.
    /// Returns the configured Document, or null if no workflow is registered for this context type.
    /// </summary>
    IT3Document? OpenContext(IT3Context context);

    /// <summary>
    /// The currently active/focused Document, or null.
    /// </summary>
    IT3Document? ActiveDocument { get; }

    /// <summary>
    /// All currently open Documents.
    /// </summary>
    IReadOnlyList<IT3Document> OpenDocuments { get; }

    /// <summary>
    /// Set a document as the active/focused one.
    /// </summary>
    void ActivateDocument(IT3Document document);

    /// <summary>
    /// Close a Document. Calls <see cref="IT3Document.CanClose"/> first.
    /// Returns false if the document refused to close.
    /// </summary>
    bool CloseDocument(IT3Document document);

    /// <summary>
    /// Close all open Documents.
    /// </summary>
    void CloseAll();

    // ─────────────────── Events ───────────────────

    /// <summary>
    /// Fired when a new Document is opened and configured.
    /// The host window should listen to this and create the tab/panel layout.
    /// </summary>
    event Action<IT3Document>? DocumentOpened;

    /// <summary>
    /// Fired when a Document is closed.
    /// </summary>
    event Action<IT3Document>? DocumentClosed;

    /// <summary>
    /// Fired when the active Document changes.
    /// </summary>
    event Action<IT3Document?>? ActiveDocumentChanged;
  }
}
