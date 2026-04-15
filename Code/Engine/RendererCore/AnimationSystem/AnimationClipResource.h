#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/ResourceManager/Resource.h>

/// \brief Animation clip resource.
class NS_RENDERERCORE_DLL nsAnimationClipResource : public nsResource
{
public:
  nsAnimationClipResource();
  ~nsAnimationClipResource();
};

using nsAnimationClipResourceHandle = nsTypedResourceHandle<nsAnimationClipResource>;
