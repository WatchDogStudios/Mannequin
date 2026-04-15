// Copyright (c) WD Studios. All rights reserved.
// Mannequin Sample: Basic Triangle
// Minimal sample demonstrating the visual test pipeline with a simple triangle render.

#include "../../Engine/RendererCore/Device/nsGALDevice.h"
#include "../../Engine/RendererCore/Utils/nsVisualTestRunner.h"

#include <cstdio>

/// Sample: render a basic colored triangle and capture for comparison.
bool RenderTriangle(nsGALDevice* pDevice, nsCapturedImage& outImage)
{
  // Begin frame
  if (NS_GAL_FAILED(pDevice->BeginFrame()))
    return false;

  // Create a simple render target
  nsGALTextureCreationDescription rtDesc;
  rtDesc.m_uiWidth = 512;
  rtDesc.m_uiHeight = 512;
  rtDesc.m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;
  rtDesc.m_bCreateRenderTarget = true;
  rtDesc.m_bAllowShaderResourceView = true;

  nsGALTextureHandle renderTarget = pDevice->CreateTexture(rtDesc);
  if (!renderTarget.IsValid())
    return false;

  // Set up render pass
  nsGALRenderingSetup setup;
  setup.m_uiRenderTargetCount = 1;
  setup.m_RenderTargetViews[0] = renderTarget.m_uiInternalID;
  setup.m_ClearColor[0] = 0.2f;
  setup.m_ClearColor[1] = 0.2f;
  setup.m_ClearColor[2] = 0.3f;
  setup.m_ClearColor[3] = 1.0f;

  pDevice->BeginRenderPass(setup);

  nsGALViewport vp;
  vp.m_fWidth = 512.0f;
  vp.m_fHeight = 512.0f;
  pDevice->SetViewport(vp);

  pDevice->SetPrimitiveTopology(nsGALPrimitiveTopology::Triangles);

  // NOTE: In a full implementation, you'd create vertex/index buffers,
  // compile shaders, create a PSO, and draw. This sample shows the pipeline structure.

  pDevice->EndRenderPass();
  pDevice->EndFrame();

  // Readback the render target for comparison
  outImage.m_uiWidth = 512;
  outImage.m_uiHeight = 512;
  outImage.m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;

  nsImageCapture::CaptureTexture(pDevice, renderTarget, 0, outImage);

  pDevice->DestroyTexture(renderTarget);
  return outImage.IsValid();
}

int main(int argc, char** argv)
{
  printf("Mannequin Sample: BasicTriangle\n");
  printf("================================\n\n");

  // Create visual test runner
  nsVisualTestRunner runner;
  runner.SetReferenceImageDir("Data/Samples/BasicTriangle/Reference");
  runner.SetOutputDir("TestOutput/Samples/BasicTriangle");

  // Register the triangle test
  nsVisualTestCase triangleTest;
  triangleTest.m_sName = "BasicTriangle";
  triangleTest.m_sCategory = "Samples";
  triangleTest.m_RenderCallback = RenderTriangle;
  runner.RegisterTest(triangleTest);

  // Run against DX12
  printf("Running BasicTriangle test on DX12...\n");
  auto summary = runner.RunTests(nsGALGraphicsAPI::DX12);

  // Export results
  runner.ExportResultsJSON(summary, "TestOutput/Samples/BasicTriangle/results.json");

  printf("\nResults: %u total, %u passed, %u failed, %u new baselines\n",
         summary.m_uiTotalTests, summary.m_uiPassed, summary.m_uiFailed, summary.m_uiNewBaselines);

  return summary.AllPassed() ? 0 : 1;
}
