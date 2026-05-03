// Copyright (c) WD Studios. All rights reserved.
// Mannequin Sample: Visual Regression Demo
// Provides passing, failing, and fatal-validation examples for the Mannequin UI.

#include "../MannequinSampleCommon.h"

#include <chrono>
#include <cstdio>
#include <string>

using namespace MannequinSample;

namespace
{
  nsCapturedImage CreateGradientReference()
  {
    return CreateImage(384, 256, [](uint32_t x, uint32_t y, uint8_t* pixel) {
      const float fx = static_cast<float>(x) / 383.0f;
      const float fy = static_cast<float>(y) / 255.0f;
      pixel[0] = static_cast<uint8_t>(30.0f + 120.0f * fx);
      pixel[1] = static_cast<uint8_t>(44.0f + 140.0f * fy);
      pixel[2] = static_cast<uint8_t>(76.0f + 110.0f * (1.0f - fx));
      pixel[3] = 255;
    });
  }

  nsCapturedImage CreateDriftedGradient()
  {
    nsCapturedImage image = CreateGradientReference();

    for (uint32_t y = 86; y < 164; ++y)
    {
      for (uint32_t x = 145; x < 238; ++x)
      {
        uint8_t* pixel = image.m_Data.data() + y * image.m_uiRowPitch + x * 4;
        pixel[0] = 245;
        pixel[1] = static_cast<uint8_t>(pixel[1] / 3);
        pixel[2] = 42;
      }
    }

    return image;
  }

  nsCapturedImage CreateMatrixOutputImage()
  {
    return CreateImage(384, 256, [](uint32_t x, uint32_t y, uint8_t* pixel) {
      const bool grid = (x % 32 == 0) || (y % 32 == 0);
      pixel[0] = grid ? 90 : 20;
      pixel[1] = grid ? 138 : 28;
      pixel[2] = grid ? 190 : 38;
      pixel[3] = 255;

      if (x > 120 && x < 264 && y > 56 && y < 200)
      {
        pixel[0] = static_cast<uint8_t>(80 + (x - 120));
        pixel[1] = static_cast<uint8_t>(220 - (y - 56));
        pixel[2] = 160;
      }
    });
  }

  nsResourceSnapshot CreateCameraMatrixSnapshot(bool valid)
  {
    nsResourceSnapshot snapshot;
    snapshot.m_sName = "CameraConstants";
    snapshot.m_sType = "ConstantBuffer";
    snapshot.m_sSlot = "b0";
    snapshot.m_sFormat = "float4x4 viewProjection";
    snapshot.m_sState = valid ? "Bound" : "Invalid";
    snapshot.m_sSummary = "Camera matrix values captured for shader-output debugging.";
    snapshot.m_uiElementCount = 16;
    snapshot.m_uiByteSize = 64;
    snapshot.m_Values.push_back(valid ? "ViewProjection[0] = float4(1.000, 0.000, 0.000, 0.000)" : "ViewProjection[0] = float4(NaN, 0.000, 0.000, 0.000)");
    snapshot.m_Values.push_back("ViewProjection[1] = float4(0.000, 1.000, 0.000, 0.000)");
    snapshot.m_Values.push_back("ViewProjection[2] = float4(0.000, 0.000, 1.000, 0.000)");
    snapshot.m_Values.push_back("ViewProjection[3] = float4(0.000, 0.000, 0.000, 1.000)");
    AddStateCheck(snapshot.m_StateChecks,
                  "Finite matrix",
                  valid,
                  valid ? "finite" : "NaN at m00",
                  "finite matrix",
                  valid ? "Camera matrix contains finite values." : "Camera matrix contains a NaN and should fail shader-output validation.",
                  "Track constant-buffer writes before draw submission; a NaN matrix usually indicates an uninitialized camera transform.",
                  "Fatal");
    AddStateCheck(snapshot.m_StateChecks,
                  "Binding slot",
                  true,
                  "b0",
                  "b0",
                  "Camera constants are assigned to the expected shader slot.",
                  "Keep shader reflection metadata aligned with the test expectation.");
    return snapshot;
  }

  nsVisualTestResult RunGoldenGradient(const std::string& apiName, const std::string& referenceDir, const std::string& outputDir)
  {
    auto startTime = std::chrono::high_resolution_clock::now();

    nsVisualTestResult result;
    result.m_sTestName = "GoldenGradient";
    nsCapturedImage image = CreateGradientReference();
    SaveComparison(result, image, image, referenceDir, outputDir, apiName);

    AddApiCallCheck(result.m_ApiCallChecks,
                    "Comparison",
                    "GoldenImage",
                    true,
                    "Rendered output matches the seeded golden reference.",
                    "Use this as the clean-pass control when checking the comparison view modes.");
    result.m_ResourceSnapshots.push_back(CreateTextureSnapshot(image,
                                                               "GoldenGradientOutput",
                                                               "RenderTarget[0]",
                                                               "Compared",
                                                               apiName + "/GoldenGradient/GoldenGradient_test.png",
                                                               "Passing render target used as a baseline sanity check."));
    result.m_ResourceSnapshots.push_back(CreateCameraMatrixSnapshot(true));

    auto endTime = std::chrono::high_resolution_clock::now();
    result.m_fRenderTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
  }

  nsVisualTestResult RunIntentionalDrift(const std::string& apiName, const std::string& referenceDir, const std::string& outputDir)
  {
    auto startTime = std::chrono::high_resolution_clock::now();

    nsVisualTestResult result;
    result.m_sTestName = "IntentionalPixelDrift";
    nsCapturedImage reference = CreateGradientReference();
    nsCapturedImage test = CreateDriftedGradient();
    SaveComparison(result, test, reference, referenceDir, outputDir, apiName);

    AddApiCallCheck(result.m_ApiCallChecks,
                    "Comparison",
                    "PixelDrift",
                    false,
                    "A deliberate block of pixels differs from the reference image.",
                    "Select Error Pixels in Mannequin to inspect the red overlay and verify failure localization.",
                    "Error");
    AddValidationMessage(result.m_ValidationMessages,
                         apiName + " Validation",
                         "Warning",
                         "Demo warning: a viewport/scissor mismatch was simulated for diagnostics display.",
                         "Warnings should remain warnings; use them to guide investigation without immediately marking the RHI fatal.");

    result.m_ResourceSnapshots.push_back(CreateTextureSnapshot(test,
                                                               "DriftedRenderTarget",
                                                               "RenderTarget[0]",
                                                               "Compared",
                                                               apiName + "/IntentionalPixelDrift/IntentionalPixelDrift_test.png",
                                                               "Failing render target with a deliberate pixel drift region."));
    result.m_ResourceSnapshots.push_back(CreateCameraMatrixSnapshot(true));

    auto endTime = std::chrono::high_resolution_clock::now();
    result.m_fRenderTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
  }

  nsVisualTestResult RunFatalState(const std::string& apiName, const std::string& referenceDir, const std::string& outputDir)
  {
    auto startTime = std::chrono::high_resolution_clock::now();

    nsVisualTestResult result;
    result.m_sTestName = "FatalMatrixState";
    nsCapturedImage image = CreateMatrixOutputImage();
    SaveComparison(result, image, image, referenceDir, outputDir, apiName);

    AddApiCallCheck(result.m_ApiCallChecks,
                    "State",
                    "ConstantBufferValidation",
                    false,
                    "The captured camera matrix intentionally contains invalid data.",
                    "Fatal resource-state checks should fail the test even if the image comparison is clean.",
                    "Fatal");
    AddValidationMessage(result.m_ValidationMessages,
                         apiName + " Validation",
                         "Error",
                         "Demo error: shader constant buffer contains a NaN before draw submission.",
                         "Errors from D3D11/D3D12/Vulkan validation should be treated as fatal.");

    result.m_ResourceSnapshots.push_back(CreateTextureSnapshot(image,
                                                               "MatrixDebugOutput",
                                                               "RenderTarget[0]",
                                                               "Compared",
                                                               apiName + "/FatalMatrixState/FatalMatrixState_test.png",
                                                               "Clean-looking output paired with a fatal internal matrix-state failure."));
    result.m_ResourceSnapshots.push_back(CreateCameraMatrixSnapshot(false));

    auto endTime = std::chrono::high_resolution_clock::now();
    result.m_fRenderTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
  }
}

int main(int argc, char** argv)
{
  printf("Mannequin Sample: VisualRegressionDemo\n");
  printf("======================================\n\n");

  const std::string referenceDir = GetArgValue(argc, argv, {"-referenceDir", "-reference-dir", "--reference-dir"}, "Data/Samples/VisualRegressionDemo/Reference");
  const std::string outputDir = GetArgValue(argc, argv, {"-outputDir", "-output-dir", "--output-dir"}, "TestOutput/Samples/VisualRegressionDemo");
  const std::string jsonPath = GetArgValue(argc, argv, {"-json", "--json", "-json-path", "--json-path"}, outputDir + "/results.json");
  const std::string filter = GetArgValue(argc, argv, {"-filter", "--filter"}, "");
  const std::string rendererName = GetArgValue(argc, argv, {"-renderer", "--renderer", "-api", "--api"}, "DX12");
  const std::string apiName = GetGraphicsAPIName(ParseGraphicsAPI(rendererName));

  printf("API:       %s\n", apiName.c_str());
  printf("Reference: %s\n", referenceDir.c_str());
  printf("Output:    %s\n", outputDir.c_str());
  printf("JSON:      %s\n", jsonPath.c_str());
  if (!filter.empty())
    printf("Filter:    %s\n", filter.c_str());

  nsVisualTestSummary summary;

  if (filter.empty() || ContainsIgnoreCase("GoldenGradient", filter))
    PushResult(summary, RunGoldenGradient(apiName, referenceDir, outputDir));

  if (filter.empty() || ContainsIgnoreCase("IntentionalPixelDrift", filter))
    PushResult(summary, RunIntentionalDrift(apiName, referenceDir, outputDir));

  if (filter.empty() || ContainsIgnoreCase("FatalMatrixState", filter))
    PushResult(summary, RunFatalState(apiName, referenceDir, outputDir));

  nsVisualTestRunner runner;
  CreateParentDirectory(jsonPath);
  if (!runner.ExportResultsJSON(summary, jsonPath))
  {
    printf("Failed to write JSON results: %s\n", jsonPath.c_str());
    return 2;
  }

  printf("\nResults: %u total, %u passed, %u failed\n",
         summary.m_uiTotalTests, summary.m_uiPassed, summary.m_uiFailed);

  return summary.AllPassed() ? 0 : 1;
}
