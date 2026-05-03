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
using System.Windows.Markup;
using System.Windows.Media;

namespace T3Foundation.Wpf.Converters
{
  /// <summary>
  /// Converts bool to Visibility. Set <see cref="Invert"/> to true for inverse logic.
  /// Usable as both StaticResource and inline MarkupExtension.
  /// </summary>
  public class BoolToVisibilityConverter : MarkupExtension, IValueConverter
  {
    public bool Invert { get; set; }

    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
      bool b = value is bool bVal && bVal;
      if (Invert) b = !b;
      return b ? Visibility.Visible : Visibility.Collapsed;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
      bool visible = value is Visibility v && v == Visibility.Visible;
      return Invert ? !visible : visible;
    }

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Inverts a boolean value.
  /// </summary>
  public class InverseBoolConverter : MarkupExtension, IValueConverter
  {
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
      => value is bool b ? !b : value;

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      => value is bool b ? !b : value;

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Converts null/non-null to Visibility. Set <see cref="Invert"/> to show when null.
  /// </summary>
  public class NullToVisibilityConverter : MarkupExtension, IValueConverter
  {
    public bool Invert { get; set; }

    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
      bool isNull = value == null;
      if (Invert) isNull = !isNull;
      return isNull ? Visibility.Collapsed : Visibility.Visible;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      => throw new NotSupportedException();

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Converts null to false, non-null to true.
  /// </summary>
  public class NullToBoolConverter : MarkupExtension, IValueConverter
  {
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
      => value != null;

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      => throw new NotSupportedException();

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Converts empty/null string to Collapsed, non-empty to Visible.
  /// </summary>
  public class StringToVisibilityConverter : MarkupExtension, IValueConverter
  {
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
      => string.IsNullOrEmpty(value as string) ? Visibility.Collapsed : Visibility.Visible;

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      => throw new NotSupportedException();

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Converts a <see cref="Color"/> to a <see cref="SolidColorBrush"/>.
  /// </summary>
  public class ColorToBrushConverter : MarkupExtension, IValueConverter
  {
    public object? Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
      if (value is Color color)
        return new SolidColorBrush(color);
      if (value is string s)
        return new SolidColorBrush((Color)ColorConverter.ConvertFromString(s));
      return null;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
      if (value is SolidColorBrush brush)
        return brush.Color;
      return default(Color);
    }

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Converts an enum value to bool for RadioButton binding.
  /// ConverterParameter is the enum value to compare against.
  /// <code>&lt;RadioButton IsChecked="{Binding MyEnum, Converter={t3:EnumToBoolConverter}, ConverterParameter={x:Static local:MyEnum.Value}}"/&gt;</code>
  /// </summary>
  public class EnumToBoolConverter : MarkupExtension, IValueConverter
  {
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
      if (value == null || parameter == null) return false;
      return value.Equals(parameter);
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
      if (value is bool b && b && parameter != null)
        return parameter;
      return Binding.DoNothing;
    }

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Simple math converter. ConverterParameter syntax: "+10", "-5", "*2", "/3".
  /// </summary>
  public class MathConverter : MarkupExtension, IValueConverter
  {
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
      if (value == null || parameter == null) return value!;

      double input = System.Convert.ToDouble(value);
      string param = parameter.ToString()!.Trim();
      if (param.Length < 2) return input;

      char op = param[0];
      if (!double.TryParse(param.Substring(1), NumberStyles.Any, CultureInfo.InvariantCulture, out double operand))
        return input;

      return op switch
      {
        '+' => input + operand,
        '-' => input - operand,
        '*' => input * operand,
        '/' => operand != 0 ? input / operand : input,
        _ => input
      };
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      => throw new NotSupportedException();

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }

  /// <summary>
  /// Applies string.Format using ConverterParameter as the format string.
  /// <code>&lt;TextBlock Text="{Binding Value, Converter={t3:StringFormatConverter}, ConverterParameter='{}{0:F2} ms'}"/&gt;</code>
  /// </summary>
  public class StringFormatConverter : MarkupExtension, IValueConverter
  {
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
      if (parameter is string format)
        return string.Format(culture, format, value);
      return value?.ToString() ?? string.Empty;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      => throw new NotSupportedException();

    public override object ProvideValue(IServiceProvider serviceProvider) => this;
  }
}
