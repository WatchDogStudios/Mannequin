#pragma once

// Copyright (c) WD Studios. All rights reserved.
// DX12 Renderer Backend for Mannequin

#include "../../Engine/RendererCore/Device/nsGALDevice.h"

#include <wrl/client.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <vector>
#include <unordered_map>

using Microsoft::WRL::ComPtr;

/// DX12 implementation of the graphics abstraction layer.
class nsGALDeviceDX12 final : public nsGALDevice
{
public:
  nsGALDeviceDX12();
  ~nsGALDeviceDX12() override;

  // --- Lifecycle ---
  nsGALResult Init(const nsGALDeviceCreationDescription& desc) override;
  void Shutdown() override;

  // --- Capabilities ---
  const nsGALDeviceCaps& GetCapabilities() const override { return m_Caps; }
  nsGALGraphicsAPI GetAPI() const override { return nsGALGraphicsAPI::DX12; }
  const char* GetAPIName() const override { return "DX12"; }

  // --- Resource Creation ---
  nsGALTextureHandle CreateTexture(const nsGALTextureCreationDescription& desc) override;
  void DestroyTexture(nsGALTextureHandle handle) override;
  nsGALBufferHandle CreateBuffer(const nsGALBufferCreationDescription& desc) override;
  void DestroyBuffer(nsGALBufferHandle handle) override;

  // --- Swap Chain ---
  nsGALSwapChainHandle CreateSwapChain(const nsGALSwapChainCreationDescription& desc) override;
  void DestroySwapChain(nsGALSwapChainHandle handle) override;
  nsGALResult Present(nsGALSwapChainHandle handle) override;
  nsGALResult ResizeSwapChain(nsGALSwapChainHandle handle, uint32_t width, uint32_t height) override;
  nsGALTextureHandle GetBackBufferTexture(nsGALSwapChainHandle handle) override;

  // --- State Objects ---
  nsGALBlendStateHandle CreateBlendState(const nsGALBlendStateCreationDescription& desc) override;
  void DestroyBlendState(nsGALBlendStateHandle handle) override;
  nsGALDepthStencilStateHandle CreateDepthStencilState(const nsGALDepthStencilStateCreationDescription& desc) override;
  void DestroyDepthStencilState(nsGALDepthStencilStateHandle handle) override;
  nsGALRasterizerStateHandle CreateRasterizerState(const nsGALRasterizerStateCreationDescription& desc) override;
  void DestroyRasterizerState(nsGALRasterizerStateHandle handle) override;
  nsGALSamplerStateHandle CreateSamplerState(const nsGALSamplerStateCreationDescription& desc) override;
  void DestroySamplerState(nsGALSamplerStateHandle handle) override;

  // --- Command Recording ---
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

  // --- Readback ---
  nsGALResult ReadbackTexture(nsGALTextureHandle handle, uint32_t subresource,
                               void* pDestBuffer, uint32_t destBufferSize,
                               uint32_t& outRowPitch) override;

  // --- Debug ---
  void PushDebugGroup(const char* szName) override;
  void PopDebugGroup() override;
  void ConsumeValidationMessages(std::vector<nsValidationMessage>& outMessages) override;

private:
  static constexpr uint32_t FrameCount = 2;

  // Core DX12 objects
  ComPtr<IDXGIFactory6> m_pFactory;
  ComPtr<ID3D12Device> m_pDevice;
  ComPtr<ID3D12InfoQueue> m_pInfoQueue;
  ComPtr<ID3D12CommandQueue> m_pCommandQueue;
  ComPtr<ID3D12CommandAllocator> m_pCommandAllocators[FrameCount];
  ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

  // Synchronization
  ComPtr<ID3D12Fence> m_pFence;
  uint64_t m_uiFenceValues[FrameCount] = {};
  HANDLE m_hFenceEvent = nullptr;
  uint32_t m_uiCurrentFrame = 0;
  uint64_t m_uiConsumedInfoQueueMessages = 0;

  // Descriptor heaps
  ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
  ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;
  ComPtr<ID3D12DescriptorHeap> m_pSRVHeap;
  ComPtr<ID3D12DescriptorHeap> m_pSamplerHeap;
  uint32_t m_uiRTVDescriptorSize = 0;
  uint32_t m_uiDSVDescriptorSize = 0;
  uint32_t m_uiSRVDescriptorSize = 0;
  uint32_t m_uiSamplerDescriptorSize = 0;

  // Resource tracking
  struct DX12Texture
  {
    ComPtr<ID3D12Resource> m_pResource;
    nsGALTextureCreationDescription m_Desc;
    D3D12_RESOURCE_STATES m_CurrentState = D3D12_RESOURCE_STATE_COMMON;
  };

  struct DX12Buffer
  {
    ComPtr<ID3D12Resource> m_pResource;
    nsGALBufferCreationDescription m_Desc;
    D3D12_RESOURCE_STATES m_CurrentState = D3D12_RESOURCE_STATE_COMMON;
  };

  struct DX12SwapChain
  {
    ComPtr<IDXGISwapChain3> m_pSwapChain;
    nsGALSwapChainCreationDescription m_Desc;
    std::vector<nsGALTextureHandle> m_BackBufferTextures;
  };

  std::unordered_map<uint64_t, DX12Texture> m_Textures;
  std::unordered_map<uint64_t, DX12Buffer> m_Buffers;
  std::unordered_map<uint64_t, DX12SwapChain> m_SwapChains;
  uint64_t m_uiNextResourceID = 1;

  nsGALDeviceCaps m_Caps;

  // Helper methods
  void WaitForGPU();
  void MoveToNextFrame();
  DXGI_FORMAT GetDXGIFormat(nsGALResourceFormat format) const;
  D3D12_RESOURCE_FLAGS GetResourceFlags(const nsGALTextureCreationDescription& desc) const;
  void TransitionResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
};

/// Backend registration function.
void nsGALRegisterBackend(nsGALGraphicsAPI api, std::unique_ptr<nsGALDevice> (*pfnCreate)());
