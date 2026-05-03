/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Windows;
using System.Windows.Input;

namespace T3Foundation.Services.Shell
{
  /// <summary>
  /// Declares everything the shell needs to host a tool window:
  /// stable identity, display metadata, where to place it by default, and
  /// a factory that produces the content view on demand.
  /// </summary>
  public sealed class T3ToolWindowDescriptor
  {
    /// <summary>
    /// Stable id used for layout persistence and lookup. Becomes the
    /// <c>x:Name</c> of the dock host's child element so Syncfusion's
    /// <c>SaveDockState()</c>/<c>LoadDockState()</c> can round-trip it.
    /// </summary>
    public string Id { get; }

    /// <summary>Display name shown on the dock tab and in menus.</summary>
    public string Title { get; set; }

    /// <summary>FontAwesome5 icon key (e.g. "Solid_Stream" for the log).</summary>
    public string? IconKey { get; set; }

    /// <summary>Factory that produces the content view. Invoked on first open.</summary>
    public Func<FrameworkElement> Factory { get; }

    /// <summary>Default placement when first opened (no persisted state).</summary>
    public T3DockSide DefaultSide { get; set; } = T3DockSide.Right;

    /// <summary>For <see cref="T3DockSide.Tabbed"/>: the target tool-window id to tab with.</summary>
    public string? TabbedWith { get; set; }

    /// <summary>Default width when docked left/right.</summary>
    public double DefaultWidth { get; set; } = 300;

    /// <summary>Default height when docked top/bottom.</summary>
    public double DefaultHeight { get; set; } = 200;

    /// <summary>If true, only one instance can be open at a time. Re-open activates the existing one.</summary>
    public bool IsSingleton { get; set; } = true;

    /// <summary>Optional menu path (e.g. "View/Outliner") for auto-registered View menu entries.</summary>
    public string? MenuPath { get; set; }

    /// <summary>Optional global keyboard shortcut to toggle the tool window.</summary>
    public KeyGesture? Shortcut { get; set; }

    public T3ToolWindowDescriptor(string id, string title, Func<FrameworkElement> factory)
    {
      if (string.IsNullOrWhiteSpace(id)) throw new ArgumentException("Id must be non-empty.", nameof(id));
      if (string.IsNullOrWhiteSpace(title)) throw new ArgumentException("Title must be non-empty.", nameof(title));

      Id = id;
      Title = title;
      Factory = factory ?? throw new ArgumentNullException(nameof(factory));
    }
  }
}
