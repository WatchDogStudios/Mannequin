/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using Syncfusion.Windows.Tools.Controls;

namespace T3Foundation.Services.Shell
{
  /// <summary>
  /// Registry of tool windows known to the shell. Apps register tool windows
  /// either via <see cref="Register"/> in their shell-window override or via
  /// <c>[T3ToolWindow]</c> attributes auto-discovered by the plugin manager.
  /// <para>
  /// The shell calls <see cref="AttachDockHost"/> once during startup to bind
  /// a Syncfusion <c>DockingManager</c>; subsequent <see cref="Open"/>/<see cref="Close"/>
  /// calls operate on that host.
  /// </para>
  /// </summary>
  public interface IT3ToolWindowRegistry
  {
    /// <summary>All registered descriptors in insertion order.</summary>
    IReadOnlyList<T3ToolWindowDescriptor> All { get; }

    /// <summary>Add a descriptor. Replaces any prior registration with the same id.</summary>
    void Register(T3ToolWindowDescriptor descriptor);

    /// <summary>Remove a descriptor and close its open instance, if any.</summary>
    void Unregister(string id);

    /// <summary>Open the tool window. No-op if already open and singleton.</summary>
    void Open(string id);

    /// <summary>Close the open instance of a tool window. No-op if not open.</summary>
    void Close(string id);

    /// <summary>Open if closed, close if open.</summary>
    void Toggle(string id);

    /// <summary>True when a tool window currently has an open instance in the dock host.</summary>
    bool IsOpen(string id);

    /// <summary>Resolve a descriptor by id; returns null when not registered.</summary>
    T3ToolWindowDescriptor? Find(string id);

    /// <summary>
    /// Bind the dock host the shell will manage. Called once by <c>T3ShellWindow</c>
    /// after its template is applied. Idempotent for the same host.
    /// </summary>
    void AttachDockHost(DockingManager dockHost);

    /// <summary>Raised after a tool window is opened or activated.</summary>
    event Action<T3ToolWindowDescriptor>? ToolWindowOpened;

    /// <summary>Raised after a tool window is closed.</summary>
    event Action<T3ToolWindowDescriptor>? ToolWindowClosed;
  }
}
