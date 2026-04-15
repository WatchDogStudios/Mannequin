#pragma once

#include <Texture/TextureDLL.h>
#include <Texture/TexConv/TexConvProcessor.h>
#include <Foundation/IO/Stream.h>

/// Describes the texture format metadata written to ns-specific texture files
class NS_TEXTURE_DLL nsTexFormat
{
public:
  nsTexFormat() = default;

  void WriteTextureHeader(nsStreamWriter& inout_stream) const;
  void ReadTextureHeader(nsStreamReader& inout_stream);

  bool m_bSRGB = false;
  nsImageAddressMode::Enum m_AddressModeU = nsImageAddressMode::Repeat;
  nsImageAddressMode::Enum m_AddressModeV = nsImageAddressMode::Repeat;
  nsImageAddressMode::Enum m_AddressModeW = nsImageAddressMode::Repeat;
  nsTextureFilterSetting::Enum m_TextureFilter = nsTextureFilterSetting::Default;
};
