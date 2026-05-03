/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Media;

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// UE-style details panel: reflects over the public properties of
  /// <see cref="SelectedObject"/> and renders an inline editor per property,
  /// grouped by <see cref="CategoryAttribute"/>.
  /// <para>
  /// v1 supports string, numeric, bool, and enum editors. Custom property
  /// types fall back to a read-only string label - apps that need richer
  /// editors should subclass and override <see cref="BuildEditor"/>.
  /// </para>
  /// <para>
  /// Not auto-registered: apps that want this panel must register it
  /// explicitly via <c>IT3ToolWindowRegistry.Register</c>.
  /// </para>
  /// </summary>
  public partial class T3DetailsPanel : UserControl
  {
    public static readonly DependencyProperty SelectedObjectProperty =
      DependencyProperty.Register(nameof(SelectedObject), typeof(object), typeof(T3DetailsPanel),
        new PropertyMetadata(null, OnSelectedObjectChanged));

    public object? SelectedObject
    {
      get => GetValue(SelectedObjectProperty);
      set => SetValue(SelectedObjectProperty, value);
    }

    public T3DetailsPanel()
    {
      InitializeComponent();
    }

    private static void OnSelectedObjectChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
      ((T3DetailsPanel)d).Rebuild();
    }

    private void Rebuild()
    {
      ItemHost.Children.Clear();
      if (SelectedObject == null) return;

      var groups = SelectedObject.GetType()
        .GetProperties(BindingFlags.Instance | BindingFlags.Public)
        .Where(p => p.GetCustomAttribute<BrowsableAttribute>()?.Browsable != false)
        .GroupBy(p => p.GetCustomAttribute<CategoryAttribute>()?.Category ?? "Properties");

      foreach (var group in groups)
      {
        ItemHost.Children.Add(BuildHeader(group.Key));
        foreach (var prop in group)
          ItemHost.Children.Add(BuildRow(prop));
      }
    }

    private TextBlock BuildHeader(string category)
    {
      return new TextBlock
      {
        Text = category,
        FontWeight = FontWeights.SemiBold,
        Foreground = (Brush)FindResource("T3Brush.TextMuted"),
        Margin = new Thickness(0, 8, 0, 4)
      };
    }

    private FrameworkElement BuildRow(PropertyInfo prop)
    {
      var grid = new Grid { Margin = new Thickness(0, 0, 0, 2) };
      grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(0.4, GridUnitType.Star) });
      grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(0.6, GridUnitType.Star) });

      var label = new TextBlock
      {
        Text = prop.Name,
        VerticalAlignment = VerticalAlignment.Center,
        TextTrimming = TextTrimming.CharacterEllipsis
      };
      Grid.SetColumn(label, 0);
      grid.Children.Add(label);

      var editor = BuildEditor(prop) ?? new TextBlock
      {
        Text = prop.GetValue(SelectedObject)?.ToString() ?? string.Empty,
        Foreground = (Brush)FindResource("T3Brush.TextMuted")
      };
      Grid.SetColumn(editor, 1);
      grid.Children.Add(editor);

      return grid;
    }

    /// <summary>
    /// Build the right-hand editor for a property. Override to add custom
    /// editors (color picker, file path browser, etc.). Return null to fall
    /// back to a read-only label.
    /// </summary>
    protected virtual FrameworkElement? BuildEditor(PropertyInfo prop)
    {
      var t = Nullable.GetUnderlyingType(prop.PropertyType) ?? prop.PropertyType;
      bool readOnly = !prop.CanWrite;

      if (t == typeof(bool))
      {
        var cb = new CheckBox { VerticalAlignment = VerticalAlignment.Center };
        cb.SetBinding(CheckBox.IsCheckedProperty, MakeBinding(prop, readOnly));
        return cb;
      }

      if (t.IsEnum)
      {
        var combo = new ComboBox { ItemsSource = Enum.GetValues(t), IsEnabled = !readOnly };
        combo.SetBinding(Selector.SelectedItemProperty, MakeBinding(prop, readOnly));
        return combo;
      }

      if (t == typeof(string) || t.IsPrimitive)
      {
        var tb = new TextBox { IsReadOnly = readOnly };
        tb.SetBinding(TextBox.TextProperty, MakeBinding(prop, readOnly));
        return tb;
      }

      return null;
    }

    private Binding MakeBinding(PropertyInfo prop, bool readOnly) => new Binding(prop.Name)
    {
      Source = SelectedObject,
      Mode = readOnly ? BindingMode.OneWay : BindingMode.TwoWay,
      UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged
    };
  }
}
