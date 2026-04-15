#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/ResourceManager/Resource.h>

/// \brief 2D texture resource.
class NS_RENDERERCORE_DLL nsTexture2DResource : public nsResource
{
public:
  nsTexture2DResource();
  ~nsTexture2DResource();
};

using nsTexture2DResourceHandle = nsTypedResourceHandle<nsTexture2DResource>;
