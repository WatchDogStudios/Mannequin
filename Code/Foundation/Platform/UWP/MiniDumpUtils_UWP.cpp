#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

#  include <Foundation/System/MiniDumpUtils.h>

nsStatus nsMiniDumpUtils::WriteExternalProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsDumpType dumpTypeOverride)
{
  NS_IGNORE_UNUSED(sDumpFile);
  NS_IGNORE_UNUSED(uiProcessID);
  NS_IGNORE_UNUSED(dumpTypeOverride);
  return nsStatus("Not implemented on UWP");
}

nsStatus nsMiniDumpUtils::LaunchMiniDumpTool(nsStringView sDumpFile, nsDumpType dumpTypeOverride)
{
  NS_IGNORE_UNUSED(sDumpFile);
  NS_IGNORE_UNUSED(dumpTypeOverride);
  return nsStatus("Not implemented on UWP");
}

#endif
