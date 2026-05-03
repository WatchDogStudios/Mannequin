/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using T3Foundation.Context;
using T3Foundation.Panel;
using T3Foundation.Services.DI;
using T3Foundation.Services.Dialog;

namespace T3Foundation.Documents
{
  /// <summary>
  /// Base implementation of <see cref="IT3Document"/>.
  /// Manages panel layout configuration and lifecycle.
  ///
  /// Subclass this and override <see cref="OnConfigure"/> to define your workflow's
  /// panel arrangement by calling <see cref="AddPanel{TPanel}"/>.
  /// </summary>
  public abstract class T3Document : IT3Document
  {
    private readonly List<T3PanelSlot> _panelSlots = new();

    public string DocumentId { get; } = Guid.NewGuid().ToString("N");
    public string DocumentName { get; protected set; } = "Document";
    public IT3Context? Context { get; private set; }
    public IReadOnlyList<T3PanelSlot> PanelLayout => _panelSlots;

    /// <summary>
    /// Configure this document for a context. Calls <see cref="OnConfigure"/> which
    /// subclasses override to add panels, then binds all panels to the context.
    /// </summary>
    public void Configure(IT3Context context)
    {
      Context = context ?? throw new ArgumentNullException(nameof(context));
      DocumentName = context.ContextName;

      _panelSlots.Clear();
      OnConfigure(context);

      // Resolve and bind all panels
      foreach (var slot in _panelSlots)
      {
        if (slot.Instance == null)
        {
          try
          {
            slot.Instance = T3ServiceCollection.Provider.GetService(slot.PanelType) as IT3Panel;
            slot.Instance ??= Activator.CreateInstance(slot.PanelType) as IT3Panel;
          }
          catch (Exception ex)
          {
            T3Core.Log($"Failed to create panel '{slot.PanelType.Name}': {ex.Message}", T3LogLevel.Error);
            continue;
          }
        }

        slot.Instance?.Bind(context);
      }

      T3Core.Log($"Document '{DocumentName}' configured with {_panelSlots.Count} panels.", T3LogLevel.Debug);
    }

    /// <summary>
    /// Override to define your workflow's panel layout by calling <see cref="AddPanel{TPanel}"/>.
    /// </summary>
    protected abstract void OnConfigure(IT3Context context);

    /// <summary>
    /// Add a panel to this document's layout.
    /// </summary>
    /// <typeparam name="TPanel">Panel type (must implement <see cref="IT3Panel"/>).</typeparam>
    /// <param name="position">Docking position.</param>
    /// <param name="header">Tab header text.</param>
    /// <param name="desiredWidth">Width hint for left/right docking.</param>
    /// <param name="desiredHeight">Height hint for bottom docking.</param>
    /// <param name="tabbedWith">Header of panel to tab alongside (for <see cref="T3DockPosition.Tabbed"/>).</param>
    protected T3PanelSlot AddPanel<TPanel>(
      T3DockPosition position,
      string header,
      double desiredWidth = 300,
      double desiredHeight = 200,
      string? tabbedWith = null) where TPanel : class, IT3Panel
    {
      var slot = new T3PanelSlot
      {
        PanelType = typeof(TPanel),
        Position = position,
        Header = header,
        DesiredWidth = desiredWidth,
        DesiredHeight = desiredHeight,
        TabbedWith = tabbedWith
      };
      _panelSlots.Add(slot);
      return slot;
    }

    /// <summary>
    /// Add a pre-created panel instance to the layout.
    /// </summary>
    protected T3PanelSlot AddPanel(IT3Panel panel, T3DockPosition position, string header)
    {
      var slot = new T3PanelSlot
      {
        PanelType = panel.GetType(),
        Position = position,
        Header = header,
        Instance = panel
      };
      _panelSlots.Add(slot);
      return slot;
    }

    public virtual void OnActivated()
    {
      T3Core.Log($"Document '{DocumentName}' activated.", T3LogLevel.Debug);
    }

    public virtual void OnDeactivated()
    {
      T3Core.Log($"Document '{DocumentName}' deactivated.", T3LogLevel.Debug);
    }

    /// <summary>
    /// Check if the document can close. If the context has unsaved changes,
    /// prompts the user via the dialog service.
    /// </summary>
    public virtual bool CanClose()
    {
      if (Context == null || !Context.IsDirty)
        return true;

      var dialogs = T3ServiceCollection.ResolveOptional<IT3DialogService>();
      if (dialogs == null)
        return true;

      var result = dialogs.ShowMessageAsync(
        "Unsaved Changes",
        $"'{DocumentName}' has unsaved changes. Close anyway?",
        T3DialogButton.YesNo).GetAwaiter().GetResult();

      return result == T3DialogResult.Yes;
    }

    /// <summary>
    /// Close the document. Unbinds all panels from the context.
    /// </summary>
    public virtual void Close()
    {
      foreach (var slot in _panelSlots)
        slot.Instance?.Unbind();

      _panelSlots.Clear();
      Context = null;

      T3Core.Log($"Document '{DocumentName}' closed.", T3LogLevel.Debug);
    }
  }
}
