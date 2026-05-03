/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// Generic hierarchy view. Apps populate <see cref="Roots"/> from their
  /// active context; the panel handles tree rendering, expansion state, and
  /// search filtering.
  /// <para>
  /// Not auto-registered: apps that want this panel must register it
  /// explicitly via <c>IT3ToolWindowRegistry.Register</c>.
  /// </para>
  /// </summary>
  public partial class T3OutlinerPanel : UserControl
  {
    public ObservableCollection<T3OutlinerNode> Roots { get; } = new ObservableCollection<T3OutlinerNode>();

    /// <summary>Raised when the user picks a node. Apps publish the payload to a context here.</summary>
    public event Action<T3OutlinerNode?>? SelectionChanged;

    public T3OutlinerPanel()
    {
      InitializeComponent();
    }

    private void OnSelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
      SelectionChanged?.Invoke(e.NewValue as T3OutlinerNode);
    }

    private void OnSearchChanged(object sender, TextChangedEventArgs e)
    {
      // v1: shallow filter on the top-level only. Apps that want recursive
      // filtering should subclass and override this behavior.
      var query = SearchBox.Text ?? string.Empty;
      foreach (var item in Tree.Items)
      {
        if (Tree.ItemContainerGenerator.ContainerFromItem(item) is TreeViewItem tvi && item is T3OutlinerNode node)
        {
          tvi.Visibility = string.IsNullOrEmpty(query) ||
                           node.Name.IndexOf(query, StringComparison.OrdinalIgnoreCase) >= 0
            ? Visibility.Visible : Visibility.Collapsed;
        }
      }
    }
  }
}
