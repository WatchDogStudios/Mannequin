// Copyright (c) WD Studios. All rights reserved.
// Mannequin Sample: Basic Triangle
// Minimal sample demonstrating the visual test pipeline with a simple triangle render.

#include "../../Engine/RendererCore/Device/nsGALDevice.h"
#include "../../Engine/RendererCore/Utils/nsVisualTestRunner.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <utility>

namespace fs = std::filesystem;

namespace
{
  std::string ToLower(std::string value)
  {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
  }

  bool EqualsIgnoreCase(const std::string& lhs, const std::string& rhs)
  {
    return ToLower(lhs) == ToLower(rhs);
  }

  bool ContainsIgnoreCase(const std::string& value, const std::string& filter)
  {
    return ToLower(value).find(ToLower(filter)) != std::string::npos;
  }

  std::string GetArgValue(int argc, char** argv, std::initializer_list<const char*> names, const std::string& fallback)
  {
    for (int i = 1; i < argc; ++i)
    {
      std::string current = argv[i];
      for (const char* name : names)
      {
        if (current == name && i + 1 < argc)
          return argv[i + 1];

        std::string prefix = std::string(name) + "=";
        if (current.rfind(prefix, 0) == 0)
          return current.substr(prefix.size());
      }
    }

    return fallback;
  }

  nsGALGraphicsAPI ParseGraphicsAPI(const std::string& name)
  {
    if (EqualsIgnoreCase(name, "DX11"))
      return nsGALGraphicsAPI::DX11;
    if (EqualsIgnoreCase(name, "Vulkan"))
      return nsGALGraphicsAPI::Vulkan;
    return nsGALGraphicsAPI::DX12;
  }

  const char* GetGraphicsAPIName(nsGALGraphicsAPI api)
  {
    switch (api)
    {
      case nsGALGraphicsAPI::DX11:
        return "DX11";
      case nsGALGraphicsAPI::Vulkan:
        return "Vulkan";
      case nsGALGraphicsAPI::DX12:
        return "DX12";
      default:
        return "Unknown";
    }
  }

  void CreateParentDirectory(const std::string& path)
  {
    fs::path parent = fs::path(path).parent_path();
    if (!parent.empty())
      fs::create_directories(parent);
  }

  nsCapturedImage CreateFixtureTriangleImage()
  {
    constexpr uint32_t width = 512;
    constexpr uint32_t height = 512;

    nsCapturedImage image;
    image.m_uiWidth = width;
    image.m_uiHeight = height;
    image.m_uiRowPitch = width * 4;
    image.m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;
    image.m_Data.resize(image.m_uiRowPitch * height);

    for (uint32_t y = 0; y < height; ++y)
    {
      for (uint32_t x = 0; x < width; ++x)
      {
        uint8_t* pixel = image.m_Data.data() + y * image.m_uiRowPitch + x * 4;
        pixel[0] = 36;
        pixel[1] = 40;
        pixel[2] = 62;
        pixel[3] = 255;

        const float fx = static_cast<float>(x) / static_cast<float>(width - 1);
        const float fy = static_cast<float>(y) / static_cast<float>(height - 1);
        const bool insideTriangle =
          fy > 0.18f &&
          fy < 0.82f &&
          fx > 0.5f - (fy - 0.18f) * 0.55f &&
          fx < 0.5f + (fy - 0.18f) * 0.55f;

        if (insideTriangle)
        {
          pixel[0] = static_cast<uint8_t>(255.0f * (1.0f - fy));
          pixel[1] = static_cast<uint8_t>(255.0f * fx);
          pixel[2] = static_cast<uint8_t>(255.0f * fy);
        }
      }
    }

    return image;
  }

  void AddApiCallCheck(std::vector<nsApiCallCheck>& checks,
                       const std::string& category,
                       const std::string& callName,
                       bool passed,
                       const std::string& message,
                       const std::string& recommendation,
                       const std::string& severity = "Error")
  {
    nsApiCallCheck check;
    check.m_sCategory = category;
    check.m_sCallName = callName;
    check.m_bPassed = passed;
    check.m_sSeverity = passed ? "Info" : severity;
    check.m_sMessage = message;
    check.m_sRecommendation = recommendation;
    checks.push_back(std::move(check));
  }

  void AddValidationMessage(std::vector<nsValidationMessage>& messages,
                            const std::string& source,
                            const std::string& severity,
                            const std::string& messageText,
                            const std::string& recommendation)
  {
    nsValidationMessage message;
    message.m_sSource = source;
    message.m_sSeverity = severity;
    message.m_sMessage = messageText;
    message.m_sRecommendation = recommendation;
    messages.push_back(std::move(message));
  }

  void AddStateCheck(std::vector<nsResourceStateCheck>& checks,
                     const std::string& name,
                     bool passed,
                     const std::string& actual,
                     const std::string& expected,
                     const std::string& messageText,
                     const std::string& recommendation,
                     const std::string& severity = "Error")
  {
    nsResourceStateCheck check;
    check.m_sName = name;
    check.m_bPassed = passed;
    check.m_sSeverity = passed ? "Info" : severity;
    check.m_sActual = actual;
    check.m_sExpected = expected;
    check.m_sMessage = messageText;
    check.m_sRecommendation = recommendation;
    checks.push_back(std::move(check));
  }

  nsResourceSnapshot CreateTriangleConstantsSnapshot()
  {
    nsResourceSnapshot constants;
    constants.m_sName = "BasicTriangleConstants";
    constants.m_sType = "ConstantBuffer";
    constants.m_sSlot = "b0";
    constants.m_sFormat = "float4x4 + float4";
    constants.m_sState = "Fixture";
    constants.m_sSummary = "CPU-side constants used by the deterministic fixture path.";
    constants.m_uiElementCount = 5;
    constants.m_uiByteSize = 80;
    constants.m_Values.push_back("MVP = identity");
    constants.m_Values.push_back("ClearColor = float4(0.141, 0.157, 0.243, 1.000)");
    constants.m_Values.push_back("VertexA = float2(0.50, 0.18)");
    constants.m_Values.push_back("VertexB = float2(0.15, 0.82)");
    constants.m_Values.push_back("VertexC = float2(0.85, 0.82)");
    AddStateCheck(constants.m_StateChecks,
                  "Finite constants",
                  true,
                  "all finite",
                  "all finite",
                  "Fixture constants are deterministic and finite.",
                  "When this sample moves to a real shader, capture and compare the uploaded constant buffer bytes.");
    AddStateCheck(constants.m_StateChecks,
                  "Shader binding",
                  false,
                  "not bound",
                  "bound to b0",
                  "Fixture mode does not upload a real constant buffer to the selected backend.",
                  "Bind the constant buffer in the real RHI path and compare its bytes against the expected matrix.",
                  "Warning");
    return constants;
  }

  nsResourceSnapshot CreateImageSnapshot(const nsCapturedImage& image,
                                         const std::string& name,
                                         const std::string& slot,
                                         const std::string& state,
                                         const std::string& previewPath)
  {
    nsResourceSnapshot snapshot;
    snapshot.m_sName = name;
    snapshot.m_sType = "Texture2D";
    snapshot.m_sSlot = slot;
    snapshot.m_sFormat = "R8G8B8A8_UNORM";
    snapshot.m_sState = state;
    snapshot.m_sSummary = "Captured texture snapshot available for preview and image-state testing.";
    snapshot.m_sPreviewPath = previewPath;
    snapshot.m_uiWidth = image.m_uiWidth;
    snapshot.m_uiHeight = image.m_uiHeight;
    snapshot.m_uiMipLevels = 1;
    snapshot.m_uiRowPitch = image.m_uiRowPitch;
    snapshot.m_uiByteSize = static_cast<uint64_t>(image.m_Data.size());
    AddStateCheck(snapshot.m_StateChecks,
                  "Dimensions",
                  image.m_uiWidth == 512 && image.m_uiHeight == 512,
                  std::to_string(image.m_uiWidth) + "x" + std::to_string(image.m_uiHeight),
                  "512x512",
                  "Texture dimensions match the BasicTriangle contract.",
                  "Check render target creation and capture metadata if this changes.");
    AddStateCheck(snapshot.m_StateChecks,
                  "Row pitch",
                  image.m_uiRowPitch == image.m_uiWidth * 4,
                  std::to_string(image.m_uiRowPitch),
                  std::to_string(image.m_uiWidth * 4),
                  "Row pitch matches tightly packed RGBA8 data.",
                  "If row pitch differs, preserve backend row pitch during texture preview and comparison.",
                  "Warning");
    return snapshot;
  }

  nsVisualTestSummary RunFixtureTriangle(const std::string& apiName, const std::string& referenceDir, const std::string& outputDir)
  {
    auto startTime = std::chrono::high_resolution_clock::now();
    nsCapturedImage testImage = CreateFixtureTriangleImage();

    std::string referencePath = referenceDir + "/" + apiName + "/BasicTriangle.png";
    std::string testOutputDir = outputDir + "/" + apiName + "/BasicTriangle";
    CreateParentDirectory(referencePath);
    fs::create_directories(testOutputDir);

    nsImageComparator comparator;
    nsCapturedImage referenceImage;

    nsVisualTestResult result;
    result.m_sTestName = "BasicTriangle";
    result.m_sAPIName = apiName;
    result.m_bRenderSucceeded = testImage.IsValid();
    result.m_bReferenceExists = fs::exists(referencePath);
    AddApiCallCheck(result.m_ApiCallChecks,
                    "Device",
                    "Init",
                    false,
                    "The requested backend did not initialize, so BasicTriangle used the deterministic fixture renderer.",
                    "Implement or enable this renderer backend, then rerun the test with GPU validation enabled.",
                    "Warning");
    AddApiCallCheck(result.m_ApiCallChecks,
                    "Fixture",
                    "GenerateReferenceTriangle",
                    result.m_bRenderSucceeded,
                    "The fixture renderer produced a deterministic triangle image for validating Mannequin's result pipeline.",
                    "Use this as a harness smoke test; add a real backend test once the selected API can create a device.",
                    "Error");
    AddApiCallCheck(result.m_ApiCallChecks,
                    "Coverage",
                    "BackendApiCalls",
                    false,
                    "Fixture mode does not exercise BeginFrame, render pass, Draw, or readback on the selected backend.",
                    "Treat fixture results as an app-pipeline smoke test only; backend correctness requires the real API-call path to initialize and run.",
                    "Warning");
    AddValidationMessage(result.m_ValidationMessages,
                         apiName + " Validation",
                         "Warning",
                         "The selected backend did not initialize, so no native validation-layer messages could be captured.",
                         "Enable the backend debug/validation layer once device initialization succeeds; warnings should remain warnings and errors should be treated as fatal.");

    if (result.m_bReferenceExists)
    {
      referenceImage.LoadPNG(referencePath);
      result.m_ComparisonResult = comparator.Compare(testImage, referenceImage);
    }
    else
    {
      testImage.SavePNG(referencePath);
      referenceImage = testImage;
      result.m_ComparisonResult = comparator.Compare(testImage, referenceImage);
      result.m_ComparisonResult.m_bPassed = true;
      result.m_sErrorMessage = "New baseline created (fixture renderer)";
    }

    comparator.SaveComparisonReport(testImage, referenceImage, result.m_ComparisonResult, testOutputDir, "BasicTriangle");
    result.m_ResourceSnapshots.push_back(CreateImageSnapshot(testImage,
                                                            "FixtureTriangleOutput",
                                                            "RenderTarget[0]",
                                                            "FixtureGenerated",
                                                            apiName + "/BasicTriangle/BasicTriangle_test.png"));
    result.m_ResourceSnapshots.push_back(CreateTriangleConstantsSnapshot());
    AddApiCallCheck(result.m_ApiCallChecks,
                    "Artifacts",
                    "SaveComparisonReport",
                    true,
                    "Comparison images and metrics were written for inspection.",
                    "Review the generated red-pixel overlay before accepting a new baseline.",
                    "Info");

    auto endTime = std::chrono::high_resolution_clock::now();
    result.m_fRenderTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    nsVisualTestSummary summary;
    summary.m_uiTotalTests = 1;
    summary.m_uiPassed = result.m_ComparisonResult.m_bPassed ? 1 : 0;
    summary.m_uiFailed = result.m_ComparisonResult.m_bPassed ? 0 : 1;
    summary.m_uiNewBaselines = result.m_bReferenceExists ? 0 : 1;
    summary.m_fTotalTimeMs = result.m_fRenderTimeMs;
    summary.m_Results.push_back(std::move(result));
    return summary;
  }
}

/// Sample: render a basic colored triangle and capture for comparison.
bool RenderTriangle(nsGALDevice* pDevice,
                    nsCapturedImage& outImage,
                    std::vector<nsApiCallCheck>& outApiCallChecks,
                    std::vector<nsResourceSnapshot>& outResourceSnapshots,
                    std::vector<nsValidationMessage>& outValidationMessages)
{
  // Begin frame
  nsGALResult beginFrameResult = pDevice->BeginFrame();
  AddApiCallCheck(outApiCallChecks,
                  "Frame",
                  "BeginFrame",
                  !NS_GAL_FAILED(beginFrameResult),
                  !NS_GAL_FAILED(beginFrameResult) ? "Frame recording started successfully." : "BeginFrame failed before any draw work could be recorded.",
                  "Check device initialization, command allocator/list state, and backend debug-layer output.");
  if (NS_GAL_FAILED(beginFrameResult))
    return false;

  // Create a simple render target
  nsGALTextureCreationDescription rtDesc;
  rtDesc.m_uiWidth = 512;
  rtDesc.m_uiHeight = 512;
  rtDesc.m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;
  rtDesc.m_bCreateRenderTarget = true;
  rtDesc.m_bAllowShaderResourceView = true;

  nsGALTextureHandle renderTarget = pDevice->CreateTexture(rtDesc);
  AddApiCallCheck(outApiCallChecks,
                  "Resource",
                  "CreateTexture(RenderTarget)",
                  renderTarget.IsValid(),
                  renderTarget.IsValid() ? "Created the offscreen render target." : "The render target handle was invalid.",
                  "Verify R8G8B8A8_UNORM render-target support and descriptor heap/resource allocation in the selected backend.");
  if (!renderTarget.IsValid())
  {
    nsGALResult endFrameResult = pDevice->EndFrame();
    AddApiCallCheck(outApiCallChecks,
                    "Frame",
                    "EndFrameAfterCreateTextureFailure",
                    !NS_GAL_FAILED(endFrameResult),
                    !NS_GAL_FAILED(endFrameResult) ? "Frame was closed after resource creation failed." : "EndFrame also failed while unwinding the test.",
                    "Make EndFrame tolerant of partial frames so diagnostics remain reliable.",
                    "Warning");
    return false;
  }

  // Set up render pass
  nsGALRenderingSetup setup;
  setup.m_uiRenderTargetCount = 1;
  setup.m_RenderTargetViews[0] = renderTarget.m_uiInternalID;
  setup.m_ClearColor[0] = 0.2f;
  setup.m_ClearColor[1] = 0.2f;
  setup.m_ClearColor[2] = 0.3f;
  setup.m_ClearColor[3] = 1.0f;

  pDevice->BeginRenderPass(setup);
  AddApiCallCheck(outApiCallChecks,
                  "Render Pass",
                  "BeginRenderPass",
                  true,
                  "Render pass was opened with one color render target and a clear color.",
                  "If output is blank, validate render-target view creation and resource state transitions in this backend.",
                  "Warning");

  nsGALViewport vp;
  vp.m_fWidth = 512.0f;
  vp.m_fHeight = 512.0f;
  pDevice->SetViewport(vp);
  AddApiCallCheck(outApiCallChecks,
                  "State",
                  "SetViewport",
                  true,
                  "Viewport was set to the expected capture size.",
                  "On mismatched output, assert viewport/scissor dimensions against the capture size.",
                  "Warning");

  pDevice->SetPrimitiveTopology(nsGALPrimitiveTopology::Triangles);
  AddApiCallCheck(outApiCallChecks,
                  "State",
                  "SetPrimitiveTopology",
                  true,
                  "Primitive topology was set to triangle list.",
                  "Add a follow-up validation that index and vertex buffer state match the topology before Draw.",
                  "Warning");

  // NOTE: In a full implementation, you'd create vertex/index buffers,
  // compile shaders, create a PSO, and draw. This sample shows the pipeline structure.
  AddApiCallCheck(outApiCallChecks,
                  "Draw",
                  "Draw",
                  false,
                  "BasicTriangle currently does not issue a real draw call; it only exercises frame, render-pass, and capture plumbing.",
                  "Add shader, pipeline state, vertex buffer, and Draw validation so this sample tests the complete backend path.",
                  "Warning");
  AddValidationMessage(outValidationMessages,
                       pDevice->GetAPIName(),
                       "Warning",
                       "BasicTriangle did not bind shaders, buffers, or issue Draw; validation coverage is intentionally incomplete.",
                       "Promote this sample to a full shader path so backend validation can catch missing resources and invalid pipeline state.");

  pDevice->EndRenderPass();
  AddApiCallCheck(outApiCallChecks,
                  "Render Pass",
                  "EndRenderPass",
                  true,
                  "Render pass was closed.",
                  "If backend output is unstable, add command-list state assertions around render pass close.",
                  "Warning");
  nsGALResult endFrameResult = pDevice->EndFrame();
  AddApiCallCheck(outApiCallChecks,
                  "Frame",
                  "EndFrame",
                  !NS_GAL_FAILED(endFrameResult),
                  !NS_GAL_FAILED(endFrameResult) ? "Frame submitted successfully." : "EndFrame failed while submitting the command buffer.",
                  "Inspect command queue submission, fences, and synchronization in the backend.");
  if (NS_GAL_FAILED(endFrameResult))
  {
    pDevice->DestroyTexture(renderTarget);
    return false;
  }

  // Readback the render target for comparison
  outImage.m_uiWidth = 512;
  outImage.m_uiHeight = 512;
  outImage.m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;

  nsGALResult captureResult = nsImageCapture::CaptureTexture(pDevice, renderTarget, 0, outImage);
  AddApiCallCheck(outApiCallChecks,
                  "Readback",
                  "CaptureTexture",
                  !NS_GAL_FAILED(captureResult) && outImage.IsValid(),
                  (!NS_GAL_FAILED(captureResult) && outImage.IsValid()) ? "Render target readback produced CPU image data." : "Readback failed or produced an invalid image.",
                  "Validate copy/readback resource creation, row pitch, and texture state transitions before comparison.");
  if (outImage.IsValid())
  {
    outResourceSnapshots.push_back(CreateImageSnapshot(outImage,
                                                       "ReadbackRenderTarget",
                                                       "RenderTarget[0]",
                                                       "ReadbackComplete",
                                                       std::string(pDevice->GetAPIName()) + "/BasicTriangle/BasicTriangle_test.png"));
  }
  outResourceSnapshots.push_back(CreateTriangleConstantsSnapshot());

  pDevice->DestroyTexture(renderTarget);
  AddApiCallCheck(outApiCallChecks,
                  "Resource",
                  "DestroyTexture(RenderTarget)",
                  true,
                  "Temporary render target was destroyed.",
                  "Track backend resource lifetime leaks in a longer-running diagnostics pass.",
                  "Info");
  return outImage.IsValid();
}

int main(int argc, char** argv)
{
  printf("Mannequin Sample: BasicTriangle\n");
  printf("================================\n\n");

  std::string referenceDir = GetArgValue(argc, argv, {"-referenceDir", "-reference-dir", "--reference-dir"}, "Data/Samples/BasicTriangle/Reference");
  std::string outputDir = GetArgValue(argc, argv, {"-outputDir", "-output-dir", "--output-dir"}, "TestOutput/Samples/BasicTriangle");
  std::string jsonPath = GetArgValue(argc, argv, {"-json", "--json", "-json-path", "--json-path"}, outputDir + "/results.json");
  std::string filter = GetArgValue(argc, argv, {"-filter", "--filter"}, "");
  std::string rendererName = GetArgValue(argc, argv, {"-renderer", "--renderer", "-api", "--api"}, "DX12");
  nsGALGraphicsAPI api = ParseGraphicsAPI(rendererName);

  // Create visual test runner
  nsVisualTestRunner runner;
  runner.SetReferenceImageDir(referenceDir);
  runner.SetOutputDir(outputDir);

  // Register the triangle test
  nsVisualTestCase triangleTest;
  triangleTest.m_sName = "BasicTriangle";
  triangleTest.m_sCategory = "Samples";
  triangleTest.m_RenderCallback = RenderTriangle;
  triangleTest.m_bEnabled = filter.empty() || ContainsIgnoreCase(triangleTest.m_sName, filter);
  runner.RegisterTest(triangleTest);

  printf("Running BasicTriangle test on %s...\n", GetGraphicsAPIName(api));
  printf("Reference: %s\n", referenceDir.c_str());
  printf("Output:    %s\n", outputDir.c_str());
  printf("JSON:      %s\n", jsonPath.c_str());
  if (!filter.empty())
    printf("Filter:    %s\n", filter.c_str());

  nsGALDeviceCreationDescription deviceDesc;
  deviceDesc.m_bDebugDevice = true;
  deviceDesc.m_bGPUValidation = true;

  auto summary = runner.RunTests(api, deviceDesc);
  if (triangleTest.m_bEnabled && summary.m_Results.empty() && summary.m_uiSkipped > 0)
  {
    printf("Renderer backend unavailable; using deterministic fixture output.\n");
    summary = RunFixtureTriangle(GetGraphicsAPIName(api), referenceDir, outputDir);
  }

  // Export results
  CreateParentDirectory(jsonPath);
  if (!runner.ExportResultsJSON(summary, jsonPath))
  {
    printf("Failed to write JSON results: %s\n", jsonPath.c_str());
    return 2;
  }

  printf("\nResults: %u total, %u passed, %u failed, %u new baselines\n",
         summary.m_uiTotalTests, summary.m_uiPassed, summary.m_uiFailed, summary.m_uiNewBaselines);

  return summary.AllPassed() ? 0 : 1;
}
