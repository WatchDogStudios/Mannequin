/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Collections.ObjectModel;

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// Tree node displayed by <see cref="T3OutlinerPanel"/>. Apps subclass to add
  /// domain payload and override <see cref="ToString"/> if a richer label is needed.
  /// </summary>
  public class T3OutlinerNode
  {
    public string Name { get; set; } = string.Empty;
    public string? IconKey { get; set; }
    public bool IsExpanded { get; set; } = true;
    public ObservableCollection<T3OutlinerNode> Children { get; } = new ObservableCollection<T3OutlinerNode>();

    /// <summary>Free-form payload: the domain object this node represents.</summary>
    public object? Tag { get; set; }

    public override string ToString() => Name;
  }
}
