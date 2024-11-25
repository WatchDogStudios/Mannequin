#include <RendererTest/RendererTestPCH.h>

#include "Basics.h"

nsTestAppRun nsRendererTestBasics::SubtestBlendStates()
{
  BeginFrame();
  BeginCommands("BlendStates");
  nsGALBlendStateHandle hState;

  nsGALBlendStateCreationDescription StateDesc;
  StateDesc.m_RenderTargetBlendDescriptions[0].m_bBlendingEnabled = true;

  if (m_iFrame == 0)
  {
    // StateDesc.m_RenderTargetBlendDescriptions[0].
  }

  if (m_iFrame == 1)
  {
    StateDesc.m_RenderTargetBlendDescriptions[0].m_SourceBlend = nsGALBlend::SrcAlpha;
    StateDesc.m_RenderTargetBlendDescriptions[0].m_DestBlend = nsGALBlend::InvSrcAlpha;
  }

  nsColor clear(0, 0, 0, 0);
  // if (StateDesc.m_bDepthClip)
  //  clear.r = 0.5f;
  // if (StateDesc.m_bFrontCounterClockwise)
  //  clear.g = 0.5f;
  // if (StateDesc.m_CullMode == nsGALCullMode::Front)
  //  clear.b = 0.5f;
  // if (StateDesc.m_CullMode == nsGALCullMode::Back)
  //  clear.b = 1.0f;

  BeginRendering(clear);

  hState = m_pDevice->CreateBlendState(StateDesc);
  NS_ASSERT_DEV(!hState.IsInvalidated(), "Couldn't create blend state!");

  nsRenderContext::GetDefaultInstance()->GetCommandEncoder()->SetBlendState(hState);

  RenderObjects(nsShaderBindFlags::NoBlendState);

  EndRendering();
  NS_TEST_IMAGE(m_iFrame, 150);
  EndCommands();
  EndFrame();

  m_pDevice->DestroyBlendState(hState);

  return m_iFrame < 1 ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}
