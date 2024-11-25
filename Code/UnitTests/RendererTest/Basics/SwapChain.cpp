#include <RendererTest/RendererTestPCH.h>

#include <Core/GameState/GameStateWindow.h>
#include <Core/Graphics/Camera.h>
#include <RendererTest/Basics/SwapChain.h>

nsResult nsRendererTestSwapChain::InitializeSubTest(nsInt32 iIdentifier)
{
  m_iFrame = -1;

  if (nsGraphicsTest::InitializeSubTest(iIdentifier).Failed())
    return NS_FAILURE;

  m_CurrentWindowSize = nsSizeU32(320, 240);

  // Window
  {
    nsWindowCreationDesc WindowCreationDesc;
    WindowCreationDesc.m_Resolution.width = m_CurrentWindowSize.width;
    WindowCreationDesc.m_Resolution.height = m_CurrentWindowSize.height;
    WindowCreationDesc.m_WindowMode = (iIdentifier == SubTests::ST_ResizeWindow) ? nsWindowMode::WindowResizable : nsWindowMode::WindowFixedResolution;
    // nsGameStateWindow will write any window size changes into the config.
    m_pWindow = NS_DEFAULT_NEW(nsGameStateWindow, WindowCreationDesc);
  }

  // SwapChain
  {
    nsGALWindowSwapChainCreationDescription swapChainDesc;
    swapChainDesc.m_pWindow = m_pWindow;
    swapChainDesc.m_SampleCount = nsGALMSAASampleCount::None;
    swapChainDesc.m_InitialPresentMode = (iIdentifier == SubTests::ST_NoVSync) ? nsGALPresentMode::Immediate : nsGALPresentMode::VSync;
    m_hSwapChain = nsGALWindowSwapChain::Create(swapChainDesc);
  }

  // Depth Texture
  if (iIdentifier != SubTests::ST_ColorOnly)
  {
    nsGALTextureCreationDescription texDesc;
    texDesc.m_uiWidth = m_CurrentWindowSize.width;
    texDesc.m_uiHeight = m_CurrentWindowSize.height;
    switch (iIdentifier)
    {
      case SubTests::ST_D16:
        texDesc.m_Format = nsGALResourceFormat::D16;
        break;
      case SubTests::ST_D24S8:
        texDesc.m_Format = nsGALResourceFormat::D24S8;
        break;
      default:
      case SubTests::ST_D32:
        texDesc.m_Format = nsGALResourceFormat::DFloat;
        break;
    }

    texDesc.m_bAllowRenderTargetView = true;
    m_hDepthStencilTexture = m_pDevice->CreateTexture(texDesc);
  }

  return NS_SUCCESS;
}

nsResult nsRendererTestSwapChain::DeInitializeSubTest(nsInt32 iIdentifier)
{
  DestroyWindow();

  if (nsGraphicsTest::DeInitializeSubTest(iIdentifier).Failed())
    return NS_FAILURE;

  return NS_SUCCESS;
}


void nsRendererTestSwapChain::ResizeTest(nsUInt32 uiInvocationCount)
{
  if (uiInvocationCount == 4)
  {
    // Not implemented on all platforms,  so we ignore the result here.
    m_pWindow->Resize(nsSizeU32(640, 480)).IgnoreResult();
  }

  if (m_pWindow->GetClientAreaSize() != m_CurrentWindowSize)
  {
    m_CurrentWindowSize = m_pWindow->GetClientAreaSize();
    m_pDevice->DestroyTexture(m_hDepthStencilTexture);
    m_hDepthStencilTexture.Invalidate();

    // Swap Chain
    {
      auto presentMode = m_pDevice->GetSwapChain<nsGALWindowSwapChain>(m_hSwapChain)->GetWindowDescription().m_InitialPresentMode;
      NS_TEST_RESULT(m_pDevice->UpdateSwapChain(m_hSwapChain, presentMode));
    }

    // Depth Texture
    {
      nsGALTextureCreationDescription texDesc;
      texDesc.m_uiWidth = m_CurrentWindowSize.width;
      texDesc.m_uiHeight = m_CurrentWindowSize.height;
      texDesc.m_Format = nsGALResourceFormat::DFloat;
      texDesc.m_bAllowRenderTargetView = true;
      m_hDepthStencilTexture = m_pDevice->CreateTexture(texDesc);
    }
  }
}

nsTestAppRun nsRendererTestSwapChain::BasicRenderLoop(nsInt32 iIdentifier, nsUInt32 uiInvocationCount)
{
  BeginFrame();
  BeginCommands("SwapChainTest");
  {
    const nsGALSwapChain* pPrimarySwapChain = m_pDevice->GetSwapChain(m_hSwapChain);

    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, m_pDevice->GetDefaultRenderTargetView(pPrimarySwapChain->GetBackBufferTexture()));
    renderingSetup.m_ClearColor = nsColor::CornflowerBlue;
    renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;
    if (!m_hDepthStencilTexture.IsInvalidated())
    {
      renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(m_pDevice->GetDefaultRenderTargetView(m_hDepthStencilTexture));
      renderingSetup.m_bClearDepth = true;
      renderingSetup.m_bClearStencil = true;
    }
    nsRectFloat viewport = nsRectFloat(0.0f, 0.0f, (float)m_pWindow->GetClientAreaSize().width, (float)m_pWindow->GetClientAreaSize().height);

    nsRenderContext::GetDefaultInstance()->BeginRendering(renderingSetup, viewport);
    m_pWindow->ProcessWindowMessages();

    nsRenderContext::GetDefaultInstance()->EndRendering();
  }
  EndCommands();
  EndFrame();

  return m_iFrame < 120 ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}

static nsRendererTestSwapChain g_SwapChainTest;
