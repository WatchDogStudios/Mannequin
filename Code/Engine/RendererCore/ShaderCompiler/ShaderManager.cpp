#include <RendererCore/ShaderCompiler/ShaderManager.h>

void nsShaderManager::Configure(const char* szActivePlatform, bool bEnableRuntimeCompilation)
{
}

void nsShaderManager::GetPermutationValues(const nsHashedString& sName, nsHybridArray<nsHashedString, 16>& out_values)
{
}

nsShaderPermutationResourceHandle nsShaderManager::PreloadSinglePermutation(
  const nsShaderResourceHandle& hShader,
  const nsHashTable<nsHashedString, nsHashedString>& permutationVars,
  bool bAllowFallback)
{
  return nsShaderPermutationResourceHandle();
}
