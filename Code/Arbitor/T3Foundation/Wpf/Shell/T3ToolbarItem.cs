/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Windows.Input;

namespace T3Foundation.Wpf.Shell
{
  public enum T3ToolbarItemKind
  {
    Button,
    Separator,
    Label,
    Spacer,
    Custom
  }

  /// <summary>
  /// A single entry in the shell's data-driven toolbar.
  /// Apps register these via <c>IT3ToolbarService</c> and the shell renders
  /// them inside the <c>syncfusion:ToolBarAdv</c> hosted at <c>PART_ToolbarHost</c>.
  /// </summary>
  public sealed class T3ToolbarItem
  {
    public T3ToolbarItemKind Kind { get; set; } = T3ToolbarItemKind.Button;

    public string? Label { get; set; }
    public string? IconKey { get; set; }
    public string? Tooltip { get; set; }

    public ICommand? Command { get; set; }
    public object? CommandParameter { get; set; }

    /// <summary>For <see cref="T3ToolbarItemKind.Custom"/>: a content factory.</summary>
    public object? Content { get; set; }

    public static T3ToolbarItem Separator() => new T3ToolbarItem { Kind = T3ToolbarItemKind.Separator };
    public static T3ToolbarItem Spacer() => new T3ToolbarItem { Kind = T3ToolbarItemKind.Spacer };
    public static T3ToolbarItem LabelText(string text) => new T3ToolbarItem { Kind = T3ToolbarItemKind.Label, Label = text };
  }
}
