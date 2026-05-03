/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Data;
using System.Windows.Markup;

namespace T3Foundation.Wpf.Converters
{
  /// <summary>
  /// Returns true only if ALL bound boolean values are true.
  /// </summary>
  public class BooleanAndConverter : MarkupExtension, IMultiValueConverter
  {
    public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
    {
      return values.All(v => v is bool b && b);
    }

    public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
      => throw new NotSupportedException();

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Returns true if ANY bound boolean value is true.
  /// </summary>
  public class BooleanOrConverter : MarkupExtension, IMultiValueConverter
  {
    public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
    {
      return values.Any(v => v is bool b && b);
    }

    public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
      => throw new NotSupportedException();

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Returns true if all bound values are equal to each other.
  /// </summary>
  public class EqualityConverter : MarkupExtension, IMultiValueConverter
  {
    public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
    {
      if (values.Length < 2) return true;
      var first = values[0];
      return values.Skip(1).All(v =>
        (first == null && v == null) ||
        (first != null && first.Equals(v)));
    }

    public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
      => throw new NotSupportedException();

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }
}
