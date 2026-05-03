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
  /// Data-driven toolbar service. The shell renders <see cref="Items"/> into
  /// <c>PART_ToolbarHost</c>; apps register entries via <see cref="AddButton"/>,
  /// <see cref="AddSeparator"/>, <see cref="AddLabel"/>, etc.
  /// </summary>
  public interface IT3ToolbarService
  {
    ObservableCollection<T3ToolbarItem> Items { get; }

    void AddButton(string label, ICommand command, string? iconKey = null, string? tooltip = null);
    void AddSeparator();
    void AddLabel(string text);
    void AddSpacer();
    void AddCustom(object content);
    void Clear();
  }
}
