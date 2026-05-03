/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using T3Foundation.Panel;

namespace T3Foundation.Documents
{
  /// <summary>
  /// Docking position hint for a panel within a document layout.
  /// The host window interprets these to create the actual docking arrangement.
  /// </summary>
  public enum T3DockPosition
  {
    /// <summary>Docked to the left side.</summary>
    Left,
    /// <summary>Docked to the right side.</summary>
    Right,
    /// <summary>Center/document area — the main content.</summary>
    Center,
    /// <summary>Docked to the bottom.</summary>
    Bottom,
    /// <summary>Tabbed alongside another panel (use <see cref="T3PanelSlot.TabbedWith"/> to specify which).</summary>
    Tabbed
  }

  /// <summary>
  /// Describes a panel's placement within a Document layout.
  /// This is a configuration object — the actual panel instance is resolved at runtime.
  /// </summary>
  public class T3PanelSlot
  {
    /// <summary>
    /// The type of panel to instantiate (must implement <see cref="IT3Panel"/>).
    /// </summary>
    public Type PanelType { get; set; } = typeof(IT3Panel);

    /// <summary>
    /// Where this panel should be docked in the layout.
    /// </summary>
    public T3DockPosition Position { get; set; } = T3DockPosition.Center;

    /// <summary>
    /// Tab/header title for this panel.
    /// </summary>
    public string Header { get; set; } = "";

    /// <summary>
    /// Desired width when docked left or right.
    /// </summary>
    public double DesiredWidth { get; set; } = 300;

    /// <summary>
    /// Desired height when docked at the bottom.
    /// </summary>
    public double DesiredHeight { get; set; } = 200;

    /// <summary>
    /// For <see cref="T3DockPosition.Tabbed"/>: the header of the panel to tab alongside.
    /// </summary>
    public string? TabbedWith { get; set; }

    /// <summary>
    /// The resolved panel instance (set at runtime by the Document).
    /// </summary>
    public IT3Panel? Instance { get; set; }
  }
}
