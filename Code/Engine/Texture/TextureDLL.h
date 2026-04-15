#pragma once

#include <Foundation/Basics.h>

#if defined(BUILDSYSTEM_BUILDING_TEXTURE_LIB)
#  define NS_TEXTURE_DLL NS_DECL_EXPORT
#else
#  define NS_TEXTURE_DLL NS_DECL_IMPORT
#endif
