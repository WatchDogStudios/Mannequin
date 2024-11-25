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
class nsRendererTestReadbackBuffer : public nsGraphicsTest
{
  using SUPER = nsGraphicsTest;

public:
  virtual const char* GetTestName() const override { return "Readback Buffer"; }

private:
  enum SubTests
  {
    ST_VertexBuffer,
    ST_IndexBuffer,
    ST_TexelBuffer,
    ST_StructuredBuffer,
  };

  virtual void SetupSubTests() override;

  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override;
  nsTestAppRun ReadbackBuffer(nsUInt32 uiInvocationCount);

private:
  nsShaderResourceHandle m_hComputeShader;

  nsDynamicArray<nsUInt8> m_BufferData;
  nsGALBufferHandle m_hBufferReadback;
  bool m_bReadbackInProgress = true;
  nsGALReadbackBufferHelper m_Readback;
};
