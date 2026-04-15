#pragma once

#include <Texture/Image/Image.h>
#include <Foundation/Containers/HybridArray.h>

/// Utility for converting between image formats
class NS_TEXTURE_DLL nsImageConversion
{
public:
  /// Represents one step in a conversion path
  struct ConversionPathNode
  {
    nsImageFormat::Enum m_sourceFormat = nsImageFormat::UNKNOWN;
    nsImageFormat::Enum m_targetFormat = nsImageFormat::UNKNOWN;
  };

  /// Convert image from one format to another
  static nsResult Convert(const nsImage& source, nsImage& ref_target, nsImageFormat::Enum targetFormat);

  /// Check if conversion between two formats is supported
  static bool IsConvertible(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat);

  /// Build a conversion path between two formats
  static nsResult BuildPath(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat, bool bSourceEqualsTarget,
    nsHybridArray<ConversionPathNode, 16>& ref_path, nsUInt32& ref_uiNumScratchBuffers);
};
