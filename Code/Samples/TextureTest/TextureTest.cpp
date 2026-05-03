// Copyright (c) WD Studios. All rights reserved.
// Mannequin Sample: Texture Test
// Exercises texture previews, sampler/resource-state checks, and warning severity.

#include "../MannequinSampleCommon.h"

#include <chrono>
#include <cstdio>

using namespace MannequinSample;

namespace
{
  nsCapturedImage CreateCheckerboardTexture()
  {
    return CreateImage(256, 256, [](uint32_t x, uint32_t y, uint8_t* pixel) {
      const bool high = ((x / 32) + (y / 32)) % 2 == 0;
      pixel[0] = high ? 230 : 28;
      pixel[1] = high ? 230 : 38;
      pixel[2] = high ? 235 : 52;
      pixel[3] = 255;
    });
  }

  uint32_t HashNoise(uint32_t x, uint32_t y)
  {
    uint32_t value = x * 1973u + y * 9277u + 1337u;
    value = (value << 13u) ^ value;
    return (value * (value * value * 15731u + 789221u) + 1376312589u) & 0x7fffffffu;
  }

  nsCapturedImage CreateNoiseTexture()
  {
    return CreateImage(256, 256, [](uint32_t x, uint32_t y, uint8_t* pixel) {
      const uint8_t value = static_cast<uint8_t>((HashNoise(x, y) >> 8u) & 0xFFu);
      pixel[0] = value;
      pixel[1] = static_cast<uint8_t>((value / 2u) + 64u);
      pixel[2] = static_cast<uint8_t>(255u - value);
      pixel[3] = 255;
    });
  }

  nsVisualTestResult RunCheckerboardTest(const std::string& apiName, const std::string& referenceDir, const std::string& outputDir)
  {
    auto startTime = std::chrono::high_resolution_clock::now();

    nsVisualTestResult result;
    result.m_sTestName = "CheckerboardTexture";

    nsCapturedImage image = CreateCheckerboardTexture();
    SaveComparison(result, image, image, referenceDir, outputDir, apiName);

    AddApiCallCheck(result.m_ApiCallChecks,
                    "Resource",
                    "CreateTexture2D",
                    true,
                    "Fixture texture data was generated as a deterministic RGBA8 checkerboard.",
                    "Use this sample to verify texture preview, row pitch, and SRV binding diagnostics.");
    AddApiCallCheck(result.m_ApiCallChecks,
                    "Shader Resource",
                    "BindSRV(t0)",
                    false,
                    "The fixture path describes the expected SRV binding but does not use a live backend.",
                    "When a backend path is available, capture the actual SRV descriptor and compare it against t0.",
                    "Warning");

    auto snapshot = CreateTextureSnapshot(image,
                                          "CheckerboardAlbedo",
                                          "t0",
                                          "ShaderResource",
                                          apiName + "/CheckerboardTexture/CheckerboardTexture_test.png",
                                          "Deterministic checkerboard texture intended to mimic an albedo SRV.");
    snapshot.m_Values.push_back("WrapMode = Repeat");
    snapshot.m_Values.push_back("Filter = Point");
    snapshot.m_Values.push_back("ExpectedBinding = PixelShader.t0");
    AddStateCheck(snapshot.m_StateChecks,
                  "Sampler filter",
                  true,
                  "Point",
                  "Point",
                  "Sampler filter is stable for texel-exact inspection.",
                  "Use linear filtering in a separate sample if testing sampling behavior.");
    result.m_ResourceSnapshots.push_back(std::move(snapshot));

    AddValidationMessage(result.m_ValidationMessages,
                         apiName + " Validation",
                         "Warning",
                         "TextureTest is running in fixture mode; no native SRV descriptor was queried.",
                         "Live D3D11/D3D12/Vulkan backends should emit validation messages from their debug layers.");

    auto endTime = std::chrono::high_resolution_clock::now();
    result.m_fRenderTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
  }

  nsVisualTestResult RunNoiseTest(const std::string& apiName, const std::string& referenceDir, const std::string& outputDir)
  {
    auto startTime = std::chrono::high_resolution_clock::now();

    nsVisualTestResult result;
    result.m_sTestName = "NoiseTextureResource";

    nsCapturedImage image = CreateNoiseTexture();
    SaveComparison(result, image, image, referenceDir, outputDir, apiName);

    AddApiCallCheck(result.m_ApiCallChecks,
                    "Resource",
                    "GenerateNoiseTexture",
                    true,
                    "Generated a deterministic noise texture for shader-resource preview testing.",
                    "Use the Resource Inspector preview to verify the texture content before debugging a shader that samples it.");

    auto snapshot = CreateTextureSnapshot(image,
                                          "BlueNoiseLUT",
                                          "t4",
                                          "ShaderResource",
                                          apiName + "/NoiseTextureResource/NoiseTextureResource_test.png",
                                          "Noise lookup texture that represents the kind of resource RenderDoc users inspect when debugging shaders.");
    snapshot.m_Values.push_back("Seed = 1337");
    snapshot.m_Values.push_back("IntendedConsumer = TemporalResolve.ps");
    snapshot.m_Values.push_back("ExpectedBinding = PixelShader.t4");
    AddStateCheck(snapshot.m_StateChecks,
                  "Deterministic seed",
                  true,
                  "1337",
                  "1337",
                  "Noise generation uses the expected deterministic seed.",
                  "Keep the seed in the test data so noisy shader output remains reproducible.");
    AddStateCheck(snapshot.m_StateChecks,
                  "Mip chain",
                  false,
                  "1 mip",
                  "full mip chain",
                  "The fixture only creates a top-level preview image.",
                  "A live backend test should capture all mips when validating sampled texture chains.",
                  "Warning");
    result.m_ResourceSnapshots.push_back(std::move(snapshot));

    auto endTime = std::chrono::high_resolution_clock::now();
    result.m_fRenderTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return result;
  }
}

int main(int argc, char** argv)
{
  printf("Mannequin Sample: TextureTest\n");
  printf("=============================\n\n");

  const std::string referenceDir = GetArgValue(argc, argv, {"-referenceDir", "-reference-dir", "--reference-dir"}, "Data/Samples/TextureTest/Reference");
  const std::string outputDir = GetArgValue(argc, argv, {"-outputDir", "-output-dir", "--output-dir"}, "TestOutput/Samples/TextureTest");
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

  if (filter.empty() || ContainsIgnoreCase("CheckerboardTexture", filter))
    PushResult(summary, RunCheckerboardTest(apiName, referenceDir, outputDir));

  if (filter.empty() || ContainsIgnoreCase("NoiseTextureResource", filter))
    PushResult(summary, RunNoiseTest(apiName, referenceDir, outputDir));

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
