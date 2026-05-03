/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using T3Foundation.Context;

namespace T3Foundation.Actions
{
  /// <summary>
  /// Groups multiple actions into a single undoable unit.
  ///
  /// When executed, all child actions run in order. When undone, all child
  /// actions are undone in reverse order. This appears as a single entry
  /// in the undo history.
  ///
  /// <code>
  /// var composite = new T3CompositeAction("Move and Rename",
  ///     new MoveAssetAction(newFolder),
  ///     new RenameAssetAction(newName));
  /// history.Execute(composite, context);
  /// // Ctrl+Z undoes both the move AND the rename in one step
  /// </code>
  /// </summary>
  public class T3CompositeAction : IT3Action
  {
    private readonly List<IT3Action> _actions;

    public string ActionName { get; }
    public string Description { get; }
    public bool IsUndoable { get; }

    /// <summary>
    /// The child actions in execution order.
    /// </summary>
    public IReadOnlyList<IT3Action> Actions => _actions;

    public T3CompositeAction(string actionName, params IT3Action[] actions)
      : this(actionName, actionName, actions)
    {
    }

    public T3CompositeAction(string actionName, string description, params IT3Action[] actions)
    {
      ActionName = actionName;
      Description = description;
      _actions = new List<IT3Action>(actions);
      // Composite is undoable only if ALL children are undoable
      IsUndoable = _actions.TrueForAll(a => a.IsUndoable);
    }

    public bool CanExecute(IT3Context context)
    {
      foreach (var action in _actions)
      {
        if (!action.CanExecute(context))
          return false;
      }
      return true;
    }

    public void Execute(IT3Context context)
    {
      int executed = 0;
      try
      {
        for (int i = 0; i < _actions.Count; i++)
        {
          _actions[i].Execute(context);
          executed++;
        }
      }
      catch (Exception ex)
      {
        // Roll back any actions that already executed
        T3Core.Log($"Composite action '{ActionName}' failed at step {executed + 1}: {ex.Message}. Rolling back.", T3LogLevel.Error);
        for (int i = executed - 1; i >= 0; i--)
        {
          try
          {
            if (_actions[i].IsUndoable)
              _actions[i].Undo(context);
          }
          catch (Exception undoEx)
          {
            T3Core.Log($"Rollback of step {i + 1} in '{ActionName}' failed: {undoEx.Message}", T3LogLevel.Error);
          }
        }
        throw;
      }
    }

    public void Undo(IT3Context context)
    {
      // Undo in reverse order
      for (int i = _actions.Count - 1; i >= 0; i--)
        _actions[i].Undo(context);
    }
  }
}
