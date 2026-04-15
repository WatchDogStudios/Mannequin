#pragma once

#include <Texture/Image/Image.h>

/// Utility for converting between image formats
class NS_TEXTURE_DLL nsImageConversion
{
public:
  /// Convert image from one format to another
  static nsResult Convert(const nsImage& source, nsImage& ref_target, nsImageFormat::Enum targetFormat);

  /// Check if conversion between two formats is supported
  static bool IsConvertible(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat);
};
