#include <RendererTest/RendererTestPCH.h>

#include <Foundation/Utilities/CommandLineUtils.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <TestFramework/Framework/TestFramework.h>
#include <TestFramework/Utilities/TestSetup.h>

#include <RendererTest/Advanced/OffscreenRenderer.h>

NS_TESTFRAMEWORK_ENTRY_POINT_BEGIN("RendererTest", "Renderer Tests")
{
  nsTextureUtils::s_bForceFullQualityAlways = true; // never allow to use low-res textures

  nsCommandLineUtils cmd;
  cmd.SetCommandLine(argc, (const char**)argv, nsCommandLineUtils::PreferOsArgs);

  if (cmd.GetBoolOption("-offscreen"))
  {
    nsOffscreenRendererTest offScreenTest;
    offScreenTest.SetCommandLineArguments(argc, (const char**)argv);
    nsRun(&offScreenTest); // Life cycle & run method calling
    const int iReturnCode = offScreenTest.GetReturnCode();
    // shutdown with exit code
    nsTestSetup::DeInitTestFramework(true);
    return iReturnCode;
  }
}
NS_TESTFRAMEWORK_ENTRY_POINT_END()
