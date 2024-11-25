#include <RendererTest/RendererTestPCH.h>

#include <Core/GameState/GameStateWindow.h>
#include <Core/Graphics/Camera.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Profiling/ProfilingUtils.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <RendererTest/Advanced/AdvancedFeatures.h>

#undef CreateWindow


void nsRendererTestAdvancedFeatures::SetupSubTests()
{
  nsStartup::StartupCoreSystems();
  SetupRenderer().AssertSuccess();
  const nsGALDeviceCapabilities& caps = nsGALDevice::GetDefaultDevice()->GetCapabilities();

  AddSubTest("01 - ReadRenderTarget", SubTests::ST_ReadRenderTarget);
  if (caps.m_bSupportsVSRenderTargetArrayIndex)
  {
    AddSubTest("02 - VertexShaderRenderTargetArrayIndex", SubTests::ST_VertexShaderRenderTargetArrayIndex);
  }
#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
  if (caps.m_bSupportsSharedTextures)
  {
    AddSubTest("03 - SharedTexture", SubTests::ST_SharedTexture);
  }
#endif

  if (caps.m_bShaderStageSupported[nsGALShaderStage::HullShader])
  {
    AddSubTest("04 - Tessellation", SubTests::ST_Tessellation);
  }
  if (caps.m_bShaderStageSupported[nsGALShaderStage::ComputeShader])
  {
    AddSubTest("05 - Compute", SubTests::ST_Compute);
  }
  AddSubTest("06 - FloatSampling", SubTests::ST_FloatSampling);
  AddSubTest("07 - ProxyTexture", SubTests::ST_ProxyTexture);

  ShutdownRenderer();
  nsStartup::ShutdownCoreSystems();
}

nsResult nsRendererTestAdvancedFeatures::InitializeSubTest(nsInt32 iIdentifier)
{
  NS_SUCCEED_OR_RETURN(nsGraphicsTest::InitializeSubTest(iIdentifier));
  NS_SUCCEED_OR_RETURN(CreateWindow(320, 240));

  if (iIdentifier == ST_ReadRenderTarget)
  {
    // Texture2D
    nsGALTextureCreationDescription desc;
    desc.SetAsRenderTarget(8, 8, nsGALResourceFormat::BGRAUByteNormalizedsRGB, nsGALMSAASampleCount::None);
    m_hTexture2D = m_pDevice->CreateTexture(desc);

    nsGALTextureResourceViewCreationDescription viewDesc;
    viewDesc.m_hTexture = m_hTexture2D;
    viewDesc.m_uiMipLevelsToUse = 1;
    viewDesc.m_uiMostDetailedMipLevel = 0;
    m_hTexture2DView = m_pDevice->CreateResourceView(viewDesc);


    m_hShader2 = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/UVColor.nsShader");
    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2D.nsShader");
  }

  if (iIdentifier == ST_ProxyTexture)
  {
    // Texture2DArray
    nsGALTextureCreationDescription desc;
    desc.SetAsRenderTarget(8, 8, nsGALResourceFormat::BGRAUByteNormalizedsRGB, nsGALMSAASampleCount::None);
    desc.m_Type = nsGALTextureType::Texture2DArray;
    desc.m_uiArraySize = 2;
    m_hTexture2DArray = m_pDevice->CreateTexture(desc);

    // Proxy texture
    m_hProxyTexture2D[0] = m_pDevice->CreateProxyTexture(m_hTexture2DArray, 0);
    m_hProxyTexture2D[1] = m_pDevice->CreateProxyTexture(m_hTexture2DArray, 1);

    // Direct view
    nsGALTextureResourceViewCreationDescription viewDesc;
    viewDesc.m_hTexture = m_hTexture2DArray;
    viewDesc.m_uiMipLevelsToUse = 1;
    viewDesc.m_uiMostDetailedMipLevel = 0;
    viewDesc.m_OverrideViewType = nsGALTextureType::Texture2D;
    viewDesc.m_uiArraySize = 1;
    m_hTexture2DArrayView[0] = m_pDevice->CreateResourceView(viewDesc);
    viewDesc.m_uiFirstArraySlice = 1;
    m_hTexture2DArrayView[1] = m_pDevice->CreateResourceView(viewDesc);

    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2D.nsShader");
    m_hShader2 = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/UVColor.nsShader");
    m_hShader3 = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/UVColor2.nsShader");
  }

  if (iIdentifier == ST_FloatSampling)
  {
    // Texture2DArray
    nsGALTextureCreationDescription desc;
    desc.SetAsRenderTarget(8, 8, nsGALResourceFormat::D16, nsGALMSAASampleCount::None);
    desc.m_Type = nsGALTextureType::Texture2DArray;
    m_hTexture2DArray = m_pDevice->CreateTexture(desc);

    m_hShader2 = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/ReadbackDepth.nsShader");
    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/SampleLevel_PointClampBorder.nsShader");

    nsGALSamplerStateCreationDescription samplerDesc;
    samplerDesc.m_MinFilter = nsGALTextureFilterMode::Point;
    samplerDesc.m_MagFilter = nsGALTextureFilterMode::Point;
    samplerDesc.m_MipFilter = nsGALTextureFilterMode::Point;
    samplerDesc.m_AddressU = nsImageAddressMode::ClampBorder;
    samplerDesc.m_AddressV = nsImageAddressMode::ClampBorder;
    samplerDesc.m_AddressW = nsImageAddressMode::ClampBorder;
    samplerDesc.m_BorderColor = nsColor::White;

    m_hDepthSamplerState = nsGALDevice::GetDefaultDevice()->CreateSamplerState(samplerDesc);
  }

  if (iIdentifier == ST_Compute)
  {
    // Texture2D as compute RW target. Note that SRGB and depth formats are not supported by most graphics cards for this purpose.
    nsEnum<nsGALResourceFormat> textureFormat;
    nsGALResourceFormat::Enum formats[] = {nsGALResourceFormat::RGBAFloat, nsGALResourceFormat::BGRAUByteNormalized, nsGALResourceFormat::RGBAUByteNormalized};
    for (auto format : formats)
    {
      if (m_pDevice->GetCapabilities().m_FormatSupport[format].IsSet(nsGALResourceFormatSupport::TextureRW))
      {
        textureFormat = format;
        break;
      }
    }
    if (!NS_TEST_BOOL(textureFormat != nsGALResourceFormat::Invalid))
      return NS_FAILURE;

    nsGALTextureCreationDescription desc;
    desc.SetAsRenderTarget(8, 8, textureFormat, nsGALMSAASampleCount::None);
    desc.m_bAllowUAV = true;
    desc.m_bAllowRenderTargetView = false;
    desc.m_uiMipLevelCount = 1;
    desc.m_ResourceAccess.m_bImmutable = false;
    m_hTexture2D = m_pDevice->CreateTexture(desc);

    nsGALTextureResourceViewCreationDescription viewDesc;
    viewDesc.m_hTexture = m_hTexture2D;
    viewDesc.m_uiMipLevelsToUse = 1;
    viewDesc.m_uiMostDetailedMipLevel = 0;
    m_hTexture2DView = m_pDevice->CreateResourceView(viewDesc);

    m_hShader2 = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/UVColorCompute.nsShader");
    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2DReadbackDepth.nsShader");
  }

  if (iIdentifier == ST_VertexShaderRenderTargetArrayIndex)
  {
    // Texture2DArray
    nsGALTextureCreationDescription desc;
    desc.SetAsRenderTarget(320 / 2, 240, nsGALResourceFormat::BGRAUByteNormalizedsRGB, nsGALMSAASampleCount::None);
    desc.m_Type = nsGALTextureType::Texture2DArray;
    desc.m_uiArraySize = 2;
    m_hTexture2DArray = m_pDevice->CreateTexture(desc);

    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Stereo.nsShader");
    m_hShader2 = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/StereoPreview.nsShader");
  }

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
  if (iIdentifier == ST_SharedTexture)
  {
    nsCVarFloat* pProfilingThreshold = (nsCVarFloat*)nsCVar::FindCVarByName("Profiling.DiscardThresholdMS");
    NS_ASSERT_DEBUG(pProfilingThreshold, "Profiling.cpp cvar was renamed");
    m_fOldProfilingThreshold = *pProfilingThreshold;
    *pProfilingThreshold = 0.0f;

    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2D.nsShader");

    const nsStringBuilder pathToSelf = nsCommandLineUtils::GetGlobalInstance()->GetParameter(0);

    nsProcessOptions opt;
    opt.m_sProcess = pathToSelf;

    nsStringBuilder sIPC;
    nsConversionUtils::ToString(nsUuid::MakeUuid(), sIPC);

    nsStringBuilder sPID;
    nsConversionUtils::ToString(nsProcess::GetCurrentProcessID(), sPID);

#  ifdef BUILDSYSTEM_ENABLE_VULKAN_SUPPORT
    constexpr const char* szDefaultRenderer = "Vulkan";
#  else
    constexpr const char* szDefaultRenderer = "DX11";
#  endif
    nsStringView sRendererName = nsCommandLineUtils::GetGlobalInstance()->GetStringOption("-renderer", 0, szDefaultRenderer);

    opt.m_Arguments.PushBack("-offscreen");
    opt.m_Arguments.PushBack("-IPC");
    opt.m_Arguments.PushBack(sIPC);
    opt.m_Arguments.PushBack("-PID");
    opt.m_Arguments.PushBack(sPID);
    opt.m_Arguments.PushBack("-renderer");
    opt.m_Arguments.PushBack(sRendererName);
    opt.m_Arguments.PushBack("-outputDir");
    opt.m_Arguments.PushBack(nsTestFramework::GetInstance()->GetAbsOutputPath());


    m_pOffscreenProcess = NS_DEFAULT_NEW(nsProcess);
    NS_SUCCEED_OR_RETURN(m_pOffscreenProcess->Launch(opt));

    m_bExiting = false;
    m_uiReceivedTextures = 0;
    m_pChannel = nsIpcChannel::CreatePipeChannel(sIPC, nsIpcChannel::Mode::Server);
    m_pProtocol = NS_DEFAULT_NEW(nsIpcProcessMessageProtocol, m_pChannel.Borrow());
    m_pProtocol->m_MessageEvent.AddEventHandler(nsMakeDelegate(&nsRendererTestAdvancedFeatures::OffscreenProcessMessageFunc, this));
    m_pChannel->Connect();
    while (m_pChannel->GetConnectionState() != nsIpcChannel::ConnectionState::Connecting)
    {
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(16));
    }
    m_SharedTextureDesc.SetAsRenderTarget(8, 8, nsGALResourceFormat::BGRAUByteNormalizedsRGB);
    m_SharedTextureDesc.m_Type = nsGALTextureType::Texture2DShared;

    m_SharedTextureQueue.Clear();
    for (nsUInt32 i = 0; i < s_SharedTextureCount; i++)
    {
      m_hSharedTextures[i] = m_pDevice->CreateSharedTexture(m_SharedTextureDesc);
      NS_TEST_BOOL(!m_hSharedTextures[i].IsInvalidated());
      m_SharedTextureQueue.PushBack({i, 0});
    }

    while (!m_pChannel->IsConnected())
    {
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(16));
    }

    nsOffscreenTest_OpenMsg msg;
    msg.m_TextureDesc = m_SharedTextureDesc;
    for (auto& hSharedTexture : m_hSharedTextures)
    {
      const nsGALSharedTexture* pSharedTexture = m_pDevice->GetSharedTexture(hSharedTexture);
      if (pSharedTexture == nullptr)
      {
        return NS_FAILURE;
      }

      msg.m_TextureHandles.PushBack(pSharedTexture->GetSharedHandle());
    }
    m_pProtocol->Send(&msg);
  }
#endif

  if (iIdentifier == ST_Tessellation)
  {
    {
      nsGeometry geom;
      geom.AddStackedSphere(0.5f, 3, 2);

      nsMeshBufferResourceDescriptor desc;
      desc.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
      desc.AddStream(nsGALVertexAttributeSemantic::TexCoord0, nsGALResourceFormat::XYFloat);
      desc.AllocateStreamsFromGeometry(geom, nsGALPrimitiveTopology::Triangles);

      m_hSphereMesh = nsResourceManager::CreateResource<nsMeshBufferResource>("UnitTest-SphereMesh", std::move(desc), "SphereMesh");
    }

    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Tessellation.nsShader");
  }

  switch (iIdentifier)
  {
    case SubTests::ST_ReadRenderTarget:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_VertexShaderRenderTargetArrayIndex:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_SharedTexture:
      m_ImgCompFrames.PushBack(100000000);
      break;
    case SubTests::ST_Tessellation:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_Compute:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_FloatSampling:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_ProxyTexture:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  return NS_SUCCESS;
}

nsResult nsRendererTestAdvancedFeatures::DeInitializeSubTest(nsInt32 iIdentifier)
{
  if (iIdentifier == ST_Tessellation)
  {
    m_hSphereMesh.Invalidate();
  }
#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
  else if (iIdentifier == ST_SharedTexture)
  {
    NS_TEST_BOOL(m_pOffscreenProcess->WaitToFinish(nsTime::MakeFromSeconds(5)).Succeeded());
    NS_TEST_BOOL(m_pOffscreenProcess->GetState() == nsProcessState::Finished);
    NS_TEST_INT(m_pOffscreenProcess->GetExitCode(), 0);
    m_pOffscreenProcess = nullptr;

    m_pProtocol = nullptr;
    m_pChannel = nullptr;

    for (nsUInt32 i = 0; i < s_SharedTextureCount; i++)
    {
      m_pDevice->DestroySharedTexture(m_hSharedTextures[i]);
      m_hSharedTextures[i].Invalidate();
    }
    m_SharedTextureQueue.Clear();

    nsStringView sPath = ":imgout/Profiling/sharedTexture.json"_nssv;
    NS_TEST_RESULT(nsProfilingUtils::SaveProfilingCapture(sPath));
    nsStringView sPath2 = ":imgout/Profiling/offscreenProfiling.json"_nssv;
    nsStringView sMergedFile = ":imgout/Profiling/sharedTexturesMerged.json"_nssv;
    NS_TEST_RESULT(nsProfilingUtils::MergeProfilingCaptures(sPath, sPath2, sMergedFile));

    nsCVarFloat* pProfilingThreshold = (nsCVarFloat*)nsCVar::FindCVarByName("Profiling.DiscardThresholdMS");
    NS_ASSERT_DEBUG(pProfilingThreshold, "Profiling.cpp cvar was renamed");
    *pProfilingThreshold = m_fOldProfilingThreshold;
  }
#endif
  else if (iIdentifier == ST_FloatSampling)
  {
    if (!m_hDepthSamplerState.IsInvalidated())
    {
      nsGALDevice::GetDefaultDevice()->DestroySamplerState(m_hDepthSamplerState);
      m_hDepthSamplerState.Invalidate();
    }
  }
  if (iIdentifier == ST_ProxyTexture)
  {
    m_hTexture2DArrayView[0].Invalidate();
    m_hTexture2DArrayView[1].Invalidate();
    for (nsUInt32 i = 0; i < 2; i++)
    {
      if (!m_hProxyTexture2D[i].IsInvalidated())
      {
        nsGALDevice::GetDefaultDevice()->DestroyProxyTexture(m_hProxyTexture2D[i]);
        m_hProxyTexture2D[i].Invalidate();
      }
    }
  }
  m_hShader2.Invalidate();
  m_hShader3.Invalidate();

  if (!m_hTexture2D.IsInvalidated())
  {
    m_pDevice->DestroyTexture(m_hTexture2D);
    m_hTexture2D.Invalidate();
  }

  if (!m_hTexture2DArray.IsInvalidated())
  {
    m_pDevice->DestroyTexture(m_hTexture2DArray);
    m_hTexture2DArray.Invalidate();
  }

  m_hTexture2DView.Invalidate();

  DestroyWindow();
  NS_SUCCEED_OR_RETURN(nsGraphicsTest::DeInitializeSubTest(iIdentifier));
  return NS_SUCCESS;
}

nsTestAppRun nsRendererTestAdvancedFeatures::RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount)
{
  m_iFrame = uiInvocationCount;
  m_bCaptureImage = false;

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
  if (iIdentifier == ST_SharedTexture)
  {
    return SharedTexture();
  }
#endif

  BeginFrame();

  switch (iIdentifier)
  {
    case SubTests::ST_ReadRenderTarget:
      ReadRenderTarget();
      break;
    case SubTests::ST_VertexShaderRenderTargetArrayIndex:
      if (!m_pDevice->GetCapabilities().m_bSupportsVSRenderTargetArrayIndex)
        return nsTestAppRun::Quit;
      VertexShaderRenderTargetArrayIndex();
      break;
    case SubTests::ST_Tessellation:
      Tessellation();
      break;
    case SubTests::ST_Compute:
      Compute();
      break;
    case SubTests::ST_FloatSampling:
      FloatSampling();
      break;
    case SubTests::ST_ProxyTexture:
      ProxyTexture();
      break;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  EndFrame();

  if (m_ImgCompFrames.IsEmpty() || m_ImgCompFrames.PeekBack() == m_iFrame)
  {
    return nsTestAppRun::Quit;
  }
  return nsTestAppRun::Continue;
}

void nsRendererTestAdvancedFeatures::ReadRenderTarget()
{
  BeginCommands("Offscreen");
  {
    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, m_pDevice->GetDefaultRenderTargetView(m_hTexture2D));
    renderingSetup.m_ClearColor = nsColor::RebeccaPurple;
    renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;

    nsRectFloat viewport = nsRectFloat(0, 0, 8, 8);
    nsRenderContext::GetDefaultInstance()->BeginRendering(renderingSetup, viewport);
    SetClipSpace();

    nsRenderContext::GetDefaultInstance()->BindShader(m_hShader2);
    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

    nsRenderContext::GetDefaultInstance()->EndRendering();
  }
  EndCommands();


  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsUInt32 uiColumns = 2;
  const nsUInt32 uiRows = 2;
  const float fElementWidth = fWidth / uiColumns;
  const float fElementHeight = fHeight / uiRows;

  const nsMat4 mMVP = CreateSimpleMVP((float)fElementWidth / (float)fElementHeight);
  BeginCommands("Texture2D");
  {
    nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0xFFFFFFFF, m_hTexture2DView);
    viewport = nsRectFloat(fElementWidth, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2DView);
    viewport = nsRectFloat(0, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2DView);
    m_bCaptureImage = true;
    viewport = nsRectFloat(fElementWidth, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2DView);
  }
  EndCommands();
}

void nsRendererTestAdvancedFeatures::FloatSampling()
{
  BeginCommands("Offscreen");
  {
    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_bDiscardDepth = true;
    renderingSetup.m_bClearDepth = true;
    renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(m_pDevice->GetDefaultRenderTargetView(m_hTexture2DArray));

    nsRectFloat viewport = nsRectFloat(0, 0, 8, 8);
    nsRenderContext::GetDefaultInstance()->BeginRendering(renderingSetup, viewport);
    SetClipSpace();

    nsRenderContext::GetDefaultInstance()->BindShader(m_hShader2);
    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

    nsRenderContext::GetDefaultInstance()->EndRendering();
  }
  EndCommands();


  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsUInt32 uiColumns = 2;
  const nsUInt32 uiRows = 2;
  const float fElementWidth = fWidth / uiColumns;
  const float fElementHeight = fHeight / uiRows;

  const nsMat4 mMVP = CreateSimpleMVP((float)fElementWidth / (float)fElementHeight);
  BeginCommands("FloatSampling");
  {
    nsRenderContext::GetDefaultInstance()->BindSamplerState("DepthSampler", m_hDepthSamplerState);
    nsRenderContext::GetDefaultInstance()->BindTexture2D("DepthTexture", m_pDevice->GetDefaultResourceView(m_hTexture2DArray));

    nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
    {
      nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::RebeccaPurple, 0xFFFFFFFF, &viewport);
      RenderObject(m_hCubeUV, mMVP, nsColor(1, 1, 1, 1), nsShaderBindFlags::None);
      EndRendering();
      if (m_ImgCompFrames.Contains(m_iFrame))
      {
        NS_TEST_IMAGE(m_iFrame, 100);
      }
    }
  }
  EndCommands();
}


void nsRendererTestAdvancedFeatures::ProxyTexture()
{
  // We render normal pattern to layer 0 and the blue pattern to layer 1.
  BeginCommands("Offscreen");
  for (nsUInt32 i = 0; i < 2; i++)
  {
    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, m_pDevice->GetDefaultRenderTargetView(m_hProxyTexture2D[i]));
    renderingSetup.m_ClearColor = nsColor::RebeccaPurple;
    renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;

    nsRectFloat viewport = nsRectFloat(0, 0, 8, 8);
    nsRenderContext::GetDefaultInstance()->BeginRendering(renderingSetup, viewport);
    SetClipSpace();

    nsRenderContext::GetDefaultInstance()->BindShader(i == 0 ? m_hShader2 : m_hShader3);
    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

    nsRenderContext::GetDefaultInstance()->EndRendering();
  }
  EndCommands();

  // Render both layers using proxy texture (2D) and manually created resource view (2DArray).
  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsUInt32 uiColumns = 2;
  const nsUInt32 uiRows = 2;
  const float fElementWidth = fWidth / uiColumns;
  const float fElementHeight = fHeight / uiRows;

  const nsMat4 mMVP = CreateSimpleMVP((float)fElementWidth / (float)fElementHeight);
  BeginCommands("Texture2DProxy");
  {
    nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0xFFFFFFFF, m_pDevice->GetDefaultResourceView(m_hProxyTexture2D[0]));
    viewport = nsRectFloat(fElementWidth, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_pDevice->GetDefaultResourceView(m_hProxyTexture2D[1]));
    viewport = nsRectFloat(0, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2DArrayView[0]);
    m_bCaptureImage = true;
    viewport = nsRectFloat(fElementWidth, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2DArrayView[1]);
  }
  EndCommands();
}

void nsRendererTestAdvancedFeatures::VertexShaderRenderTargetArrayIndex()
{
  m_bCaptureImage = true;
  const nsMat4 mMVP = CreateSimpleMVP((m_pWindow->GetClientAreaSize().width / 2.0f) / (float)m_pWindow->GetClientAreaSize().height);
  BeginCommands("Offscreen Stereo");
  {
    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, m_pDevice->GetDefaultRenderTargetView(m_hTexture2DArray));
    renderingSetup.m_ClearColor = nsColor::RebeccaPurple;
    renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;

    nsRectFloat viewport = nsRectFloat(0, 0, m_pWindow->GetClientAreaSize().width / 2.0f, (float)m_pWindow->GetClientAreaSize().height);
    nsRenderContext::GetDefaultInstance()->BeginRendering(renderingSetup, viewport);
    SetClipSpace();

    nsRenderContext::GetDefaultInstance()->BindShader(m_hShader, nsShaderBindFlags::None);
    ObjectCB* ocb = nsRenderContext::GetConstantBufferData<ObjectCB>(m_hObjectTransformCB);
    ocb->m_MVP = mMVP;
    ocb->m_Color = nsColor(1, 1, 1, 1);
    nsRenderContext::GetDefaultInstance()->BindConstantBuffer("PerObject", m_hObjectTransformCB);
    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(m_hCubeUV);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer(0xFFFFFFFF, 0, 2).IgnoreResult();

    nsRenderContext::GetDefaultInstance()->EndRendering();
  }
  EndCommands();


  BeginCommands("Texture2DArray");
  {
    nsRectFloat viewport = nsRectFloat(0, 0, (float)m_pWindow->GetClientAreaSize().width, (float)m_pWindow->GetClientAreaSize().height);

    nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::RebeccaPurple, 0xFFFFFFFF, &viewport);

    nsRenderContext::GetDefaultInstance()->BindTexture2D("DiffuseTexture", m_pDevice->GetDefaultResourceView(m_hTexture2DArray));

    nsRenderContext::GetDefaultInstance()->BindShader(m_hShader2);
    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

    EndRendering();
    if (m_bCaptureImage && m_ImgCompFrames.Contains(m_iFrame))
    {
      NS_TEST_IMAGE(m_iFrame, 100);
    }
  }
  EndCommands();
}

void nsRendererTestAdvancedFeatures::Tessellation()
{
  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsMat4 mMVP = CreateSimpleMVP((float)fWidth / (float)fHeight);
  BeginCommands("Tessellation");
  {
    nsRectFloat viewport = nsRectFloat(0, 0, fWidth, fHeight);
    nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::RebeccaPurple, 0xFFFFFFFF, &viewport);
    RenderObject(m_hSphereMesh, mMVP, nsColor(1, 1, 1, 1), nsShaderBindFlags::None);

    EndRendering();
    if (m_ImgCompFrames.Contains(m_iFrame))
    {
      NS_TEST_IMAGE(m_iFrame, 100);
    }
  }
  EndCommands();
}


void nsRendererTestAdvancedFeatures::Compute()
{
  BeginCommands("Compute");
  {
    nsUInt32 uiWidth = 8;
    nsUInt32 uiHeight = 8;

    nsRenderContext::GetDefaultInstance()->BeginCompute("Compute");
    {
      nsRenderContext::GetDefaultInstance()->BindShader(m_hShader2);

      nsGALTextureUnorderedAccessViewHandle hFilterOutput;
      {
        nsGALTextureUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = m_hTexture2D;
        desc.m_uiMipLevelToUse = 0;
        desc.m_uiFirstArraySlice = 0;
        desc.m_uiArraySize = 1;
        hFilterOutput = m_pDevice->CreateUnorderedAccessView(desc);
      }
      nsRenderContext::GetDefaultInstance()->BindUAV("OutputTexture", hFilterOutput);

      // The compute shader uses [numthreads(8, 8, 1)], so we need to compute how many of these groups we need to dispatch to fill the entire image.
      constexpr nsUInt32 uiThreadsX = 8;
      constexpr nsUInt32 uiThreadsY = 8;
      const nsUInt32 uiDispatchX = (uiWidth + uiThreadsX - 1) / uiThreadsX;
      const nsUInt32 uiDispatchY = (uiHeight + uiThreadsY - 1) / uiThreadsY;
      // As the image is exactly as big as one of our groups, we need to dispatch exactly one group:
      NS_TEST_INT(uiDispatchX, 1);
      NS_TEST_INT(uiDispatchY, 1);
      nsRenderContext::GetDefaultInstance()->Dispatch(uiDispatchX, uiDispatchY, 1).AssertSuccess();
    }
    nsRenderContext::GetDefaultInstance()->EndCompute();
  }
  EndCommands();


  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;

  const nsMat4 mMVP = CreateSimpleMVP((float)fWidth / (float)fHeight);
  BeginCommands("Texture2D");
  {
    m_bCaptureImage = true;
    nsRectFloat viewport = nsRectFloat(0, 0, fWidth, fHeight);
    RenderCube(viewport, mMVP, 0xFFFFFFFF, m_hTexture2DView);
  }
  EndCommands();
}

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
nsTestAppRun nsRendererTestAdvancedFeatures::SharedTexture()
{
  if (m_pOffscreenProcess->GetState() != nsProcessState::Running)
  {
    NS_TEST_BOOL(m_bExiting);
    return nsTestAppRun::Quit;
  }

  m_pProtocol->WaitForMessages(nsTime::MakeFromMilliseconds(16)).IgnoreResult();

  nsOffscreenTest_SharedTexture texture = m_SharedTextureQueue.PeekFront();
  m_SharedTextureQueue.PopFront();

  nsStringBuilder sTemp;
  sTemp.SetFormat("Render {}:{}|{}", m_uiReceivedTextures, texture.m_uiCurrentTextureIndex, texture.m_uiCurrentSemaphoreValue);
  NS_PROFILE_SCOPE(sTemp);
  BeginFrame();
  {
    const nsGALSharedTexture* pSharedTexture = m_pDevice->GetSharedTexture(m_hSharedTextures[texture.m_uiCurrentTextureIndex]);
    NS_ASSERT_DEV(pSharedTexture != nullptr, "Shared texture did not resolve");

    pSharedTexture->WaitSemaphoreGPU(texture.m_uiCurrentSemaphoreValue);

    const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
    const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
    const nsUInt32 uiColumns = 1;
    const nsUInt32 uiRows = 1;
    const float fElementWidth = fWidth / uiColumns;
    const float fElementHeight = fHeight / uiRows;

    const nsMat4 mMVP = CreateSimpleMVP((float)fElementWidth / (float)fElementHeight);
    BeginCommands("Texture2D");
    {
      nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
      m_bCaptureImage = true;
      viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);

      nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::RebeccaPurple, 0xFFFFFFFF, &viewport);

      nsRenderContext::GetDefaultInstance()->BindTexture2D("DiffuseTexture", m_pDevice->GetDefaultResourceView(m_hSharedTextures[texture.m_uiCurrentTextureIndex]));
      RenderObject(m_hCubeUV, mMVP, nsColor(1, 1, 1, 1), nsShaderBindFlags::None);

      EndRendering();
      if (!m_bExiting && m_uiReceivedTextures > 10)
      {
        NS_TEST_IMAGE(0, 10);

        nsOffscreenTest_CloseMsg msg;
        NS_TEST_BOOL(m_pProtocol->Send(&msg));
        m_bExiting = true;
      }
    }
    EndCommands();

    texture.m_uiCurrentSemaphoreValue++;
    pSharedTexture->SignalSemaphoreGPU(texture.m_uiCurrentSemaphoreValue);
    nsRenderContext::GetDefaultInstance()->ResetContextState();
  }
  EndFrame();

  if (m_SharedTextureQueue.IsEmpty() || !m_pChannel->IsConnected())
  {
    m_SharedTextureQueue.PushBack(texture);
  }
  else if (!m_bExiting)
  {
    nsOffscreenTest_RenderMsg msg;
    msg.m_Texture = texture;
    NS_TEST_BOOL(m_pProtocol->Send(&msg));
  }

  return nsTestAppRun::Continue;
}

void nsRendererTestAdvancedFeatures::OffscreenProcessMessageFunc(const nsProcessMessage* pMsg)
{
  if (const auto* pAction = nsDynamicCast<const nsOffscreenTest_RenderResponseMsg*>(pMsg))
  {
    m_uiReceivedTextures++;
    nsStringBuilder sTemp;
    sTemp.SetFormat("Receive {}|{}", pAction->m_Texture.m_uiCurrentTextureIndex, pAction->m_Texture.m_uiCurrentSemaphoreValue);
    NS_PROFILE_SCOPE(sTemp);
    m_SharedTextureQueue.PushBack(pAction->m_Texture);
  }
}
#endif

static nsRendererTestAdvancedFeatures g_AdvancedFeaturesTest;
