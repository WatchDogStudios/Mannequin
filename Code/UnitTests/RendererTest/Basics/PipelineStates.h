#pragma once

#include "../TestClass/TestClass.h"
#include <RendererCore/Textures/Texture2DResource.h>

class nsRendererTestPipelineStates : public nsGraphicsTest
{
public:
  virtual const char* GetTestName() const override { return "PipelineStates"; }

private:
  enum SubTests
  {
    ST_MostBasicShader,
    ST_ViewportScissor,
    ST_VertexBuffer,
    ST_IndexBuffer,
    ST_ConstantBuffer,
    ST_StructuredBuffer,
    ST_Texture2D,
    ST_Texture2DArray,
    ST_GenerateMipMaps,
    ST_PushConstants,
    ST_SetsSlots,
    ST_Timestamps,
    ST_OcclusionQueries,
  };

  enum ImageCaptureFrames
  {
    DefaultCapture = 5,
    StructuredBuffer_InitialData = 5,
    StructuredBuffer_CopyToTempStorage = 6,
    StructuredBuffer_CopyToTempStorage2 = 7,
    StructuredBuffer_Transient1 = 8,
    StructuredBuffer_Transient2 = 9,
    Timestamps_MaxWaitTime = nsMath::MaxValue<nsUInt32>(),
  };

  virtual void SetupSubTests() override
  {
    AddSubTest("01 - MostBasicShader", SubTests::ST_MostBasicShader);
    AddSubTest("02 - ViewportScissor", SubTests::ST_ViewportScissor);
    AddSubTest("03 - VertexBuffer", SubTests::ST_VertexBuffer);
    AddSubTest("04 - IndexBuffer", SubTests::ST_IndexBuffer);
    AddSubTest("05 - ConstantBuffer", SubTests::ST_ConstantBuffer);
    AddSubTest("06 - StructuredBuffer", SubTests::ST_StructuredBuffer);
    AddSubTest("07 - Texture2D", SubTests::ST_Texture2D);
    AddSubTest("08 - Texture2DArray", SubTests::ST_Texture2DArray);
    AddSubTest("09 - GenerateMipMaps", SubTests::ST_GenerateMipMaps);
    AddSubTest("10 - PushConstants", SubTests::ST_PushConstants);
    AddSubTest("11 - SetsSlots", SubTests::ST_SetsSlots);
    AddSubTest("12 - Timestamps", SubTests::ST_Timestamps); // Disabled due to CI failure on AMD.
    AddSubTest("13 - OcclusionQueries", SubTests::ST_OcclusionQueries);
  }

  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override;

  void RenderBlock(nsMeshBufferResourceHandle mesh, nsColor clearColor = nsColor::CornflowerBlue, nsUInt32 uiRenderTargetClearMask = 0xFFFFFFFF, nsRectFloat* pViewport = nullptr, nsRectU32* pScissor = nullptr);

  void MostBasicTriangleTest();
  void ViewportScissorTest();
  void VertexBufferTest();
  void IndexBufferTest();
  void ConstantBufferTest();
  void StructuredBufferTest();
  void Texture2D();
  void Texture2DArray();
  void GenerateMipMaps();
  void PushConstantsTest();
  void SetsSlotsTest();
  nsTestAppRun Timestamps();
  nsTestAppRun OcclusionQueries();

private:
  nsShaderResourceHandle m_hMostBasicTriangleShader;
  nsShaderResourceHandle m_hNDCPositionOnlyShader;
  nsShaderResourceHandle m_hConstantBufferShader;
  nsShaderResourceHandle m_hPushConstantsShader;
  nsShaderResourceHandle m_hInstancingShader;

  nsMeshBufferResourceHandle m_hTriangleMesh;
  nsMeshBufferResourceHandle m_hSphereMesh;

  nsConstantBufferStorageHandle m_hTestPerFrameConstantBuffer;
  nsConstantBufferStorageHandle m_hTestColorsConstantBuffer;
  nsConstantBufferStorageHandle m_hTestPositionsConstantBuffer;

  nsGALBufferHandle m_hInstancingData;
  nsGALBufferHandle m_hInstancingDataTransient;
  nsGALBufferResourceViewHandle m_hInstancingDataView_8_4;
  nsGALBufferResourceViewHandle m_hInstancingDataView_12_4;

  nsGALTextureHandle m_hTexture2D;
  nsGALTextureResourceViewHandle m_hTexture2D_Mip0;
  nsGALTextureResourceViewHandle m_hTexture2D_Mip1;
  nsGALTextureResourceViewHandle m_hTexture2D_Mip2;
  nsGALTextureResourceViewHandle m_hTexture2D_Mip3;
  nsGALTextureHandle m_hTexture2DArray;
  nsGALTextureResourceViewHandle m_hTexture2DArray_Layer0_Mip0;
  nsGALTextureResourceViewHandle m_hTexture2DArray_Layer0_Mip1;
  nsGALTextureResourceViewHandle m_hTexture2DArray_Layer1_Mip0;
  nsGALTextureResourceViewHandle m_hTexture2DArray_Layer1_Mip1;

  // Timestamps / Occlusion Queries test
  nsInt32 m_iDelay = 0;
  nsTime m_CPUTime[2];
  nsTime m_GPUTime[2];
  nsGALTimestampHandle m_timestamps[2];
  nsGALOcclusionHandle m_queries[4];
  nsGALFenceHandle m_hFence = {};
};
