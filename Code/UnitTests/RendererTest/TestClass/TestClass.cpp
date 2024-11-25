#include <RendererTest/RendererTestPCH.h>

#include "TestClass.h"
#include <Core/ResourceManager/ResourceManager.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <RendererFoundation/Device/DeviceFactory.h>
#include <RendererFoundation/Device/SwapChain.h>
#include <RendererFoundation/Resources/Texture.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>
#include <Texture/Image/ImageUtils.h>

nsGraphicsTest::nsGraphicsTest() = default;


nsResult nsGraphicsTest::InitializeTest()
{
  return NS_SUCCESS;
}

nsResult nsGraphicsTest::DeInitializeTest()
{
  return NS_SUCCESS;
}

nsResult nsGraphicsTest::InitializeSubTest(nsInt32 iIdentifier)
{
  m_iFrame = -1;
  m_bCaptureImage = false;
  m_ImgCompFrames.Clear();

  // initialize everything up to 'core'
  nsStartup::StartupCoreSystems();

  if (SetupRenderer().Failed())
    return NS_FAILURE;
  return NS_SUCCESS;
}

nsResult nsGraphicsTest::DeInitializeSubTest(nsInt32 iIdentifier)
{
  m_Readback.Reset();
  ShutdownRenderer();
  // shut down completely
  nsStartup::ShutdownCoreSystems();
  nsMemoryTracker::DumpMemoryLeaks();
  return NS_SUCCESS;
}

nsSizeU32 nsGraphicsTest::GetResolution() const
{
  return m_pWindow->GetClientAreaSize();
}


nsResult nsGraphicsTest::CreateRenderer(nsGALDevice*& out_pDevice)
{
  {
    nsFileSystem::SetSpecialDirectory("testout", nsTestFramework::GetInstance()->GetAbsOutputPath());

    nsStringBuilder sBaseDir = ">sdk/Data/Base/";
    nsStringBuilder sReadDir(">sdk/", nsTestFramework::GetInstance()->GetRelTestDataPath());
    sReadDir.PathParentDirectory();

    NS_SUCCEED_OR_RETURN(nsFileSystem::AddDataDirectory(">sdk/Output/", "ShaderCache", "shadercache", nsDataDirUsage::AllowWrites)); // for shader files

    NS_SUCCEED_OR_RETURN(nsFileSystem::AddDataDirectory(sBaseDir, "Base"));

    NS_SUCCEED_OR_RETURN(nsFileSystem::AddDataDirectory(">nstest/", "ImageComparisonDataDir", "imgout", nsDataDirUsage::AllowWrites));

    NS_SUCCEED_OR_RETURN(nsFileSystem::AddDataDirectory(sReadDir, "UnitTestData"));

    sReadDir.Set(">sdk/", nsTestFramework::GetInstance()->GetRelTestDataPath());
    NS_SUCCEED_OR_RETURN(nsFileSystem::AddDataDirectory(sReadDir, "ImageComparisonDataDir"));
  }

#ifdef BUILDSYSTEM_ENABLE_VULKAN_SUPPORT
  constexpr const char* szDefaultRenderer = "Vulkan";
#else
  constexpr const char* szDefaultRenderer = "DX11";
#endif

  nsStringView sRendererName = nsCommandLineUtils::GetGlobalInstance()->GetStringOption("-renderer", 0, szDefaultRenderer);
  const char* szShaderModel = "";
  const char* szShaderCompiler = "";
  nsGALDeviceFactory::GetShaderModelAndCompiler(sRendererName, szShaderModel, szShaderCompiler);

  nsShaderManager::Configure(szShaderModel, true);
  if (nsPlugin::LoadPlugin(szShaderCompiler).Failed())
    nsLog::Warning("Shader compiler '{}' plugin not found", szShaderCompiler);

  // Create a device
  {
    nsGALDeviceCreationDescription DeviceInit;
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    DeviceInit.m_bDebugDevice = true;
#endif
    out_pDevice = nsGALDeviceFactory::CreateDevice(sRendererName, nsFoundation::GetDefaultAllocator(), DeviceInit);
    if (out_pDevice->Init().Failed())
      return NS_FAILURE;

    nsGALDevice::SetDefaultDevice(out_pDevice);
  }

  if (sRendererName.IsEqual_NoCase("DX11"))
  {
    if (out_pDevice->GetCapabilities().m_sAdapterName == "Microsoft Basic Render Driver" || out_pDevice->GetCapabilities().m_sAdapterName.StartsWith_NoCase("Intel(R) UHD Graphics"))
    {
      // Use different images for comparison when running the D3D11 Reference Device
      nsTestFramework::GetInstance()->SetImageReferenceOverrideFolderName("Images_Reference_D3D11Ref");
    }
    else if (out_pDevice->GetCapabilities().m_sAdapterName.FindSubString_NoCase("AMD") || out_pDevice->GetCapabilities().m_sAdapterName.FindSubString_NoCase("Radeon"))
    {
      // Line rendering is different on AMD and requires separate images for tests rendering lines.
      nsTestFramework::GetInstance()->SetImageReferenceOverrideFolderName("Images_Reference_D3D11AMD");
    }
    else if (out_pDevice->GetCapabilities().m_sAdapterName.FindSubString_NoCase("Nvidia") || out_pDevice->GetCapabilities().m_sAdapterName.FindSubString_NoCase("GeForce"))
    {
      // Line rendering is different on Nvidia and requires separate images for tests rendering lines.
      nsTestFramework::GetInstance()->SetImageReferenceOverrideFolderName("Images_Reference_D3D11Nvidia");
    }
    else
    {
      nsTestFramework::GetInstance()->SetImageReferenceOverrideFolderName("");
    }
  }
  else if (sRendererName.IsEqual_NoCase("Vulkan"))
  {
    if (out_pDevice->GetCapabilities().m_sAdapterName.FindSubString_NoCase("llvmpipe"))
    {
      nsTestFramework::GetInstance()->SetImageReferenceOverrideFolderName("Images_Reference_LLVMPIPE");
    }
    else if (out_pDevice->GetCapabilities().m_sAdapterName.FindSubString_NoCase("SwiftShader"))
    {
      nsTestFramework::GetInstance()->SetImageReferenceOverrideFolderName("Images_Reference_SwiftShader");
    }
    else
    {
      nsTestFramework::GetInstance()->SetImageReferenceOverrideFolderName("Images_Reference_Vulkan");
    }
  }
  return NS_SUCCESS;
}

nsResult nsGraphicsTest::SetupRenderer()
{
  NS_SUCCEED_OR_RETURN(nsGraphicsTest::CreateRenderer(m_pDevice));

  m_hObjectTransformCB = nsRenderContext::CreateConstantBufferStorage<ObjectCB>();
  m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Default.nsShader");

  {
    // Unit cube mesh
    nsGeometry geom;
    geom.AddBox(nsVec3(1.0f), true);

    nsGALPrimitiveTopology::Enum Topology = nsGALPrimitiveTopology::Triangles;
    nsMeshBufferResourceDescriptor desc;
    desc.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
    desc.AddStream(nsGALVertexAttributeSemantic::TexCoord0, nsGALResourceFormat::RGFloat);
    desc.AllocateStreamsFromGeometry(geom, Topology);

    m_hCubeUV = nsResourceManager::GetOrCreateResource<nsMeshBufferResource>("Texture2DBox", std::move(desc), "Texture2DBox");
  }

  nsStartup::StartupHighLevelSystems();
  return NS_SUCCESS;
}

void nsGraphicsTest::ShutdownRenderer()
{
  m_Readback.Reset();
  NS_ASSERT_DEV(m_pWindow == nullptr, "DestroyWindow needs to be called before ShutdownRenderer");
  m_hShader.Invalidate();
  m_hCubeUV.Invalidate();

  nsRenderContext::DeleteConstantBufferStorage(m_hObjectTransformCB);
  m_hObjectTransformCB.Invalidate();

  nsStartup::ShutdownHighLevelSystems();

  nsResourceManager::FreeAllUnusedResources();

  if (m_pDevice)
  {
    m_pDevice->Shutdown().IgnoreResult();
    NS_DEFAULT_DELETE(m_pDevice);
  }

  nsFileSystem::RemoveDataDirectoryGroup("ImageComparisonDataDir");
}

nsResult nsGraphicsTest::CreateWindow(nsUInt32 uiResolutionX, nsUInt32 uiResolutionY)
{
  // Create a window for rendering
  {
    nsWindowCreationDesc WindowCreationDesc;
    WindowCreationDesc.m_Resolution.width = uiResolutionX;
    WindowCreationDesc.m_Resolution.height = uiResolutionY;
    WindowCreationDesc.m_bShowMouseCursor = true;
    WindowCreationDesc.m_bClipMouseCursor = false;
    WindowCreationDesc.m_bSetForegroundOnInit = false;

    m_pWindow = NS_DEFAULT_NEW(nsWindow);
    if (m_pWindow->Initialize(WindowCreationDesc).Failed())
      return NS_FAILURE;
  }

  // Create a Swapchain
  {
    nsGALWindowSwapChainCreationDescription swapChainDesc;
    swapChainDesc.m_pWindow = m_pWindow;
    swapChainDesc.m_SampleCount = nsGALMSAASampleCount::None;
    m_hSwapChain = nsGALWindowSwapChain::Create(swapChainDesc);
    if (m_hSwapChain.IsInvalidated())
    {
      return NS_FAILURE;
    }
  }

  {
    nsGALTextureCreationDescription texDesc;
    texDesc.m_uiWidth = uiResolutionX;
    texDesc.m_uiHeight = uiResolutionY;
    texDesc.m_Format = nsGALResourceFormat::D24S8;
    texDesc.m_bAllowRenderTargetView = true;

    m_hDepthStencilTexture = m_pDevice->CreateTexture(texDesc);
    if (m_hDepthStencilTexture.IsInvalidated())
    {
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

void nsGraphicsTest::DestroyWindow()
{
  if (m_pDevice)
  {
    if (!m_hSwapChain.IsInvalidated())
    {
      m_pDevice->DestroySwapChain(m_hSwapChain);
      m_hSwapChain.Invalidate();
    }

    if (!m_hDepthStencilTexture.IsInvalidated())
    {
      m_pDevice->DestroyTexture(m_hDepthStencilTexture);
      m_hDepthStencilTexture.Invalidate();
    }
    m_pDevice->WaitIdle();
  }


  if (m_pWindow)
  {
    m_pWindow->DestroyWindow();
    NS_DEFAULT_DELETE(m_pWindow);
  }
}

void nsGraphicsTest::BeginFrame()
{
  m_pDevice->EnqueueFrameSwapChain(m_hSwapChain);
  m_pDevice->BeginFrame(m_iFrame);
}

void nsGraphicsTest::EndFrame()
{
  m_pWindow->ProcessWindowMessages();

  nsRenderContext::GetDefaultInstance()->ResetContextState();

  m_pDevice->EndFrame();

  nsTaskSystem::FinishFrameTasks();
}


void nsGraphicsTest::BeginCommands(const char* szPassName)
{
  NS_ASSERT_DEV(m_pEncoder == nullptr, "Call EndCommands first before calling BeginCommands again");
  m_pEncoder = m_pDevice->BeginCommands(szPassName);
}


void nsGraphicsTest::EndCommands()
{
  NS_ASSERT_DEV(m_pEncoder != nullptr, "Call BeginCommands first before calling EndCommands");
  m_pDevice->EndCommands(m_pEncoder);
  m_pEncoder = nullptr;
}

nsGALCommandEncoder* nsGraphicsTest::BeginRendering(nsColor clearColor, nsUInt32 uiRenderTargetClearMask, nsRectFloat* pViewport, nsRectU32* pScissor)
{
  const nsGALSwapChain* pPrimarySwapChain = m_pDevice->GetSwapChain(m_hSwapChain);

  nsGALRenderingSetup renderingSetup;
  renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, m_pDevice->GetDefaultRenderTargetView(pPrimarySwapChain->GetBackBufferTexture()));
  renderingSetup.m_ClearColor = clearColor;
  renderingSetup.m_uiRenderTargetClearMask = uiRenderTargetClearMask;
  if (!m_hDepthStencilTexture.IsInvalidated())
  {
    renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(m_pDevice->GetDefaultRenderTargetView(m_hDepthStencilTexture));
    renderingSetup.m_bClearDepth = true;
    renderingSetup.m_bClearStencil = true;
  }
  nsRectFloat viewport = nsRectFloat(0.0f, 0.0f, (float)m_pWindow->GetClientAreaSize().width, (float)m_pWindow->GetClientAreaSize().height);
  if (pViewport)
  {
    viewport = *pViewport;
  }

  nsRenderContext::GetDefaultInstance()->BeginRendering(renderingSetup, viewport);
  nsRectU32 scissor = nsRectU32(0, 0, m_pWindow->GetClientAreaSize().width, m_pWindow->GetClientAreaSize().height);
  if (pScissor)
  {
    scissor = *pScissor;
  }

  auto pCommandEncoder = nsRenderContext::GetDefaultInstance()->GetCommandEncoder();
  pCommandEncoder->SetScissorRect(scissor);

  SetClipSpace();
  return pCommandEncoder;
}

void nsGraphicsTest::EndRendering()
{
  nsRenderContext::GetDefaultInstance()->EndRendering();
  m_pWindow->ProcessWindowMessages();
}

void nsGraphicsTest::SetClipSpace()
{
  static nsHashedString sClipSpaceFlipped = nsMakeHashedString("CLIP_SPACE_FLIPPED");
  static nsHashedString sTrue = nsMakeHashedString("TRUE");
  static nsHashedString sFalse = nsMakeHashedString("FALSE");
  nsClipSpaceYMode::Enum clipSpace = nsClipSpaceYMode::RenderToTextureDefault;
  nsRenderContext::GetDefaultInstance()->SetShaderPermutationVariable(sClipSpaceFlipped, clipSpace == nsClipSpaceYMode::Flipped ? sTrue : sFalse);
}

void nsGraphicsTest::RenderCube(nsRectFloat viewport, nsMat4 mMVP, nsUInt32 uiRenderTargetClearMask, nsGALTextureResourceViewHandle hSRV)
{
  nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::RebeccaPurple, uiRenderTargetClearMask, &viewport);

  nsRenderContext::GetDefaultInstance()->BindTexture2D("DiffuseTexture", hSRV);
  RenderObject(m_hCubeUV, mMVP, nsColor(1, 1, 1, 1), nsShaderBindFlags::None);
  EndRendering();
  if (m_bCaptureImage && m_ImgCompFrames.Contains(m_iFrame))
  {
    NS_TEST_IMAGE(m_iFrame, 100);
  }
};


nsMat4 nsGraphicsTest::CreateSimpleMVP(float fAspectRatio)
{
  nsCamera cam;
  cam.SetCameraMode(nsCameraMode::PerspectiveFixedFovX, 90, 0.5f, 1000.0f);
  cam.LookAt(nsVec3(0, 0, 0), nsVec3(0, 0, -1), nsVec3(0, 0, 1));
  nsMat4 mProj;
  cam.GetProjectionMatrix(fAspectRatio, mProj);
  nsMat4 mView = cam.GetViewMatrix();

  nsMat4 mTransform = nsMat4::MakeTranslation(nsVec3(0.0f, 0.0f, -1.2f));
  return mProj * mView * mTransform;
}

nsResult nsGraphicsTest::GetImage(nsImage& ref_img, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber)
{
  auto pCommandEncoder = nsRenderContext::GetDefaultInstance()->GetCommandEncoder();

  nsGALTextureHandle hBBTexture = m_pDevice->GetSwapChain(m_hSwapChain)->GetBackBufferTexture();
  const nsGALTexture* pBackbuffer = nsGALDevice::GetDefaultDevice()->GetTexture(hBBTexture);
  m_Readback.ReadbackTexture(*pCommandEncoder, hBBTexture);
  pCommandEncoder->Flush();
  // Wait for results
  {
    nsEnum<nsGALAsyncResult> res = m_Readback.GetReadbackResult(nsTime::MakeFromHours(1));
    NS_ASSERT_ALWAYS(res == nsGALAsyncResult::Ready, "Readback of texture failed");
  }

  nsGALTextureSubresource sourceSubResource;
  nsArrayPtr<nsGALTextureSubresource> sourceSubResources(&sourceSubResource, 1);
  nsHybridArray<nsGALSystemMemoryDescription, 1> memory;
  nsReadbackTextureLock lock = m_Readback.LockTexture(sourceSubResources, memory);
  NS_ASSERT_ALWAYS(lock, "Failed to lock readback texture");
  nsTextureUtils::CopySubResourceToImage(pBackbuffer->GetDescription(), sourceSubResource, memory[0], ref_img, true);

  return NS_SUCCESS;
}

nsMeshBufferResourceHandle nsGraphicsTest::CreateMesh(const nsGeometry& geom, const char* szResourceName)
{
  nsMeshBufferResourceHandle hMesh;
  hMesh = nsResourceManager::GetExistingResource<nsMeshBufferResource>(szResourceName);

  if (hMesh.IsValid())
    return hMesh;

  nsGALPrimitiveTopology::Enum Topology = nsGALPrimitiveTopology::Triangles;
  if (geom.GetLines().GetCount() > 0)
    Topology = nsGALPrimitiveTopology::Lines;

  nsMeshBufferResourceDescriptor desc;
  desc.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
  desc.AddStream(nsGALVertexAttributeSemantic::Color0, nsGALResourceFormat::RGBAUByteNormalized);
  desc.AllocateStreamsFromGeometry(geom, Topology);

  hMesh = nsResourceManager::GetOrCreateResource<nsMeshBufferResource>(szResourceName, std::move(desc), szResourceName);

  return hMesh;
}

nsMeshBufferResourceHandle nsGraphicsTest::CreateSphere(nsInt32 iSubDivs, float fRadius)
{
  nsGeometry geom;
  geom.AddGeodesicSphere(fRadius, static_cast<nsUInt8>(iSubDivs));

  nsStringBuilder sName;
  sName.SetFormat("Sphere_{0}", iSubDivs);

  return CreateMesh(geom, sName);
}

nsMeshBufferResourceHandle nsGraphicsTest::CreateTorus(nsInt32 iSubDivs, float fInnerRadius, float fOuterRadius)
{
  nsGeometry geom;
  geom.AddTorus(fInnerRadius, fOuterRadius, static_cast<nsUInt16>(iSubDivs), static_cast<nsUInt16>(iSubDivs), true);

  nsStringBuilder sName;
  sName.SetFormat("Torus_{0}", iSubDivs);

  return CreateMesh(geom, sName);
}

nsMeshBufferResourceHandle nsGraphicsTest::CreateBox(float fWidth, float fHeight, float fDepth)
{
  nsGeometry geom;
  geom.AddBox(nsVec3(fWidth, fHeight, fDepth), false);

  nsStringBuilder sName;
  sName.SetFormat("Box_{0}_{1}_{2}", nsArgF(fWidth, 1), nsArgF(fHeight, 1), nsArgF(fDepth, 1));

  return CreateMesh(geom, sName);
}

nsMeshBufferResourceHandle nsGraphicsTest::CreateLineBox(float fWidth, float fHeight, float fDepth)
{
  nsGeometry geom;
  geom.AddLineBox(nsVec3(fWidth, fHeight, fDepth));

  nsStringBuilder sName;
  sName.SetFormat("LineBox_{0}_{1}_{2}", nsArgF(fWidth, 1), nsArgF(fHeight, 1), nsArgF(fDepth, 1));

  return CreateMesh(geom, sName);
}

void nsGraphicsTest::RenderObject(nsMeshBufferResourceHandle hObject, const nsMat4& mTransform, const nsColor& color, nsBitflags<nsShaderBindFlags> ShaderBindFlags)
{
  nsRenderContext::GetDefaultInstance()->BindShader(m_hShader, ShaderBindFlags);

  ObjectCB* ocb = nsRenderContext::GetConstantBufferData<ObjectCB>(m_hObjectTransformCB);
  ocb->m_MVP = mTransform;
  ocb->m_Color = color;

  nsRenderContext::GetDefaultInstance()->BindConstantBuffer("PerObject", m_hObjectTransformCB);

  nsRenderContext::GetDefaultInstance()->BindMeshBuffer(hObject);
  nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().IgnoreResult();
}
