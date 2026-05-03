/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using T3Foundation.Context;

namespace T3Foundation.Actions
{
  /// <summary>
  /// A discrete unit of business logic that operates on a Context.
  ///
  /// Actions encapsulate all domain logic OUTSIDE of Panels and Documents.
  /// Panels invoke Actions; they never contain business logic themselves.
  ///
  /// Actions can optionally support undo/redo by implementing <see cref="Undo"/>.
  ///
  /// <code>
  /// public class RenameAssetAction : IT3Action
  /// {
  ///     private readonly string _newName;
  ///     private string? _previousName;
  ///
  ///     public string ActionName => "Rename Asset";
  ///     public string Description => $"Rename to '{_newName}'";
  ///     public bool IsUndoable => true;
  ///
  ///     public RenameAssetAction(string newName) { _newName = newName; }
  ///
  ///     public bool CanExecute(IT3Context context) => !context.IsReadOnly;
  ///
  ///     public void Execute(IT3Context context)
  ///     {
  ///         _previousName = context.GetProperty&lt;string&gt;("name");
  ///         context.SetProperty("name", _newName);
  ///     }
  ///
  ///     public void Undo(IT3Context context)
  ///     {
  ///         context.SetProperty("name", _previousName);
  ///     }
  /// }
  /// </code>
  ///
  /// Based on Workflow-Driven Tool Design
  /// (See Remedy's Talk: https://www.youtube.com/watch?v=kAfb0yx07Po)
  /// </summary>
  public interface IT3Action
  {
    /// <summary>
    /// Short name for display in menus and undo history.
    /// </summary>
    string ActionName { get; }

    /// <summary>
    /// Human-readable description of what this action does.
    /// </summary>
    string Description { get; }

    /// <summary>
    /// Whether this action supports undo. If false, <see cref="Undo"/> should not be called.
    /// </summary>
    bool IsUndoable { get; }

    /// <summary>
    /// Check whether this action can currently be executed against the given Context.
    /// </summary>
    bool CanExecute(IT3Context context);

    /// <summary>
    /// Execute the action against the given Context.
    /// Implementations should capture any state needed for <see cref="Undo"/> before making changes.
    /// </summary>
    void Execute(IT3Context context);

    /// <summary>
    /// Reverse the effect of <see cref="Execute"/>. Only called if <see cref="IsUndoable"/> is true.
    /// </summary>
    void Undo(IT3Context context);
  }
}
