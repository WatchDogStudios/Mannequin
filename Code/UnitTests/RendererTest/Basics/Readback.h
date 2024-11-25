#pragma once

#include "../TestClass/TestClass.h"
#include <RendererCore/Textures/Texture2DResource.h>

/// \brief Tests texture readback from the GPU
///
/// Only renderable textures are tested.
/// The test first renders a pattern into the 8x8 texture.
/// Then the texture is read back and uploaded again to another texture.
/// The readback result is converted to RGBA32float and compared to a reference image. As these only change depending on channel count the MapImageNumberToString function is overwritten to always point to the same images.
/// Finally the original an re-uploaded texture is rendered to the screen and the result is again compared to a reference image.
///
/// The subtest list is dynamic, only formats that support render and sample are tested.
class nsRendererTestReadback : public nsGraphicsTest
{
  using SUPER = nsGraphicsTest;

public:
  virtual const char* GetTestName() const override { return "Readback"; }

private:
  virtual void SetupSubTests() override;

  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult GetImage(nsImage& ref_img, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber) override;
  virtual void MapImageNumberToString(const char* szTestName, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber, nsStringBuilder& out_sString) const override;
  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override;
  nsTestAppRun Readback(nsUInt32 uiInvocationCount);

  void CompareReadbackImage(nsImage&& image);
  void CompareUploadImage();

private:
  nsDynamicArray<nsEnum<nsGALResourceFormat>> m_TestableFormats;
  nsDynamicArray<nsString> m_TestableFormatStrings;

  nsGALResourceFormat::Enum m_Format = nsGALResourceFormat::Invalid;
  nsShaderResourceHandle m_hUVColorShader;
  nsShaderResourceHandle m_hUVColorIntShader;
  nsShaderResourceHandle m_hUVColorUIntShader;
  nsShaderResourceHandle m_hUVColorDepthShader;
  nsShaderResourceHandle m_hTexture2DShader;
  nsShaderResourceHandle m_hTexture2DDepthShader;
  nsShaderResourceHandle m_hTexture2DIntShader;
  nsShaderResourceHandle m_hTexture2DUIntShader;
  nsGALTextureHandle m_hTexture2DReadback;
  nsGALTextureHandle m_hTexture2DUpload;
  mutable nsImage m_ReadBackResult;
  mutable nsString m_sReadBackReferenceImage;
  bool m_bReadbackInProgress = true;
  nsGALReadbackTextureHelper m_Readback;
};
