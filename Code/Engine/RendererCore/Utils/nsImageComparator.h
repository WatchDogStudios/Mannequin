#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Mannequin Visual Test Pipeline - Image Comparison using NVIDIA FLIP
// Wraps the FLIP library to provide perceptual image comparison metrics.

#include "nsImageCapture.h"

#include <string>
#include <vector>

/// Result of an image comparison.
struct nsImageComparisonResult
{
  float m_fMeanError = 0.0f;     // Mean FLIP error (0 = identical, 1 = maximally different)
  float m_fMaxError = 0.0f;      // Maximum per-pixel FLIP error
  float m_fMedianError = 0.0f;   // Median FLIP error
  float m_f95thPercentile = 0.0f; // 95th percentile error
  uint32_t m_uiPixelsAboveThreshold = 0; // Number of pixels exceeding threshold
  uint32_t m_uiTotalPixels = 0;
  bool m_bPassed = false;         // True if comparison is within tolerance

  /// Percentage of pixels that differ beyond threshold.
  float GetFailurePercentage() const
  {
    return m_uiTotalPixels > 0 ? (static_cast<float>(m_uiPixelsAboveThreshold) / m_uiTotalPixels) * 100.0f : 0.0f;
  }
};

/// Configuration for image comparison thresholds.
struct nsImageComparisonConfig
{
  float m_fMeanErrorThreshold = 0.05f;       // Max acceptable mean FLIP error
  float m_fPerPixelThreshold = 0.1f;         // Per-pixel FLIP error threshold
  float m_fMaxFailurePercentage = 1.0f;      // Max percentage of pixels that can exceed per-pixel threshold
  float m_fMonitorDistance = 0.7f;            // Viewing distance in meters (FLIP parameter)
  float m_fMonitorWidth = 0.7f;              // Monitor width in meters (FLIP parameter)
  float m_fMonitorResolutionX = 1920.0f;     // Monitor horizontal resolution (FLIP parameter)
};

/// FLIP-based image comparison engine.
class nsImageComparator
{
public:
  nsImageComparator();
  ~nsImageComparator();

  /// Set comparison configuration.
  void SetConfig(const nsImageComparisonConfig& config) { m_Config = config; }
  const nsImageComparisonConfig& GetConfig() const { return m_Config; }

  /// Compare two images using FLIP.
  /// Both images must have the same dimensions.
  nsImageComparisonResult Compare(const nsCapturedImage& testImage, const nsCapturedImage& referenceImage) const;

  /// Compare a test image against a reference image file.
  nsImageComparisonResult CompareWithFile(const nsCapturedImage& testImage, const std::string& referenceImagePath) const;

  /// Generate a heatmap visualization of the differences.
  /// The heatmap is a color-coded RGBA8 image where:
  /// - Green = identical
  /// - Yellow = minor difference
  /// - Red = major difference
  bool GenerateHeatmap(const nsCapturedImage& testImage, const nsCapturedImage& referenceImage,
                       nsCapturedImage& outHeatmap) const;

  /// Generate and save a full comparison report: test image, reference image, heatmap, and diff stats.
  bool SaveComparisonReport(const nsCapturedImage& testImage, const nsCapturedImage& referenceImage,
                            const nsImageComparisonResult& result, const std::string& outputDir,
                            const std::string& testName) const;

private:
  nsImageComparisonConfig m_Config;
};
