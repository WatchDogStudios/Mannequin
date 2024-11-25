#include <RendererTest/RendererTestPCH.h>

#include <Core/GameState/GameStateWindow.h>
#include <Core/Graphics/Camera.h>
#include <Foundation/Math/ColorScheme.h>
#include <Foundation/Threading/ThreadUtils.h>
#include <RendererTest/Basics/PipelineStates.h>

#include <RendererTest/../../../Data/UnitTests/RendererTest/Shaders/TestConstants.h>
#include <RendererTest/../../../Data/UnitTests/RendererTest/Shaders/TestInstancing.h>
#include <RendererTest/../../../Data/UnitTests/RendererTest/Shaders/TestPushConstants.h>

NS_DEFINE_AS_POD_TYPE(nsTestShaderData);

namespace
{
  nsTransform CreateTransform(const nsUInt32 uiColumns, const nsUInt32 uiRows, nsUInt32 x, nsUInt32 y)
  {
    nsTransform t = nsTransform::MakeIdentity();
    t.m_vScale = nsVec3(1.0f / float(uiColumns), 1.0f / float(uiRows), 1);
    t.m_vPosition = nsVec3(nsMath::Lerp(-1.f, 1.f, (float(x) + 0.5f) / float(uiColumns)), nsMath::Lerp(1.f, -1.f, (float(y) + 0.5f) / float(uiRows)), 0);
    if (nsClipSpaceYMode::RenderToTextureDefault == nsClipSpaceYMode::Flipped)
    {
      nsTransform flipY = nsTransform::MakeIdentity();
      flipY.m_vScale.y *= -1.0f;
      t = flipY * t;
    }
    return t;
  }

  void FillStructuredBuffer(nsHybridArray<nsTestShaderData, 16>& ref_instanceData, nsUInt32 uiColorOffset = 0, nsUInt32 uiSlotOffset = 0)
  {
    ref_instanceData.SetCount(16);
    const nsUInt32 uiColumns = 4;
    const nsUInt32 uiRows = 2;

    for (nsUInt32 x = 0; x < uiColumns; ++x)
    {
      for (nsUInt32 y = 0; y < uiRows; ++y)
      {
        nsTestShaderData& instance = ref_instanceData[uiSlotOffset + x * uiRows + y];
        const float fColorIndex = float(uiColorOffset + x * uiRows + y) / 32.0f;
        instance.InstanceColor = nsColorScheme::LightUI(fColorIndex).GetAsVec4();
        nsTransform t = CreateTransform(uiColumns, uiRows, x, y);
        instance.InstanceTransform = t;
      }
    }
  }

  struct ImgColor
  {
    NS_DECLARE_POD_TYPE();
    nsUInt8 b;
    nsUInt8 g;
    nsUInt8 r;
    nsUInt8 a;
  };

  void CreateImage(nsImage& ref_image, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiMipLevelCount, bool bMipLevelIsBlue, nsUInt8 uiFixedBlue = 0)
  {
    nsImageHeader header;
    header.SetImageFormat(nsImageFormat::B8G8R8A8_UNORM_SRGB);
    header.SetWidth(uiWidth);
    header.SetHeight(uiHeight);
    header.SetNumMipLevels(uiMipLevelCount);

    ref_image.ResetAndAlloc(header);
    for (nsUInt32 m = 0; m < uiMipLevelCount; m++)
    {
      const nsUInt32 uiHeight = ref_image.GetHeight(m);
      const nsUInt32 uiWidth = ref_image.GetWidth(m);

      const nsUInt8 uiBlue = bMipLevelIsBlue ? static_cast<nsUInt8>(255.0f * float(m) / (uiMipLevelCount - 1)) : uiFixedBlue;
      for (nsUInt32 y = 0; y < uiHeight; y++)
      {
        const nsUInt8 uiGreen = static_cast<nsUInt8>(255.0f * float(y) / (uiHeight - 1));
        for (nsUInt32 x = 0; x < uiWidth; x++)
        {
          ImgColor* pColor = ref_image.GetPixelPointer<ImgColor>(m, 0u, 0u, x, y);
          pColor->a = 255;
          pColor->b = uiBlue;
          pColor->g = uiGreen;
          pColor->r = static_cast<nsUInt8>(255.0f * float(x) / (uiWidth - 1));
        }
      }
    }
  }


} // namespace

nsResult nsRendererTestPipelineStates::InitializeSubTest(nsInt32 iIdentifier)
{
  {
    m_iDelay = 0;
    m_CPUTime[0] = {};
    m_CPUTime[1] = {};
    m_GPUTime[0] = {};
    m_GPUTime[1] = {};
    m_timestamps[0] = {};
    m_timestamps[1] = {};
    m_queries[0] = {};
    m_queries[1] = {};
    m_queries[2] = {};
    m_queries[3] = {};
    m_hFence = {};
  }

  NS_SUCCEED_OR_RETURN(nsGraphicsTest::InitializeSubTest(iIdentifier));
  NS_SUCCEED_OR_RETURN(CreateWindow(320, 240));
  m_hMostBasicTriangleShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/MostBasicTriangle.nsShader");
  m_hNDCPositionOnlyShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/NDCPositionOnly.nsShader");
  m_hConstantBufferShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/ConstantBuffer.nsShader");
  m_hPushConstantsShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/PushConstants.nsShader");
  m_hInstancingShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Instancing.nsShader");

  {
    nsMeshBufferResourceDescriptor desc;
    desc.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
    desc.AllocateStreams(3);

    if (nsClipSpaceYMode::RenderToTextureDefault == nsClipSpaceYMode::Flipped)
    {
      desc.SetVertexData<nsVec3>(0, 0, nsVec3(1.f, 1.f, 0.0f));
      desc.SetVertexData<nsVec3>(0, 1, nsVec3(-1.f, 1.f, 0.0f));
      desc.SetVertexData<nsVec3>(0, 2, nsVec3(0.f, -1.f, 0.0f));
    }
    else
    {
      desc.SetVertexData<nsVec3>(0, 0, nsVec3(1.f, -1.f, 0.0f));
      desc.SetVertexData<nsVec3>(0, 1, nsVec3(-1.f, -1.f, 0.0f));
      desc.SetVertexData<nsVec3>(0, 2, nsVec3(0.f, 1.f, 0.0f));
    }

    m_hTriangleMesh = nsResourceManager::CreateResource<nsMeshBufferResource>("UnitTest-TriangleMesh", std::move(desc), "TriangleMesh");
  }
  {
    nsGeometry geom;
    geom.AddStackedSphere(0.5f, 16, 16);

    nsMeshBufferResourceDescriptor desc;
    desc.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
    desc.AllocateStreamsFromGeometry(geom, nsGALPrimitiveTopology::Triangles);

    m_hSphereMesh = nsResourceManager::CreateResource<nsMeshBufferResource>("UnitTest-SphereMesh", std::move(desc), "SphereMesh");
  }
  m_hTestPerFrameConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsTestPerFrame>();
  m_hTestColorsConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsTestColors>();
  m_hTestPositionsConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsTestPositions>();

  {
    nsGALBufferCreationDescription desc;
    desc.m_uiStructSize = sizeof(nsTestShaderData);
    desc.m_uiTotalSize = 16 * desc.m_uiStructSize;
    desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
    desc.m_ResourceAccess.m_bImmutable = false;

    // We only fill the first 8 elements with data. The rest is dynamically updated during testing.
    nsHybridArray<nsTestShaderData, 16> instanceData;
    FillStructuredBuffer(instanceData);
    m_hInstancingData = m_pDevice->CreateBuffer(desc, instanceData.GetByteArrayPtr());

    desc.m_BufferFlags |= nsGALBufferUsageFlags::Transient;
    m_hInstancingDataTransient = m_pDevice->CreateBuffer(desc);

    nsGALBufferResourceViewCreationDescription viewDesc;
    viewDesc.m_hBuffer = m_hInstancingData;
    viewDesc.m_uiFirstElement = 8;
    viewDesc.m_uiNumElements = 4;
    m_hInstancingDataView_8_4 = m_pDevice->CreateResourceView(viewDesc);
    viewDesc.m_uiFirstElement = 12;
    m_hInstancingDataView_12_4 = m_pDevice->CreateResourceView(viewDesc);
  }

  {
    // Texture2D
    nsGALTextureCreationDescription desc;
    desc.m_uiWidth = 8;
    desc.m_uiHeight = 8;
    desc.m_uiMipLevelCount = 4;
    desc.m_Format = nsGALResourceFormat::BGRAUByteNormalizedsRGB;

    nsImage coloredMips;
    CreateImage(coloredMips, desc.m_uiWidth, desc.m_uiHeight, desc.m_uiMipLevelCount, true);

    if (iIdentifier == SubTests::ST_GenerateMipMaps)
    {
      // Clear all mips except the fist one and let them be regenerated.
      desc.m_ResourceAccess.m_bImmutable = false;
      desc.m_bAllowDynamicMipGeneration = true;
      for (nsUInt32 m = 1; m < desc.m_uiMipLevelCount; m++)
      {
        const nsUInt32 uiHeight = coloredMips.GetHeight(m);
        const nsUInt32 uiWidth = coloredMips.GetWidth(m);
        for (nsUInt32 y = 0; y < uiHeight; y++)
        {
          for (nsUInt32 x = 0; x < uiWidth; x++)
          {
            ImgColor* pColor = coloredMips.GetPixelPointer<ImgColor>(m, 0u, 0u, x, y);
            pColor->a = 255;
            pColor->b = 0;
            pColor->g = 0;
            pColor->r = 0;
          }
        }
      }
    }

    nsHybridArray<nsGALSystemMemoryDescription, 4> initialData;
    initialData.SetCount(desc.m_uiMipLevelCount);
    for (nsUInt32 m = 0; m < desc.m_uiMipLevelCount; m++)
    {
      nsGALSystemMemoryDescription& memoryDesc = initialData[m];
      memoryDesc.m_pData = coloredMips.GetSubImageView(m).GetByteBlobPtr();
      memoryDesc.m_uiRowPitch = static_cast<nsUInt32>(coloredMips.GetRowPitch(m));
      memoryDesc.m_uiSlicePitch = static_cast<nsUInt32>(coloredMips.GetDepthPitch(m));
    }
    m_hTexture2D = m_pDevice->CreateTexture(desc, initialData);

    nsGALTextureResourceViewCreationDescription viewDesc;
    viewDesc.m_hTexture = m_hTexture2D;
    viewDesc.m_uiMostDetailedMipLevel = 0;
    viewDesc.m_uiMipLevelsToUse = 1;
    m_hTexture2D_Mip0 = m_pDevice->CreateResourceView(viewDesc);
    viewDesc.m_uiMostDetailedMipLevel = 1;
    m_hTexture2D_Mip1 = m_pDevice->CreateResourceView(viewDesc);
    viewDesc.m_uiMostDetailedMipLevel = 2;
    m_hTexture2D_Mip2 = m_pDevice->CreateResourceView(viewDesc);
    viewDesc.m_uiMostDetailedMipLevel = 3;
    m_hTexture2D_Mip3 = m_pDevice->CreateResourceView(viewDesc);
  }

  {
    // Texture2DArray
    nsGALTextureCreationDescription desc;
    desc.m_uiWidth = 8;
    desc.m_uiHeight = 8;
    desc.m_uiMipLevelCount = 4;
    desc.m_uiArraySize = 2;
    desc.m_Type = nsGALTextureType::Texture2DArray;
    desc.m_Format = nsGALResourceFormat::BGRAUByteNormalizedsRGB;

    nsImage coloredMips[2];
    CreateImage(coloredMips[0], desc.m_uiWidth, desc.m_uiHeight, desc.m_uiMipLevelCount, false, 0);
    CreateImage(coloredMips[1], desc.m_uiWidth, desc.m_uiHeight, desc.m_uiMipLevelCount, false, 255);

    nsHybridArray<nsGALSystemMemoryDescription, 8> initialData;
    initialData.SetCount(desc.m_uiArraySize * desc.m_uiMipLevelCount);
    for (nsUInt32 l = 0; l < desc.m_uiArraySize; l++)
    {
      for (nsUInt32 m = 0; m < desc.m_uiMipLevelCount; m++)
      {
        nsGALSystemMemoryDescription& memoryDesc = initialData[m + l * desc.m_uiMipLevelCount];

        memoryDesc.m_pData = coloredMips[l].GetSubImageView(m).GetByteBlobPtr();
        memoryDesc.m_uiRowPitch = static_cast<nsUInt32>(coloredMips[l].GetRowPitch(m));
        memoryDesc.m_uiSlicePitch = static_cast<nsUInt32>(coloredMips[l].GetDepthPitch(m));
      }
    }
    m_hTexture2DArray = m_pDevice->CreateTexture(desc, initialData);

    nsGALTextureResourceViewCreationDescription viewDesc;
    viewDesc.m_hTexture = m_hTexture2DArray;
    viewDesc.m_uiMipLevelsToUse = 1;
    viewDesc.m_uiFirstArraySlice = 0;
    viewDesc.m_uiMostDetailedMipLevel = 0;
    m_hTexture2DArray_Layer0_Mip0 = m_pDevice->CreateResourceView(viewDesc);
    viewDesc.m_uiMostDetailedMipLevel = 1;
    m_hTexture2DArray_Layer0_Mip1 = m_pDevice->CreateResourceView(viewDesc);

    viewDesc.m_uiFirstArraySlice = 1;
    viewDesc.m_uiMostDetailedMipLevel = 0;
    m_hTexture2DArray_Layer1_Mip0 = m_pDevice->CreateResourceView(viewDesc);
    viewDesc.m_uiMostDetailedMipLevel = 1;
    m_hTexture2DArray_Layer1_Mip1 = m_pDevice->CreateResourceView(viewDesc);
  }

  switch (iIdentifier)
  {
    case SubTests::ST_MostBasicShader:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_ViewportScissor:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_VertexBuffer:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_IndexBuffer:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_ConstantBuffer:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_StructuredBuffer:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::StructuredBuffer_InitialData);
      m_ImgCompFrames.PushBack(ImageCaptureFrames::StructuredBuffer_CopyToTempStorage);
      m_ImgCompFrames.PushBack(ImageCaptureFrames::StructuredBuffer_CopyToTempStorage2);
      m_ImgCompFrames.PushBack(ImageCaptureFrames::StructuredBuffer_Transient1);
      m_ImgCompFrames.PushBack(ImageCaptureFrames::StructuredBuffer_Transient2);
      break;
    case SubTests::ST_GenerateMipMaps:
    case SubTests::ST_Texture2D:
    {
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2D.nsShader");
    }
    break;
    case SubTests::ST_Texture2DArray:
    {
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2DArray.nsShader");
    }
    break;
    case SubTests::ST_PushConstants:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      break;
    case SubTests::ST_SetsSlots:
      m_ImgCompFrames.PushBack(ImageCaptureFrames::DefaultCapture);
      m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/SetsSlots.nsShader");
      break;
    case SubTests::ST_Timestamps:
    case SubTests::ST_OcclusionQueries:
      break;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  return NS_SUCCESS;
}

nsResult nsRendererTestPipelineStates::DeInitializeSubTest(nsInt32 iIdentifier)
{
  m_hTriangleMesh.Invalidate();
  m_hSphereMesh.Invalidate();

  m_hMostBasicTriangleShader.Invalidate();
  m_hNDCPositionOnlyShader.Invalidate();
  m_hConstantBufferShader.Invalidate();
  m_hPushConstantsShader.Invalidate();
  m_hInstancingShader.Invalidate();

  m_hTestPerFrameConstantBuffer.Invalidate();
  m_hTestColorsConstantBuffer.Invalidate();
  m_hTestPositionsConstantBuffer.Invalidate();

  if (!m_hInstancingData.IsInvalidated())
  {
    m_pDevice->DestroyBuffer(m_hInstancingData);
    m_hInstancingData.Invalidate();
  }
  if (!m_hInstancingDataTransient.IsInvalidated())
  {
    m_pDevice->DestroyBuffer(m_hInstancingDataTransient);
    m_hInstancingDataTransient.Invalidate();
  }
  m_hInstancingDataView_8_4.Invalidate();
  m_hInstancingDataView_12_4.Invalidate();

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
  m_hTexture2D_Mip0.Invalidate();
  m_hTexture2D_Mip1.Invalidate();
  m_hTexture2D_Mip2.Invalidate();
  m_hTexture2D_Mip3.Invalidate();
  m_hShader.Invalidate();

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_queries); i++)
  {
    m_queries[i] = {};
  }
  m_hFence = {};

  DestroyWindow();
  NS_SUCCEED_OR_RETURN(nsGraphicsTest::DeInitializeSubTest(iIdentifier));
  return NS_SUCCESS;
}

nsTestAppRun nsRendererTestPipelineStates::RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount)
{
  m_iFrame = uiInvocationCount;
  m_bCaptureImage = false;
  BeginFrame();

  switch (iIdentifier)
  {
    case SubTests::ST_MostBasicShader:
      MostBasicTriangleTest();
      break;
    case SubTests::ST_ViewportScissor:
      ViewportScissorTest();
      break;
    case SubTests::ST_VertexBuffer:
      VertexBufferTest();
      break;
    case SubTests::ST_IndexBuffer:
      IndexBufferTest();
      break;
    case SubTests::ST_ConstantBuffer:
      ConstantBufferTest();
      break;
    case SubTests::ST_StructuredBuffer:
      StructuredBufferTest();
      break;
    case SubTests::ST_Texture2D:
      Texture2D();
      break;
    case SubTests::ST_Texture2DArray:
      Texture2DArray();
      break;
    case SubTests::ST_GenerateMipMaps:
      GenerateMipMaps();
      break;
    case SubTests::ST_PushConstants:
      PushConstantsTest();
      break;
    case SubTests::ST_SetsSlots:
      SetsSlotsTest();
      break;
    case SubTests::ST_Timestamps:
    {
      auto res = Timestamps();
      EndFrame();
      return res;
    }
    break;
    case SubTests::ST_OcclusionQueries:
    {
      auto res = OcclusionQueries();
      EndFrame();
      return res;
    }
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

void nsRendererTestPipelineStates::RenderBlock(nsMeshBufferResourceHandle mesh, nsColor clearColor, nsUInt32 uiRenderTargetClearMask, nsRectFloat* pViewport, nsRectU32* pScissor)
{
  BeginCommands("MostBasicTriangle");
  {
    nsGALCommandEncoder* pCommandEncoder = BeginRendering(clearColor, uiRenderTargetClearMask, pViewport, pScissor);
    {

      if (mesh.IsValid())
      {
        nsRenderContext::GetDefaultInstance()->BindShader(m_hNDCPositionOnlyShader);
        nsRenderContext::GetDefaultInstance()->BindMeshBuffer(mesh);
        nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();
      }
      else
      {
        nsRenderContext::GetDefaultInstance()->BindShader(m_hMostBasicTriangleShader);
        nsRenderContext::GetDefaultInstance()->BindNullMeshBuffer(nsGALPrimitiveTopology::Triangles, 1);
        nsRenderContext::GetDefaultInstance()->DrawMeshBuffer(1).AssertSuccess();
      }
    }

    EndRendering();
    if (m_bCaptureImage && m_ImgCompFrames.Contains(m_iFrame))
    {
      NS_TEST_IMAGE(m_iFrame, 100);
    }
  }

  EndCommands();
}



void nsRendererTestPipelineStates::MostBasicTriangleTest()
{
  m_bCaptureImage = true;
  RenderBlock({}, nsColor::RebeccaPurple);
}

void nsRendererTestPipelineStates::ViewportScissorTest()
{
  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsUInt32 uiColumns = 2;
  const nsUInt32 uiRows = 2;
  const float fElementWidth = fWidth / uiColumns;
  const float fElementHeight = fHeight / uiRows;

  nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
  RenderBlock({}, nsColor::CornflowerBlue, 0xFFFFFFFF, &viewport);

  viewport = nsRectFloat(fElementWidth, fElementHeight, fElementWidth, fElementHeight);
  RenderBlock({}, nsColor::Green, 0, &viewport);

  viewport = nsRectFloat(0, 0, fElementWidth, fHeight);
  nsRectU32 scissor = nsRectU32(0, (nsUInt32)fElementHeight, (nsUInt32)fElementWidth, (nsUInt32)fElementHeight);
  RenderBlock({}, nsColor::Green, 0, &viewport, &scissor);

  m_bCaptureImage = true;
  viewport = nsRectFloat(0, 0, fWidth, fHeight);
  scissor = nsRectU32((nsUInt32)fElementWidth, 0, (nsUInt32)fElementWidth, (nsUInt32)fElementHeight);
  RenderBlock({}, nsColor::Green, 0, &viewport, &scissor);
}

void nsRendererTestPipelineStates::VertexBufferTest()
{
  m_bCaptureImage = true;
  RenderBlock(m_hTriangleMesh, nsColor::RebeccaPurple);
}

void nsRendererTestPipelineStates::IndexBufferTest()
{
  m_bCaptureImage = true;
  RenderBlock(m_hSphereMesh, nsColor::Orange);
}

void nsRendererTestPipelineStates::PushConstantsTest()
{
  const nsUInt32 uiColumns = 4;
  const nsUInt32 uiRows = 2;

  BeginCommands("PushConstantsTest");
  {
    nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::CornflowerBlue, 0xFFFFFFFF);
    nsRenderContext* pContext = nsRenderContext::GetDefaultInstance();
    {
      pContext->BindShader(m_hPushConstantsShader);
      pContext->BindNullMeshBuffer(nsGALPrimitiveTopology::Triangles, 1);

      for (nsUInt32 x = 0; x < uiColumns; ++x)
      {
        for (nsUInt32 y = 0; y < uiRows; ++y)
        {
          nsTestData constants;
          nsTransform t = CreateTransform(uiColumns, uiRows, x, y);
          constants.Vertex0 = (t * nsVec3(1.f, -1.f, 0.0f)).GetAsVec4(1.0f);
          constants.Vertex1 = (t * nsVec3(-1.f, -1.f, 0.0f)).GetAsVec4(1.0f);
          constants.Vertex2 = (t * nsVec3(-0.f, 1.f, 0.0f)).GetAsVec4(1.0f);
          constants.VertexColor = nsColorScheme::LightUI(float(x * uiRows + y) / (uiColumns * uiRows)).GetAsVec4();

          pContext->SetPushConstants("nsTestData", constants);
          pContext->DrawMeshBuffer(1).AssertSuccess();
        }
      }
    }
    EndRendering();
    if (m_ImgCompFrames.Contains(m_iFrame))
    {
      NS_TEST_IMAGE(m_iFrame, 100);
    }
  }
  EndCommands();
}

void nsRendererTestPipelineStates::SetsSlotsTest()
{
  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsUInt32 uiColumns = 2;
  const nsUInt32 uiRows = 2;
  const float fElementWidth = fWidth / uiColumns;
  const float fElementHeight = fHeight / uiRows;

  const nsMat4 mMVP = CreateSimpleMVP((float)fElementWidth / (float)fElementHeight);

  auto constants = nsRenderContext::GetConstantBufferData<nsTestPerFrame>(m_hTestPerFrameConstantBuffer);
  constants->Time = 1.0f;
  nsRenderContext* pContext = nsRenderContext::GetDefaultInstance();
  pContext->BindConstantBuffer("nsTestPerFrame", m_hTestPerFrameConstantBuffer);

  BeginCommands("SetsSlots");
  {
    nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0xFFFFFFFF, m_hTexture2D_Mip0);
    viewport = nsRectFloat(fElementWidth, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip1);
    viewport = nsRectFloat(0, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip2);
    m_bCaptureImage = true;
    viewport = nsRectFloat(fElementWidth, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip3);
  }
  EndCommands();
}

void nsRendererTestPipelineStates::ConstantBufferTest()
{
  const nsUInt32 uiColumns = 4;
  const nsUInt32 uiRows = 2;

  BeginCommands("ConstantBufferTest");
  {
    nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::CornflowerBlue, 0xFFFFFFFF);
    nsRenderContext* pContext = nsRenderContext::GetDefaultInstance();
    {
      pContext->BindConstantBuffer("nsTestColors", m_hTestColorsConstantBuffer);
      pContext->BindConstantBuffer("nsTestPositions", m_hTestPositionsConstantBuffer);
      pContext->BindShader(m_hConstantBufferShader);
      pContext->BindNullMeshBuffer(nsGALPrimitiveTopology::Triangles, 1);

      for (nsUInt32 x = 0; x < uiColumns; ++x)
      {
        for (nsUInt32 y = 0; y < uiRows; ++y)
        {
          {
            auto constants = nsRenderContext::GetConstantBufferData<nsTestColors>(m_hTestColorsConstantBuffer);
            constants->VertexColor = nsColorScheme::LightUI(float(x * uiRows + y) / (uiColumns * uiRows)).GetAsVec4();
          }
          {
            nsTransform t = CreateTransform(uiColumns, uiRows, x, y);
            auto constants = nsRenderContext::GetConstantBufferData<nsTestPositions>(m_hTestPositionsConstantBuffer);
            constants->Vertex0 = (t * nsVec3(1.f, -1.f, 0.0f)).GetAsVec4(1.0f);
            constants->Vertex1 = (t * nsVec3(-1.f, -1.f, 0.0f)).GetAsVec4(1.0f);
            constants->Vertex2 = (t * nsVec3(-0.f, 1.f, 0.0f)).GetAsVec4(1.0f);
          }
          pContext->DrawMeshBuffer(1).AssertSuccess();
        }
      }
    }
    EndRendering();
    if (m_ImgCompFrames.Contains(m_iFrame))
    {
      NS_TEST_IMAGE(m_iFrame, 100);
    }
  }
  EndCommands();
}


void nsRendererTestPipelineStates::StructuredBufferTest()
{
  BeginCommands("InstancingTest");
  {
    if (m_iFrame == ImageCaptureFrames::StructuredBuffer_CopyToTempStorage)
    {
      // Replace the elements at [0, 3] with more green ones by offsetting the color by 16.
      nsHybridArray<nsTestShaderData, 16> instanceData;
      FillStructuredBuffer(instanceData, 16 /*green*/);
      m_pEncoder->UpdateBuffer(m_hInstancingData, 0, instanceData.GetArrayPtr().GetSubArray(0, 4).ToByteArray(), nsGALUpdateMode::CopyToTempStorage);
    }
    if (m_iFrame == ImageCaptureFrames::StructuredBuffer_CopyToTempStorage2)
    {
      // Replace the elements at [8, 15] with the same data as the original 8 elements. We will render these afterwards using custom buffer views.
      nsHybridArray<nsTestShaderData, 16> instanceData;
      FillStructuredBuffer(instanceData);
      m_pEncoder->UpdateBuffer(m_hInstancingData, sizeof(nsTestShaderData) * 8, instanceData.GetArrayPtr().GetSubArray(0, 8).ToByteArray(), nsGALUpdateMode::CopyToTempStorage);
    }
    nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::CornflowerBlue, 0xFFFFFFFF);

    nsRenderContext* pContext = nsRenderContext::GetDefaultInstance();
    {
      pContext->BindShader(m_hInstancingShader);
      pContext->BindMeshBuffer(m_hTriangleMesh);

      if (m_iFrame <= ImageCaptureFrames::StructuredBuffer_CopyToTempStorage)
      {
        pContext->BindBuffer("instancingData", m_pDevice->GetDefaultResourceView(m_hInstancingData));
        pContext->DrawMeshBuffer(1, 0, 8).AssertSuccess();
      }
      else if (m_iFrame == ImageCaptureFrames::StructuredBuffer_CopyToTempStorage2)
      {
        // Use the second half of the buffer to render the 8 triangles using two draw calls.
        pContext->BindBuffer("instancingData", m_hInstancingDataView_8_4);
        pContext->DrawMeshBuffer(1, 0, 4).AssertSuccess();
        pContext->BindBuffer("instancingData", m_hInstancingDataView_12_4);
        pContext->DrawMeshBuffer(1, 0, 4).AssertSuccess();
      }
      else if (m_iFrame == ImageCaptureFrames::StructuredBuffer_Transient1)
      {
        nsHybridArray<nsTestShaderData, 16> instanceData;
        FillStructuredBuffer(instanceData, 16 /*green*/);
        // Update the entire buffer in lots of little upload calls with greener versions.
        for (nsUInt32 i = 0; i < 16; i++)
        {
          pCommandEncoder->UpdateBuffer(m_hInstancingDataTransient, i * sizeof(nsTestShaderData), instanceData.GetArrayPtr().GetSubArray(i, 1).ToByteArray(), nsGALUpdateMode::AheadOfTime);
        }
        pContext->BindBuffer("instancingData", m_pDevice->GetDefaultResourceView(m_hInstancingDataTransient));
        pContext->DrawMeshBuffer(1, 0, 8).AssertSuccess();
      }
      else if (m_iFrame == ImageCaptureFrames::StructuredBuffer_Transient2)
      {
        nsHybridArray<nsTestShaderData, 16> instanceData;
        FillStructuredBuffer(instanceData);
        // Update with one single update call for the first 8 elements matching the initial state.
        pCommandEncoder->UpdateBuffer(m_hInstancingDataTransient, 0, instanceData.GetArrayPtr().GetSubArray(0, 8).ToByteArray(), nsGALUpdateMode::AheadOfTime);
        pContext->BindBuffer("instancingData", m_pDevice->GetDefaultResourceView(m_hInstancingDataTransient));
        pContext->DrawMeshBuffer(1, 0, 8).AssertSuccess();
      }
    }
  }
  EndRendering();
  if (m_ImgCompFrames.Contains(m_iFrame))
  {
    NS_TEST_IMAGE(m_iFrame, 100);
  }
  EndCommands();
}

void nsRendererTestPipelineStates::Texture2D()
{
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
    RenderCube(viewport, mMVP, 0xFFFFFFFF, m_hTexture2D_Mip0);
    viewport = nsRectFloat(fElementWidth, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip1);
    viewport = nsRectFloat(0, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip2);
    m_bCaptureImage = true;
    viewport = nsRectFloat(fElementWidth, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip3);
  }
  EndCommands();
}

void nsRendererTestPipelineStates::Texture2DArray()
{
  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsUInt32 uiColumns = 2;
  const nsUInt32 uiRows = 2;
  const float fElementWidth = fWidth / uiColumns;
  const float fElementHeight = fHeight / uiRows;

  const nsMat4 mMVP = CreateSimpleMVP((float)fElementWidth / (float)fElementHeight);

  BeginCommands("Texture2DArray");
  {
    nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0xFFFFFFFF, m_hTexture2DArray_Layer0_Mip0);
    viewport = nsRectFloat(fElementWidth, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2DArray_Layer0_Mip1);
    viewport = nsRectFloat(0, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2DArray_Layer1_Mip0);
    m_bCaptureImage = true;
    viewport = nsRectFloat(fElementWidth, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2DArray_Layer1_Mip1);
  }
  EndCommands();
}

void nsRendererTestPipelineStates::GenerateMipMaps()
{
  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsUInt32 uiColumns = 2;
  const nsUInt32 uiRows = 2;
  const float fElementWidth = fWidth / uiColumns;
  const float fElementHeight = fHeight / uiRows;

  const nsMat4 mMVP = CreateSimpleMVP((float)fElementWidth / (float)fElementHeight);
  BeginCommands("GenerateMipMaps");
  {
    nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
    m_pEncoder->GenerateMipMaps(m_pDevice->GetDefaultResourceView(m_hTexture2D));

    RenderCube(viewport, mMVP, 0xFFFFFFFF, m_hTexture2D_Mip0);
    viewport = nsRectFloat(fElementWidth, 0, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip1);
    viewport = nsRectFloat(0, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip2);
    m_bCaptureImage = true;
    viewport = nsRectFloat(fElementWidth, fElementHeight, fElementWidth, fElementHeight);
    RenderCube(viewport, mMVP, 0, m_hTexture2D_Mip3);
  }
  EndCommands();
}

nsTestAppRun nsRendererTestPipelineStates::Timestamps()
{
  BeginCommands("Timestamps");
  {
    nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::RebeccaPurple, 0xFFFFFFFF);

    if (m_iFrame == 2)
    {
      m_CPUTime[0] = nsTime::Now();
      m_timestamps[0] = pCommandEncoder->InsertTimestamp();
    }
    nsRenderContext::GetDefaultInstance()->BindShader(m_hNDCPositionOnlyShader);
    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(m_hSphereMesh);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

    if (m_iFrame == 2)
      m_timestamps[1] = pCommandEncoder->InsertTimestamp();

    EndRendering();

    if (m_iFrame == 2)
    {
      m_hFence = pCommandEncoder->InsertFence();
      pCommandEncoder->Flush();
    }
  }
  EndCommands();


  if (m_iFrame >= 2)
  {
    // #TODO_VULKAN Our CPU / GPU timestamp calibration is not precise enough to allow comparing between zones reliably. Need to implement VK_KHR_calibrated_timestamps.
    const nsTime epsilon = nsTime::MakeFromMilliseconds(16);
    nsEnum<nsGALAsyncResult> fenceResult = m_pDevice->GetFenceResult(m_hFence);
    if (fenceResult == nsGALAsyncResult::Ready)
    {
      if (m_pDevice->GetTimestampResult(m_timestamps[0], m_GPUTime[0]) == nsGALAsyncResult::Ready && m_pDevice->GetTimestampResult(m_timestamps[1], m_GPUTime[1]) == nsGALAsyncResult::Ready)
      {
        m_CPUTime[1] = nsTime::Now();
        NS_TEST_BOOL_MSG(m_CPUTime[0] <= (m_GPUTime[0] + epsilon), "%.4f < %.4f", m_CPUTime[0].GetMilliseconds(), m_GPUTime[0].GetMilliseconds());
        NS_TEST_BOOL_MSG(m_GPUTime[0] <= m_GPUTime[1], "%.4f < %.4f", m_GPUTime[0].GetMilliseconds(), m_GPUTime[1].GetMilliseconds());
        NS_TEST_BOOL_MSG(m_GPUTime[1] <= (m_CPUTime[1] + epsilon), "%.4f < %.4f", m_GPUTime[1].GetMilliseconds(), m_CPUTime[1].GetMilliseconds());
        nsTestFramework::GetInstance()->Output(nsTestOutput::Message, "Timestamp results received after %d frames or %.2f ms (%d frames after fence)", m_iFrame - 2, (nsTime::Now() - m_CPUTime[0]).GetMilliseconds(), m_iDelay);
        return nsTestAppRun::Quit;
      }
      else
      {
        m_iDelay++;
      }
    }
  }

  if (m_iFrame >= 100)
  {
    nsLog::Error("Timestamp results did not complete in 100 frames / {} seconds", (nsTime::Now() - m_CPUTime[0]).AsFloatInSeconds());
    return nsTestAppRun::Quit;
  }
  return nsTestAppRun::Continue;
}

nsTestAppRun nsRendererTestPipelineStates::OcclusionQueries()
{
  BeginCommands("OcclusionQueries");
  {
    nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::RebeccaPurple, 0xFFFFFFFF);

    // #TODO_VULKAN Vulkan will assert if we don't render something bogus here. The reason is that occlusion queries must be started and stopped within the same render pass. However, as we start the render pass lazily within nsGALCommandEncoderImplVulkan::FlushDeferredStateChanges, the BeginOcclusionQuery call is actually still outside the render pass.
    nsRenderContext::GetDefaultInstance()->BindShader(m_hNDCPositionOnlyShader);
    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(m_hTriangleMesh);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

    if (m_iFrame == 2)
    {
      NS_TEST_BOOL(m_queries[0].IsInvalidated());
      m_queries[0] = pCommandEncoder->BeginOcclusionQuery(nsGALQueryType::NumSamplesPassed);
      NS_TEST_BOOL(!m_queries[0].IsInvalidated());
      pCommandEncoder->EndOcclusionQuery(m_queries[0]);

      NS_TEST_BOOL(m_queries[1].IsInvalidated());
      m_queries[1] = pCommandEncoder->BeginOcclusionQuery(nsGALQueryType::AnySamplesPassed);
      NS_TEST_BOOL(!m_queries[1].IsInvalidated());
      pCommandEncoder->EndOcclusionQuery(m_queries[1]);

      m_queries[2] = pCommandEncoder->BeginOcclusionQuery(nsGALQueryType::NumSamplesPassed);
    }
    else if (m_iFrame == 3)
    {
      m_queries[3] = pCommandEncoder->BeginOcclusionQuery(nsGALQueryType::AnySamplesPassed);
    }

    nsRenderContext::GetDefaultInstance()->BindMeshBuffer(m_hSphereMesh);
    nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

    if (m_iFrame == 2)
    {
      pCommandEncoder->EndOcclusionQuery(m_queries[2]);
    }
    else if (m_iFrame == 3)
    {
      pCommandEncoder->EndOcclusionQuery(m_queries[3]);
    }
    EndRendering();

    if (m_iFrame == 3)
    {
      m_CPUTime[0] = nsTime::Now();
      m_hFence = pCommandEncoder->InsertFence();
      pCommandEncoder->Flush();
    }
  }
  EndCommands();

  if (m_iFrame >= 3)
  {
    nsEnum<nsGALAsyncResult> fenceResult = m_pDevice->GetFenceResult(m_hFence);
    if (fenceResult == nsGALAsyncResult::Ready)
    {
      nsEnum<nsGALAsyncResult> queryResults[4];
      nsUInt64 queryValues[4];
      for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_queries); i++)
      {
        queryResults[i] = m_pDevice->GetOcclusionQueryResult(m_queries[i], queryValues[i]);
        if (!NS_TEST_BOOL(queryResults[i] != nsGALAsyncResult::Expired))
        {
          return nsTestAppRun::Quit;
        }
      }

      bool bAllReady = true;
      for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_queries); i++)
      {
        if (queryResults[i] != nsGALAsyncResult::Ready)
          bAllReady = false;
      }

      if (bAllReady)
      {
        nsTestFramework::GetInstance()->Output(nsTestOutput::Message, "Occlusion query results received after %d frames or %.2f ms (%d frames after fence)", m_iFrame - 3, (nsTime::Now() - m_CPUTime[0]).GetMilliseconds(), m_iDelay);

        NS_TEST_INT(queryValues[0], 0);
        NS_TEST_INT(queryValues[1], 0);

        NS_TEST_BOOL(queryValues[2] >= 1);
        NS_TEST_BOOL(queryValues[3] >= 1);
        return nsTestAppRun::Quit;
      }
      else
      {
        m_iDelay++;
      }
    }
  }

  if (m_iFrame >= 100)
  {
    nsLog::Error("Occlusion query results did not complete in 100 frames / {} seconds", (nsTime::Now() - m_CPUTime[0]).AsFloatInSeconds());
    return nsTestAppRun::Quit;
  }

  return nsTestAppRun::Continue;
}

static nsRendererTestPipelineStates g_PipelineStatesTest;
