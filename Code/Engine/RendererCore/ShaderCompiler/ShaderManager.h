#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/HashedString.h>

class nsShaderResource;

template <typename T>
class nsTypedResourceHandle;

using nsShaderResourceHandle = nsTypedResourceHandle<nsShaderResource>;

class nsShaderPermutationResource;
using nsShaderPermutationResourceHandle = nsTypedResourceHandle<nsShaderPermutationResource>;

/// \brief Manages shader loading and permutation caching.
class NS_RENDERERCORE_DLL nsShaderManager
{
public:
  static void Configure(const char* szActivePlatform, bool bEnableRuntimeCompilation);

  static void GetPermutationValues(const nsHashedString& sName, nsHybridArray<nsHashedString, 16>& out_values);

  static nsShaderPermutationResourceHandle PreloadSinglePermutation(
    const nsShaderResourceHandle& hShader,
    const nsHashTable<nsHashedString, nsHashedString>& permutationVars,
    bool bAllowFallback);
};
