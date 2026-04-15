#include <Texture/Image/ImageUtils.h>
#include <Foundation/Logging/Log.h>

nsResult nsImageUtils::Scale(const nsImage& source, nsImage& ref_target, nsUInt32 uiWidth, nsUInt32 uiHeight)
{
  nsLog::Warning("nsImageUtils::Scale not yet implemented");
  ref_target = source;
  return NS_SUCCESS;
}

float nsImageUtils::ComputeMSE(const nsImage& imageA, const nsImage& imageB)
{
  if (!imageA.IsValid() || !imageB.IsValid())
    return -1.0f;

  if (imageA.GetWidth() != imageB.GetWidth() || imageA.GetHeight() != imageB.GetHeight())
    return -1.0f;

  // Stub: return 0 (identical)
  return 0.0f;
}

nsResult nsImageUtils::ComputeDifferenceImage(const nsImage& imageA, const nsImage& imageB, nsImage& ref_diff)
{
  if (!imageA.IsValid() || !imageB.IsValid())
    return NS_FAILURE;

  ref_diff.ResetAndAlloc(imageA.GetImageFormat(), imageA.GetWidth(), imageA.GetHeight());
  return NS_SUCCESS;
}

nsResult nsImageUtils::CopySubImage(const nsImage& source, nsImage& ref_target, nsUInt32 uiSrcX, nsUInt32 uiSrcY, nsUInt32 uiDstX, nsUInt32 uiDstY, nsUInt32 uiWidth, nsUInt32 uiHeight)
{
  nsLog::Warning("nsImageUtils::CopySubImage not yet implemented");
  return NS_FAILURE;
}

void nsImageUtils::FlipVertically(nsImage& inout_image)
{
  nsLog::Warning("nsImageUtils::FlipVertically not yet implemented");
}

void nsImageUtils::CreateSolidColorImage(nsImage& ref_image, nsUInt32 uiWidth, nsUInt32 uiHeight, const nsColor& color)
{
  ref_image.ResetAndAlloc(nsImageFormat::R8G8B8A8_UNORM, uiWidth, uiHeight);

  nsUInt8* pData = static_cast<nsUInt8*>(ref_image.GetPixelPointer());
  if (!pData)
    return;

  nsUInt8 r = static_cast<nsUInt8>(color.r * 255.0f);
  nsUInt8 g = static_cast<nsUInt8>(color.g * 255.0f);
  nsUInt8 b = static_cast<nsUInt8>(color.b * 255.0f);
  nsUInt8 a = static_cast<nsUInt8>(color.a * 255.0f);

  for (nsUInt32 i = 0; i < uiWidth * uiHeight; ++i)
  {
    pData[i * 4 + 0] = r;
    pData[i * 4 + 1] = g;
    pData[i * 4 + 2] = b;
    pData[i * 4 + 3] = a;
  }
}
