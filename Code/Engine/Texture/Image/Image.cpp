#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>
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
    case nsImageFormat::R8G8B8_UNORM:
      return 3;
    case nsImageFormat::R8G8B8A8_UNORM:
    case nsImageFormat::R8G8B8A8_UNORM_SRGB:
    case nsImageFormat::B8G8R8A8_UNORM:
    case nsImageFormat::B8G8R8A8_UNORM_SRGB:
      return 4;
    case nsImageFormat::R16G16B16A16_FLOAT:
      return 8;
    case nsImageFormat::R32G32B32A32_FLOAT:
      return 16;
    case nsImageFormat::BC1_UNORM:
      return 0; // block compressed
    case nsImageFormat::BC3_UNORM:
    case nsImageFormat::BC7_UNORM:
      return 0; // block compressed
    default:
      return 0;
  }
}

static nsUInt32 GetMipDimension(nsUInt32 uiBase, nsUInt32 uiMipLevel)
{
  nsUInt32 val = uiBase >> uiMipLevel;
  return val > 0 ? val : 1;
}

// nsImageFormat static methods

bool nsImageFormat::IsSrgb(Enum format)
{
  return format == R8G8B8A8_UNORM_SRGB || format == B8G8R8A8_UNORM_SRGB;
}

bool nsImageFormat::IsCompressed(Enum format)
{
  return format == BC1_UNORM || format == BC3_UNORM || format == BC7_UNORM;
}

const char* nsImageFormat::GetName(Enum format)
{
  switch (format)
  {
    case R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
    case R8G8B8A8_UNORM_SRGB: return "R8G8B8A8_UNORM_SRGB";
    case B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
    case B8G8R8A8_UNORM_SRGB: return "B8G8R8A8_UNORM_SRGB";
    case R32G32B32A32_FLOAT: return "R32G32B32A32_FLOAT";
    case R16G16B16A16_FLOAT: return "R16G16B16A16_FLOAT";
    case R8_UNORM: return "R8_UNORM";
    case R8G8_UNORM: return "R8G8_UNORM";
    case BC1_UNORM: return "BC1_UNORM";
    case BC3_UNORM: return "BC3_UNORM";
    case BC7_UNORM: return "BC7_UNORM";
    case UNKNOWN: return "UNKNOWN";
    default: return "UNKNOWN";
  }
}

// nsImage implementation

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
    if (bpp > 0)
    {
      uiTotalSize += static_cast<nsUInt64>(w) * h * d * bpp * m_uiNumFaces * m_uiNumArrayIndices;
    }
    else
    {
      // Block compressed: 4x4 blocks
      nsUInt32 bw = (w + 3) / 4;
      nsUInt32 bh = (h + 3) / 4;
      nsUInt32 blockSize = (format == nsImageFormat::BC1_UNORM) ? 8 : 16;
      uiTotalSize += static_cast<nsUInt64>(bw) * bh * d * blockSize * m_uiNumFaces * m_uiNumArrayIndices;
    }
  }

  m_Data.SetCountUninitialized(static_cast<nsUInt32>(uiTotalSize));
}

void nsImage::ResetAndAlloc(nsImageFormat::Enum format, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiDepth, nsUInt32 uiMipLevels)
{
  m_Data.Clear();
  AllocateImageData(format, uiWidth, uiHeight, uiDepth, uiMipLevels);
}

void nsImage::ResetAndMove(nsImage&& other)
{
  *this = std::move(other);
}

nsResult nsImage::Convert(nsImageFormat::Enum targetFormat)
{
  if (m_Format == targetFormat)
    return NS_SUCCESS;

  nsImage converted;
  nsResult res = nsImageConversion::Convert(*this, converted, targetFormat);
  if (res.Succeeded())
  {
    *this = std::move(converted);
  }
  return res;
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

const void* nsImage::GetPixelPointer(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex, nsUInt32 x, nsUInt32 y) const
{
  if (m_Data.IsEmpty())
    return nullptr;

  nsUInt32 bpp = GetBytesPerPixel(m_Format);
  if (bpp == 0)
    return m_Data.GetData();

  nsUInt64 offset = (static_cast<nsUInt64>(y) * GetMipDimension(m_uiWidth, uiMipLevel) + x) * bpp;
  if (offset >= m_Data.GetCount())
    return m_Data.GetData();

  return m_Data.GetData() + offset;
}

void* nsImage::GetPixelPointer(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex, nsUInt32 x, nsUInt32 y)
{
  if (m_Data.IsEmpty())
    return nullptr;

  nsUInt32 bpp = GetBytesPerPixel(m_Format);
  if (bpp == 0)
    return m_Data.GetData();

  nsUInt64 offset = (static_cast<nsUInt64>(y) * GetMipDimension(m_uiWidth, uiMipLevel) + x) * bpp;
  if (offset >= m_Data.GetCount())
    return m_Data.GetData();

  return m_Data.GetData() + offset;
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

nsByteBlobPtr nsImage::GetByteBlobPtr()
{
  return nsByteBlobPtr(m_Data.GetData(), m_Data.GetCount());
}

nsConstByteBlobPtr nsImage::GetByteBlobPtr() const
{
  return nsConstByteBlobPtr(m_Data.GetData(), m_Data.GetCount());
}

nsImage nsImage::GetSubImageView(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex) const
{
  nsImage subImage;
  subImage.m_uiWidth = GetWidth(uiMipLevel);
  subImage.m_uiHeight = GetHeight(uiMipLevel);
  subImage.m_uiDepth = GetDepth(uiMipLevel);
  subImage.m_uiMipLevels = 1;
  subImage.m_uiNumFaces = 1;
  subImage.m_uiNumArrayIndices = 1;
  subImage.m_Format = m_Format;
  // Copy relevant data
  nsUInt64 size = static_cast<nsUInt64>(subImage.m_uiWidth) * subImage.m_uiHeight * GetBytesPerPixel(m_Format);
  if (size > 0 && !m_Data.IsEmpty())
  {
    subImage.m_Data.SetCountUninitialized(static_cast<nsUInt32>(size));
    const void* pSrc = GetPixelPointer(uiMipLevel, uiFace, uiArrayIndex);
    if (pSrc)
      memcpy(subImage.m_Data.GetData(), pSrc, static_cast<size_t>(size));
  }
  return subImage;
}
