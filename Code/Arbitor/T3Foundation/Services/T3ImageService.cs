/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using System;
using System.IO;
using System.Windows.Media.Imaging;

namespace T3Foundation.Services
{
  /// <summary>
  /// Service for loading, comparing, and managing test images.
  /// Provides BitmapImage objects suitable for WPF display.
  /// </summary>
  public class T3ImageService
  {
    /// <summary>
    /// Load an image from disk as a WPF BitmapImage.
    /// </summary>
    public static BitmapImage LoadImage(string path)
    {
      if (string.IsNullOrEmpty(path) || !File.Exists(path))
        return null;

      try
      {
        var bitmap = new BitmapImage();
        bitmap.BeginInit();
        bitmap.UriSource = new Uri(path, UriKind.Absolute);
        bitmap.CacheOption = BitmapCacheOption.OnLoad;
        bitmap.EndInit();
        bitmap.Freeze(); // Thread-safe
        return bitmap;
      }
      catch (Exception ex)
      {
        T3Core.Log($"Failed to load image '{path}': {ex.Message}", T3LogLevel.Error);
        return null;
      }
    }

    /// <summary>
    /// Load test, reference, and diff images for side-by-side comparison.
    /// </summary>
    public static (BitmapImage Test, BitmapImage Reference, BitmapImage Diff) LoadComparisonSet(
      string testPath, string refPath, string diffPath)
    {
      return (
        testPath != null ? LoadImage(testPath) : null,
        refPath != null ? LoadImage(refPath) : null,
        diffPath != null ? LoadImage(diffPath) : null
      );
    }
  }
}
