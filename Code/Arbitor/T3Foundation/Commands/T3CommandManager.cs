/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Input;

namespace T3Foundation.Commands
{
  /// <summary>
  /// A registered keyboard shortcut binding.
  /// </summary>
  public class T3KeyBinding
  {
    public string Id { get; }
    public ICommand Command { get; }
    public KeyGesture Gesture { get; }
    public string Description { get; }

    public T3KeyBinding(string id, ICommand command, KeyGesture gesture, string description)
    {
      Id = id;
      Command = command;
      Gesture = gesture;
      Description = description;
    }

    /// <summary>
    /// Human-readable shortcut text (e.g. "Ctrl+Shift+R").
    /// </summary>
    public string ShortcutText => Gesture.GetDisplayStringForCulture(System.Globalization.CultureInfo.CurrentCulture);
  }

  /// <summary>
  /// Centralized keyboard shortcut and command binding manager.
  /// Register commands with key gestures, then apply them to windows.
  /// </summary>
  public class T3CommandManager
  {
    private readonly Dictionary<string, T3KeyBinding> _bindings = new();

    /// <summary>
    /// Register a command with a keyboard shortcut.
    /// </summary>
    /// <param name="id">Unique identifier (e.g. "app.runTests").</param>
    /// <param name="command">The ICommand to execute.</param>
    /// <param name="gesture">Key gesture (e.g. new KeyGesture(Key.F5)).</param>
    /// <param name="description">Human-readable description for UI display.</param>
    public void Register(string id, ICommand command, KeyGesture gesture, string description)
    {
      _bindings[id] = new T3KeyBinding(id, command, gesture, description);
      T3Core.Log($"Registered shortcut: {id} -> {gesture.GetDisplayStringForCulture(System.Globalization.CultureInfo.CurrentCulture)}", T3LogLevel.Debug);
    }

    /// <summary>
    /// Remove a registered command binding.
    /// </summary>
    public void Unregister(string id)
    {
      _bindings.Remove(id);
    }

    /// <summary>
    /// Get all registered bindings.
    /// </summary>
    public IReadOnlyList<T3KeyBinding> GetBindings() => _bindings.Values.ToList();

    /// <summary>
    /// Apply all registered keyboard bindings to a Window.
    /// </summary>
    public void ApplyTo(Window window)
    {
      foreach (var binding in _bindings.Values)
      {
        window.InputBindings.Add(new KeyBinding(binding.Command, binding.Gesture));
      }
      T3Core.Log($"Applied {_bindings.Count} keyboard bindings to {window.GetType().Name}.", T3LogLevel.Debug);
    }

    /// <summary>
    /// Remove all registered bindings from a Window.
    /// </summary>
    public void RemoveFrom(Window window)
    {
      var toRemove = window.InputBindings
        .OfType<KeyBinding>()
        .Where(kb => _bindings.Values.Any(b => b.Command == kb.Command))
        .ToList();

      foreach (var kb in toRemove)
        window.InputBindings.Remove(kb);
    }
  }
}
