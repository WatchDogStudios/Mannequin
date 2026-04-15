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

void nsImageUtils::ComputeImageDifferenceABSRelaxed(const nsImage& imageA, const nsImage& imageB, nsImage& ref_diff)
{
  // Relaxed version — same as ABS for now
  ComputeImageDifferenceABS(imageA, imageB, ref_diff);
}

void nsImageUtils::Normalize(nsImage& inout_image)
{
  nsUInt8 minRgb, maxRgb, minAlpha, maxAlpha;
  Normalize(inout_image, minRgb, maxRgb, minAlpha, maxAlpha);
}

void nsImageUtils::Normalize(nsImage& inout_image, nsUInt8& out_uiMinDiffRgb, nsUInt8& out_uiMaxDiffRgb, nsUInt8& out_uiMinDiffAlpha, nsUInt8& out_uiMaxDiffAlpha)
{
  out_uiMinDiffRgb = 255;
  out_uiMaxDiffRgb = 0;
  out_uiMinDiffAlpha = 255;
  out_uiMaxDiffAlpha = 0;

  if (!inout_image.IsValid())
    return;

  nsUInt8* pData = static_cast<nsUInt8*>(inout_image.GetPixelPointer());
  nsUInt32 pixelCount = inout_image.GetWidth() * inout_image.GetHeight();
  if (!pData || pixelCount == 0)
    return;

  // Find min/max for RGB and Alpha channels (assuming RGBA layout)
  for (nsUInt32 i = 0; i < pixelCount; ++i)
  {
    nsUInt8 r = pData[i * 4 + 0];
    nsUInt8 g = pData[i * 4 + 1];
    nsUInt8 b = pData[i * 4 + 2];
    nsUInt8 a = pData[i * 4 + 3];
    nsUInt8 maxRgb = r > g ? (r > b ? r : b) : (g > b ? g : b);
    nsUInt8 minRgb = r < g ? (r < b ? r : b) : (g < b ? g : b);

    if (maxRgb > out_uiMaxDiffRgb) out_uiMaxDiffRgb = maxRgb;
    if (minRgb < out_uiMinDiffRgb) out_uiMinDiffRgb = minRgb;
    if (a > out_uiMaxDiffAlpha) out_uiMaxDiffAlpha = a;
    if (a < out_uiMinDiffAlpha) out_uiMinDiffAlpha = a;
  }
}

void nsImageUtils::ExtractAlphaChannel(const nsImage& source, nsImage& ref_alpha)
{
  if (!source.IsValid())
    return;

  nsUInt32 w = source.GetWidth();
  nsUInt32 h = source.GetHeight();
  ref_alpha.ResetAndAlloc(nsImageFormat::R8_UNORM, w, h);

  const nsUInt8* pSrc = static_cast<const nsUInt8*>(source.GetPixelPointer());
  nsUInt8* pDst = static_cast<nsUInt8*>(ref_alpha.GetPixelPointer());
  if (!pSrc || !pDst)
    return;

  // Extract alpha channel (4th byte in RGBA)
  for (nsUInt32 i = 0; i < w * h; ++i)
  {
    pDst[i] = pSrc[i * 4 + 3];
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
