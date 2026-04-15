#include <RendererCore/ShaderCompiler/ShaderCompiler.h>
#include <Foundation/Logging/Log.h>

nsShaderCompiler::nsShaderCompiler() = default;
nsShaderCompiler::~nsShaderCompiler() = default;

nsResult nsShaderCompiler::CompileShaderPermutationForPlatforms(
  nsStringView sFile,
  const nsArrayPtr<const nsPermutationVar>& permVars,
  nsLogInterface* pLog,
  nsStringView sPlatform)
{
  nsLog::Warning("nsShaderCompiler::CompileShaderPermutationForPlatforms — stub, not yet implemented");
  return NS_SUCCESS;
}
