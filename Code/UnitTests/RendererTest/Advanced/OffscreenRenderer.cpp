#include <RendererTest/RendererTestPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Profiling/ProfilingUtils.h>
#include <Foundation/Time/Clock.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/SharedTextureSwapChain.h>
#include <RendererTest/Advanced/OffscreenRenderer.h>
#include <RendererTest/TestClass/TestClass.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsOffscreenTest_SharedTexture, nsNoBase, 1, nsRTTIDefaultAllocator<nsOffscreenTest_SharedTexture>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("CurrentTextureIndex", m_uiCurrentTextureIndex),
    NS_MEMBER_PROPERTY("CurrentSemaphoreValue", m_uiCurrentSemaphoreValue),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsOffscreenTest_OpenMsg, 1, nsRTTIDefaultAllocator<nsOffscreenTest_OpenMsg>)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("TextureDesc", m_TextureDesc),
      NS_ARRAY_MEMBER_PROPERTY("TextureHandles", m_TextureHandles),
    }
    NS_END_PROPERTIES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsOffscreenTest_CloseMsg, 1, nsRTTIDefaultAllocator<nsOffscreenTest_CloseMsg>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsOffscreenTest_RenderMsg, 1, nsRTTIDefaultAllocator<nsOffscreenTest_RenderMsg>)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("Texture", m_Texture),
    }
    NS_END_PROPERTIES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsOffscreenTest_RenderResponseMsg, 1, nsRTTIDefaultAllocator<nsOffscreenTest_RenderResponseMsg>)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("Texture", m_Texture),
    }
    NS_END_PROPERTIES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsOffscreenRendererTest::nsOffscreenRendererTest()
  : nsApplication("nsOffscreenRendererTest")

{
}

nsOffscreenRendererTest::~nsOffscreenRendererTest() = default;

void nsOffscreenRendererTest::Run()
{
  NS_PROFILE_SCOPE("Run");

  nsClock::GetGlobalClock()->Update();

  if (!m_pProtocol->ProcessMessages())
  {
    m_pProtocol->WaitForMessages(nsTime::MakeFromMilliseconds(8)).IgnoreResult();
  }

  // do the rendering
  if (!m_RequestedFrames.IsEmpty())
  {
    nsOffscreenTest_RenderMsg action = m_RequestedFrames[0];
    m_RequestedFrames.RemoveAtAndCopy(0);

    auto device = nsGALDevice::GetDefaultDevice();

    auto pSwapChain = const_cast<nsGALSharedTextureSwapChain*>(nsGALDevice::GetDefaultDevice()->GetSwapChain<nsGALSharedTextureSwapChain>(m_hSwapChain));
    NS_ASSERT_DEBUG(pSwapChain, "SwapChain should have been created at this point");
    NS_ANALYSIS_ASSUME(pSwapChain != nullptr);
    pSwapChain->Arm(action.m_Texture.m_uiCurrentTextureIndex, action.m_Texture.m_uiCurrentSemaphoreValue);

    nsStringBuilder sTemp;
    sTemp.SetFormat("Render {}|{}", action.m_Texture.m_uiCurrentTextureIndex, action.m_Texture.m_uiCurrentSemaphoreValue);
    NS_PROFILE_SCOPE(sTemp);

    m_pDevice->EnqueueFrameSwapChain(m_hSwapChain);
    device->BeginFrame();

    nsGALCommandEncoder* pCommandEncoder = device->BeginCommands(sTemp);

    nsGALRenderingSetup renderingSetup;
    nsGALRenderTargetViewHandle hBackbufferRTV = device->GetDefaultRenderTargetView(pSwapChain->GetRenderTargets().m_hRTs[0]);
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, hBackbufferRTV);
    renderingSetup.m_ClearColor = nsColor::Pink;
    renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;

    nsRectFloat viewport = nsRectFloat(0, 0, 8, 8);
    nsRenderContext::GetDefaultInstance()->BeginRendering(renderingSetup, viewport);
    nsGraphicsTest::SetClipSpace();

    nsRenderContext::GetDefaultInstance()->BindShader(m_hScreenShader);
    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

    nsRenderContext::GetDefaultInstance()->EndRendering();

    device->EndCommands(pCommandEncoder);

    device->EndFrame();
    nsRenderContext::GetDefaultInstance()->ResetContextState();
  }

  if (m_RequestedFrames.IsEmpty() && m_bExiting)
  {
    SetReturnCode(0);
    RequestApplicationQuit();
  }

  // needs to be called once per frame
  nsResourceManager::PerFrameUpdate();

  // tell the task system to finish its work for this frame
  // this has to be done at the very end, so that the task system will only use up the time that is left in this frame for
  // uploading GPU data etc.
  nsTaskSystem::FinishFrameTasks();
}

void nsOffscreenRendererTest::OnPresent(nsUInt32 uiCurrentTexture, nsUInt64 uiCurrentSemaphoreValue)
{
  nsStringBuilder sTemp;
  sTemp.SetFormat("Response {}|{}", uiCurrentTexture, uiCurrentSemaphoreValue);
  NS_PROFILE_SCOPE(sTemp);

  nsOffscreenTest_RenderResponseMsg msg = {};
  msg.m_Texture.m_uiCurrentSemaphoreValue = uiCurrentSemaphoreValue;
  msg.m_Texture.m_uiCurrentTextureIndex = uiCurrentTexture;
  m_pProtocol->Send(&msg);
}

void nsOffscreenRendererTest::AfterCoreSystemsStartup()
{
  SUPER::AfterCoreSystemsStartup();

  nsGraphicsTest::CreateRenderer(m_pDevice).AssertSuccess();

  nsGlobalLog::AddLogWriter(nsLoggingEvent::Handler(&nsLogWriter::HTML::LogMessageHandler, &m_LogHTML));
  nsStringBuilder sLogFile;
  sLogFile.SetFormat(":imgout/OffscreenLog.htm");
  m_LogHTML.BeginLog(sLogFile, "OffscreenRenderer"_nssv);

  // Setup Shaders and Materials
  {
    m_hScreenShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/UVColor.nsShader");
  }

  if (nsCommandLineUtils::GetGlobalInstance()->GetStringOption("-IPC").IsEmpty())
  {
    NS_REPORT_FAILURE("Command Line does not contain -IPC parameter");
    SetReturnCode(-1);
    RequestApplicationQuit();
    return;
  }

  if (nsCommandLineUtils::GetGlobalInstance()->GetStringOption("-PID").IsEmpty())
  {
    NS_REPORT_FAILURE("Command Line does not contain -PID parameter");
    SetReturnCode(-2);
    RequestApplicationQuit();
    return;
  }

  m_iHostPID = 0;
  if (nsConversionUtils::StringToInt64(nsCommandLineUtils::GetGlobalInstance()->GetStringOption("-PID"), m_iHostPID).Failed())
  {
    NS_REPORT_FAILURE("Command Line -PID parameter could not be converted to int");
    SetReturnCode(-3);
    RequestApplicationQuit();
    return;
  }

  nsLog::Debug("Host Process ID: {0}", m_iHostPID);

  m_pChannel = nsIpcChannel::CreatePipeChannel(nsCommandLineUtils::GetGlobalInstance()->GetStringOption("-IPC"), nsIpcChannel::Mode::Client);
  m_pProtocol = NS_DEFAULT_NEW(nsIpcProcessMessageProtocol, m_pChannel.Borrow());
  m_pProtocol->m_MessageEvent.AddEventHandler(nsMakeDelegate(&nsOffscreenRendererTest::MessageFunc, this));
  m_pChannel->Connect();

  while (!m_pChannel->IsConnected())
  {
    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(16));
  }

  nsStartup::StartupHighLevelSystems();
}

void nsOffscreenRendererTest::BeforeHighLevelSystemsShutdown()
{
  nsStringView sPath = ":imgout/Profiling/offscreenProfiling.json"_nssv;
  NS_TEST_RESULT(nsProfilingUtils::SaveProfilingCapture(sPath));

  auto pDevice = nsGALDevice::GetDefaultDevice();

  pDevice->DestroySwapChain(m_hSwapChain);
  // This guarantees that when the process exits no shared textures are still being modified by the GPU so the main process can savely delete the resources.
  pDevice->WaitIdle();

  m_hSwapChain.Invalidate();

  m_hScreenShader.Invalidate();

  m_pProtocol = nullptr;
  m_pChannel = nullptr;

  nsGlobalLog::RemoveLogWriter(nsLoggingEvent::Handler(&nsLogWriter::HTML::LogMessageHandler, &m_LogHTML));
  m_LogHTML.EndLog();

  SUPER::BeforeHighLevelSystemsShutdown();
}

void nsOffscreenRendererTest::BeforeCoreSystemsShutdown()
{
  nsResourceManager::FreeAllUnusedResources();

  if (m_pDevice)
  {
    m_pDevice->Shutdown().IgnoreResult();
    NS_DEFAULT_DELETE(m_pDevice);
  }

  SUPER::BeforeCoreSystemsShutdown();
}

void nsOffscreenRendererTest::MessageFunc(const nsProcessMessage* pMsg)
{
  if (const auto* pAction = nsDynamicCast<const nsOffscreenTest_OpenMsg*>(pMsg))
  {
    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
    NS_ASSERT_DEBUG(m_hSwapChain.IsInvalidated(), "SwapChain creation should only happen once");

    nsGALSharedTextureSwapChainCreationDescription desc;
    desc.m_TextureDesc = pAction->m_TextureDesc;
    desc.m_Textures = pAction->m_TextureHandles;
    desc.m_OnPresent = nsMakeDelegate(&nsOffscreenRendererTest::OnPresent, this);

    m_hSwapChain = nsGALSharedTextureSwapChain::Create(desc);
    if (m_hSwapChain.IsInvalidated())
    {
      NS_REPORT_FAILURE("Failed to create shared texture swapchain");
      SetReturnCode(-4);
      RequestApplicationQuit();
    }
  }
  else if (const auto* pAction = nsDynamicCast<const nsOffscreenTest_CloseMsg*>(pMsg))
  {
    m_bExiting = true;
  }
  else if (const auto* pAction = nsDynamicCast<const nsOffscreenTest_RenderMsg*>(pMsg))
  {
    NS_ASSERT_DEBUG(m_bExiting == false, "No new frame requests should come in at this point.");
    m_RequestedFrames.PushBack(*pAction);
  }
}
