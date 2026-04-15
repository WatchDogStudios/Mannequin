#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Graphics Abstraction Layer - Resource Descriptions

#include "nsGALDeclarations.h"
#include <string>
#include <cstdint>

/// Description for creating a texture.
struct nsGALTextureCreationDescription
{
  uint32_t m_uiWidth = 0;
  uint32_t m_uiHeight = 0;
  uint32_t m_uiDepth = 1;
  uint32_t m_uiMipLevelCount = 1;
  uint32_t m_uiArraySize = 1;
  uint32_t m_uiSampleCount = 1;
  nsGALResourceFormat m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;
  nsGALResourceUsage m_Usage = nsGALResourceUsage::Default;
  bool m_bCreateRenderTarget = false;
  bool m_bAllowShaderResourceView = true;
  bool m_bAllowUAV = false;
};

/// Description for creating a buffer.
struct nsGALBufferCreationDescription
{
  uint32_t m_uiByteSize = 0;
  uint32_t m_uiStructSize = 0;
  nsGALResourceUsage m_Usage = nsGALResourceUsage::Default;
  bool m_bIsVertexBuffer = false;
  bool m_bIsIndexBuffer = false;
  bool m_bIsConstantBuffer = false;
  bool m_bAllowShaderResourceView = false;
  bool m_bAllowUAV = false;
};

/// Description for a swap chain.
struct nsGALSwapChainCreationDescription
{
  void* m_pWindowHandle = nullptr;
  uint32_t m_uiWidth = 0;
  uint32_t m_uiHeight = 0;
  nsGALResourceFormat m_BackBufferFormat = nsGALResourceFormat::R8G8B8A8_UNORM;
  nsGALResourceFormat m_DepthStencilFormat = nsGALResourceFormat::D24_UNORM_S8_UINT;
  uint32_t m_uiBackBufferCount = 2;
  bool m_bVSync = false;
  bool m_bFullScreen = false;
};

/// Description for a sampler state.
struct nsGALSamplerStateCreationDescription
{
  nsGALFilterMode m_MinFilter = nsGALFilterMode::Linear;
  nsGALFilterMode m_MagFilter = nsGALFilterMode::Linear;
  nsGALFilterMode m_MipFilter = nsGALFilterMode::Linear;
  nsGALTextureAddressMode m_AddressU = nsGALTextureAddressMode::Wrap;
  nsGALTextureAddressMode m_AddressV = nsGALTextureAddressMode::Wrap;
  nsGALTextureAddressMode m_AddressW = nsGALTextureAddressMode::Wrap;
  float m_fMaxAnisotropy = 1.0f;
  float m_fMipLODBias = 0.0f;
  float m_fMinLOD = 0.0f;
  float m_fMaxLOD = 1000.0f;
  nsGALCompareFunc m_CompareFunc = nsGALCompareFunc::Never;
  float m_BorderColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

/// Blend state description for a single render target.
struct nsGALBlendRTDescription
{
  bool m_bBlendEnabled = false;
  nsGALBlendFactor m_SrcBlend = nsGALBlendFactor::One;
  nsGALBlendFactor m_DestBlend = nsGALBlendFactor::Zero;
  nsGALBlendOp m_BlendOp = nsGALBlendOp::Add;
  nsGALBlendFactor m_SrcBlendAlpha = nsGALBlendFactor::One;
  nsGALBlendFactor m_DestBlendAlpha = nsGALBlendFactor::Zero;
  nsGALBlendOp m_BlendOpAlpha = nsGALBlendOp::Add;
  uint8_t m_uiWriteMask = 0x0F;
};

/// Full blend state description.
struct nsGALBlendStateCreationDescription
{
  bool m_bAlphaToCoverage = false;
  bool m_bIndependentBlend = false;
  nsGALBlendRTDescription m_RenderTargetBlendDescriptions[8];
};

/// Depth-stencil state description.
struct nsGALDepthStencilStateCreationDescription
{
  bool m_bDepthTest = true;
  bool m_bDepthWrite = true;
  nsGALCompareFunc m_DepthTestFunc = nsGALCompareFunc::Less;
  bool m_bStencilTest = false;
  uint8_t m_uiStencilReadMask = 0xFF;
  uint8_t m_uiStencilWriteMask = 0xFF;
};

/// Rasterizer state description.
struct nsGALRasterizerStateCreationDescription
{
  nsGALCullMode m_CullMode = nsGALCullMode::Back;
  nsGALFillMode m_FillMode = nsGALFillMode::Solid;
  bool m_bFrontCounterClockwise = false;
  int32_t m_iDepthBias = 0;
  float m_fDepthBiasClamp = 0.0f;
  float m_fSlopeScaledDepthBias = 0.0f;
  bool m_bDepthClipEnable = true;
  bool m_bScissorEnable = false;
  bool m_bMultisampleEnable = false;
};

/// Render pass description for framebuffer setup.
struct nsGALRenderingSetup
{
  static constexpr uint32_t MaxRenderTargets = 8;

  uint32_t m_uiRenderTargetCount = 0;
  uint64_t m_RenderTargetViews[MaxRenderTargets] = {};
  uint64_t m_DepthStencilView = 0;
  float m_ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  float m_fClearDepth = 1.0f;
  uint8_t m_uiClearStencil = 0;
  bool m_bClearColor = true;
  bool m_bClearDepth = true;
  bool m_bClearStencil = false;
};

/// Viewport description.
struct nsGALViewport
{
  float m_fX = 0.0f;
  float m_fY = 0.0f;
  float m_fWidth = 0.0f;
  float m_fHeight = 0.0f;
  float m_fMinDepth = 0.0f;
  float m_fMaxDepth = 1.0f;
};

/// Scissor rect description.
struct nsGALScissorRect
{
  int32_t m_iLeft = 0;
  int32_t m_iTop = 0;
  int32_t m_iRight = 0;
  int32_t m_iBottom = 0;
};
