#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Strings/String.h>

/// \brief Describes a mesh resource, including vertex/index data and sub-meshes.
class NS_RENDERERCORE_DLL nsMeshResourceDescriptor
{
public:
  nsMeshResourceDescriptor();
  ~nsMeshResourceDescriptor();

  struct SubMesh
  {
    nsUInt32 m_uiFirstTriangle = 0;
    nsUInt32 m_uiNumTriangles = 0;
    nsUInt32 m_uiMaterialIndex = 0;
  };

  void Clear();

  nsDynamicArray<SubMesh>& SubMeshes() { return m_SubMeshes; }
  const nsDynamicArray<SubMesh>& SubMeshes() const { return m_SubMeshes; }

  nsBoundingBox GetBounds() const;
  void SetBounds(const nsBoundingBox& bounds);

private:
  nsDynamicArray<SubMesh> m_SubMeshes;
  nsBoundingBox m_Bounds;
};
