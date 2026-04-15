#pragma once

#include <Texture/TextureDLL.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Vec2.h>

// Forward declarations
class nsImageFormat;

/// Image format enum
struct NS_TEXTURE_DLL nsImageFormat_Enum
{
  enum Enum
  {
    R8G8B8A8_UNORM,
    R8G8B8A8_UNORM_SRGB,
    B8G8R8A8_UNORM,
    B8G8R8A8_UNORM_SRGB,
    R32G32B32A32_FLOAT,
    R16G16B16A16_FLOAT,
    R8_UNORM,
    R8G8_UNORM,
    BC1_UNORM,
    BC3_UNORM,
    BC7_UNORM,
    UNKNOWN,
    NUM_FORMATS
  };
};

using nsImageFormat = nsImageFormat_Enum;

/// Core image class for texture manipulation and comparison
class NS_TEXTURE_DLL nsImage
{
public:
  nsImage();
  nsImage(const nsImage& other);
  nsImage(nsImage&& other) noexcept;
  ~nsImage();

  nsImage& operator=(const nsImage& other);
  nsImage& operator=(nsImage&& other) noexcept;

  /// Initialize with given dimensions and format
  void AllocateImageData(nsImageFormat::Enum format, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiDepth = 1, nsUInt32 uiMipLevels = 1);

  /// Reset to empty state
  void ResetAndAlloc(nsImageFormat::Enum format, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiDepth = 1, nsUInt32 uiMipLevels = 1);

  /// Load image from file
  nsResult LoadFrom(nsStringView sPath);

  /// Save image to file
  nsResult SaveTo(nsStringView sPath) const;

  /// Get raw pixel data
  const void* GetPixelPointer(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0) const;
  void* GetPixelPointer(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0);

  template <typename T>
  const T* GetPixelPointer(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0) const
  {
    return reinterpret_cast<const T*>(GetPixelPointer(uiMipLevel, uiFace, uiArrayIndex));
  }

  /// Access dimensions
  nsUInt32 GetWidth(nsUInt32 uiMipLevel = 0) const;
  nsUInt32 GetHeight(nsUInt32 uiMipLevel = 0) const;
  nsUInt32 GetDepth(nsUInt32 uiMipLevel = 0) const;
  nsUInt32 GetNumMipLevels() const;
  nsUInt32 GetNumFaces() const;
  nsUInt32 GetNumArrayIndices() const;
  nsImageFormat::Enum GetImageFormat() const;

  /// Check if image data is allocated
  bool IsValid() const;

  /// Row pitch in bytes
  nsUInt64 GetRowPitch(nsUInt32 uiMipLevel = 0) const;

  /// Depth pitch in bytes
  nsUInt64 GetDepthPitch(nsUInt32 uiMipLevel = 0) const;

  /// Total data size in bytes
  nsUInt64 GetDataSize() const;

private:
  nsUInt32 m_uiWidth = 0;
  nsUInt32 m_uiHeight = 0;
  nsUInt32 m_uiDepth = 1;
  nsUInt32 m_uiMipLevels = 1;
  nsUInt32 m_uiNumFaces = 1;
  nsUInt32 m_uiNumArrayIndices = 1;
  nsImageFormat::Enum m_Format = nsImageFormat::UNKNOWN;
  nsDynamicArray<nsUInt8> m_Data;
};
