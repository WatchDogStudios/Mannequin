#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Foundation/Types/Types.h>

/// \brief Vertex attribute semantic for mesh buffers.
struct nsGALVertexAttributeSemantic
{
  enum Enum
  {
    Position,
    Normal,
    Tangent,
    Color0,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
    Color6,
    Color7,
    TexCoord0,
    TexCoord1,
    TexCoord2,
    TexCoord3,
    TexCoord4,
    TexCoord5,
    TexCoord6,
    TexCoord7,
    TexCoord8,
    TexCoord9,
    BiTangent,
    BoneIndices0,
    BoneIndices1,
    BoneWeights0,
    BoneWeights1,
  };
};

/// \brief Utility functions for mesh buffer handling.
class NS_RENDERERCORE_DLL nsMeshBufferUtils
{
public:
  static nsUInt32 GetBytesPerVertexAttribute(nsGALVertexAttributeSemantic::Enum semantic);
};
