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
  /// Undo/redo stack for a single Document's editing session.
  ///
  /// Each Document should own one T3ActionHistory instance. Actions are executed
  /// through this class so they are automatically recorded for undo/redo.
  ///
  /// <code>
  /// var history = new T3ActionHistory();
  /// history.Execute(new RenameAssetAction("NewName"), context);
  /// history.Undo(context);   // reverts to old name
  /// history.Redo(context);   // re-applies "NewName"
  /// </code>
  /// </summary>
  public class T3ActionHistory
  {
    private readonly Stack<IT3Action> _undoStack = new();
    private readonly Stack<IT3Action> _redoStack = new();
    private int _maxHistorySize;

    /// <summary>
    /// Whether there are actions that can be undone.
    /// </summary>
    public bool CanUndo => _undoStack.Count > 0;

    /// <summary>
    /// Whether there are actions that can be redone.
    /// </summary>
    public bool CanRedo => _redoStack.Count > 0;

    /// <summary>
    /// Number of actions in the undo stack.
    /// </summary>
    public int UndoCount => _undoStack.Count;

    /// <summary>
    /// Number of actions in the redo stack.
    /// </summary>
    public int RedoCount => _redoStack.Count;

    /// <summary>
    /// Fired after an action is executed, undone, or redone.
    /// </summary>
    public event Action<IT3Action>? ActionExecuted;

    /// <summary>
    /// Fired when the undo/redo state changes (stack sizes change).
    /// </summary>
    public event Action? StateChanged;

    /// <param name="maxHistorySize">
    /// Maximum number of actions to keep in the undo stack. 0 = unlimited.
    /// When the limit is reached, the oldest action is discarded.
    /// </param>
    public T3ActionHistory(int maxHistorySize = 0)
    {
      _maxHistorySize = maxHistorySize;
    }

    /// <summary>
    /// Execute an action and push it onto the undo stack.
    /// Clears the redo stack (new action invalidates any redo history).
    /// </summary>
    /// <returns>True if the action was executed successfully.</returns>
    public bool Execute(IT3Action action, IT3Context context)
    {
      if (!action.CanExecute(context))
      {
        T3Core.Log($"Action '{action.ActionName}' cannot execute in current context.", T3LogLevel.Warning);
        return false;
      }

      try
      {
        action.Execute(context);
      }
      catch (Exception ex)
      {
        T3Core.Log($"Action '{action.ActionName}' failed: {ex.Message}", T3LogLevel.Error);
        return false;
      }

      if (action.IsUndoable)
      {
        _undoStack.Push(action);
        TrimUndoStack();
      }

      // New action invalidates redo history
      _redoStack.Clear();

      T3Core.Log($"Executed action '{action.ActionName}'.", T3LogLevel.Debug);
      ActionExecuted?.Invoke(action);
      StateChanged?.Invoke();
      return true;
    }

    /// <summary>
    /// Undo the most recent action.
    /// </summary>
    /// <returns>True if an action was undone.</returns>
    public bool Undo(IT3Context context)
    {
      if (!CanUndo)
        return false;

      var action = _undoStack.Pop();
      try
      {
        action.Undo(context);
      }
      catch (Exception ex)
      {
        T3Core.Log($"Undo of '{action.ActionName}' failed: {ex.Message}", T3LogLevel.Error);
        // Push it back — the undo failed, stack should remain consistent
        _undoStack.Push(action);
        return false;
      }

      _redoStack.Push(action);

      T3Core.Log($"Undid action '{action.ActionName}'.", T3LogLevel.Debug);
      ActionExecuted?.Invoke(action);
      StateChanged?.Invoke();
      return true;
    }

    /// <summary>
    /// Redo the most recently undone action.
    /// </summary>
    /// <returns>True if an action was redone.</returns>
    public bool Redo(IT3Context context)
    {
      if (!CanRedo)
        return false;

      var action = _redoStack.Pop();
      try
      {
        action.Execute(context);
      }
      catch (Exception ex)
      {
        T3Core.Log($"Redo of '{action.ActionName}' failed: {ex.Message}", T3LogLevel.Error);
        _redoStack.Push(action);
        return false;
      }

      _undoStack.Push(action);

      T3Core.Log($"Redid action '{action.ActionName}'.", T3LogLevel.Debug);
      ActionExecuted?.Invoke(action);
      StateChanged?.Invoke();
      return true;
    }

    /// <summary>
    /// Clear all undo and redo history.
    /// </summary>
    public void Clear()
    {
      _undoStack.Clear();
      _redoStack.Clear();
      StateChanged?.Invoke();
    }

    /// <summary>
    /// Peek at the next action that would be undone, without removing it.
    /// </summary>
    public IT3Action? PeekUndo() => CanUndo ? _undoStack.Peek() : null;

    /// <summary>
    /// Peek at the next action that would be redone, without removing it.
    /// </summary>
    public IT3Action? PeekRedo() => CanRedo ? _redoStack.Peek() : null;

    private void TrimUndoStack()
    {
      if (_maxHistorySize <= 0 || _undoStack.Count <= _maxHistorySize)
        return;

      // Stack doesn't support removing from the bottom, so rebuild
      var items = _undoStack.ToArray();
      _undoStack.Clear();
      // items[0] is the top (most recent), items[^1] is the bottom (oldest)
      // Keep only the most recent _maxHistorySize items
      for (int i = _maxHistorySize - 1; i >= 0; i--)
        _undoStack.Push(items[i]);
    }
  }
}
