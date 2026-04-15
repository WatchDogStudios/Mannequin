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

nsUInt32 nsImageUtils::ComputeMeanSquareError(const nsImage& differenceImage, nsUInt32 uiBlockSize)
{
  // Stub: return 0 (no error)
  return 0;
}

void nsImageUtils::ComputeImageDifferenceABS(const nsImage& imageA, const nsImage& imageB, nsImage& ref_diff)
{
  if (!imageA.IsValid() || !imageB.IsValid())
    return;

  ref_diff.ResetAndAlloc(imageA.GetImageFormat(), imageA.GetWidth(), imageA.GetHeight());
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

void nsImageUtils::CropImage(const nsImage& source, const nsVec2I32& vOffset, const nsSizeU32& size, nsImage& ref_output)
{
  nsLog::Warning("nsImageUtils::CropImage not yet implemented");
  ref_output.ResetAndAlloc(source.GetImageFormat(), size.width, size.height);
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

void nsImageUtils::CreateImageDiffHtml(nsStringBuilder& ref_html, nsStringView sTitle,
  const nsImage& expectedRgb, const nsImage& expectedAlpha,
  const nsImage& actualRgb, const nsImage& actualAlpha,
  const nsImage& diffRgb, const nsImage& diffAlpha,
  nsUInt32 uiMSE, nsUInt32 uiMSEThreshold,
  nsUInt32 uiMinDiffRgb, nsUInt32 uiMaxDiffRgb,
  nsUInt32 uiMinDiffAlpha, nsUInt32 uiMaxDiffAlpha)
{
  ref_html.Clear();
  ref_html.AppendFormat("<html><head><title>{}</title></head><body>", sTitle);
  ref_html.AppendFormat("<h1>{}</h1>", sTitle);
  ref_html.AppendFormat("<p>MSE: {} (threshold: {})</p>", uiMSE, uiMSEThreshold);
  ref_html.AppendFormat("<p>RGB diff range: [{}, {}]</p>", uiMinDiffRgb, uiMaxDiffRgb);
  ref_html.AppendFormat("<p>Alpha diff range: [{}, {}]</p>", uiMinDiffAlpha, uiMaxDiffAlpha);
  ref_html.Append("</body></html>");
}
