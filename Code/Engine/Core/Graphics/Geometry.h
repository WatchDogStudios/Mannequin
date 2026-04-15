#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Vec3.h>

/// \brief Generates geometric primitives.
class NS_CORE_DLL nsGeometry
{
public:
  struct Vertex
  {
    nsVec3 m_vPosition;
    nsVec3 m_vNormal;
    nsVec2 m_vTexCoord;
    nsColor m_Color;
    nsUInt16 m_uiBoneIndex = 0;
  };

  struct Triangle
  {
    nsUInt32 m_uiVertexIndices[3];
  };

  struct Line
  {
    nsUInt32 m_uiVertexIndices[2];
  };

  nsGeometry();
  ~nsGeometry();

  void Clear();

  void AddVertex(const nsVec3& vPos, const nsVec3& vNormal, const nsVec2& vTexCoord, const nsColor& color = nsColor::White);

  void AddPolygon(const nsArrayPtr<nsUInt32>& polygon, bool bFlipWinding = false);
  void AddTriangle(nsUInt32 uiVertexIndex0, nsUInt32 uiVertexIndex1, nsUInt32 uiVertexIndex2);
  void AddLine(nsUInt32 uiVertexIndex0, nsUInt32 uiVertexIndex1);

  void AddBox(const nsVec3& vFullExtents, bool bExtraVerticesForTexturing, const nsColor& color = nsColor::White, const nsMat4& mTransform = nsMat4::MakeIdentity());
  void AddSphere(float fRadius, nsUInt16 uiSegments, nsUInt16 uiStacks, const nsColor& color = nsColor::White, const nsMat4& mTransform = nsMat4::MakeIdentity());
  void AddCylinder(float fRadiusTop, float fRadiusBottom, float fHeight, bool bCapTop, bool bCapBottom, nsUInt16 uiSegments, const nsColor& color = nsColor::White, const nsMat4& mTransform = nsMat4::MakeIdentity());
  void AddCone(float fRadius, float fHeight, bool bCap, nsUInt16 uiSegments, const nsColor& color = nsColor::White, const nsMat4& mTransform = nsMat4::MakeIdentity());
  void AddTorus(float fInnerRadius, float fOuterRadius, nsUInt16 uiSegments, nsUInt16 uiSegmentDetail, bool bExtraVerticesForTexturing, const nsColor& color = nsColor::White, const nsMat4& mTransform = nsMat4::MakeIdentity());
  void AddGeodesicSphere(float fRadius, nsUInt8 uiSubDivisions, const nsColor& color = nsColor::White, const nsMat4& mTransform = nsMat4::MakeIdentity());

  nsUInt32 GetVertexCount() const { return m_Vertices.GetCount(); }
  const nsDynamicArray<Vertex>& GetVertices() const { return m_Vertices; }
  const nsDynamicArray<Triangle>& GetTriangles() const { return m_Triangles; }
  const nsDynamicArray<Line>& GetLines() const { return m_Lines; }

  void ComputeFaceNormals();
  void ComputeSmoothVertexNormals();

  void SetAllVertexColor(const nsColor& color);

  void Transform(const nsMat4& mTransform, bool bTransformPolyNormals);

private:
  nsDynamicArray<Vertex> m_Vertices;
  nsDynamicArray<Triangle> m_Triangles;
  nsDynamicArray<Line> m_Lines;
};
