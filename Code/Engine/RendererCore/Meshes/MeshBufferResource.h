#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/ResourceManager/Resource.h>

/// \brief GPU mesh buffer resource.
class NS_RENDERERCORE_DLL nsMeshBufferResource : public nsResource
{
public:
  nsMeshBufferResource();
  ~nsMeshBufferResource();
};

using nsMeshBufferResourceHandle = nsTypedResourceHandle<nsMeshBufferResource>;
