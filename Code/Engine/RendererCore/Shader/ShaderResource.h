#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/ResourceManager/Resource.h>

/// \brief Shader resource that describes a shader program.
class NS_RENDERERCORE_DLL nsShaderResource : public nsResource
{
public:
  nsShaderResource();
  ~nsShaderResource();
};

using nsShaderResourceHandle = nsTypedResourceHandle<nsShaderResource>;
