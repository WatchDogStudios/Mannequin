#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Vulkan Renderer Backend for Mannequin (Stub)

#include "../../Engine/RendererCore/Device/nsGALDevice.h"

/// Vulkan implementation of the graphics abstraction layer.
/// TODO: Full implementation pending — this is the structural skeleton.
class nsGALDeviceVulkan final : public nsGALDevice
{
public:
  nsGALDeviceVulkan();
  ~nsGALDeviceVulkan() override;

  nsGALResult Init(const nsGALDeviceCreationDescription& desc) override;
  void Shutdown() override;

  const nsGALDeviceCaps& GetCapabilities() const override { return m_Caps; }
  nsGALGraphicsAPI GetAPI() const override { return nsGALGraphicsAPI::Vulkan; }
  const char* GetAPIName() const override { return "Vulkan"; }

  nsGALTextureHandle CreateTexture(const nsGALTextureCreationDescription& desc) override;
  void DestroyTexture(nsGALTextureHandle handle) override;
  nsGALBufferHandle CreateBuffer(const nsGALBufferCreationDescription& desc) override;
  void DestroyBuffer(nsGALBufferHandle handle) override;

  nsGALSwapChainHandle CreateSwapChain(const nsGALSwapChainCreationDescription& desc) override;
  void DestroySwapChain(nsGALSwapChainHandle handle) override;
  nsGALResult Present(nsGALSwapChainHandle handle) override;
  nsGALResult ResizeSwapChain(nsGALSwapChainHandle handle, uint32_t width, uint32_t height) override;
  nsGALTextureHandle GetBackBufferTexture(nsGALSwapChainHandle handle) override;

  nsGALBlendStateHandle CreateBlendState(const nsGALBlendStateCreationDescription& desc) override;
  void DestroyBlendState(nsGALBlendStateHandle handle) override;
  nsGALDepthStencilStateHandle CreateDepthStencilState(const nsGALDepthStencilStateCreationDescription& desc) override;
  void DestroyDepthStencilState(nsGALDepthStencilStateHandle handle) override;
  nsGALRasterizerStateHandle CreateRasterizerState(const nsGALRasterizerStateCreationDescription& desc) override;
  void DestroyRasterizerState(nsGALRasterizerStateHandle handle) override;
  nsGALSamplerStateHandle CreateSamplerState(const nsGALSamplerStateCreationDescription& desc) override;
  void DestroySamplerState(nsGALSamplerStateHandle handle) override;

  nsGALResult BeginFrame() override;
  nsGALResult EndFrame() override;
  void BeginRenderPass(const nsGALRenderingSetup& setup) override;
  void EndRenderPass() override;
  void SetViewport(const nsGALViewport& viewport) override;
  void SetScissorRect(const nsGALScissorRect& rect) override;
  void SetBlendState(nsGALBlendStateHandle handle) override;
  void SetDepthStencilState(nsGALDepthStencilStateHandle handle) override;
  void SetRasterizerState(nsGALRasterizerStateHandle handle) override;
  void SetPrimitiveTopology(nsGALPrimitiveTopology topology) override;
  void SetVertexBuffer(uint32_t slot, nsGALBufferHandle handle, uint32_t stride, uint32_t offset = 0) override;
  void SetIndexBuffer(nsGALBufferHandle handle, bool b32Bit = false) override;
  void Draw(uint32_t vertexCount, uint32_t startVertex = 0) override;
  void DrawIndexed(uint32_t indexCount, uint32_t startIndex = 0, int32_t baseVertex = 0) override;
  void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

  nsGALResult ReadbackTexture(nsGALTextureHandle handle, uint32_t subresource,
                               void* pDestBuffer, uint32_t destBufferSize,
                               uint32_t& outRowPitch) override;

  void PushDebugGroup(const char* szName) override;
  void PopDebugGroup() override;

private:
  nsGALDeviceCaps m_Caps;
  // TODO: VkInstance, VkDevice, VkQueue, VkCommandPool, VkCommandBuffer, etc.
};
