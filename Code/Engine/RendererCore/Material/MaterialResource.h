#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/ResourceManager/Resource.h>

/// \brief Material resource controlling how geometry is rendered.
class NS_RENDERERCORE_DLL nsMaterialResource : public nsResource
{
public:
  nsMaterialResource();
  ~nsMaterialResource();
};

using nsMaterialResourceHandle = nsTypedResourceHandle<nsMaterialResource>;
