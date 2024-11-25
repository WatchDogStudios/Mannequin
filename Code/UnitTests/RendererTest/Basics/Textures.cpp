#include <RendererTest/RendererTestPCH.h>

#include "Basics.h"
#include <Core/Graphics/Camera.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererCore/Textures/TextureCubeResource.h>

nsTestAppRun nsRendererTestBasics::SubtestTextures2D()
{
  BeginFrame();
  BeginCommands("Textures2D");

  nsRenderContext::GetDefaultInstance()->SetDefaultTextureFilter(nsTextureFilterSetting::FixedTrilinear);

  const nsInt32 iNumFrames = 14;

  m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/Textured.nsShader");
  nsEnum<nsGALResourceFormat> textureFormat;
  nsStringView sTextureResourceId;

  if (m_iFrame == 0)
  {
    textureFormat = nsGALResourceFormat::RGBAUByteNormalizedsRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_ABGR_Mips_D.dds";
  }

  if (m_iFrame == 1)
  {
    textureFormat = nsGALResourceFormat::RGBAUByteNormalizedsRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_ABGR_NoMips_D.dds";
  }

  if (m_iFrame == 2)
  {
    textureFormat = nsGALResourceFormat::BGRAUByteNormalizedsRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_ARGB_Mips_D.dds";
  }

  if (m_iFrame == 3)
  {
    textureFormat = nsGALResourceFormat::BGRAUByteNormalizedsRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_ARGB_NoMips_D.dds";
  }

  if (m_iFrame == 4)
  {
    textureFormat = nsGALResourceFormat::BC1sRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_DXT1_Mips_D.dds";
  }

  if (m_iFrame == 5)
  {
    textureFormat = nsGALResourceFormat::BC1sRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_DXT1_NoMips_D.dds";
  }

  if (m_iFrame == 6)
  {
    textureFormat = nsGALResourceFormat::BC2sRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_DXT3_Mips_D.dds";
  }

  if (m_iFrame == 7)
  {
    textureFormat = nsGALResourceFormat::BC2sRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_DXT3_NoMips_D.dds";
  }

  if (m_iFrame == 8)
  {
    textureFormat = nsGALResourceFormat::BC3sRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_DXT5_Mips_D.dds";
  }

  if (m_iFrame == 9)
  {
    textureFormat = nsGALResourceFormat::BC3sRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_DXT5_NoMips_D.dds";
  }

  if (m_iFrame == 10)
  {
    textureFormat = nsGALResourceFormat::BGRAUByteNormalizedsRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_RGB_Mips_D.dds";
  }

  if (m_iFrame == 11)
  {
    textureFormat = nsGALResourceFormat::BGRAUByteNormalizedsRGB;
    sTextureResourceId = "SharedData/Textures/nsLogo_RGB_NoMips_D.dds";
  }

  if (m_iFrame == 12)
  {
    textureFormat = nsGALResourceFormat::B5G6R5UNormalized;
    sTextureResourceId = "SharedData/Textures/nsLogo_R5G6B5_NoMips_D.dds";
  }

  if (m_iFrame == 13)
  {
    textureFormat = nsGALResourceFormat::B5G6R5UNormalized;
    sTextureResourceId = "SharedData/Textures/nsLogo_R5G6B5_MipsD.dds";
  }

  const bool bSupported = m_pDevice->GetCapabilities().m_FormatSupport[textureFormat].AreAllSet(nsGALResourceFormatSupport::Texture);
  if (bSupported)
  {
    m_hTexture2D = nsResourceManager::LoadResource<nsTexture2DResource>(sTextureResourceId);
    nsRenderContext::GetDefaultInstance()->BindTexture2D("DiffuseTexture", m_hTexture2D);
  }
  BeginRendering(nsColor::Black);

  if (bSupported)
  {
    RenderObjects(nsShaderBindFlags::Default);
  }

  EndRendering();

  if (bSupported)
  {
    NS_TEST_IMAGE(m_iFrame, 300);
  }

  EndCommands();
  EndFrame();

  return m_iFrame < (iNumFrames - 1) ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}


nsTestAppRun nsRendererTestBasics::SubtestTextures3D()
{
  BeginFrame();
  BeginCommands("Textures3D");
  nsRenderContext::GetDefaultInstance()->SetDefaultTextureFilter(nsTextureFilterSetting::FixedTrilinear);

  const nsInt32 iNumFrames = 1;

  m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/TexturedVolume.nsShader");

  if (m_iFrame == 0)
  {
    m_hTexture2D = nsResourceManager::LoadResource<nsTexture2DResource>("SharedData/Textures/Volume/nsLogo_Volume_A8_NoMips_D.dds");
  }

  nsRenderContext::GetDefaultInstance()->BindTexture2D("DiffuseTexture", m_hTexture2D);

  BeginRendering(nsColor::Black);

  RenderObjects(nsShaderBindFlags::Default);

  NS_TEST_IMAGE(m_iFrame, 100);
  EndRendering();
  EndCommands();
  EndFrame();

  return m_iFrame < (iNumFrames - 1) ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}


nsTestAppRun nsRendererTestBasics::SubtestTexturesCube()
{
  BeginFrame();
  BeginCommands("TexturesCube");

  nsRenderContext::GetDefaultInstance()->SetDefaultTextureFilter(nsTextureFilterSetting::FixedTrilinear);

  const nsInt32 iNumFrames = 12;

  m_hShader = nsResourceManager::LoadResource<nsShaderResource>("RendererTest/Shaders/TexturedCube.nsShader");

  if (m_iFrame == 0)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_XRGB_NoMips_D.dds");
  }

  if (m_iFrame == 1)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_XRGB_Mips_D.dds");
  }

  if (m_iFrame == 2)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_RGBA_NoMips_D.dds");
  }

  if (m_iFrame == 3)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_RGBA_Mips_D.dds");
  }

  if (m_iFrame == 4)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_DXT1_NoMips_D.dds");
  }

  if (m_iFrame == 5)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_DXT1_Mips_D.dds");
  }

  if (m_iFrame == 6)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_DXT3_NoMips_D.dds");
  }

  if (m_iFrame == 7)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_DXT3_Mips_D.dds");
  }

  if (m_iFrame == 8)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_DXT5_NoMips_D.dds");
  }

  if (m_iFrame == 9)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_DXT5_Mips_D.dds");
  }

  if (m_iFrame == 10)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_RGB_NoMips_D.dds");
  }

  if (m_iFrame == 11)
  {
    m_hTextureCube = nsResourceManager::LoadResource<nsTextureCubeResource>("SharedData/Textures/Cubemap/nsLogo_Cube_RGB_Mips_D.dds");
  }

  nsRenderContext::GetDefaultInstance()->BindTextureCube("DiffuseTexture", m_hTextureCube);

  BeginRendering(nsColor::Black);

  RenderObjects(nsShaderBindFlags::Default);

  EndRendering();
  NS_TEST_IMAGE(m_iFrame, 200);
  EndCommands();
  EndFrame();

  return m_iFrame < (iNumFrames - 1) ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}
