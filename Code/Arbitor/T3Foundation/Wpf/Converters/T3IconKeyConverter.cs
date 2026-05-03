/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using T3Foundation.Wpf.MarkupExtensions;

namespace T3Foundation.Wpf.Converters
{
  /// <summary>
  /// Converts a FontAwesome5 icon key string (e.g. "Solid_FolderOpen") into a
  /// renderable visual. Used in data-bound contexts where the icon name varies
  /// per item (toolbar buttons, tool-window registry, etc.).
  /// <para>
  /// <c>parameter</c> may be a <see cref="double"/> size (defaults to 14).
  /// Returns <see cref="DependencyProperty.UnsetValue"/> when the key is empty.
  /// </para>
  /// </summary>
  public class T3IconKeyConverter : IValueConverter
  {
    public double Size { get; set; } = 14;

    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
      var name = value as string;
      if (string.IsNullOrWhiteSpace(name)) return DependencyProperty.UnsetValue;

      double size = Size;
      if (parameter is double d) size = d;
      else if (parameter is string s && double.TryParse(s, NumberStyles.Any, CultureInfo.InvariantCulture, out var ps)) size = ps;

      var ext = new T3IconExtension { Name = name, Size = size };
      return ext.ProvideValue(null!);
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      => throw new NotSupportedException();
  }
}
