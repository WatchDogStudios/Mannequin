#include <RendererTest/RendererTestPCH.h>

#include <Core/GameState/GameStateWindow.h>
#include <Core/Graphics/Camera.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <RendererFoundation/RendererReflection.h>
#include <RendererFoundation/Resources/Texture.h>
#include <RendererTest/Basics/ReadbackBuffer.h>

void nsRendererTestReadbackBuffer::SetupSubTests()
{
  nsStartup::StartupCoreSystems();
  SetupRenderer().AssertSuccess();

  const nsGALDeviceCapabilities& caps = nsGALDevice::GetDefaultDevice()->GetCapabilities();

  AddSubTest("VertexBuffer", SubTests::ST_VertexBuffer);
  AddSubTest("IndexBuffer", SubTests::ST_IndexBuffer);
  if (nsGALDevice::GetDefaultDevice()->GetCapabilities().m_bSupportsTexelBuffer)
  {
    AddSubTest("TexelBuffer", SubTests::ST_TexelBuffer);
  }
  AddSubTest("StructuredBuffer", SubTests::ST_StructuredBuffer);

  ShutdownRenderer();
  nsStartup::ShutdownCoreSystems();
}

nsResult nsRendererTestReadbackBuffer::InitializeSubTest(nsInt32 iIdentifier)
{
  m_iFrame = -1;
  m_bReadbackInProgress = true;

  NS_SUCCEED_OR_RETURN(nsGraphicsTest::InitializeSubTest(iIdentifier));
  NS_SUCCEED_OR_RETURN(CreateWindow(320, 240));

  // m_hUVColorShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/ReadbackFloat.nsShader");

  switch (iIdentifier)
  {
    case ST_VertexBuffer:
    {
      nsDynamicArray<float> data;
      data.SetCountUninitialized(128);
      for (nsUInt32 i = 0; i < 128; i++)
      {
        data[i] = (float)i;
      }
      m_BufferData = data.GetByteArrayPtr();

      m_hBufferReadback = m_pDevice->CreateVertexBuffer(4 * sizeof(float), 32, m_BufferData.GetByteArrayPtr());
      NS_ASSERT_DEBUG(!m_hBufferReadback.IsInvalidated(), "Failed to create buffer");
    }
    break;
    case ST_IndexBuffer:
    {
      nsDynamicArray<nsUInt32> data;
      data.SetCountUninitialized(128);
      for (nsUInt32 i = 0; i < 128; i++)
      {
        data[i] = i;
      }
      m_BufferData = data.GetByteArrayPtr();

      m_hBufferReadback = m_pDevice->CreateIndexBuffer(nsGALIndexType::UInt, 128, m_BufferData.GetByteArrayPtr());
      NS_ASSERT_DEBUG(!m_hBufferReadback.IsInvalidated(), "Failed to create buffer");
    }
    break;
    case ST_TexelBuffer:
    {
      nsDynamicArray<nsUInt32> data;
      data.SetCountUninitialized(128);
      for (nsUInt32 i = 0; i < 128; i++)
      {
        data[i] = i;
      }
      m_BufferData = data.GetByteArrayPtr();

      nsGALBufferCreationDescription desc;
      desc.m_uiStructSize = sizeof(nsUInt32);
      desc.m_uiTotalSize = 128 * desc.m_uiStructSize;
      desc.m_BufferFlags = nsGALBufferUsageFlags::TexelBuffer | nsGALBufferUsageFlags::ShaderResource;
      desc.m_Format = nsGALResourceFormat::RGBAUByteNormalized;
      desc.m_ResourceAccess.m_bImmutable = false;
      m_hBufferReadback = m_pDevice->CreateBuffer(desc, m_BufferData.GetByteArrayPtr());
      NS_ASSERT_DEBUG(!m_hBufferReadback.IsInvalidated(), "Failed to create buffer");
    }
    break;
    case ST_StructuredBuffer:
    {
      nsDynamicArray<nsColor> data;
      data.SetCountUninitialized(128);
      for (nsUInt32 i = 0; i < 128; i++)
      {
        data[i] = nsColor::MakeFromKelvin(1000 + 10 * i);
      }
      m_BufferData = data.GetByteArrayPtr();

      nsGALBufferCreationDescription desc;
      desc.m_uiStructSize = sizeof(nsColor);
      desc.m_uiTotalSize = 128 * desc.m_uiStructSize;
      desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
      desc.m_ResourceAccess.m_bImmutable = false;
      m_hBufferReadback = m_pDevice->CreateBuffer(desc, m_BufferData.GetByteArrayPtr());
      NS_ASSERT_DEBUG(!m_hBufferReadback.IsInvalidated(), "Failed to create buffer");
    }
    break;
    default:
      break;
  }

  return NS_SUCCESS;
}

nsResult nsRendererTestReadbackBuffer::DeInitializeSubTest(nsInt32 iIdentifier)
{
  m_Readback.Reset();
  if (!m_hBufferReadback.IsInvalidated())
  {
    m_pDevice->DestroyBuffer(m_hBufferReadback);
    m_hBufferReadback.Invalidate();
  }

  m_hComputeShader.Invalidate();

  DestroyWindow();

  if (nsGraphicsTest::DeInitializeSubTest(iIdentifier).Failed())
    return NS_FAILURE;

  return NS_SUCCESS;
}

nsTestAppRun nsRendererTestReadbackBuffer::RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount)
{
  m_iFrame = uiInvocationCount;
  m_bCaptureImage = false;
  BeginFrame();
  nsTestAppRun res = ReadbackBuffer(uiInvocationCount);
  EndFrame();
  return res;
}


nsTestAppRun nsRendererTestReadbackBuffer::ReadbackBuffer(nsUInt32 uiInvocationCount)
{
  if (m_iFrame == 1)
  {
    BeginCommands("Readback Buffer");
    // Queue readback
    {
      m_bReadbackInProgress = true;
      m_Readback.ReadbackBuffer(*m_pEncoder, m_hBufferReadback);
    }

    // Wait for results
    {
      nsEnum<nsGALAsyncResult> res = m_Readback.GetReadbackResult(nsTime::MakeFromHours(1));
      NS_ASSERT_ALWAYS(res == nsGALAsyncResult::Ready, "Readback of texture failed");
    }

    // Readback result
    {
      m_bReadbackInProgress = false;
      {
        nsArrayPtr<const nsUInt8> memory;
        nsReadbackBufferLock lock = m_Readback.LockBuffer(memory);
        NS_ASSERT_ALWAYS(lock, "Failed to lock readback buffer");

        if (NS_TEST_INT(memory.GetCount(), m_BufferData.GetCount()))
        {
          NS_TEST_INT(nsMemoryUtils::Compare(memory.GetPtr(), m_BufferData.GetData(), memory.GetCount()), 0);
        }
      }
    }
    EndCommands();
  }

  return m_bReadbackInProgress ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}

static nsRendererTestReadbackBuffer g_ReadbackBufferTest;
