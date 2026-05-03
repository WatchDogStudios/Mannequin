#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Graphics Abstraction Layer - Device Interface
// This is the primary abstraction that all renderer backends must implement.

#include "nsGALDeclarations.h"
#include "nsGALDescriptions.h"
#include "nsGALHandles.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>

/// Device capabilities reported by the backend.
struct nsGALDeviceCaps
{
  nsGALGraphicsAPI m_API = nsGALGraphicsAPI::None;
  std::string m_sAdapterName;
  uint64_t m_uiDedicatedVideoMemory = 0;
  uint32_t m_uiMaxTextureSize = 0;
  uint32_t m_uiMaxRenderTargets = 0;
  uint32_t m_uiMaxComputeWorkGroupSize[3] = {0, 0, 0};
  bool m_bSupportsCompute = false;
  bool m_bSupportsTessellation = false;
  bool m_bSupportsGeometryShaders = false;
  bool m_bSupportsMultiViewport = false;
  bool m_bSupportsConservativeRaster = false;
  bool m_bSupportsRaytracing = false;
  bool m_bSupportsMeshShaders = false;
  bool m_bSupportsVariableRateShading = false;
  bool m_bShaderStageSupported[static_cast<int>(nsGALShaderStage::Count)] = {};
};

/// Description for creating a device.
struct nsGALDeviceCreationDescription
{
  bool m_bDebugDevice = false;
  bool m_bGPUValidation = false;
  int32_t m_iPreferredAdapterIndex = -1; // -1 = auto-select
};

/// Backend validation/debug layer message captured during a test run.
struct nsValidationMessage
{
  std::string m_sSource;
  std::string m_sSeverity = "Info";
  std::string m_sMessage;
  std::string m_sRecommendation;
};

/// Abstract GPU device. All renderer backends derive from this.
class nsGALDevice
{
public:
  virtual ~nsGALDevice() = default;

  // --- Lifecycle ---
  virtual nsGALResult Init(const nsGALDeviceCreationDescription& desc) = 0;
  virtual void Shutdown() = 0;

  // --- Capabilities ---
  virtual const nsGALDeviceCaps& GetCapabilities() const = 0;
  virtual nsGALGraphicsAPI GetAPI() const = 0;
  virtual const char* GetAPIName() const = 0;

  // --- Resource Creation ---
  virtual nsGALTextureHandle CreateTexture(const nsGALTextureCreationDescription& desc) = 0;
  virtual void DestroyTexture(nsGALTextureHandle handle) = 0;

  virtual nsGALBufferHandle CreateBuffer(const nsGALBufferCreationDescription& desc) = 0;
  virtual void DestroyBuffer(nsGALBufferHandle handle) = 0;

  // --- Swap Chain ---
  virtual nsGALSwapChainHandle CreateSwapChain(const nsGALSwapChainCreationDescription& desc) = 0;
  virtual void DestroySwapChain(nsGALSwapChainHandle handle) = 0;
  virtual nsGALResult Present(nsGALSwapChainHandle handle) = 0;
  virtual nsGALResult ResizeSwapChain(nsGALSwapChainHandle handle, uint32_t width, uint32_t height) = 0;
  virtual nsGALTextureHandle GetBackBufferTexture(nsGALSwapChainHandle handle) = 0;

  // --- State Objects ---
  virtual nsGALBlendStateHandle CreateBlendState(const nsGALBlendStateCreationDescription& desc) = 0;
  virtual void DestroyBlendState(nsGALBlendStateHandle handle) = 0;

  virtual nsGALDepthStencilStateHandle CreateDepthStencilState(const nsGALDepthStencilStateCreationDescription& desc) = 0;
  virtual void DestroyDepthStencilState(nsGALDepthStencilStateHandle handle) = 0;

  virtual nsGALRasterizerStateHandle CreateRasterizerState(const nsGALRasterizerStateCreationDescription& desc) = 0;
  virtual void DestroyRasterizerState(nsGALRasterizerStateHandle handle) = 0;

  virtual nsGALSamplerStateHandle CreateSamplerState(const nsGALSamplerStateCreationDescription& desc) = 0;
  virtual void DestroySamplerState(nsGALSamplerStateHandle handle) = 0;

  // --- Command Recording ---

  /// Begin a new frame. Must be called before any rendering commands.
  virtual nsGALResult BeginFrame() = 0;

  /// End the current frame. Submits all recorded commands.
  virtual nsGALResult EndFrame() = 0;

  /// Begin a render pass with the given setup.
  virtual void BeginRenderPass(const nsGALRenderingSetup& setup) = 0;

  /// End the current render pass.
  virtual void EndRenderPass() = 0;

  /// Set the viewport.
  virtual void SetViewport(const nsGALViewport& viewport) = 0;

  /// Set scissor rectangle.
  virtual void SetScissorRect(const nsGALScissorRect& rect) = 0;

  /// Bind blend state.
  virtual void SetBlendState(nsGALBlendStateHandle handle) = 0;

  /// Bind depth-stencil state.
  virtual void SetDepthStencilState(nsGALDepthStencilStateHandle handle) = 0;

  /// Bind rasterizer state.
  virtual void SetRasterizerState(nsGALRasterizerStateHandle handle) = 0;

  /// Set primitive topology.
  virtual void SetPrimitiveTopology(nsGALPrimitiveTopology topology) = 0;

  /// Bind a vertex buffer.
  virtual void SetVertexBuffer(uint32_t slot, nsGALBufferHandle handle, uint32_t stride, uint32_t offset = 0) = 0;

  /// Bind an index buffer.
  virtual void SetIndexBuffer(nsGALBufferHandle handle, bool b32Bit = false) = 0;

  /// Draw non-indexed geometry.
  virtual void Draw(uint32_t vertexCount, uint32_t startVertex = 0) = 0;

  /// Draw indexed geometry.
  virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndex = 0, int32_t baseVertex = 0) = 0;

  /// Dispatch compute shader.
  virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

  // --- Readback (critical for visual testing) ---

  /// Read texture data back to CPU. Blocks until data is available.
  virtual nsGALResult ReadbackTexture(nsGALTextureHandle handle, uint32_t subresource,
                                       void* pDestBuffer, uint32_t destBufferSize,
                                       uint32_t& outRowPitch) = 0;

  // --- Debug ---
  virtual void PushDebugGroup(const char* szName) = 0;
  virtual void PopDebugGroup() = 0;
  virtual void ConsumeValidationMessages(std::vector<nsValidationMessage>& outMessages) { (void)outMessages; }

  // --- Factory ---

  /// Create a device for the specified API. Returns nullptr if the API is not available.
  static std::unique_ptr<nsGALDevice> CreateDevice(nsGALGraphicsAPI api);

  /// Get a list of available APIs on the current platform.
  static std::vector<nsGALGraphicsAPI> GetAvailableAPIs();

protected:
  nsGALDevice() = default;
};
