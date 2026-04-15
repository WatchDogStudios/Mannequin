#pragma once

#include <Texture/Image/Image.h>
#include <Foundation/Math/Size.h>

/// Utility class for image operations (scaling, comparison, etc.)
class NS_TEXTURE_DLL nsImageUtils
{
public:
  /// Scale image to new dimensions
  static nsResult Scale(const nsImage& source, nsImage& ref_target, nsUInt32 uiWidth, nsUInt32 uiHeight);

  /// Compare two images, returns mean squared error (float version)
  static float ComputeMSE(const nsImage& imageA, const nsImage& imageB);

  /// Compute the mean square error from a difference image, with a block size
  static nsUInt32 ComputeMeanSquareError(const nsImage& differenceImage, nsUInt32 uiBlockSize);

  /// Compute absolute difference image
  static void ComputeImageDifferenceABS(const nsImage& imageA, const nsImage& imageB, nsImage& ref_diff);

  /// Compute difference image
  static nsResult ComputeDifferenceImage(const nsImage& imageA, const nsImage& imageB, nsImage& ref_diff);

  /// Copy a sub-region of one image to another
  static nsResult CopySubImage(const nsImage& source, nsImage& ref_target, nsUInt32 uiSrcX, nsUInt32 uiSrcY, nsUInt32 uiDstX, nsUInt32 uiDstY, nsUInt32 uiWidth, nsUInt32 uiHeight);

  /// Crop image to a sub-region
  static void CropImage(const nsImage& source, const nsVec2I32& vOffset, const nsSizeU32& size, nsImage& ref_output);

  /// Flip image vertically
  static void FlipVertically(nsImage& inout_image);

  /// Create a solid color image
  static void CreateSolidColorImage(nsImage& ref_image, nsUInt32 uiWidth, nsUInt32 uiHeight, const nsColor& color);

  /// Generate an HTML file showing image differences
  static void CreateImageDiffHtml(nsStringBuilder& ref_html, nsStringView sTitle,
    const nsImage& expectedRgb, const nsImage& expectedAlpha,
    const nsImage& actualRgb, const nsImage& actualAlpha,
    const nsImage& diffRgb, const nsImage& diffAlpha,
    nsUInt32 uiMSE, nsUInt32 uiMSEThreshold,
    nsUInt32 uiMinDiffRgb, nsUInt32 uiMaxDiffRgb,
    nsUInt32 uiMinDiffAlpha, nsUInt32 uiMaxDiffAlpha);

  /// Compute absolute difference image with relaxed tolerance
  static void ComputeImageDifferenceABSRelaxed(const nsImage& imageA, const nsImage& imageB, nsImage& ref_diff);

  /// Normalize image pixel values to [0, 255] range
  static void Normalize(nsImage& inout_image);

  /// Normalize image pixel values and output min/max differences per channel
  static void Normalize(nsImage& inout_image, nsUInt8& out_uiMinDiffRgb, nsUInt8& out_uiMaxDiffRgb, nsUInt8& out_uiMinDiffAlpha, nsUInt8& out_uiMaxDiffAlpha);

  /// Extract the alpha channel into a separate grayscale image
  static void ExtractAlphaChannel(const nsImage& source, nsImage& ref_alpha);
};
