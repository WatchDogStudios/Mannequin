/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Shapes;
using FontAwesome5;
using FontAwesome5.Extensions;
using T3Foundation.Services.DI;

namespace T3Foundation.Wpf.MarkupExtensions
{
  /// <summary>
  /// Resolves a FontAwesome5 icon by name in XAML as a TextBlock with the icon glyph.
  /// Usage: &lt;ContentControl Content="{t3:T3Icon Kind=Solid_Play, Size=16}"/&gt;
  /// </summary>
  public class T3IconExtension : MarkupExtension
  {
    public EFontAwesomeIcon Kind { get; set; } = EFontAwesomeIcon.None;
    public string? Name { get; set; }
    public double Size { get; set; } = 16;
    public Brush? Foreground { get; set; }

    public T3IconExtension() { }
    public T3IconExtension(EFontAwesomeIcon kind) => Kind = kind;
    public T3IconExtension(string name) => Name = name;

    public override object ProvideValue(IServiceProvider serviceProvider)
    {
      var icon = Kind;
      if (icon == EFontAwesomeIcon.None && !string.IsNullOrWhiteSpace(Name))
        Enum.TryParse(Name, ignoreCase: true, out icon);

      if (icon == EFontAwesomeIcon.None ||
          !icon.GetSvg(out string pathData, out int width, out int height))
      {
        return new TextBlock
        {
          Text = "□",
          Width = Size,
          Height = Size,
          Foreground = Foreground ?? new SolidColorBrush(Color.FromRgb(205, 216, 232)),
          TextAlignment = TextAlignment.Center
        };
      }

      var path = new Path
      {
        Data = Geometry.Parse(pathData),
        Fill = Foreground ?? new SolidColorBrush(Color.FromRgb(205, 216, 232)),
        Stretch = Stretch.Uniform
      };

      var canvas = new Canvas
      {
        Width = width,
        Height = height
      };
      canvas.Children.Add(path);

      return new Viewbox
      {
        Width = Size,
        Height = Size,
        Stretch = Stretch.Uniform,
        Child = canvas
      };
    }
  }

  /// <summary>
  /// Resolves a service from the T3 DI container in XAML.
  /// Usage: &lt;ContentControl Content="{t3:T3Service Type=local:IMyService}"/&gt;
  /// </summary>
  public class T3ServiceExtension : MarkupExtension
  {
    public Type? Type { get; set; }

    public T3ServiceExtension() { }
    public T3ServiceExtension(Type type) => Type = type;

    public override object? ProvideValue(IServiceProvider serviceProvider)
    {
      if (Type == null)
        throw new InvalidOperationException("T3ServiceExtension requires a Type.");

      try
      {
        return T3ServiceCollection.Provider.GetService(Type);
      }
      catch
      {
        // At design time the container may not be built
        return null;
      }
    }
  }
}
