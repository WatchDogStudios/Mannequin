#pragma once

#include <Texture/Image/Image.h>

/// Utility class for image operations (scaling, comparison, etc.)
class NS_TEXTURE_DLL nsImageUtils
{
public:
  /// Scale image to new dimensions
  static nsResult Scale(const nsImage& source, nsImage& ref_target, nsUInt32 uiWidth, nsUInt32 uiHeight);

  /// Compare two images, returns mean squared error
  static float ComputeMSE(const nsImage& imageA, const nsImage& imageB);

  /// Compute difference image
  static nsResult ComputeDifferenceImage(const nsImage& imageA, const nsImage& imageB, nsImage& ref_diff);

  /// Copy a sub-region of one image to another
  static nsResult CopySubImage(const nsImage& source, nsImage& ref_target, nsUInt32 uiSrcX, nsUInt32 uiSrcY, nsUInt32 uiDstX, nsUInt32 uiDstY, nsUInt32 uiWidth, nsUInt32 uiHeight);

  /// Flip image vertically
  static void FlipVertically(nsImage& inout_image);

  /// Create a solid color image
  static void CreateSolidColorImage(nsImage& ref_image, nsUInt32 uiWidth, nsUInt32 uiHeight, const nsColor& color);
};
