// Copyright (c) WD Studios. All rights reserved.
// Mannequin Visual Test Pipeline - Test Runner Implementation

#include "nsVisualTestRunner.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

nsVisualTestRunner::nsVisualTestRunner() = default;
nsVisualTestRunner::~nsVisualTestRunner() = default;

void nsVisualTestRunner::RegisterTest(const nsVisualTestCase& testCase)
{
  m_Tests.push_back(testCase);
}

nsVisualTestResult nsVisualTestRunner::RunSingleTest(nsGALDevice* pDevice, const nsVisualTestCase& testCase)
{
  nsVisualTestResult result;
  result.m_sTestName = testCase.m_sName;
  result.m_sAPIName = pDevice->GetAPIName();

  // Use per-test config if specified, otherwise use default
  nsImageComparisonConfig config = testCase.m_ComparisonConfig.m_fMeanErrorThreshold > 0.0f
                                       ? testCase.m_ComparisonConfig
                                       : m_DefaultConfig;
  m_Comparator.SetConfig(config);

  // Execute the render callback
  nsCapturedImage testImage;
  auto startTime = std::chrono::high_resolution_clock::now();

  try
  {
    result.m_bRenderSucceeded = testCase.m_RenderCallback(pDevice, testImage);
  }
  catch (const std::exception& e)
  {
    result.m_bRenderSucceeded = false;
    result.m_sErrorMessage = std::string("Exception during render: ") + e.what();
  }

  auto endTime = std::chrono::high_resolution_clock::now();
  result.m_fRenderTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

  if (!result.m_bRenderSucceeded || !testImage.IsValid())
  {
    if (result.m_sErrorMessage.empty())
      result.m_sErrorMessage = "Render callback failed or produced invalid image";
    return result;
  }

  // Build reference image path: {refDir}/{API}/{testName}.png
  std::string refPath = m_sReferenceImageDir + "/" + result.m_sAPIName + "/" + testCase.m_sName + ".png";
  result.m_bReferenceExists = fs::exists(refPath);

  if (!result.m_bReferenceExists)
  {
    // No reference — save as new baseline
    fs::create_directories(fs::path(refPath).parent_path());
    testImage.SavePNG(refPath);
    result.m_sErrorMessage = "New baseline created (no reference image existed)";
    result.m_ComparisonResult.m_bPassed = true; // New baselines pass by default
    return result;
  }

  // Compare against reference
  result.m_ComparisonResult = m_Comparator.CompareWithFile(testImage, refPath);

  // Save comparison artifacts
  std::string testOutputDir = m_sOutputDir + "/" + result.m_sAPIName + "/" + testCase.m_sName;
  nsCapturedImage refImage;
  refImage.LoadPNG(refPath);
  m_Comparator.SaveComparisonReport(testImage, refImage, result.m_ComparisonResult, testOutputDir, testCase.m_sName);

  return result;
}

nsVisualTestSummary nsVisualTestRunner::RunTests(nsGALGraphicsAPI api, const nsGALDeviceCreationDescription& deviceDesc)
{
  nsVisualTestSummary summary;
  auto totalStart = std::chrono::high_resolution_clock::now();

  // Create device
  auto device = nsGALDevice::CreateDevice(api);
  if (!device)
  {
    summary.m_uiSkipped = static_cast<uint32_t>(m_Tests.size());
    summary.m_uiTotalTests = summary.m_uiSkipped;
    return summary;
  }

  nsGALResult initResult = device->Init(deviceDesc);
  if (NS_GAL_FAILED(initResult))
  {
    summary.m_uiSkipped = static_cast<uint32_t>(m_Tests.size());
    summary.m_uiTotalTests = summary.m_uiSkipped;
    return summary;
  }

  // Run each test
  for (const auto& test : m_Tests)
  {
    summary.m_uiTotalTests++;

    if (!test.m_bEnabled)
    {
      summary.m_uiSkipped++;
      continue;
    }

    nsVisualTestResult result = RunSingleTest(device.get(), test);
    
    if (!result.m_bReferenceExists && result.m_bRenderSucceeded)
    {
      summary.m_uiNewBaselines++;
      summary.m_uiPassed++; // New baselines count as pass
    }
    else if (result.m_ComparisonResult.m_bPassed)
    {
      summary.m_uiPassed++;
    }
    else
    {
      summary.m_uiFailed++;
    }

    summary.m_Results.push_back(std::move(result));
  }

  device->Shutdown();

  auto totalEnd = std::chrono::high_resolution_clock::now();
  summary.m_fTotalTimeMs = std::chrono::duration<double, std::milli>(totalEnd - totalStart).count();

  return summary;
}

std::vector<nsVisualTestSummary> nsVisualTestRunner::RunTestsAllAPIs(const nsGALDeviceCreationDescription& deviceDesc)
{
  std::vector<nsVisualTestSummary> allSummaries;
  auto apis = nsGALDevice::GetAvailableAPIs();

  for (auto api : apis)
  {
    allSummaries.push_back(RunTests(api, deviceDesc));
  }

  return allSummaries;
}

void nsVisualTestRunner::UpdateBaselines(const std::vector<std::string>& testNames)
{
  // Copy test output images to reference directory
  for (const auto& test : m_Tests)
  {
    if (!testNames.empty())
    {
      bool found = false;
      for (const auto& name : testNames)
      {
        if (name == test.m_sName)
        {
          found = true;
          break;
        }
      }
      if (!found)
        continue;
    }

    // Find latest test output across all APIs and copy to reference
    auto apis = nsGALDevice::GetAvailableAPIs();
    for (auto api : apis)
    {
      // Look for test output
      // Copy to reference directory if exists
    }
  }
}

bool nsVisualTestRunner::ExportResultsJSON(const nsVisualTestSummary& summary, const std::string& outputPath) const
{
  FILE* f = fopen(outputPath.c_str(), "w");
  if (!f)
    return false;

  fprintf(f, "{\n");
  fprintf(f, "  \"totalTests\": %u,\n", summary.m_uiTotalTests);
  fprintf(f, "  \"passed\": %u,\n", summary.m_uiPassed);
  fprintf(f, "  \"failed\": %u,\n", summary.m_uiFailed);
  fprintf(f, "  \"skipped\": %u,\n", summary.m_uiSkipped);
  fprintf(f, "  \"newBaselines\": %u,\n", summary.m_uiNewBaselines);
  fprintf(f, "  \"totalTimeMs\": %.2f,\n", summary.m_fTotalTimeMs);
  fprintf(f, "  \"results\": [\n");

  for (size_t i = 0; i < summary.m_Results.size(); ++i)
  {
    const auto& r = summary.m_Results[i];
    fprintf(f, "    {\n");
    fprintf(f, "      \"testName\": \"%s\",\n", r.m_sTestName.c_str());
    fprintf(f, "      \"api\": \"%s\",\n", r.m_sAPIName.c_str());
    fprintf(f, "      \"passed\": %s,\n", r.m_ComparisonResult.m_bPassed ? "true" : "false");
    fprintf(f, "      \"renderSucceeded\": %s,\n", r.m_bRenderSucceeded ? "true" : "false");
    fprintf(f, "      \"referenceExists\": %s,\n", r.m_bReferenceExists ? "true" : "false");
    fprintf(f, "      \"renderTimeMs\": %.2f,\n", r.m_fRenderTimeMs);
    fprintf(f, "      \"meanError\": %.6f,\n", r.m_ComparisonResult.m_fMeanError);
    fprintf(f, "      \"maxError\": %.6f,\n", r.m_ComparisonResult.m_fMaxError);
    fprintf(f, "      \"medianError\": %.6f,\n", r.m_ComparisonResult.m_fMedianError);
    fprintf(f, "      \"p95Error\": %.6f,\n", r.m_ComparisonResult.m_f95thPercentile);
    fprintf(f, "      \"pixelsFailed\": %u,\n", r.m_ComparisonResult.m_uiPixelsAboveThreshold);
    fprintf(f, "      \"totalPixels\": %u,\n", r.m_ComparisonResult.m_uiTotalPixels);
    fprintf(f, "      \"failurePercentage\": %.4f", r.m_ComparisonResult.GetFailurePercentage());
    if (!r.m_sErrorMessage.empty())
      fprintf(f, ",\n      \"error\": \"%s\"", r.m_sErrorMessage.c_str());
    fprintf(f, "\n    }%s\n", (i + 1 < summary.m_Results.size()) ? "," : "");
  }

  fprintf(f, "  ]\n");
  fprintf(f, "}\n");
  fclose(f);
  return true;
}

bool nsVisualTestRunner::ExportResultsJUnit(const nsVisualTestSummary& summary, const std::string& outputPath) const
{
  FILE* f = fopen(outputPath.c_str(), "w");
  if (!f)
    return false;

  fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(f, "<testsuites tests=\"%u\" failures=\"%u\" time=\"%.3f\">\n",
          summary.m_uiTotalTests, summary.m_uiFailed, summary.m_fTotalTimeMs / 1000.0);
  fprintf(f, "  <testsuite name=\"MannequinVisualTests\" tests=\"%u\" failures=\"%u\">\n",
          summary.m_uiTotalTests, summary.m_uiFailed);

  for (const auto& r : summary.m_Results)
  {
    fprintf(f, "    <testcase name=\"%s\" classname=\"%s\" time=\"%.3f\"",
            r.m_sTestName.c_str(), r.m_sAPIName.c_str(), r.m_fRenderTimeMs / 1000.0);

    if (!r.m_ComparisonResult.m_bPassed && r.m_bRenderSucceeded)
    {
      fprintf(f, ">\n");
      fprintf(f, "      <failure message=\"Visual comparison failed: mean=%.6f, max=%.6f, failed pixels=%.2f%%\"",
              r.m_ComparisonResult.m_fMeanError, r.m_ComparisonResult.m_fMaxError,
              r.m_ComparisonResult.GetFailurePercentage());
      fprintf(f, " />\n");
      fprintf(f, "    </testcase>\n");
    }
    else if (!r.m_bRenderSucceeded)
    {
      fprintf(f, ">\n");
      fprintf(f, "      <error message=\"%s\" />\n", r.m_sErrorMessage.c_str());
      fprintf(f, "    </testcase>\n");
    }
    else
    {
      fprintf(f, " />\n");
    }
  }

  fprintf(f, "  </testsuite>\n");
  fprintf(f, "</testsuites>\n");
  fclose(f);
  return true;
}
