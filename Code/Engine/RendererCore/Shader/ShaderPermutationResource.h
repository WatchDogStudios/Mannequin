#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <RendererCore/ShaderCompiler/PermutationGenerator.h>
#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Bitflags.h>

/// \brief Shader stage flags.
struct nsGALShaderStageFlags
{
  using StorageType = nsUInt32;

  enum Enum : nsUInt32
  {
    VertexShader = NS_BIT(0),
    HullShader = NS_BIT(1),
    DomainShader = NS_BIT(2),
    GeometryShader = NS_BIT(3),
    PixelShader = NS_BIT(4),
    ComputeShader = NS_BIT(5),
    Default = 0
  };

  struct Bits
  {
    StorageType VertexShader : 1;
    StorageType HullShader : 1;
    StorageType DomainShader : 1;
    StorageType GeometryShader : 1;
    StorageType PixelShader : 1;
    StorageType ComputeShader : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsGALShaderStageFlags);

/// \brief Shader resource type.
struct nsGALShaderResourceType
{
  enum Enum
  {
    Unknown,
    ConstantBuffer,
    Texture,
    TextureRW,
    TexelBuffer,
    TexelBufferRW,
    StructuredBuffer,
    StructuredBufferRW,
    Sampler,
    PushConstants,
  };
};

/// \brief Shader texture type.
struct nsGALShaderTextureType
{
  enum Enum
  {
    Unknown,
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture2DMS,
    Texture2DMSArray,
    Texture3D,
    TextureCube,
    TextureCubeArray,
  };
};

/// \brief Describes a shader resource binding.
struct nsShaderResourceBinding
{
  nsHashedString m_sName;
  nsGALShaderResourceType::Enum m_ResourceType = nsGALShaderResourceType::Unknown;
  nsGALShaderTextureType::Enum m_TextureType = nsGALShaderTextureType::Unknown;
  nsBitflags<nsGALShaderStageFlags> m_Stages;
  nsUInt32 m_uiArraySize = 1;
  nsInt32 m_iSet = -1;
  nsInt32 m_iSlot = -1;
};

/// \brief Vertex attribute description for shader input.
struct nsGALVertexAttribute
{
  nsUInt32 m_eSemantic = 0;
  nsUInt32 m_eFormat = 0;
  nsUInt8 m_uiLocation = 0;
  nsUInt8 m_uiOffset = 0;
};

/// \brief Compiled shader bytecode for a single stage.
struct nsGALShaderByteCode
{
  nsHybridArray<nsShaderResourceBinding, 8> m_ShaderResourceBindings;
  nsHybridArray<nsGALVertexAttribute, 16> m_ShaderVertexInput;
  nsDynamicArray<nsUInt8> m_ByteCode;
};

/// \brief A compiled shader permutation resource.
class NS_RENDERERCORE_DLL nsShaderPermutationResource : public nsResource
{
public:
  nsShaderPermutationResource();
  ~nsShaderPermutationResource();

  nsArrayPtr<const nsPermutationVar> GetPermutationVars() const;
  const nsGALShaderByteCode* GetShaderByteCode(nsUInt32 uiStage) const;
};

using nsShaderPermutationResourceHandle = nsTypedResourceHandle<nsShaderPermutationResource>;
