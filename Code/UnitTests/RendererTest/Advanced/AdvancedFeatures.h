#pragma once

#include "../TestClass/TestClass.h"
#include <Foundation/Communication/IpcChannel.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererTest/Advanced/OffscreenRenderer.h>

class nsRendererTestAdvancedFeatures : public nsGraphicsTest
{
public:
  virtual const char* GetTestName() const override { return "AdvancedFeatures"; }

private:
  enum SubTests
  {
    ST_ReadRenderTarget,
    ST_VertexShaderRenderTargetArrayIndex,
    ST_SharedTexture,
    ST_Tessellation,
    ST_Compute,
    ST_FloatSampling, // Either natively or emulated sampling of floating point textures e.g. depth textures.
    ST_ProxyTexture,
  };

  enum ImageCaptureFrames
  {
    DefaultCapture = 5,

  };

  virtual void SetupSubTests() override;

  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override;

  void ReadRenderTarget();
  void FloatSampling();
  void ProxyTexture();
  void VertexShaderRenderTargetArrayIndex();
  void Tessellation();
  void Compute();
  nsTestAppRun SharedTexture();
  void OffscreenProcessMessageFunc(const nsProcessMessage* pMsg);


private:
  nsShaderResourceHandle m_hShader2;
  nsShaderResourceHandle m_hShader3;

  nsGALTextureHandle m_hTexture2D;
  nsGALTextureResourceViewHandle m_hTexture2DView;
  nsGALTextureHandle m_hTexture2DArray;

  // Proxy texture test
  nsGALTextureHandle m_hProxyTexture2D[2];
  nsGALTextureResourceViewHandle m_hTexture2DArrayView[2];

  // Float sampling test
  nsGALSamplerStateHandle m_hDepthSamplerState;

  // Tessellation Test
  nsMeshBufferResourceHandle m_hSphereMesh;

  // Shared Texture Test
#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
  nsUniquePtr<nsProcess> m_pOffscreenProcess;
  nsUniquePtr<nsIpcChannel> m_pChannel;
  nsUniquePtr<nsIpcProcessMessageProtocol> m_pProtocol;
  nsGALTextureCreationDescription m_SharedTextureDesc;

  static constexpr nsUInt32 s_SharedTextureCount = 3;

  bool m_bExiting = false;
  nsGALTextureHandle m_hSharedTextures[s_SharedTextureCount];
  nsDeque<nsOffscreenTest_SharedTexture> m_SharedTextureQueue;
  nsUInt32 m_uiReceivedTextures = 0;
  float m_fOldProfilingThreshold = 0.0f;
  nsShaderUtils::nsBuiltinShader m_CopyShader;
#endif
};
