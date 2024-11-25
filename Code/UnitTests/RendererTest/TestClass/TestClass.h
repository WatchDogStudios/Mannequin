#pragma once

#include <Core/Graphics/Geometry.h>
#include <Core/System/Window.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/Meshes/MeshResource.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/ShaderCompiler/ShaderCompiler.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/ReadbackHelper.h>
#include <TestFramework/Framework/TestBaseClass.h>

#undef CreateWindow

class nsImage;

struct ObjectCB
{
  nsMat4 m_MVP;
  nsColor m_Color;
};

class nsGraphicsTest : public nsTestBaseClass
{
public:
  static nsResult CreateRenderer(nsGALDevice*& out_pDevice);
  static void SetClipSpace();

public:
  nsGraphicsTest();

  virtual nsResult GetImage(nsImage& ref_img, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber) override;

protected:
  virtual void SetupSubTests() override {}
  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override { return nsTestAppRun::Quit; }

  virtual nsResult InitializeTest() override;
  virtual nsResult DeInitializeTest() override;
  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;

  nsSizeU32 GetResolution() const;

protected:
  nsResult SetupRenderer();
  void ShutdownRenderer();

  nsResult CreateWindow(nsUInt32 uiResolutionX = 960, nsUInt32 uiResolutionY = 540);
  void DestroyWindow();

  void BeginFrame();
  void EndFrame();

  void BeginCommands(const char* szPassName);
  void EndCommands();

  nsGALCommandEncoder* BeginRendering(nsColor clearColor, nsUInt32 uiRenderTargetClearMask = 0xFFFFFFFF, nsRectFloat* pViewport = nullptr, nsRectU32* pScissor = nullptr);
  void EndRendering();

  /// \brief Renders a unit cube and makes an image comparison if m_bCaptureImage is set and the current frame is in m_ImgCompFrames.
  /// \param viewport Viewport to render into.
  /// \param mMVP Model View Projection matrix for camera. Use CreateSimpleMVP for convenience.
  /// \param uiRenderTargetClearMask What render targets if any should be cleared.
  /// \param hSRV The texture to render onto the cube.
  void RenderCube(nsRectFloat viewport, nsMat4 mMVP, nsUInt32 uiRenderTargetClearMask, nsGALTextureResourceViewHandle hSRV);

  nsMat4 CreateSimpleMVP(float fAspectRatio);


  nsMeshBufferResourceHandle CreateMesh(const nsGeometry& geom, const char* szResourceName);
  nsMeshBufferResourceHandle CreateSphere(nsInt32 iSubDivs, float fRadius);
  nsMeshBufferResourceHandle CreateTorus(nsInt32 iSubDivs, float fInnerRadius, float fOuterRadius);
  nsMeshBufferResourceHandle CreateBox(float fWidth, float fHeight, float fDepth);
  nsMeshBufferResourceHandle CreateLineBox(float fWidth, float fHeight, float fDepth);
  void RenderObject(nsMeshBufferResourceHandle hObject, const nsMat4& mTransform, const nsColor& color, nsBitflags<nsShaderBindFlags> ShaderBindFlags = nsShaderBindFlags::Default);

  nsWindow* m_pWindow = nullptr;
  nsGALDevice* m_pDevice = nullptr;
  nsGALCommandEncoder* m_pEncoder = nullptr;

  nsGALSwapChainHandle m_hSwapChain;
  nsGALTextureHandle m_hDepthStencilTexture;

  nsConstantBufferStorageHandle m_hObjectTransformCB;
  nsShaderResourceHandle m_hShader;
  nsMeshBufferResourceHandle m_hCubeUV;

  nsInt32 m_iFrame = 0;
  bool m_bCaptureImage = false;
  nsHybridArray<nsUInt32, 8> m_ImgCompFrames;
  nsGALReadbackTextureHelper m_Readback;
};
