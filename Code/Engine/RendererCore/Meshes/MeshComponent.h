#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/World/World.h>

/// \brief Component that renders a mesh.
class NS_RENDERERCORE_DLL nsMeshComponent : public nsComponent
{
  NS_ADD_DYNAMIC_REFLECTION(nsMeshComponent, nsComponent);

public:
  nsMeshComponent();
  ~nsMeshComponent();
};
