#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Graphics Abstraction Layer - Core declarations

#include <cstdint>

/// Enumerates all supported graphics API backends.
enum class nsGALGraphicsAPI : uint8_t
{
  None = 0,
  DX11,
  DX12,
  Vulkan,
  PS5_NDA, // Prospero GNM — NDA protected
  Count
};

/// Result type for GAL operations.
enum class nsGALResult : uint8_t
{
  Success = 0,
  Failure,
  NotSupported,
  DeviceLost,
  OutOfMemory,
  InvalidArgument
};

#define NS_GAL_SUCCEEDED(result) ((result) == nsGALResult::Success)
#define NS_GAL_FAILED(result) ((result) != nsGALResult::Success)

/// Format for textures and render targets.
enum class nsGALResourceFormat : uint16_t
{
  Unknown = 0,

  // Standard formats
  R8G8B8A8_UNORM,
  R8G8B8A8_SNORM,
  R8G8B8A8_UINT,
  R8G8B8A8_SRGB,
  B8G8R8A8_UNORM,
  B8G8R8A8_SRGB,

  // Float formats
  R16G16B16A16_FLOAT,
  R32G32B32A32_FLOAT,
  R32G32B32_FLOAT,
  R16G16_FLOAT,
  R32_FLOAT,

  // Integer formats
  R32_UINT,
  R32_SINT,
  R16_UINT,

  // Depth formats
  D16_UNORM,
  D24_UNORM_S8_UINT,
  D32_FLOAT,
  D32_FLOAT_S8X24_UINT,

  // Compressed formats
  BC1_UNORM,
  BC1_SRGB,
  BC3_UNORM,
  BC3_SRGB,
  BC7_UNORM,
  BC7_SRGB,

  Count
};

/// Usage flags for GPU resources.
enum class nsGALResourceUsage : uint32_t
{
  Default = 0,
  Immutable = 1 << 0,
  Dynamic = 1 << 1,
  Staging = 1 << 2,
  RenderTarget = 1 << 3,
  DepthStencil = 1 << 4,
  UnorderedAccess = 1 << 5,
  ShaderResource = 1 << 6
};

inline nsGALResourceUsage operator|(nsGALResourceUsage a, nsGALResourceUsage b)
{
  return static_cast<nsGALResourceUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool operator&(nsGALResourceUsage a, nsGALResourceUsage b)
{
  return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

/// Shader stages.
enum class nsGALShaderStage : uint8_t
{
  Vertex = 0,
  Hull,
  Domain,
  Geometry,
  Pixel,
  Compute,
  Count
};

/// Primitive topology.
enum class nsGALPrimitiveTopology : uint8_t
{
  Points = 0,
  Lines,
  LineStrip,
  Triangles,
  TriangleStrip,
  PatchList
};

/// Blend factors.
enum class nsGALBlendFactor : uint8_t
{
  Zero = 0,
  One,
  SrcColor,
  InvSrcColor,
  SrcAlpha,
  InvSrcAlpha,
  DestAlpha,
  InvDestAlpha,
  DestColor,
  InvDestColor
};

/// Blend operations.
enum class nsGALBlendOp : uint8_t
{
  Add = 0,
  Subtract,
  RevSubtract,
  Min,
  Max
};

/// Comparison functions.
enum class nsGALCompareFunc : uint8_t
{
  Never = 0,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always
};

/// Cull mode.
enum class nsGALCullMode : uint8_t
{
  None = 0,
  Front,
  Back
};

/// Fill mode.
enum class nsGALFillMode : uint8_t
{
  Solid = 0,
  Wireframe
};

/// Filter mode for samplers.
enum class nsGALFilterMode : uint8_t
{
  Point = 0,
  Linear,
  Anisotropic
};

/// Address mode for samplers.
enum class nsGALTextureAddressMode : uint8_t
{
  Wrap = 0,
  Mirror,
  Clamp,
  Border
};

/// Returns true if the format is a depth format.
inline bool nsGALResourceFormatIsDepth(nsGALResourceFormat fmt)
{
  return fmt == nsGALResourceFormat::D16_UNORM ||
         fmt == nsGALResourceFormat::D24_UNORM_S8_UINT ||
         fmt == nsGALResourceFormat::D32_FLOAT ||
         fmt == nsGALResourceFormat::D32_FLOAT_S8X24_UINT;
}

/// Returns the byte size per pixel for uncompressed formats.
uint32_t nsGALResourceFormatGetBitsPerPixel(nsGALResourceFormat fmt);
