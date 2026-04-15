#include <Texture/nsTexFormat/nsTexFormat.h>
#include <Foundation/Logging/Log.h>

void nsTexFormat::WriteTextureHeader(nsStreamWriter& inout_stream) const
{
  inout_stream << m_bSRGB;
  inout_stream << static_cast<nsUInt8>(m_AddressModeU);
  inout_stream << static_cast<nsUInt8>(m_AddressModeV);
  inout_stream << static_cast<nsUInt8>(m_AddressModeW);
  inout_stream << static_cast<nsUInt8>(m_TextureFilter);
}

void nsTexFormat::ReadTextureHeader(nsStreamReader& inout_stream)
{
  nsUInt8 tmp;
  inout_stream >> m_bSRGB;
  inout_stream >> tmp; m_AddressModeU = static_cast<nsImageAddressMode::Enum>(tmp);
  inout_stream >> tmp; m_AddressModeV = static_cast<nsImageAddressMode::Enum>(tmp);
  inout_stream >> tmp; m_AddressModeW = static_cast<nsImageAddressMode::Enum>(tmp);
  inout_stream >> tmp; m_TextureFilter = static_cast<nsTextureFilterSetting::Enum>(tmp);
}
