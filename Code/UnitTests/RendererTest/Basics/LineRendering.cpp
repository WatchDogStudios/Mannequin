#include <RendererTest/RendererTestPCH.h>

#include "Basics.h"

nsTestAppRun nsRendererTestBasics::SubtestLineRendering()
{
  BeginFrame();
  BeginCommands("RendererTest");

  nsColor clear(0, 0, 0, 0);
  BeginRendering(clear);

  RenderLineObjects(nsShaderBindFlags::Default);

  EndRendering();
  NS_TEST_LINE_IMAGE(0, 150);
  EndCommands();
  EndFrame();

  return m_iFrame < 0 ? nsTestAppRun::Continue : nsTestAppRun::Quit;
}
