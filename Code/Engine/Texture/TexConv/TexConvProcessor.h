#pragma once

#include <Texture/TextureDLL.h>
#include <Texture/Image/Image.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/String.h>

/// Channel mapping for texture conversion
struct NS_TEXTURE_DLL nsTexConvChannelMapping
{
  nsInt8 m_iInput = 0;
  nsUInt8 m_uiChannelMask = 0;
};

/// Texture conversion processor
class NS_TEXTURE_DLL nsTexConvProcessor
{
public:
  nsResult Process();

  nsDynamicArray<nsString> m_InputFiles;
  nsString m_sOutputFile;
  nsImage m_OutputImage;

  nsTexConvChannelMapping m_ChannelMapping[4];
};
