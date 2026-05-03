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
  public class T3ToolbarService : IT3ToolbarService
  {
    public ObservableCollection<T3ToolbarItem> Items { get; } = new ObservableCollection<T3ToolbarItem>();

    public void AddButton(string label, ICommand command, string? iconKey = null, string? tooltip = null)
    {
      Items.Add(new T3ToolbarItem
      {
        Kind = T3ToolbarItemKind.Button,
        Label = label,
        Command = command,
        IconKey = iconKey,
        Tooltip = tooltip ?? label
      });
    }

    public void AddSeparator() => Items.Add(T3ToolbarItem.Separator());
    public void AddLabel(string text) => Items.Add(T3ToolbarItem.LabelText(text));
    public void AddSpacer() => Items.Add(T3ToolbarItem.Spacer());

    public void AddCustom(object content)
    {
      Items.Add(new T3ToolbarItem
      {
        Kind = T3ToolbarItemKind.Custom,
        Content = content
      });
    }

    public void Clear() => Items.Clear();
  }
}
