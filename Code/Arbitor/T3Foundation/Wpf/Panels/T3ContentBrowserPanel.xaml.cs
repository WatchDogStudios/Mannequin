/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Controls;
using System.Windows.Media;

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// Single asset entry rendered as a tile.
  /// </summary>
  public class T3ContentItem
  {
    public string Name { get; set; } = string.Empty;
    public string? Path { get; set; }
    public ImageSource? Thumbnail { get; set; }
    public object? Tag { get; set; }
  }

  /// <summary>
  /// Asset/content browser. Apps populate <see cref="Items"/>; the panel
  /// handles thumbnail rendering, search filtering, and selection events.
  /// <para>
  /// Not auto-registered: apps that want this panel must register it
  /// explicitly via <c>IT3ToolWindowRegistry.Register</c>.
  /// </para>
  /// </summary>
  public partial class T3ContentBrowserPanel : UserControl
  {
    public ObservableCollection<T3ContentItem> Items { get; } = new ObservableCollection<T3ContentItem>();
    public ObservableCollection<T3ContentItem> FilteredItems { get; } = new ObservableCollection<T3ContentItem>();

    public string CurrentPath
    {
      get => Breadcrumb.Text;
      set => Breadcrumb.Text = value;
    }

    public event Action<T3ContentItem?>? SelectionChanged;

    public T3ContentBrowserPanel()
    {
      InitializeComponent();
      Items.CollectionChanged += (_, __) => RebuildFiltered();
    }

    private void RebuildFiltered()
    {
      var query = SearchBox.Text ?? string.Empty;
      FilteredItems.Clear();
      foreach (var item in Items)
      {
        if (string.IsNullOrEmpty(query) ||
            item.Name.IndexOf(query, StringComparison.OrdinalIgnoreCase) >= 0)
        {
          FilteredItems.Add(item);
        }
      }
    }

    private void OnSearchChanged(object sender, TextChangedEventArgs e) => RebuildFiltered();

    private void OnSelectionChanged(object sender, SelectionChangedEventArgs e)
    {
      SelectionChanged?.Invoke(ItemList.SelectedItem as T3ContentItem);
    }
  }
}
