#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/ResourceManager/Resource.h>

/// \brief Mesh resource that combines a mesh buffer with materials.
class NS_RENDERERCORE_DLL nsMeshResource : public nsResource
{
public:
  nsMeshResource();
  ~nsMeshResource();
};

using nsMeshResourceHandle = nsTypedResourceHandle<nsMeshResource>;
