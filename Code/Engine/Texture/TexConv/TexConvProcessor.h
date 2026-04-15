#pragma once

#include <Texture/TextureDLL.h>
#include <Texture/Image/Image.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/IO/Stream.h>

// --- Enums used by nsTexConvDescriptor ---

struct NS_TEXTURE_DLL nsTexConvOutputType
{
  enum Enum
  {
    None = 0,
    Texture2D = 1,
    Volume = 2,
    Cubemap = 3,
    Atlas = 4,
  };
};

struct NS_TEXTURE_DLL nsTexConvUsage
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Auto = 0,
    Color = 1,
    Linear = 2,
    Hdr = 3,
    NormalMap = 4,
    NormalMap_Inverted = 5,
    BumpMap = 6,
  };
};

struct NS_TEXTURE_DLL nsTexConvMipmapMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None = 0,
    Linear = 1,
    Kaiser = 2,
  };
};

struct NS_TEXTURE_DLL nsTexConvTargetPlatform
{
  using StorageType = nsUInt8;

  enum Enum
  {
    PC = 0,
    Android = 1,
  };
};

struct NS_TEXTURE_DLL nsTexConvCompressionMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None = 0,
    Medium = 1,
    High = 2,
  };
};

struct NS_TEXTURE_DLL nsImageAddressMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Repeat = 0,
    Clamp = 1,
    ClampBorder = 2,
    Mirror = 3,
  };
};

struct NS_TEXTURE_DLL nsTextureFilterSetting
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Nearest = 0,
    Linear = 1,
    Trilinear = 2,
    Aniso2x = 3,
    Aniso4x = 4,
    Aniso8x = 5,
    Aniso16x = 6,
    Lowest = 7,
    Low = 8,
    Default = 9,
    High = 10,
    Highest = 11,
  };
};

struct NS_TEXTURE_DLL nsTexConvBumpMapFilter
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Finite = 0,
    Sobel = 1,
    Scharr = 2,
  };
};

struct NS_TEXTURE_DLL nsTexConvChannelValue
{
  enum Enum
  {
    Red = 0,
    Green = 1,
    Blue = 2,
    Alpha = 3,
    Black = 4,
    White = 5,
  };
};

/// Describes a single channel mapping (input image index + which channel to read)
struct NS_TEXTURE_DLL nsTexConvChannelMapping
{
  nsInt8 m_iInputImageIndex = -1;
  nsTexConvChannelValue::Enum m_ChannelValue = nsTexConvChannelValue::Red;
};

/// Contains four channel mappings for one output slice (R, G, B, A)
struct NS_TEXTURE_DLL nsTexConvSliceChannelMapping
{
  nsTexConvChannelMapping m_Channel[4];
};

/// Full descriptor for a texture conversion job
struct NS_TEXTURE_DLL nsTexConvDescriptor
{
  nsTexConvOutputType::Enum m_OutputType = nsTexConvOutputType::None;
  nsTexConvUsage::Enum m_Usage = nsTexConvUsage::Auto;
  nsTexConvMipmapMode::Enum m_MipmapMode = nsTexConvMipmapMode::Linear;
  nsTexConvTargetPlatform::Enum m_TargetPlatform = nsTexConvTargetPlatform::PC;
  nsTexConvCompressionMode::Enum m_CompressionMode = nsTexConvCompressionMode::Medium;
  nsTexConvBumpMapFilter::Enum m_BumpMapFilter = nsTexConvBumpMapFilter::Finite;

  nsImageAddressMode::Enum m_AddressModeU = nsImageAddressMode::Repeat;
  nsImageAddressMode::Enum m_AddressModeV = nsImageAddressMode::Repeat;
  nsImageAddressMode::Enum m_AddressModeW = nsImageAddressMode::Repeat;
  nsTextureFilterSetting::Enum m_FilterMode = nsTextureFilterSetting::Default;

  nsDynamicArray<nsString> m_InputFiles;
  nsDynamicArray<nsTexConvSliceChannelMapping> m_ChannelMappings;

  nsString m_sTextureAtlasDescFile;

  nsUInt32 m_uiThumbnailOutputResolution = 0;
  nsUInt32 m_uiLowResMipmaps = 0;
  nsUInt32 m_uiMinResolution = 16;
  nsUInt32 m_uiMaxResolution = 8 * 1024;
  nsUInt32 m_uiDownscaleSteps = 0;

  nsUInt64 m_uiAssetHash = 0;
  nsUInt16 m_uiAssetVersion = 0;

  bool m_bFlipHorizontal = false;
  bool m_bPremultiplyAlpha = false;
  bool m_bPreserveMipmapCoverage = false;
  nsUInt8 m_uiDilateColor = 0;
  float m_fMipmapAlphaThreshold = 0.5f;
  float m_fHdrExposureBias = 0.0f;
  float m_fMaxValue = 64000.0f;
};

/// Placeholder for texture atlas output data
class NS_TEXTURE_DLL nsTextureAtlasData
{
public:
  nsResult CopyToStream(nsStreamWriter& inout_stream) const;
};

/// Texture conversion processor
class NS_TEXTURE_DLL nsTexConvProcessor
{
public:
  nsResult Process();

  nsTexConvDescriptor m_Descriptor;

  nsImage m_OutputImage;
  nsImage m_ThumbnailOutputImage;
  nsImage m_LowResOutputImage;

  nsTextureAtlasData m_TextureAtlas;
};
