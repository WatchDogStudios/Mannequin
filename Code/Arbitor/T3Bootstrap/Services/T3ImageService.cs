/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

#nullable enable

using System;
using System.IO;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using T3Foundation;

namespace T3.Services
{
  /// <summary>
  /// Service for loading, comparing, and managing test images.
  /// Provides BitmapImage objects suitable for WPF display.
  /// </summary>
  public static class T3ImageService
  {
    /// <summary>
    /// Load an image from disk as a WPF BitmapImage.
    /// </summary>
    public static BitmapImage? LoadImage(string path)
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
    public static (BitmapImage? Test, BitmapImage? Reference, BitmapImage? Diff) LoadComparisonSet(
      string? testPath, string? refPath, string? diffPath)
    {
      return (
        testPath != null ? LoadImage(testPath) : null,
        refPath != null ? LoadImage(refPath) : null,
        diffPath != null ? LoadImage(diffPath) : null
      );
    }

    /// <summary>
    /// Builds a rendered-image overlay where differing pixels are marked in red.
    /// </summary>
    public static ImageSource? CreateErrorOverlayImage(string? testPath, string? referencePath)
    {
      var testImage = LoadImage(testPath ?? "");
      if (testImage == null)
        return null;

      var referenceImage = LoadImage(referencePath ?? "");
      if (referenceImage == null)
        return testImage;

      try
      {
        var test = ConvertToBgra32(testImage);
        var reference = ConvertToBgra32(referenceImage);

        int width = test.PixelWidth;
        int height = test.PixelHeight;
        int stride = width * 4;
        var pixels = new byte[stride * height];
        test.CopyPixels(pixels, stride, 0);

        int referenceWidth = Math.Min(width, reference.PixelWidth);
        int referenceHeight = Math.Min(height, reference.PixelHeight);
        int referenceStride = reference.PixelWidth * 4;
        var referencePixels = new byte[referenceStride * reference.PixelHeight];
        reference.CopyPixels(referencePixels, referenceStride, 0);

        for (int y = 0; y < referenceHeight; ++y)
        {
          for (int x = 0; x < referenceWidth; ++x)
          {
            int pixelIndex = y * stride + x * 4;
            int referenceIndex = y * referenceStride + x * 4;
            int error =
              Math.Abs(pixels[pixelIndex + 0] - referencePixels[referenceIndex + 0]) +
              Math.Abs(pixels[pixelIndex + 1] - referencePixels[referenceIndex + 1]) +
              Math.Abs(pixels[pixelIndex + 2] - referencePixels[referenceIndex + 2]);

            if (error <= 18)
              continue;

            pixels[pixelIndex + 0] = 24;
            pixels[pixelIndex + 1] = 24;
            pixels[pixelIndex + 2] = 255;
            pixels[pixelIndex + 3] = 255;
          }
        }

        var overlay = BitmapSource.Create(
          width,
          height,
          test.DpiX,
          test.DpiY,
          PixelFormats.Bgra32,
          null,
          pixels,
          stride);
        overlay.Freeze();
        return overlay;
      }
      catch (Exception ex)
      {
        T3Core.Log($"Failed to create error overlay image: {ex.Message}", T3LogLevel.Error);
        return testImage;
      }
    }

    private static BitmapSource ConvertToBgra32(BitmapSource source)
    {
      if (source.Format == PixelFormats.Bgra32)
        return source;

      var converted = new FormatConvertedBitmap();
      converted.BeginInit();
      converted.Source = source;
      converted.DestinationFormat = PixelFormats.Bgra32;
      converted.EndInit();
      converted.Freeze();
      return converted;
    }
  }
}
