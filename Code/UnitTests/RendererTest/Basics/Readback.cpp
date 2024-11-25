#include <RendererTest/RendererTestPCH.h>

#include <Core/GameState/GameStateWindow.h>
#include <Core/Graphics/Camera.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <RendererFoundation/RendererReflection.h>
#include <RendererFoundation/Resources/Texture.h>
#include <RendererTest/Basics/Readback.h>

void nsRendererTestReadback::SetupSubTests()
{
  nsStartup::StartupCoreSystems();
  SetupRenderer().AssertSuccess();

  const nsGALDeviceCapabilities& caps = nsGALDevice::GetDefaultDevice()->GetCapabilities();

  m_TestableFormats.Clear();
  for (nsUInt32 i = 1; i < nsGALResourceFormat::ENUM_COUNT; i++)
  {
    switch (i)
    {
      case nsGALResourceFormat::AUByteNormalized:      // What use is this format over RUByteNormalized?
      case nsGALResourceFormat::RGB10A2UInt:           // no nsImage support
      case nsGALResourceFormat::RGB10A2UIntNormalized: // no nsImage support
      case nsGALResourceFormat::D24S8:                 // no stencil readback implemented in Vulkan
        break;

      default:
      {
        if (caps.m_FormatSupport[i].AreAllSet(nsGALResourceFormatSupport::Texture | nsGALResourceFormatSupport::RenderTarget))
        {
          m_TestableFormats.PushBack((nsGALResourceFormat::Enum)i);
        }
      }
    }
  }

  m_TestableFormatStrings.Reserve(m_TestableFormats.GetCount());
  for (nsGALResourceFormat::Enum format : m_TestableFormats)
  {
    nsStringBuilder sFormat;
    nsReflectionUtils::EnumerationToString(nsGetStaticRTTI<nsGALResourceFormat>(), format, sFormat, nsReflectionUtils::EnumConversionMode::ValueNameOnly);
    m_TestableFormatStrings.PushBack(sFormat);
    AddSubTest(m_TestableFormatStrings.PeekBack(), format);
  }

  ShutdownRenderer();
  nsStartup::ShutdownCoreSystems();
}

nsResult nsRendererTestReadback::InitializeSubTest(nsInt32 iIdentifier)
{
  m_iFrame = -1;
  m_bReadbackInProgress = true;

  NS_SUCCEED_OR_RETURN(nsGraphicsTest::InitializeSubTest(iIdentifier));
  NS_SUCCEED_OR_RETURN(CreateWindow(320, 240));

  m_hUVColorShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/ReadbackFloat.nsShader");
  m_hUVColorIntShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/ReadbackInt.nsShader");
  m_hUVColorUIntShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/ReadbackUInt.nsShader");
  m_hUVColorDepthShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/ReadbackDepth.nsShader");

  m_hTexture2DShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2D.nsShader");
  m_hTexture2DDepthShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2DReadbackDepth.nsShader");
  m_hTexture2DIntShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2DReadbackInt.nsShader");
  m_hTexture2DUIntShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Texture2DReadbackUInt.nsShader");

  // Texture2D
  {
    m_Format = (nsGALResourceFormat::Enum)iIdentifier;
    nsGALTextureCreationDescription desc;
    desc.SetAsRenderTarget(8, 8, m_Format, nsGALMSAASampleCount::None);
    m_hTexture2DReadback = m_pDevice->CreateTexture(desc);

    NS_ASSERT_DEBUG(!m_hTexture2DReadback.IsInvalidated(), "Failed to create readback texture");
  }

  return NS_SUCCESS;
}

nsResult nsRendererTestReadback::DeInitializeSubTest(nsInt32 iIdentifier)
{
  m_Readback.Reset();
  m_ReadBackResult.Clear();
  if (!m_hTexture2DReadback.IsInvalidated())
  {
    m_pDevice->DestroyTexture(m_hTexture2DReadback);
    m_hTexture2DReadback.Invalidate();
  }
  if (!m_hTexture2DUpload.IsInvalidated())
  {
    m_pDevice->DestroyTexture(m_hTexture2DUpload);
    m_hTexture2DUpload.Invalidate();
  }
  m_hShader.Invalidate();
  m_hUVColorShader.Invalidate();
  m_hUVColorIntShader.Invalidate();
  m_hUVColorUIntShader.Invalidate();
  m_hTexture2DShader.Invalidate();
  m_hTexture2DDepthShader.Invalidate();
  m_hTexture2DIntShader.Invalidate();
  m_hTexture2DUIntShader.Invalidate();
  m_hUVColorDepthShader.Invalidate();

  DestroyWindow();

  if (nsGraphicsTest::DeInitializeSubTest(iIdentifier).Failed())
    return NS_FAILURE;

  return NS_SUCCESS;
}

nsResult nsRendererTestReadback::GetImage(nsImage& ref_img, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber)
{
  if (m_ReadBackResult.IsValid())
  {
    ref_img.ResetAndCopy(m_ReadBackResult);
    m_ReadBackResult.Clear();
    return NS_SUCCESS;
  }

  return SUPER::GetImage(ref_img, subTest, uiImageNumber);
}


void nsRendererTestReadback::MapImageNumberToString(const char* szTestName, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber, nsStringBuilder& out_sString) const
{
  if (!m_sReadBackReferenceImage.IsEmpty())
  {
    out_sString = m_sReadBackReferenceImage;
    m_sReadBackReferenceImage.Clear();
    return;
  }

  return SUPER::MapImageNumberToString(szTestName, subTest, uiImageNumber, out_sString);
}

void nsRendererTestReadback::CompareReadbackImage(nsImage&& image)
{
  nsStringBuilder sTemp;
  nsUInt8 uiChannels = nsGALResourceFormat::GetChannelCount(m_Format);
  if (nsGALResourceFormat::IsDepthFormat(m_Format))
  {
    sTemp = "Readback_Depth";
  }
  else
  {
    sTemp.SetFormat("Readback_Color{}Channel", uiChannels);
  }
  m_sReadBackReferenceImage = sTemp;
  m_ReadBackResult.ResetAndMove(std::move(image));

  NS_TEST_IMAGE(0, 1);
}

void nsRendererTestReadback::CompareUploadImage()
{
  nsStringBuilder sTemp;
  nsUInt8 uiChannels = nsGALResourceFormat::GetChannelCount(m_Format);
  if (nsGALResourceFormat::IsDepthFormat(m_Format))
  {
    sTemp = "Readback_Upload_Depth";
  }
  else
  {
    sTemp.SetFormat("Readback_Upload_Color{}Channel", uiChannels);
  }
  m_sReadBackReferenceImage = sTemp;
  NS_TEST_IMAGE(1, 3);
}

nsTestAppRun nsRendererTestReadback::RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount)
{
  m_iFrame = uiInvocationCount;
  m_bCaptureImage = false;
  BeginFrame();
  nsTestAppRun res = Readback(uiInvocationCount);
  EndFrame();
  return res;
}


nsTestAppRun nsRendererTestReadback::Readback(nsUInt32 uiInvocationCount)
{
  const float fWidth = (float)m_pWindow->GetClientAreaSize().width;
  const float fHeight = (float)m_pWindow->GetClientAreaSize().height;
  const nsUInt32 uiColumns = 2;
  const nsUInt32 uiRows = 2;
  const float fElementWidth = fWidth / uiColumns;
  const float fElementHeight = fHeight / uiRows;

  const nsMat4 mMVP = CreateSimpleMVP((float)fElementWidth / (float)fElementHeight);
  const bool bIsDepthTexture = nsGALResourceFormat::IsDepthFormat(m_Format);
  const bool bIsIntTexture = nsGALResourceFormat::IsIntegerFormat(m_Format);
  const bool bIsSigned = nsGALResourceFormat::IsSignedFormat(m_Format);
  if (m_iFrame == 1)
  {
    BeginCommands("Offscreen");
    {
      nsGALRenderingSetup renderingSetup;

      nsShaderResourceHandle shader;
      if (bIsDepthTexture)
      {
        renderingSetup.m_bDiscardDepth = true;
        renderingSetup.m_bClearDepth = true;
        renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(m_pDevice->GetDefaultRenderTargetView(m_hTexture2DReadback));
        shader = m_hUVColorDepthShader;
      }
      else
      {
        renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, m_pDevice->GetDefaultRenderTargetView(m_hTexture2DReadback));
        if (bIsIntTexture)
        {
          shader = bIsSigned ? m_hUVColorIntShader : m_hUVColorUIntShader;
        }
        else
        {
          shader = m_hUVColorShader;
        }
      }
      renderingSetup.m_ClearColor = nsColor::RebeccaPurple;
      renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;

      nsRectFloat viewport = nsRectFloat(0, 0, 8, 8);
      nsRenderContext::GetDefaultInstance()->BeginRendering(renderingSetup, viewport);
      SetClipSpace();

      nsRenderContext::GetDefaultInstance()->BindShader(shader);
      nsRenderContext::GetDefaultInstance()->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
      nsRenderContext::GetDefaultInstance()->DrawMeshBuffer().AssertSuccess();

      nsRenderContext::GetDefaultInstance()->EndRendering();
    }

    // Queue readback
    {
      m_Readback.ReadbackTexture(*m_pEncoder, m_hTexture2DReadback);
    }

    // Wait for results
    {
      nsEnum<nsGALAsyncResult> res = m_Readback.GetReadbackResult(nsTime::MakeFromHours(1));
      NS_ASSERT_ALWAYS(res == nsGALAsyncResult::Ready, "Readback of texture failed");
    }

    // Readback result
    {
      m_bReadbackInProgress = false;

      const nsGALTexture* pBackbuffer = nsGALDevice::GetDefaultDevice()->GetTexture(m_hTexture2DReadback);
      nsImage readBackResult;
      {
        nsGALTextureSubresource sourceSubResource;
        nsArrayPtr<nsGALTextureSubresource> sourceSubResources(&sourceSubResource, 1);
        nsHybridArray<nsGALSystemMemoryDescription, 1> memory;
        nsReadbackTextureLock lock = m_Readback.LockTexture(sourceSubResources, memory);
        NS_ASSERT_ALWAYS(lock, "Failed to lock readback texture");
        nsTextureUtils::CopySubResourceToImage(pBackbuffer->GetDescription(), sourceSubResource, memory[0], readBackResult, false);
      }

      {
        nsGALSystemMemoryDescription MemDesc;
        MemDesc.m_pData = readBackResult.GetByteBlobPtr();
        MemDesc.m_uiRowPitch = static_cast<nsUInt32>(readBackResult.GetRowPitch());
        MemDesc.m_uiSlicePitch = static_cast<nsUInt32>(readBackResult.GetDepthPitch());
        nsArrayPtr<nsGALSystemMemoryDescription> SysMemDescs(&MemDesc, 1);

        nsGALTextureCreationDescription desc;
        desc.m_uiWidth = 8;
        desc.m_uiHeight = 8;
        desc.m_Format = m_Format;
        m_hTexture2DUpload = m_pDevice->CreateTexture(desc, SysMemDescs);
        NS_ASSERT_DEV(!m_hTexture2DUpload.IsInvalidated(), "Texture creation failed");
      }


      NS_TEST_BOOL(readBackResult.Convert(nsImageFormat::R32G32B32A32_FLOAT).Succeeded());
      if (bIsIntTexture)
      {
        // For int textures, we multiply by 127 in the shader. We reverse this here to make all formats fit into the [0-1] float range.
        const nsImageFormat::Enum imageFormat = readBackResult.GetImageFormat();
        nsUInt64 uiNumElements = nsUInt64(8) * readBackResult.GetByteBlobPtr().GetCount() / (nsUInt64)nsImageFormat::GetBitsPerPixel(imageFormat);
        // Work with single channels instead of pixels
        uiNumElements *= nsImageFormat::GetBitsPerPixel(imageFormat) / 32;

        const nsUInt32 uiStride = 4;
        void* targetPointer = readBackResult.GetByteBlobPtr().GetPtr();
        while (uiNumElements)
        {
          nsUInt8 uiChannels = nsGALResourceFormat::GetChannelCount(m_Format);
          float& pixel = *reinterpret_cast<float*>(targetPointer);
          if (bIsDepthTexture)
          {
            // Don't normalize alpha channel which was added by the format extension from R to RGBA.
            if ((uiNumElements % 4) != 1)
              pixel /= nsMath::MaxValue<nsUInt16>();
          }
          else
          {
            // Don't normalize alpha channel if it was added by the format extension to RGBA.
            if (uiChannels == 4 || ((uiNumElements % 4) != 1))
              pixel /= 127.0f;
          }

          targetPointer = nsMemoryUtils::AddByteOffset(targetPointer, uiStride);
          uiNumElements--;
        }
      }
      CompareReadbackImage(std::move(readBackResult));
    }
    EndCommands();
  }

  BeginCommands("Readback");
  {
    if (bIsDepthTexture || nsGALResourceFormat::IsFloatFormat(m_Format))
    {
      m_hShader = m_hTexture2DDepthShader;
    }
    else if (bIsIntTexture)
    {
      m_hShader = nsGALResourceFormat::IsSignedFormat(m_Format) ? m_hTexture2DIntShader : m_hTexture2DUIntShader;
    }
    else
    {
      m_hShader = m_hTexture2DShader;
    }

    {
      nsRectFloat viewport = nsRectFloat(0, 0, fElementWidth, fElementHeight);
      RenderCube(viewport, mMVP, 0xFFFFFFFF, m_pDevice->GetDefaultResourceView(m_hTexture2DReadback));
    }
    if (!m_bReadbackInProgress)
    {
      nsRectFloat viewport = nsRectFloat(fElementWidth, 0, fElementWidth, fElementHeight);

      nsGALCommandEncoder* pCommandEncoder = BeginRendering(nsColor::RebeccaPurple, 0, &viewport);
      nsRenderContext::GetDefaultInstance()->BindTexture2D("DiffuseTexture", m_pDevice->GetDefaultResourceView(m_hTexture2DUpload));
      RenderObject(m_hCubeUV, mMVP, nsColor(1, 1, 1, 1), nsShaderBindFlags::None);
      EndRendering();
      CompareUploadImage();
    }
  }
  EndCommands();
  return m_bReadbackInProgress ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}

static nsRendererTestReadback g_ReadbackTest;
