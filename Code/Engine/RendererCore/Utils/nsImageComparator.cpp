// Copyright (c) WD Studios. All rights reserved.
// Mannequin Visual Test Pipeline - FLIP Image Comparison Implementation

#include "nsImageComparator.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>

// FLIP library integration
// When the FLIP submodule is available, include its header.
// Otherwise, fall back to a simple per-pixel comparison.
#if __has_include(<FLIP.h>)
  #define NS_HAS_FLIP 1
  #include <FLIP.h>
#else
  #define NS_HAS_FLIP 0
#endif

nsImageComparator::nsImageComparator() = default;
nsImageComparator::~nsImageComparator() = default;

nsImageComparisonResult nsImageComparator::Compare(const nsCapturedImage& testImage,
                                                    const nsCapturedImage& referenceImage) const
{
  nsImageComparisonResult result;

  if (!testImage.IsValid() || !referenceImage.IsValid())
    return result;

  if (testImage.m_uiWidth != referenceImage.m_uiWidth || testImage.m_uiHeight != referenceImage.m_uiHeight)
    return result;

  const uint32_t w = testImage.m_uiWidth;
  const uint32_t h = testImage.m_uiHeight;
  result.m_uiTotalPixels = w * h;

#if NS_HAS_FLIP
  // Use NVIDIA FLIP for perceptual comparison
  // FLIP computes a per-pixel error metric that models human perception.
  // TODO: Integrate FLIP::evaluate() once submodule include paths are configured.
  // For now, fall through to the manual comparison below.
#endif

  // Fallback: Manual per-pixel FLIP-inspired comparison
  // Uses a simplified color-difference metric until FLIP is fully integrated.
  double totalError = 0.0;
  float maxError = 0.0f;
  std::vector<float> errors(w * h);

  for (uint32_t y = 0; y < h; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      float testRGBA[4], refRGBA[4];
      testImage.GetPixelFloat(x, y, testRGBA);
      referenceImage.GetPixelFloat(x, y, refRGBA);

      // Perceptual color difference (simplified CIE76-like metric)
      float dr = testRGBA[0] - refRGBA[0];
      float dg = testRGBA[1] - refRGBA[1];
      float db = testRGBA[2] - refRGBA[2];
      float da = testRGBA[3] - refRGBA[3];
      float error = std::sqrt(dr * dr + dg * dg + db * db + da * da) / 2.0f; // Normalize to [0,1]

      errors[y * w + x] = error;
      totalError += error;
      maxError = std::max(maxError, error);

      if (error > m_Config.m_fPerPixelThreshold)
        result.m_uiPixelsAboveThreshold++;
    }
  }

  result.m_fMeanError = static_cast<float>(totalError / (w * h));
  result.m_fMaxError = maxError;

  // Compute median and 95th percentile
  std::sort(errors.begin(), errors.end());
  result.m_fMedianError = errors[errors.size() / 2];
  result.m_f95thPercentile = errors[static_cast<size_t>(errors.size() * 0.95)];

  // Determine pass/fail
  result.m_bPassed = (result.m_fMeanError <= m_Config.m_fMeanErrorThreshold) &&
                     (result.GetFailurePercentage() <= m_Config.m_fMaxFailurePercentage);

  return result;
}

nsImageComparisonResult nsImageComparator::CompareWithFile(const nsCapturedImage& testImage,
                                                            const std::string& referenceImagePath) const
{
  nsCapturedImage refImage;
  if (!refImage.LoadPNG(referenceImagePath))
  {
    nsImageComparisonResult result;
    // If no reference image exists, this is a new test — save the test image as reference
    return result;
  }

  return Compare(testImage, refImage);
}

bool nsImageComparator::GenerateHeatmap(const nsCapturedImage& testImage, const nsCapturedImage& referenceImage,
                                         nsCapturedImage& outHeatmap) const
{
  if (!testImage.IsValid() || !referenceImage.IsValid())
    return false;

  if (testImage.m_uiWidth != referenceImage.m_uiWidth || testImage.m_uiHeight != referenceImage.m_uiHeight)
    return false;

  const uint32_t w = testImage.m_uiWidth;
  const uint32_t h = testImage.m_uiHeight;

  outHeatmap.m_uiWidth = w;
  outHeatmap.m_uiHeight = h;
  outHeatmap.m_uiRowPitch = w * 4;
  outHeatmap.m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;
  outHeatmap.m_Data.resize(w * h * 4);

  for (uint32_t y = 0; y < h; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      float testRGBA[4], refRGBA[4];
      testImage.GetPixelFloat(x, y, testRGBA);
      referenceImage.GetPixelFloat(x, y, refRGBA);

      float dr = testRGBA[0] - refRGBA[0];
      float dg = testRGBA[1] - refRGBA[1];
      float db = testRGBA[2] - refRGBA[2];
      float error = std::sqrt(dr * dr + dg * dg + db * db) / 1.732f; // Normalize by sqrt(3)

      // Heatmap coloring: green → yellow → red
      uint8_t r, g, b;
      if (error < 0.5f)
      {
        float t = error * 2.0f;
        r = static_cast<uint8_t>(t * 255);
        g = 255;
        b = 0;
      }
      else
      {
        float t = (error - 0.5f) * 2.0f;
        r = 255;
        g = static_cast<uint8_t>((1.0f - t) * 255);
        b = 0;
      }

      uint8_t* dst = outHeatmap.m_Data.data() + (y * w + x) * 4;
      dst[0] = r;
      dst[1] = g;
      dst[2] = b;
      dst[3] = 255;
    }
  }

  return true;
}

bool nsImageComparator::SaveComparisonReport(const nsCapturedImage& testImage, const nsCapturedImage& referenceImage,
                                              const nsImageComparisonResult& result, const std::string& outputDir,
                                              const std::string& testName) const
{
  namespace fs = std::filesystem;
  fs::create_directories(outputDir);

  std::string basePath = outputDir + "/" + testName;

  // Save test image
  testImage.SavePNG(basePath + "_test.png");

  // Save reference image
  referenceImage.SavePNG(basePath + "_reference.png");

  // Generate and save heatmap
  nsCapturedImage heatmap;
  if (GenerateHeatmap(testImage, referenceImage, heatmap))
  {
    heatmap.SavePNG(basePath + "_diff.png");
  }

  // Save text report
  FILE* f = fopen((basePath + "_report.txt").c_str(), "w");
  if (f)
  {
    fprintf(f, "Mannequin Visual Test Report: %s\n", testName.c_str());
    fprintf(f, "============================================\n");
    fprintf(f, "Result:          %s\n", result.m_bPassed ? "PASSED" : "FAILED");
    fprintf(f, "Mean Error:      %.6f (threshold: %.6f)\n", result.m_fMeanError, m_Config.m_fMeanErrorThreshold);
    fprintf(f, "Max Error:       %.6f\n", result.m_fMaxError);
    fprintf(f, "Median Error:    %.6f\n", result.m_fMedianError);
    fprintf(f, "95th Percentile: %.6f\n", result.m_f95thPercentile);
    fprintf(f, "Pixels Failed:   %u / %u (%.2f%%)\n",
            result.m_uiPixelsAboveThreshold, result.m_uiTotalPixels, result.GetFailurePercentage());
    fclose(f);
  }

  return true;
}
