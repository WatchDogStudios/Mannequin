/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Collections.ObjectModel;
using System.Windows.Input;
using T3Foundation.Wpf.Shell;

namespace T3Foundation.Services.Shell
{
  /// <summary>
  /// Builder returned by <see cref="IT3MenuService.AddMenu"/>. Chained calls
  /// add children to the most recently created menu.
  /// </summary>
  public interface IT3MenuBuilder
  {
    IT3MenuBuilder AddItem(string header, ICommand command, KeyGesture? shortcut = null, string? iconKey = null);
    IT3MenuBuilder AddSeparator();
    IT3MenuBuilder AddSubMenu(string header, System.Action<IT3MenuBuilder> configure);
  }

  /// <summary>
  /// Data-driven menu service. The shell renders <see cref="Items"/> into
  /// <c>PART_MenuHost</c>; apps register entries via <see cref="AddMenu"/>.
  /// </summary>
  public interface IT3MenuService
  {
    /// <summary>The flat top-level menu items. Bound by the shell template.</summary>
    ObservableCollection<T3MenuItem> Items { get; }

    /// <summary>Begin a top-level menu (e.g. "File", "Edit", "View"). Returns a builder.</summary>
    IT3MenuBuilder AddMenu(string header);

    /// <summary>Remove a top-level menu by header. No-op if not found.</summary>
    void RemoveMenu(string header);

    /// <summary>Clear all menus.</summary>
    void Clear();
  }
}
