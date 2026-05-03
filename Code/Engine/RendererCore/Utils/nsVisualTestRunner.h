#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Mannequin Visual Test Pipeline - Test Runner
// Orchestrates automated visual regression testing across graphics APIs.

#include "nsImageCapture.h"
#include "nsImageComparator.h"
#include "../Device/nsGALDevice.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

/// Diagnostic result for a graphics API call or call sequence.
struct nsApiCallCheck
{
  std::string m_sCallName;
  std::string m_sCategory;
  std::string m_sSeverity = "Info";
  bool m_bPassed = true;
  std::string m_sMessage;
  std::string m_sRecommendation;
};

/// Assertion against internal object/resource state.
struct nsResourceStateCheck
{
  std::string m_sName;
  std::string m_sSeverity = "Info";
  bool m_bPassed = true;
  std::string m_sActual;
  std::string m_sExpected;
  std::string m_sMessage;
  std::string m_sRecommendation;
};

/// Snapshot of an RHI resource, shader resource, buffer, matrix, or other renderer state object.
struct nsResourceSnapshot
{
  std::string m_sName;
  std::string m_sType;
  std::string m_sSlot;
  std::string m_sFormat;
  std::string m_sState;
  std::string m_sSummary;
  std::string m_sPreviewPath;
  uint32_t m_uiWidth = 0;
  uint32_t m_uiHeight = 0;
  uint32_t m_uiDepth = 0;
  uint32_t m_uiMipLevels = 0;
  uint32_t m_uiElementCount = 0;
  uint32_t m_uiRowPitch = 0;
  uint64_t m_uiByteSize = 0;
  std::vector<std::string> m_Values;
  std::vector<nsResourceStateCheck> m_StateChecks;
};

/// Result for a single visual test case.
struct nsVisualTestResult
{
  std::string m_sTestName;
  std::string m_sAPIName;
  nsImageComparisonResult m_ComparisonResult;
  bool m_bRenderSucceeded = false;
  bool m_bReferenceExists = false;
  double m_fRenderTimeMs = 0.0;
  std::string m_sErrorMessage;
  std::vector<nsApiCallCheck> m_ApiCallChecks;
  std::vector<nsValidationMessage> m_ValidationMessages;
  std::vector<nsResourceSnapshot> m_ResourceSnapshots;
};

/// Summary for a full test suite run.
struct nsVisualTestSummary
{
  std::vector<nsVisualTestResult> m_Results;
  uint32_t m_uiTotalTests = 0;
  uint32_t m_uiPassed = 0;
  uint32_t m_uiFailed = 0;
  uint32_t m_uiSkipped = 0;
  uint32_t m_uiNewBaselines = 0; // Tests where no reference existed and a new baseline was saved
  double m_fTotalTimeMs = 0.0;

  bool AllPassed() const { return m_uiFailed == 0; }
};

/// Callback invoked for each test. Implementations render their test scene
/// and return the rendered image for comparison.
using nsVisualTestCallback = std::function<bool(nsGALDevice* pDevice,
                                                nsCapturedImage& outImage,
                                                std::vector<nsApiCallCheck>& outApiCallChecks,
                                                std::vector<nsResourceSnapshot>& outResourceSnapshots,
                                                std::vector<nsValidationMessage>& outValidationMessages)>;

/// A single visual test case.
struct nsVisualTestCase
{
  std::string m_sName;
  std::string m_sCategory;
  nsVisualTestCallback m_RenderCallback;
  nsImageComparisonConfig m_ComparisonConfig; // Per-test override
  bool m_bEnabled = true;
};

/// The main visual test runner — core of Mannequin.
/// Runs test scenes across specified graphics APIs, captures results,
/// compares against reference images, and generates reports.
class nsVisualTestRunner
{
public:
  nsVisualTestRunner();
  ~nsVisualTestRunner();

  /// Set the directory containing reference images.
  void SetReferenceImageDir(const std::string& dir) { m_sReferenceImageDir = dir; }

  /// Set the directory where test outputs (captures, diffs, reports) are saved.
  void SetOutputDir(const std::string& dir) { m_sOutputDir = dir; }

  /// Set the default comparison configuration.
  void SetDefaultConfig(const nsImageComparisonConfig& config) { m_DefaultConfig = config; }

  /// Register a visual test case.
  void RegisterTest(const nsVisualTestCase& testCase);

  /// Run all registered tests against a single API.
  nsVisualTestSummary RunTests(nsGALGraphicsAPI api, const nsGALDeviceCreationDescription& deviceDesc = {});

  /// Run all registered tests against all available APIs.
  std::vector<nsVisualTestSummary> RunTestsAllAPIs(const nsGALDeviceCreationDescription& deviceDesc = {});

  /// Update reference images from current test results.
  /// If tests are specified, only update those; otherwise update all.
  void UpdateBaselines(const std::vector<std::string>& testNames = {});

  /// Get registered test count.
  size_t GetTestCount() const { return m_Tests.size(); }

  /// Export results as JSON for Arbitor GUI consumption.
  bool ExportResultsJSON(const nsVisualTestSummary& summary, const std::string& outputPath) const;

  /// Export results as JUnit XML for CI integration.
  bool ExportResultsJUnit(const nsVisualTestSummary& summary, const std::string& outputPath) const;

private:
  nsVisualTestResult RunSingleTest(nsGALDevice* pDevice, const nsVisualTestCase& testCase);

  std::vector<nsVisualTestCase> m_Tests;
  nsImageComparisonConfig m_DefaultConfig;
  nsImageComparator m_Comparator;
  std::string m_sReferenceImageDir = "Data/UnitTests/RendererTest/ReferenceImages";
  std::string m_sOutputDir = "TestOutput";
};
