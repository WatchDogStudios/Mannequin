/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

namespace T3Foundation.Wpf.Shell
{
  /// <summary>
  /// A named docking layout snapshot. The shell switches between workspaces
  /// via the title-bar dropdown; layouts are persisted by <c>IT3LayoutService</c>.
  /// <para>
  /// <see cref="LayoutXml"/> is the blob produced by Syncfusion's
  /// <c>DockingManager.SaveDockState()</c> and consumed by <c>LoadDockState()</c>.
  /// </para>
  /// </summary>
  public sealed class T3WorkspaceDescriptor
  {
    public string Name { get; }
    public string IconKey { get; }
    public string? LayoutXml { get; set; }

    public T3WorkspaceDescriptor(string name, string iconKey = "Solid_ThLarge", string? layoutXml = null)
    {
      Name = name;
      IconKey = iconKey;
      LayoutXml = layoutXml;
    }

    public override string ToString() => Name;
  }
}
