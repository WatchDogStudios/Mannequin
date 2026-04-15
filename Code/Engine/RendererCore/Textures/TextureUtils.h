#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Status.h>

/// \brief Utility functions for texture processing.
class NS_RENDERERCORE_DLL nsTextureUtils
{
public:
  static nsStringView GetUsageString(nsUInt32 uiUsage);
  static bool IsCompressedFormat(nsUInt32 uiFormat);
  static nsUInt32 GetRowPitch(nsUInt32 uiFormat, nsUInt32 uiWidth);
};
