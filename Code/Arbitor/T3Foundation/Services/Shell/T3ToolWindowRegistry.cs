/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Windows;
using Syncfusion.Windows.Tools.Controls;

namespace T3Foundation.Services.Shell
{
  /// <summary>
  /// Default <see cref="IT3ToolWindowRegistry"/> implementation. Singleton per app.
  /// </summary>
  public class T3ToolWindowRegistry : IT3ToolWindowRegistry
  {
    private readonly Dictionary<string, T3ToolWindowDescriptor> _descriptors = new Dictionary<string, T3ToolWindowDescriptor>();
    private readonly List<T3ToolWindowDescriptor> _ordered = new List<T3ToolWindowDescriptor>();
    private readonly Dictionary<string, FrameworkElement> _open = new Dictionary<string, FrameworkElement>();
    private DockingManager? _dock;

    public IReadOnlyList<T3ToolWindowDescriptor> All => _ordered;

    public event Action<T3ToolWindowDescriptor>? ToolWindowOpened;
    public event Action<T3ToolWindowDescriptor>? ToolWindowClosed;

    public void Register(T3ToolWindowDescriptor descriptor)
    {
      if (descriptor == null) throw new ArgumentNullException(nameof(descriptor));

      if (_descriptors.TryGetValue(descriptor.Id, out var existing))
      {
        // Replace: drop the old descriptor + close any open instance bound to it.
        Close(existing.Id);
        _ordered.Remove(existing);
      }

      _descriptors[descriptor.Id] = descriptor;
      _ordered.Add(descriptor);
      T3Core.Log($"ToolWindow registered: {descriptor.Id}", T3LogLevel.Debug);
    }

    public void Unregister(string id)
    {
      if (!_descriptors.TryGetValue(id, out var descriptor))
        return;

      Close(id);
      _descriptors.Remove(id);
      _ordered.Remove(descriptor);
    }

    public T3ToolWindowDescriptor? Find(string id)
    {
      _descriptors.TryGetValue(id, out var d);
      return d;
    }

    public bool IsOpen(string id) => _open.ContainsKey(id);

    public void AttachDockHost(DockingManager dockHost)
    {
      if (dockHost == null) throw new ArgumentNullException(nameof(dockHost));
      _dock = dockHost;
    }

    public void Open(string id)
    {
      if (_dock == null)
      {
        T3Core.Log($"ToolWindowRegistry.Open('{id}') called before dock host attached.", T3LogLevel.Warning);
        return;
      }

      if (!_descriptors.TryGetValue(id, out var descriptor))
      {
        T3Core.Log($"ToolWindow '{id}' is not registered.", T3LogLevel.Warning);
        return;
      }

      if (descriptor.IsSingleton && _open.TryGetValue(id, out var existing))
      {
        // Activate existing instance: bring its host tab to focus.
        DockingManager.SetState(existing, DockState.Dock);
        ToolWindowOpened?.Invoke(descriptor);
        return;
      }

      FrameworkElement content;
      try
      {
        content = descriptor.Factory();
      }
      catch (Exception ex)
      {
        T3Core.Log($"ToolWindow '{id}' factory threw: {ex.Message}", T3LogLevel.Error);
        return;
      }

      // Attached properties MUST be set BEFORE the child is added to Children -
      // Syncfusion's DockingManager reads them during the add to seed initial state.
      content.Name = SanitizeName(id);
      DockingManager.SetHeader(content, descriptor.Title);
      DockingManager.SetDesiredWidthInDockedMode(content, descriptor.DefaultWidth);
      DockingManager.SetDesiredHeightInDockedMode(content, descriptor.DefaultHeight);

      switch (descriptor.DefaultSide)
      {
        case T3DockSide.Left:
          DockingManager.SetSideInDockedMode(content, DockSide.Left);
          DockingManager.SetState(content, DockState.Dock);
          break;
        case T3DockSide.Right:
          DockingManager.SetSideInDockedMode(content, DockSide.Right);
          DockingManager.SetState(content, DockState.Dock);
          break;
        case T3DockSide.Top:
          DockingManager.SetSideInDockedMode(content, DockSide.Top);
          DockingManager.SetState(content, DockState.Dock);
          break;
        case T3DockSide.Bottom:
          DockingManager.SetSideInDockedMode(content, DockSide.Bottom);
          DockingManager.SetState(content, DockState.Dock);
          break;
        case T3DockSide.Document:
          DockingManager.SetState(content, DockState.Document);
          break;
        case T3DockSide.Tabbed:
          DockingManager.SetSideInDockedMode(content, DockSide.Tabbed);
          if (!string.IsNullOrEmpty(descriptor.TabbedWith))
            DockingManager.SetTargetNameInDockedMode(content, SanitizeName(descriptor.TabbedWith!));
          DockingManager.SetState(content, DockState.Dock);
          break;
      }

      _dock.Children.Add(content);
      _open[id] = content;

      T3Core.Log($"ToolWindow opened: {id}", T3LogLevel.Debug);
      ToolWindowOpened?.Invoke(descriptor);
    }

    public void Close(string id)
    {
      if (_dock == null) return;
      if (!_open.TryGetValue(id, out var content)) return;
      if (!_descriptors.TryGetValue(id, out var descriptor)) return;

      _dock.Children.Remove(content);
      _open.Remove(id);

      T3Core.Log($"ToolWindow closed: {id}", T3LogLevel.Debug);
      ToolWindowClosed?.Invoke(descriptor);
    }

    public void Toggle(string id)
    {
      if (IsOpen(id)) Close(id);
      else Open(id);
    }

    /// <summary>
    /// Syncfusion's <c>SaveDockState()</c>/<c>LoadDockState()</c> identifies
    /// children by element <c>Name</c>, which must be a valid XAML name token.
    /// We replace anything outside <c>[A-Za-z0-9_]</c> with underscores and
    /// guarantee a non-digit first character.
    /// </summary>
    private static string SanitizeName(string id)
    {
      if (string.IsNullOrEmpty(id)) return "_";
      var chars = new char[id.Length];
      for (int i = 0; i < id.Length; i++)
      {
        char c = id[i];
        chars[i] = (char.IsLetterOrDigit(c) || c == '_') ? c : '_';
      }
      string name = new string(chars);
      if (char.IsDigit(name[0])) name = "_" + name;
      return name;
    }
  }
}
