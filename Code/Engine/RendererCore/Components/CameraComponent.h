#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/World/World.h>

/// \brief Component that provides a camera for rendering.
class NS_RENDERERCORE_DLL nsCameraComponent : public nsComponent
{
  NS_ADD_DYNAMIC_REFLECTION(nsCameraComponent, nsComponent);

public:
  nsCameraComponent();
  ~nsCameraComponent();
};
