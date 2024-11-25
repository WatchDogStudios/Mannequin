#pragma once

#include "../TestClass/TestClass.h"
#include <RendererCore/Textures/Texture2DResource.h>

class nsRendererTestSwapChain : public nsGraphicsTest
{
public:
  virtual const char* GetTestName() const override { return "SwapChain"; }

private:
  enum SubTests
  {
    ST_ColorOnly,
    ST_D16,
    ST_D24S8,
    ST_D32,
    ST_NoVSync,
    ST_ResizeWindow,
  };

  virtual void SetupSubTests() override
  {
    AddSubTest("Color Only", SubTests::ST_ColorOnly);
    AddSubTest("Depth D16", SubTests::ST_D16);
    AddSubTest("Depth D24S8", SubTests::ST_D24S8);
    AddSubTest("Depth D32", SubTests::ST_D32);
    AddSubTest("No VSync", SubTests::ST_NoVSync);
    AddSubTest("Resize Window", SubTests::ST_ResizeWindow);
  }

  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override;
  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override;

  void ResizeTest(nsUInt32 uiInvocationCount);
  nsTestAppRun BasicRenderLoop(nsInt32 iIdentifier, nsUInt32 uiInvocationCount);

  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override
  {
    m_iFrame = uiInvocationCount;

    switch (iIdentifier)
    {
      case SubTests::ST_ResizeWindow:
        ResizeTest(uiInvocationCount);
        [[fallthrough]];
      case SubTests::ST_ColorOnly:
      case SubTests::ST_D16:
      case SubTests::ST_D24S8:
      case SubTests::ST_D32:
      case SubTests::ST_NoVSync:
        return BasicRenderLoop(iIdentifier, uiInvocationCount);
      default:
        NS_ASSERT_NOT_IMPLEMENTED;
        break;
    }
    return nsTestAppRun::Quit;
  }

  nsSizeU32 m_CurrentWindowSize = nsSizeU32(320, 240);
};
