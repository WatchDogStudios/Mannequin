#pragma once

#include <Texture/TextureDLL.h>
#include <Texture/Image/Image.h>
#include <Foundation/Strings/String.h>

/// Descriptor for a texture comparison job
struct NS_TEXTURE_DLL nsTexComparerDescriptor
{
  nsString m_sActualFile;
  nsString m_sExpectedFile;
  nsUInt32 m_MeanSquareErrorThreshold = 0;
  bool m_bRelaxedComparison = false;
};

/// Compares two textures and generates comparison reports
class NS_TEXTURE_DLL nsTexComparer
{
public:
  nsResult Compare();

  nsTexComparerDescriptor m_Descriptor;

  // Output images
  nsImage m_OutputImageDiffRgb;
  nsImage m_OutputImageDiffAlpha;

  nsImage m_ExtractedExpectedRgb;
  nsImage m_ExtractedExpectedAlpha;
  nsImage m_ExtractedActualRgb;
  nsImage m_ExtractedActualAlpha;

  // Output metrics
  nsUInt32 m_OutputMSE = 0;
  bool m_bExceededMSE = false;

  nsUInt32 m_uiOutputMinDiffRgb = 0;
  nsUInt32 m_uiOutputMaxDiffRgb = 0;
  nsUInt32 m_uiOutputMinDiffAlpha = 0;
  nsUInt32 m_uiOutputMaxDiffAlpha = 0;
};
