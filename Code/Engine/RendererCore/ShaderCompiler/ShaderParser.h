#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <RendererCore/ShaderCompiler/PermutationGenerator.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/HashedString.h>

/// \brief Parses shader files to extract permutation variables.
class NS_RENDERERCORE_DLL nsShaderParser
{
public:
  static void ParsePermutationSection(
    nsStreamReader& inout_stream,
    nsHybridArray<nsHashedString, 16>& out_permVars,
    nsHybridArray<nsPermutationVar, 16>& out_fixedPermVars);
};
