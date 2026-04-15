#pragma once

#include <Foundation/Basics.h>

#if defined(BUILDSYSTEM_BUILDING_GAMEENGINE_LIB)
#  define NS_GAMEENGINE_DLL NS_DECL_EXPORT
#else
#  define NS_GAMEENGINE_DLL NS_DECL_IMPORT
#endif
