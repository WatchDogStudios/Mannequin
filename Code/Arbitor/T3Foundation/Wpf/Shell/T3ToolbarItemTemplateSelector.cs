/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Windows;
using System.Windows.Controls;

namespace T3Foundation.Wpf.Shell
{
  /// <summary>
  /// Picks one of the named DataTemplates exposed by the shell template
  /// based on a <see cref="T3ToolbarItem.Kind"/>.
  /// </summary>
  public class T3ToolbarItemTemplateSelector : DataTemplateSelector
  {
    public DataTemplate? ButtonTemplate { get; set; }
    public DataTemplate? SeparatorTemplate { get; set; }
    public DataTemplate? LabelTemplate { get; set; }
    public DataTemplate? SpacerTemplate { get; set; }
    public DataTemplate? CustomTemplate { get; set; }

    public override DataTemplate? SelectTemplate(object item, DependencyObject container)
    {
      if (item is T3ToolbarItem t)
      {
        switch (t.Kind)
        {
          case T3ToolbarItemKind.Button: return ButtonTemplate;
          case T3ToolbarItemKind.Separator: return SeparatorTemplate;
          case T3ToolbarItemKind.Label: return LabelTemplate;
          case T3ToolbarItemKind.Spacer: return SpacerTemplate;
          case T3ToolbarItemKind.Custom: return CustomTemplate;
        }
      }
      return base.SelectTemplate(item, container);
    }
  }
}
