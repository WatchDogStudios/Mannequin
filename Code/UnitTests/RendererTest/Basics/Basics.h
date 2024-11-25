#pragma once

#include "../TestClass/TestClass.h"
#include <RendererCore/Textures/Texture2DResource.h>

class nsRendererTestBasics : public nsGraphicsTest
{
public:
  virtual const char* GetTestName() const override { return "Basics"; }

private:
  enum SubTests
  {
    ST_ClearScreen,
    ST_RasterizerStates,
    ST_BlendStates,
    ST_Textures2D,
    ST_Textures3D,
    ST_TexturesCube,
    ST_LineRendering,
  };

  virtual void SetupSubTests() override
  {
    AddSubTest("Clear Screen", SubTests::ST_ClearScreen);
    AddSubTest("Rasterizer States", SubTests::ST_RasterizerStates);
    AddSubTest("Blend States", SubTests::ST_BlendStates);
    AddSubTest("2D Textures", SubTests::ST_Textures2D);
    // AddSubTest("3D Textures", SubTests::ST_Textures3D); /// \todo 3D Texture support is currently not implemented
    AddSubTest("Cube Textures", SubTests::ST_TexturesCube);
    AddSubTest("Line Rendering", SubTests::ST_LineRendering);
  }

  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;

  nsTestAppRun SubtestClearScreen();
  nsTestAppRun SubtestRasterizerStates();
  nsTestAppRun SubtestBlendStates();
  nsTestAppRun SubtestTextures2D();
  nsTestAppRun SubtestTextures3D();
  nsTestAppRun SubtestTexturesCube();
  nsTestAppRun SubtestLineRendering();

  void RenderObjects(nsBitflags<nsShaderBindFlags> ShaderBindFlags);
  void RenderLineObjects(nsBitflags<nsShaderBindFlags> ShaderBindFlags);

  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override
  {
    m_iFrame = uiInvocationCount;

    if (iIdentifier == SubTests::ST_ClearScreen)
      return SubtestClearScreen();

    if (iIdentifier == SubTests::ST_RasterizerStates)
      return SubtestRasterizerStates();

    if (iIdentifier == SubTests::ST_BlendStates)
      return SubtestBlendStates();

    if (iIdentifier == SubTests::ST_Textures2D)
      return SubtestTextures2D();

    if (iIdentifier == SubTests::ST_Textures3D)
      return SubtestTextures3D();

    if (iIdentifier == SubTests::ST_TexturesCube)
      return SubtestTexturesCube();

    if (iIdentifier == SubTests::ST_LineRendering)
      return SubtestLineRendering();

    return nsTestAppRun::Quit;
  }

  nsMeshBufferResourceHandle m_hSphere;
  nsMeshBufferResourceHandle m_hSphere2;
  nsMeshBufferResourceHandle m_hTorus;
  nsMeshBufferResourceHandle m_hLongBox;
  nsMeshBufferResourceHandle m_hLineBox;
  nsTexture2DResourceHandle m_hTexture2D;
  nsTextureCubeResourceHandle m_hTextureCube;
};
