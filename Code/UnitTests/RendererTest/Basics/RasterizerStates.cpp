#include <RendererTest/RendererTestPCH.h>

#include "Basics.h"
#include <Core/Graphics/Camera.h>

nsTestAppRun nsRendererTestBasics::SubtestRasterizerStates()
{
  BeginFrame();
  BeginCommands("RasterizerStates");
  nsGALRasterizerStateHandle hState;

  nsGALRasterizerStateCreationDescription RasterStateDesc;

  if (m_iFrame == 0)
  {
    RasterStateDesc.m_bFrontCounterClockwise = false;
    RasterStateDesc.m_bWireFrame = false;
    RasterStateDesc.m_CullMode = nsGALCullMode::None;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 1)
  {
    RasterStateDesc.m_bFrontCounterClockwise = false;
    RasterStateDesc.m_bWireFrame = false;
    RasterStateDesc.m_CullMode = nsGALCullMode::Back;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 2)
  {
    RasterStateDesc.m_bFrontCounterClockwise = false;
    RasterStateDesc.m_bWireFrame = false;
    RasterStateDesc.m_CullMode = nsGALCullMode::Front;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 3)
  {
    RasterStateDesc.m_bFrontCounterClockwise = true;
    RasterStateDesc.m_bWireFrame = false;
    RasterStateDesc.m_CullMode = nsGALCullMode::Back;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 4)
  {
    RasterStateDesc.m_bFrontCounterClockwise = true;
    RasterStateDesc.m_bWireFrame = false;
    RasterStateDesc.m_CullMode = nsGALCullMode::Front;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 5)
  {
    RasterStateDesc.m_bFrontCounterClockwise = false;
    RasterStateDesc.m_bWireFrame = false;
    RasterStateDesc.m_CullMode = nsGALCullMode::Back;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 6)
  {
    RasterStateDesc.m_bFrontCounterClockwise = false;
    RasterStateDesc.m_bWireFrame = false;
    RasterStateDesc.m_CullMode = nsGALCullMode::Back;
    RasterStateDesc.m_bScissorTest = true;
  }

  if (m_iFrame == 7)
  {
    RasterStateDesc.m_bFrontCounterClockwise = false;
    RasterStateDesc.m_bWireFrame = true;
    RasterStateDesc.m_CullMode = nsGALCullMode::None;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 8)
  {
    RasterStateDesc.m_bFrontCounterClockwise = false;
    RasterStateDesc.m_bWireFrame = true;
    RasterStateDesc.m_CullMode = nsGALCullMode::Back;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 9)
  {
    RasterStateDesc.m_bFrontCounterClockwise = false;
    RasterStateDesc.m_bWireFrame = true;
    RasterStateDesc.m_CullMode = nsGALCullMode::Front;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 10)
  {
    RasterStateDesc.m_bFrontCounterClockwise = true;
    RasterStateDesc.m_bWireFrame = true;
    RasterStateDesc.m_CullMode = nsGALCullMode::Back;
    RasterStateDesc.m_bScissorTest = false;
  }

  if (m_iFrame == 11)
  {
    RasterStateDesc.m_bFrontCounterClockwise = true;
    RasterStateDesc.m_bWireFrame = true;
    RasterStateDesc.m_CullMode = nsGALCullMode::Front;
    RasterStateDesc.m_bScissorTest = false;
  }

  nsColor clear(0, 0, 0, 0);
  if (!RasterStateDesc.m_bFrontCounterClockwise)
    clear.g = 0.5f;
  if (RasterStateDesc.m_CullMode == nsGALCullMode::Front)
    clear.b = 0.5f;
  if (RasterStateDesc.m_CullMode == nsGALCullMode::Back)
    clear.b = 1.0f;

  BeginRendering(clear);

  hState = m_pDevice->CreateRasterizerState(RasterStateDesc);
  NS_ASSERT_DEV(!hState.IsInvalidated(), "Couldn't create rasterizer state!");

  nsRenderContext::GetDefaultInstance()->GetCommandEncoder()->SetRasterizerState(hState);

  nsRenderContext::GetDefaultInstance()->GetCommandEncoder()->SetScissorRect(nsRectU32(100, 50, GetResolution().width / 2, GetResolution().height / 2));

  RenderObjects(nsShaderBindFlags::NoRasterizerState);

  EndRendering();
  if (RasterStateDesc.m_bWireFrame)
  {
    nsStringView sRendererName = m_pDevice->GetRenderer();
    const bool bRandomlyChangesLineThicknessOnDriverUpdate = sRendererName.IsEqual_NoCase("DX11") && m_pDevice->GetCapabilities().m_sAdapterName.FindSubString_NoCase("Nvidia");

    NS_TEST_LINE_IMAGE(m_iFrame, bRandomlyChangesLineThicknessOnDriverUpdate ? 1000 : 300);
  }
  else
    NS_TEST_IMAGE(m_iFrame, 200);
  EndCommands();
  EndFrame();

  m_pDevice->DestroyRasterizerState(hState);

  return m_iFrame < 11 ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}
