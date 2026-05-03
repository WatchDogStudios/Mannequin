/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Windows.Controls;
using T3Foundation.Context;

namespace T3Foundation.Panel
{
  /// <summary>
  /// Base UserControl for T3 Panels. Implements <see cref="IT3Panel"/> with
  /// automatic Context binding and change subscription.
  ///
  /// Subclass this to create reusable visual components:
  /// <code>
  /// public class PropertyEditorPanel : T3PanelBase
  /// {
  ///     public PropertyEditorPanel() : base("Property Editor") { }
  ///
  ///     protected override void OnBound(IT3Context context)
  ///     {
  ///         // Populate UI from context properties
  ///     }
  ///
  ///     protected override void OnContextPropertyChanged(string path)
  ///     {
  ///         // Update specific UI elements when data changes
  ///     }
  /// }
  /// </code>
  ///
  /// Remember: Panels are DUMB. No business logic. Only display data and invoke Actions.
  /// </summary>
  public abstract class T3PanelBase : UserControl, IT3Panel
  {
    private readonly Action<string> _changeHandler;

    public string PanelId { get; } = Guid.NewGuid().ToString("N");
    public string PanelName { get; }
    public IT3Context? BoundContext { get; private set; }

    protected T3PanelBase(string panelName)
    {
      PanelName = panelName;
      _changeHandler = OnContextPropertyChanged;
    }

    /// <summary>
    /// Bind this panel to a Context. Sets the WPF DataContext and subscribes
    /// to all property changes via the wildcard "*" path.
    /// </summary>
    public void Bind(IT3Context context)
    {
      if (BoundContext != null)
        Unbind();

      BoundContext = context;

      // Subscribe to all property changes
      context.SubscribeToChanges("*", _changeHandler);

      // Set the WPF DataContext so XAML bindings work
      DataContext = context;

      try
      {
        OnBound(context);
      }
      catch (Exception ex)
      {
        T3Core.Log($"Panel '{PanelName}' OnBound error: {ex.Message}", T3LogLevel.Error);
      }

      T3Core.Log($"Panel '{PanelName}' bound to context '{context.ContextName}'.", T3LogLevel.Debug);
    }

    /// <summary>
    /// Unbind from the current Context. Clears DataContext and unsubscribes.
    /// </summary>
    public void Unbind()
    {
      if (BoundContext == null)
        return;

      BoundContext.UnsubscribeFromChanges("*", _changeHandler);

      try
      {
        OnUnbound();
      }
      catch (Exception ex)
      {
        T3Core.Log($"Panel '{PanelName}' OnUnbound error: {ex.Message}", T3LogLevel.Error);
      }

      DataContext = null;
      var previousName = BoundContext.ContextName;
      BoundContext = null;

      T3Core.Log($"Panel '{PanelName}' unbound from context '{previousName}'.", T3LogLevel.Debug);
    }

    /// <summary>
    /// Called after binding to a new Context. Override to populate your UI
    /// from the context's initial data.
    /// </summary>
    protected virtual void OnBound(IT3Context context) { }

    /// <summary>
    /// Called before unbinding from the current Context. Override to clean up
    /// panel-specific state.
    /// </summary>
    protected virtual void OnUnbound() { }

    /// <summary>
    /// Called when any property in the bound Context changes.
    /// Override to update specific UI elements reactively.
    /// </summary>
    /// <param name="path">The property path that changed.</param>
    protected virtual void OnContextPropertyChanged(string path) { }

    /// <summary>
    /// Helper to create a <see cref="T3PropertyAccessor"/> scoped to a subsection
    /// of the bound context's data.
    /// </summary>
    protected T3PropertyAccessor? CreateAccessor(string prefix)
    {
      if (BoundContext == null) return null;
      return new T3PropertyAccessor(BoundContext, prefix);
    }
  }
}
