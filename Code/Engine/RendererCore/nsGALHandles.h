#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Graphics Abstraction Layer - Opaque Handle Types

#include <cstdint>

/// Generic GPU resource handle. Backend-specific implementations store
/// their native pointers/indices in the m_uiInternalID field.
template <typename Tag>
struct nsGALHandle
{
  static constexpr uint64_t InvalidID = ~0ULL;

  uint64_t m_uiInternalID = InvalidID;

  nsGALHandle() = default;
  explicit nsGALHandle(uint64_t id) : m_uiInternalID(id) {}

  bool IsValid() const { return m_uiInternalID != InvalidID; }
  void Invalidate() { m_uiInternalID = InvalidID; }

  bool operator==(const nsGALHandle& other) const { return m_uiInternalID == other.m_uiInternalID; }
  bool operator!=(const nsGALHandle& other) const { return m_uiInternalID != other.m_uiInternalID; }
};

// Handle tag types
struct nsGALTextureTag {};
struct nsGALBufferTag {};
struct nsGALSwapChainTag {};
struct nsGALShaderTag {};
struct nsGALSamplerStateTag {};
struct nsGALBlendStateTag {};
struct nsGALDepthStencilStateTag {};
struct nsGALRasterizerStateTag {};
struct nsGALRenderTargetViewTag {};
struct nsGALResourceViewTag {};
struct nsGALUnorderedAccessViewTag {};
struct nsGALPipelineStateTag {};

// Typed handle aliases
using nsGALTextureHandle = nsGALHandle<nsGALTextureTag>;
using nsGALBufferHandle = nsGALHandle<nsGALBufferTag>;
using nsGALSwapChainHandle = nsGALHandle<nsGALSwapChainTag>;
using nsGALShaderHandle = nsGALHandle<nsGALShaderTag>;
using nsGALSamplerStateHandle = nsGALHandle<nsGALSamplerStateTag>;
using nsGALBlendStateHandle = nsGALHandle<nsGALBlendStateTag>;
using nsGALDepthStencilStateHandle = nsGALHandle<nsGALDepthStencilStateTag>;
using nsGALRasterizerStateHandle = nsGALHandle<nsGALRasterizerStateTag>;
using nsGALRenderTargetViewHandle = nsGALHandle<nsGALRenderTargetViewTag>;
using nsGALResourceViewHandle = nsGALHandle<nsGALResourceViewTag>;
using nsGALUnorderedAccessViewHandle = nsGALHandle<nsGALUnorderedAccessViewTag>;
using nsGALPipelineStateHandle = nsGALHandle<nsGALPipelineStateTag>;
