#pragma once

#include <Texture/TextureDLL.h>
#include <Foundation/Strings/String.h>

/// Compares two textures and generates comparison reports
class NS_TEXTURE_DLL nsTexComparer
{
public:
  nsResult Compare();

  nsString m_sInputFileA;
  nsString m_sInputFileB;
  nsString m_sOutputFile;
  nsString m_sHtmlTitle;

  nsUInt32 m_uiMeanErrorThreshold = 0;
  nsUInt32 m_uiMaxErrorThreshold = 0;
};
