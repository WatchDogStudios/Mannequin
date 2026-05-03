/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Collections.ObjectModel;
using System.Windows.Input;

namespace T3Foundation.Wpf.Shell
{
  /// <summary>
  /// A node in the shell's data-driven menu. Composes into a tree:
  /// top-level items become menu headers, items with <see cref="Command"/>
  /// become invocable leaves, items with <see cref="IsSeparator"/> render
  /// as separators.
  /// </summary>
  public sealed class T3MenuItem
  {
    public string Header { get; set; } = string.Empty;

    /// <summary>Optional FontAwesome5 icon key (e.g. "Solid_FolderOpen").</summary>
    public string? IconKey { get; set; }

    public ICommand? Command { get; set; }
    public object? CommandParameter { get; set; }
    public KeyGesture? Shortcut { get; set; }

    public bool IsSeparator { get; set; }

    /// <summary>Child items - populated for sub-menus.</summary>
    public ObservableCollection<T3MenuItem> Children { get; } = new ObservableCollection<T3MenuItem>();

    public static T3MenuItem Separator() => new T3MenuItem { IsSeparator = true };
  }
}
