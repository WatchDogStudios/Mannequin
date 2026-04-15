#include <Foundation/Platform/PlatformDesc.h>
#if NS_ENABLED(NS_PLATFORM_PLAYSTATION_5)

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsPlatformDesc);

nsPlatformDesc g_PlatformDescWin("Prospero", "Console");


const nsPlatformDesc* nsPlatformDesc::s_pThisPlatform = &g_PlatformDescWin;

#endif
