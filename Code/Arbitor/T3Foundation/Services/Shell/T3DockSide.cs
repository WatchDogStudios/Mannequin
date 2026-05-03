/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

namespace T3Foundation.Services.Shell
{
  /// <summary>
  /// Default placement of a tool window inside the shell's DockingManager.
  /// Maps to Syncfusion's <c>DockSide</c>/<c>DockState</c> attached properties.
  /// </summary>
  public enum T3DockSide
  {
    /// <summary>Anchored to the left edge of the dock host.</summary>
    Left,

    /// <summary>Anchored to the right edge.</summary>
    Right,

    /// <summary>Anchored to the top edge.</summary>
    Top,

    /// <summary>Anchored to the bottom edge.</summary>
    Bottom,

    /// <summary>Hosted inside the central document container (UE-style asset editors).</summary>
    Document,

    /// <summary>Tabbed alongside another tool window. Requires <c>TabbedWith</c>.</summary>
    Tabbed
  }
}
