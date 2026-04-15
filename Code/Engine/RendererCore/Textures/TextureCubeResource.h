#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/ResourceManager/Resource.h>

/// \brief Cube map texture resource.
class NS_RENDERERCORE_DLL nsTextureCubeResource : public nsResource
{
public:
  nsTextureCubeResource();
  ~nsTextureCubeResource();
};

using nsTextureCubeResourceHandle = nsTypedResourceHandle<nsTextureCubeResource>;
