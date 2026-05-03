/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using T3Foundation.Services.Shell;

namespace T3Foundation.Plugin
{
  /// <summary>
  /// Marks a <see cref="System.Windows.FrameworkElement"/>-derived type as a tool window
  /// auto-discoverable by <c>T3PluginManager</c>. The framework instantiates the type
  /// via its parameterless constructor; for DI-resolved views, register manually via
  /// <c>IT3ToolWindowRegistry.Register</c> instead.
  /// </summary>
  [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
  public sealed class T3ToolWindowAttribute : Attribute
  {
    public string Id { get; }
    public string Title { get; }
    public T3DockSide DefaultSide { get; set; } = T3DockSide.Right;
    public string? IconKey { get; set; }
    public string? TabbedWith { get; set; }
    public double DefaultWidth { get; set; } = 300;
    public double DefaultHeight { get; set; } = 200;
    public bool IsSingleton { get; set; } = true;
    public string? MenuPath { get; set; }

    public T3ToolWindowAttribute(string id, string title)
    {
      Id = id;
      Title = title;
    }
  }
}
