/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.ObjectModel;
using System.Windows.Input;
using T3Foundation.Wpf.Shell;

namespace T3Foundation.Services.Shell
{
  public class T3MenuService : IT3MenuService
  {
    public ObservableCollection<T3MenuItem> Items { get; } = new ObservableCollection<T3MenuItem>();

    public IT3MenuBuilder AddMenu(string header)
    {
      var menu = new T3MenuItem { Header = header };
      Items.Add(menu);
      return new Builder(menu);
    }

    public void RemoveMenu(string header)
    {
      for (int i = Items.Count - 1; i >= 0; i--)
      {
        if (Items[i].Header == header) Items.RemoveAt(i);
      }
    }

    public void Clear() => Items.Clear();

    /// <summary>
    /// Builder operates on a single menu node, appending children (items,
    /// separators, sub-menus) as a chained DSL.
    /// </summary>
    private sealed class Builder : IT3MenuBuilder
    {
      private readonly T3MenuItem _node;
      public Builder(T3MenuItem node) { _node = node; }

      public IT3MenuBuilder AddItem(string header, ICommand command, KeyGesture? shortcut = null, string? iconKey = null)
      {
        _node.Children.Add(new T3MenuItem
        {
          Header = header,
          Command = command,
          Shortcut = shortcut,
          IconKey = iconKey
        });
        return this;
      }

      public IT3MenuBuilder AddSeparator()
      {
        _node.Children.Add(T3MenuItem.Separator());
        return this;
      }

      public IT3MenuBuilder AddSubMenu(string header, Action<IT3MenuBuilder> configure)
      {
        if (configure == null) throw new ArgumentNullException(nameof(configure));
        var sub = new T3MenuItem { Header = header };
        _node.Children.Add(sub);
        configure(new Builder(sub));
        return this;
      }
    }
  }
}
