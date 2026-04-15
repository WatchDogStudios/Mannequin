#include <Texture/Image/Image.h>
#include <Foundation/Logging/Log.h>

// Helper: bytes per pixel for uncompressed formats
static nsUInt32 GetBytesPerPixel(nsImageFormat::Enum format)
{
  switch (format)
  {
    case nsImageFormat::R8_UNORM:
      return 1;
    case nsImageFormat::R8G8_UNORM:
      return 2;
    case nsImageFormat::R8G8B8A8_UNORM:
    case nsImageFormat::R8G8B8A8_UNORM_SRGB:
    case nsImageFormat::B8G8R8A8_UNORM:
    case nsImageFormat::B8G8R8A8_UNORM_SRGB:
      return 4;
    case nsImageFormat::R16G16B16A16_FLOAT:
      return 8;
    case nsImageFormat::R32G32B32A32_FLOAT:
      return 16;
    default:
      return 0;
  }
}

static nsUInt32 GetMipDimension(nsUInt32 uiBase, nsUInt32 uiMipLevel)
{
  nsUInt32 val = uiBase >> uiMipLevel;
  return val > 0 ? val : 1;
}

nsImage::nsImage() = default;

nsImage::nsImage(const nsImage& other)
  : m_uiWidth(other.m_uiWidth)
  , m_uiHeight(other.m_uiHeight)
  , m_uiDepth(other.m_uiDepth)
  , m_uiMipLevels(other.m_uiMipLevels)
  , m_uiNumFaces(other.m_uiNumFaces)
  , m_uiNumArrayIndices(other.m_uiNumArrayIndices)
  , m_Format(other.m_Format)
  , m_Data(other.m_Data)
{
}

nsImage::nsImage(nsImage&& other) noexcept
  : m_uiWidth(other.m_uiWidth)
  , m_uiHeight(other.m_uiHeight)
  , m_uiDepth(other.m_uiDepth)
  , m_uiMipLevels(other.m_uiMipLevels)
  , m_uiNumFaces(other.m_uiNumFaces)
  , m_uiNumArrayIndices(other.m_uiNumArrayIndices)
  , m_Format(other.m_Format)
  , m_Data(std::move(other.m_Data))
{
  other.m_uiWidth = 0;
  other.m_uiHeight = 0;
  other.m_uiDepth = 1;
  other.m_uiMipLevels = 1;
  other.m_Format = nsImageFormat::UNKNOWN;
}

nsImage::~nsImage() = default;

nsImage& nsImage::operator=(const nsImage& other)
{
  if (this != &other)
  {
    m_uiWidth = other.m_uiWidth;
    m_uiHeight = other.m_uiHeight;
    m_uiDepth = other.m_uiDepth;
    m_uiMipLevels = other.m_uiMipLevels;
    m_uiNumFaces = other.m_uiNumFaces;
    m_uiNumArrayIndices = other.m_uiNumArrayIndices;
    m_Format = other.m_Format;
    m_Data = other.m_Data;
  }
  return *this;
}

nsImage& nsImage::operator=(nsImage&& other) noexcept
{
  if (this != &other)
  {
    m_uiWidth = other.m_uiWidth;
    m_uiHeight = other.m_uiHeight;
    m_uiDepth = other.m_uiDepth;
    m_uiMipLevels = other.m_uiMipLevels;
    m_uiNumFaces = other.m_uiNumFaces;
    m_uiNumArrayIndices = other.m_uiNumArrayIndices;
    m_Format = other.m_Format;
    m_Data = std::move(other.m_Data);
    other.m_uiWidth = 0;
    other.m_uiHeight = 0;
    other.m_uiDepth = 1;
    other.m_uiMipLevels = 1;
    other.m_Format = nsImageFormat::UNKNOWN;
  }
  return *this;
}

void nsImage::AllocateImageData(nsImageFormat::Enum format, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiDepth, nsUInt32 uiMipLevels)
{
  m_Format = format;
  m_uiWidth = uiWidth;
  m_uiHeight = uiHeight;
  m_uiDepth = uiDepth;
  m_uiMipLevels = uiMipLevels;

  nsUInt64 uiTotalSize = 0;
  nsUInt32 bpp = GetBytesPerPixel(format);
  for (nsUInt32 mip = 0; mip < uiMipLevels; ++mip)
  {
    nsUInt32 w = GetMipDimension(uiWidth, mip);
    nsUInt32 h = GetMipDimension(uiHeight, mip);
    nsUInt32 d = GetMipDimension(uiDepth, mip);
    uiTotalSize += static_cast<nsUInt64>(w) * h * d * bpp * m_uiNumFaces * m_uiNumArrayIndices;
  }

  m_Data.SetCountUninitialized(static_cast<nsUInt32>(uiTotalSize));
}

void nsImage::ResetAndAlloc(nsImageFormat::Enum format, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiDepth, nsUInt32 uiMipLevels)
{
  m_Data.Clear();
  AllocateImageData(format, uiWidth, uiHeight, uiDepth, uiMipLevels);
}

nsResult nsImage::LoadFrom(nsStringView sPath)
{
  nsLog::Warning("nsImage::LoadFrom not yet implemented");
  return NS_FAILURE;
}

nsResult nsImage::SaveTo(nsStringView sPath) const
{
  nsLog::Warning("nsImage::SaveTo not yet implemented");
  return NS_FAILURE;
}

const void* nsImage::GetPixelPointer(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex) const
{
  if (m_Data.IsEmpty())
    return nullptr;

  return m_Data.GetData();
}

void* nsImage::GetPixelPointer(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex)
{
  if (m_Data.IsEmpty())
    return nullptr;

  return m_Data.GetData();
}

nsUInt32 nsImage::GetWidth(nsUInt32 uiMipLevel) const
{
  return GetMipDimension(m_uiWidth, uiMipLevel);
}

nsUInt32 nsImage::GetHeight(nsUInt32 uiMipLevel) const
{
  return GetMipDimension(m_uiHeight, uiMipLevel);
}

nsUInt32 nsImage::GetDepth(nsUInt32 uiMipLevel) const
{
  return GetMipDimension(m_uiDepth, uiMipLevel);
}

nsUInt32 nsImage::GetNumMipLevels() const
{
  return m_uiMipLevels;
}

nsUInt32 nsImage::GetNumFaces() const
{
  return m_uiNumFaces;
}

nsUInt32 nsImage::GetNumArrayIndices() const
{
  return m_uiNumArrayIndices;
}

nsImageFormat::Enum nsImage::GetImageFormat() const
{
  return m_Format;
}

bool nsImage::IsValid() const
{
  return m_uiWidth > 0 && m_uiHeight > 0 && !m_Data.IsEmpty();
}

nsUInt64 nsImage::GetRowPitch(nsUInt32 uiMipLevel) const
{
  return static_cast<nsUInt64>(GetMipDimension(m_uiWidth, uiMipLevel)) * GetBytesPerPixel(m_Format);
}

nsUInt64 nsImage::GetDepthPitch(nsUInt32 uiMipLevel) const
{
  return GetRowPitch(uiMipLevel) * GetMipDimension(m_uiHeight, uiMipLevel);
}

nsUInt64 nsImage::GetDataSize() const
{
  return m_Data.GetCount();
}
