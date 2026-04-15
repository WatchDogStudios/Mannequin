#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <RendererCore/ShaderCompiler/PermutationGenerator.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Status.h>

/// \brief Compiles shader permutations for one or more platforms.
class NS_RENDERERCORE_DLL nsShaderCompiler
{
public:
  nsShaderCompiler();
  ~nsShaderCompiler();

  nsResult CompileShaderPermutationForPlatforms(
    nsStringView sFile,
    const nsArrayPtr<const nsPermutationVar>& permVars,
    nsLogInterface* pLog,
    nsStringView sPlatform);
};
