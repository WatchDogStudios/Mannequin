/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Windows.Media;

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// Single log row displayed by <see cref="T3OutputLogPanel"/>.
  /// </summary>
  public sealed class T3LogEntry
  {
    public string Text { get; }
    public T3LogLevel Level { get; }
    public Brush LevelBrush { get; }

    public T3LogEntry(string text, T3LogLevel level)
    {
      Text = text;
      Level = level;
      LevelBrush = LevelToBrush(level);
    }

    private static Brush LevelToBrush(T3LogLevel level)
    {
      switch (level)
      {
        case T3LogLevel.Debug: return new SolidColorBrush(Color.FromRgb(0xA0, 0xA0, 0xA0));   // muted
        case T3LogLevel.Info: return new SolidColorBrush(Color.FromRgb(0xE0, 0xE0, 0xE0));    // text
        case T3LogLevel.Warning: return new SolidColorBrush(Color.FromRgb(0xFF, 0xB3, 0x47)); // warning
        case T3LogLevel.Error: return new SolidColorBrush(Color.FromRgb(0xFF, 0x6B, 0x35));   // accent2
        case T3LogLevel.Critical: return new SolidColorBrush(Color.FromRgb(0xFF, 0x4D, 0x4D));// error
        default: return Brushes.White;
      }
    }
  }
}
