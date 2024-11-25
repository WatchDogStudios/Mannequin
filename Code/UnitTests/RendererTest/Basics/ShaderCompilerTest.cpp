#include <RendererTest/RendererTestPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <RendererCore/Shader/ShaderPermutationResource.h>
#include <RendererCore/ShaderCompiler/ShaderCompiler.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererFoundation/RendererReflection.h>
#include <RendererTest/Basics/ShaderCompilerTest.h>

void nsRendererTestShaderCompiler::SetupSubTests()
{
  AddSubTest("Shader Resources", SubTests::ST_ShaderResources);
}

nsResult nsRendererTestShaderCompiler::InitializeSubTest(nsInt32 iIdentifier)
{
  NS_SUCCEED_OR_RETURN(nsGraphicsTest::InitializeSubTest(iIdentifier));

  m_hUVColorShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/ShaderCompilerTest.nsShader");

  return NS_SUCCESS;
}

nsResult nsRendererTestShaderCompiler::DeInitializeSubTest(nsInt32 iIdentifier)
{
  // m_hShader.Invalidate();
  m_hUVColorShader.Invalidate();

  if (nsGraphicsTest::DeInitializeSubTest(iIdentifier).Failed())
    return NS_FAILURE;

  return NS_SUCCESS;
}

nsTestAppRun nsRendererTestShaderCompiler::RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount)
{
  nsHashTable<nsHashedString, nsHashedString> m_PermutationVariables;
  nsShaderPermutationResourceHandle m_hActiveShaderPermutation = nsShaderManager::PreloadSinglePermutation(m_hUVColorShader, m_PermutationVariables, false);

  if (!NS_TEST_BOOL(m_hActiveShaderPermutation.IsValid()))
    return nsTestAppRun::Quit;

  {
    nsResourceLock<nsShaderPermutationResource> pResource(m_hActiveShaderPermutation, nsResourceAcquireMode::BlockTillLoaded);

    nsArrayPtr<const nsPermutationVar> permutationVars = static_cast<const nsShaderPermutationResource*>(pResource.GetPointer())->GetPermutationVars();

    const nsGALShaderByteCode* pVertex = pResource->GetShaderByteCode(nsGALShaderStage::VertexShader);
    NS_TEST_BOOL(pVertex);
    const auto& vertexDecl = pVertex->m_ShaderVertexInput;
    if (NS_TEST_INT(vertexDecl.GetCount(), 12))
    {
      auto CheckVertexDecl = [&](nsGALVertexAttributeSemantic::Enum semantic, nsGALResourceFormat::Enum format, nsUInt8 uiLocation)
      {
        NS_TEST_INT(vertexDecl[uiLocation].m_eSemantic, semantic);
        NS_TEST_INT(vertexDecl[uiLocation].m_eFormat, format);
        NS_TEST_INT(vertexDecl[uiLocation].m_uiLocation, uiLocation);
      };
      CheckVertexDecl(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::RGBAFloat, 0);
      CheckVertexDecl(nsGALVertexAttributeSemantic::Normal, nsGALResourceFormat::RGBFloat, 1);
      CheckVertexDecl(nsGALVertexAttributeSemantic::Tangent, nsGALResourceFormat::RGFloat, 2);
      CheckVertexDecl(nsGALVertexAttributeSemantic::Color0, nsGALResourceFormat::RFloat, 3);
      CheckVertexDecl(nsGALVertexAttributeSemantic::Color7, nsGALResourceFormat::RGBAUInt, 4);
      CheckVertexDecl(nsGALVertexAttributeSemantic::TexCoord0, nsGALResourceFormat::RGBUInt, 5);
      CheckVertexDecl(nsGALVertexAttributeSemantic::TexCoord9, nsGALResourceFormat::RGUInt, 6);
      CheckVertexDecl(nsGALVertexAttributeSemantic::BiTangent, nsGALResourceFormat::RUInt, 7);
      CheckVertexDecl(nsGALVertexAttributeSemantic::BoneWeights0, nsGALResourceFormat::RGBAInt, 8);
      CheckVertexDecl(nsGALVertexAttributeSemantic::BoneWeights1, nsGALResourceFormat::RGBInt, 9);
      CheckVertexDecl(nsGALVertexAttributeSemantic::BoneIndices0, nsGALResourceFormat::RGInt, 10);
      CheckVertexDecl(nsGALVertexAttributeSemantic::BoneIndices1, nsGALResourceFormat::RInt, 11);
    }

    const nsGALShaderByteCode* pPixel = pResource->GetShaderByteCode(nsGALShaderStage::PixelShader);
    NS_TEST_BOOL(pPixel);
    const nsHybridArray<nsShaderResourceBinding, 8>& bindings = pPixel->m_ShaderResourceBindings;
    {
      auto CheckBinding = [&](nsStringView sName, nsGALShaderResourceType::Enum descriptorType, nsGALShaderTextureType::Enum textureType = nsGALShaderTextureType::Unknown, nsBitflags<nsGALShaderStageFlags> stages = nsGALShaderStageFlags::PixelShader, nsUInt32 uiArraySize = 1)
      {
        for (nsUInt32 i = 0; i < bindings.GetCount(); ++i)
        {
          if (bindings[i].m_sName.GetView() == sName)
          {
            NS_TEST_INT(bindings[i].m_ResourceType, descriptorType);
            NS_TEST_INT(bindings[i].m_TextureType, textureType);
            NS_TEST_INT(bindings[i].m_Stages.GetValue(), stages.GetValue());
            NS_TEST_INT(bindings[i].m_uiArraySize, uiArraySize);
            return;
          }
        }
        NS_TEST_BOOL_MSG(false, "Shader resource not found in binding list");
      };
      const nsGALDeviceCapabilities& caps = nsGALDevice::GetDefaultDevice()->GetCapabilities();
      CheckBinding("PointClampSampler"_nssv, nsGALShaderResourceType::Sampler);
      CheckBinding("PerFrame"_nssv, nsGALShaderResourceType::ConstantBuffer);
      // CheckBinding("RES_Texture1D"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture1D);
      // CheckBinding("RES_Texture1DArray"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture1DArray);
      CheckBinding("RES_Texture2D"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2D);
      CheckBinding("RES_Texture2DArray"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2DArray);
      CheckBinding("RES_Texture2DMS"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2DMS);
      if (caps.m_bSupportsMultiSampledArrays)
      {
        CheckBinding("RES_Texture2DMSArray"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2DMSArray);
      }
      CheckBinding("RES_Texture3D"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture3D);
      CheckBinding("RES_TextureCube"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::TextureCube);
      CheckBinding("RES_TextureCubeArray"_nssv, nsGALShaderResourceType::Texture, nsGALShaderTextureType::TextureCubeArray);

      if (caps.m_bSupportsTexelBuffer)
      {
        CheckBinding("RES_Buffer"_nssv, nsGALShaderResourceType::TexelBuffer);
      }
      CheckBinding("RES_StructuredBuffer"_nssv, nsGALShaderResourceType::StructuredBuffer);
      CheckBinding("RES_ByteAddressBuffer"_nssv, nsGALShaderResourceType::StructuredBuffer);

      // CheckBinding("RES_RWTexture1D"_nssv, nsGALShaderResourceType::TextureRW, nsGALShaderTextureType::Texture1D);
      // CheckBinding("RES_RWTexture1DArray"_nssv, nsGALShaderResourceType::TextureRW, nsGALShaderTextureType::Texture1DArray);
      CheckBinding("RES_RWTexture2D"_nssv, nsGALShaderResourceType::TextureRW, nsGALShaderTextureType::Texture2D);
      CheckBinding("RES_RWTexture2DArray"_nssv, nsGALShaderResourceType::TextureRW, nsGALShaderTextureType::Texture2DArray);
      CheckBinding("RES_RWTexture3D"_nssv, nsGALShaderResourceType::TextureRW, nsGALShaderTextureType::Texture3D);
      if (caps.m_bSupportsTexelBuffer)
      {
        CheckBinding("RES_RWBuffer"_nssv, nsGALShaderResourceType::TexelBufferRW);
      }
      CheckBinding("RES_RWStructuredBuffer"_nssv, nsGALShaderResourceType::StructuredBufferRW);
      CheckBinding("RES_RWByteAddressBuffer"_nssv, nsGALShaderResourceType::StructuredBufferRW);

      CheckBinding("RES_AppendStructuredBuffer"_nssv, nsGALShaderResourceType::StructuredBufferRW);
      CheckBinding("RES_ConsumeStructuredBuffer"_nssv, nsGALShaderResourceType::StructuredBufferRW);
    }
  }

  return nsTestAppRun::Quit;
}


static nsRendererTestShaderCompiler g_ShaderCompilerTest;
