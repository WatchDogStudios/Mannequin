#pragma once

#include "../TestClass/TestClass.h"
#include <RendererCore/Textures/Texture2DResource.h>

class nsRendererTestShaderCompiler : public nsGraphicsTest
{
  using SUPER = nsGraphicsTest;

  enum SubTests
  {
    ST_ShaderResources,
  };

public:
  virtual const char* GetTestName() const override { return "ShaderCompiler"; }

private:
  virtual void SetupSubTests() override;

  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override;

private:
  nsShaderResourceHandle m_hUVColorShader;
};
