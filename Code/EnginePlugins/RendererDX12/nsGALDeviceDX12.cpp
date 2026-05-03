// Copyright (c) WD Studios. All rights reserved.
// DX12 Renderer Backend Implementation

#include "nsGALDeviceDX12.h"

#include <d3d12sdklayers.h>
#include <utility>
#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace
{
  const char* ToValidationSeverity(D3D12_MESSAGE_SEVERITY severity)
  {
    switch (severity)
    {
      case D3D12_MESSAGE_SEVERITY_CORRUPTION:
      case D3D12_MESSAGE_SEVERITY_ERROR:
        return "Fatal";
      case D3D12_MESSAGE_SEVERITY_WARNING:
        return "Warning";
      case D3D12_MESSAGE_SEVERITY_INFO:
      case D3D12_MESSAGE_SEVERITY_MESSAGE:
      default:
        return "Info";
    }
  }
}

// --- Auto-registration ---
static std::unique_ptr<nsGALDevice> CreateDX12Device()
{
  return std::make_unique<nsGALDeviceDX12>();
}

struct DX12BackendRegistrar
{
  DX12BackendRegistrar() { nsGALRegisterBackend(nsGALGraphicsAPI::DX12, &CreateDX12Device); }
};
static DX12BackendRegistrar s_DX12Registrar;

// --- Construction ---
nsGALDeviceDX12::nsGALDeviceDX12() = default;
nsGALDeviceDX12::~nsGALDeviceDX12() { Shutdown(); }

// --- Lifecycle ---
nsGALResult nsGALDeviceDX12::Init(const nsGALDeviceCreationDescription& desc)
{
  UINT dxgiFactoryFlags = 0;

  // Enable debug layer
  if (desc.m_bDebugDevice)
  {
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
      debugController->EnableDebugLayer();
      dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

      if (desc.m_bGPUValidation)
      {
        ComPtr<ID3D12Debug1> debugController1;
        if (SUCCEEDED(debugController.As(&debugController1)))
          debugController1->SetEnableGPUBasedValidation(TRUE);
      }
    }
  }

  // Create DXGI factory
  if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_pFactory))))
    return nsGALResult::Failure;

  // Enumerate adapters and select
  ComPtr<IDXGIAdapter1> adapter;
  ComPtr<IDXGIAdapter1> bestAdapter;
  SIZE_T maxVideoMem = 0;

  for (UINT i = 0; m_pFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
  {
    DXGI_ADAPTER_DESC1 adapterDesc;
    adapter->GetDesc1(&adapterDesc);

    if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
      continue;

    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
    {
      if (desc.m_iPreferredAdapterIndex >= 0 && static_cast<int32_t>(i) == desc.m_iPreferredAdapterIndex)
      {
        bestAdapter = adapter;
        break;
      }
      if (adapterDesc.DedicatedVideoMemory > maxVideoMem)
      {
        maxVideoMem = adapterDesc.DedicatedVideoMemory;
        bestAdapter = adapter;
      }
    }
  }

  if (!bestAdapter)
    return nsGALResult::Failure;

  // Create device
  if (FAILED(D3D12CreateDevice(bestAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice))))
    return nsGALResult::Failure;
  m_pDevice.As(&m_pInfoQueue);

  // Fill capabilities
  DXGI_ADAPTER_DESC1 adapterDesc;
  bestAdapter->GetDesc1(&adapterDesc);

  m_Caps.m_API = nsGALGraphicsAPI::DX12;
  char adapterName[128];
  wcstombs(adapterName, adapterDesc.Description, sizeof(adapterName));
  m_Caps.m_sAdapterName = adapterName;
  m_Caps.m_uiDedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;
  m_Caps.m_uiMaxTextureSize = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
  m_Caps.m_uiMaxRenderTargets = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
  m_Caps.m_uiMaxComputeWorkGroupSize[0] = D3D12_CS_THREAD_GROUP_MAX_X;
  m_Caps.m_uiMaxComputeWorkGroupSize[1] = D3D12_CS_THREAD_GROUP_MAX_Y;
  m_Caps.m_uiMaxComputeWorkGroupSize[2] = D3D12_CS_THREAD_GROUP_MAX_Z;
  m_Caps.m_bSupportsCompute = true;
  m_Caps.m_bSupportsTessellation = true;
  m_Caps.m_bSupportsGeometryShaders = true;
  m_Caps.m_bSupportsMultiViewport = true;
  for (int i = 0; i < static_cast<int>(nsGALShaderStage::Count); ++i)
    m_Caps.m_bShaderStageSupported[i] = true;

  // Check advanced features
  D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
  if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
    m_Caps.m_bSupportsRaytracing = (options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);

  D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
  if (SUCCEEDED(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))))
    m_Caps.m_bSupportsMeshShaders = (options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1);

  // Create command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  if (FAILED(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
    return nsGALResult::Failure;

  // Create command allocators
  for (uint32_t i = 0; i < FrameCount; ++i)
  {
    if (FAILED(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                  IID_PPV_ARGS(&m_pCommandAllocators[i]))))
      return nsGALResult::Failure;
  }

  // Create command list
  if (FAILED(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                           m_pCommandAllocators[0].Get(), nullptr,
                                           IID_PPV_ARGS(&m_pCommandList))))
    return nsGALResult::Failure;
  m_pCommandList->Close();

  // Create fence
  if (FAILED(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
    return nsGALResult::Failure;
  m_hFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

  // Create descriptor heaps
  auto CreateHeap = [&](D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count, bool shaderVisible,
                        ComPtr<ID3D12DescriptorHeap>& outHeap) -> bool
  {
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = type;
    heapDesc.NumDescriptors = count;
    heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    return SUCCEEDED(m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&outHeap)));
  };

  if (!CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 64, false, m_pRTVHeap))
    return nsGALResult::Failure;
  if (!CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 32, false, m_pDSVHeap))
    return nsGALResult::Failure;
  if (!CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024, true, m_pSRVHeap))
    return nsGALResult::Failure;
  if (!CreateHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 64, true, m_pSamplerHeap))
    return nsGALResult::Failure;

  m_uiRTVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  m_uiDSVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  m_uiSRVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  m_uiSamplerDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

  return nsGALResult::Success;
}

void nsGALDeviceDX12::Shutdown()
{
  if (m_pDevice)
  {
    WaitForGPU();

    m_Textures.clear();
    m_Buffers.clear();
    m_SwapChains.clear();

    if (m_hFenceEvent)
    {
      CloseHandle(m_hFenceEvent);
      m_hFenceEvent = nullptr;
    }

    m_pFence.Reset();
    m_pCommandList.Reset();
    for (auto& alloc : m_pCommandAllocators)
      alloc.Reset();
    m_pCommandQueue.Reset();
    m_pRTVHeap.Reset();
    m_pDSVHeap.Reset();
    m_pSRVHeap.Reset();
    m_pSamplerHeap.Reset();
    m_pDevice.Reset();
    m_pFactory.Reset();
  }
}

// --- Synchronization Helpers ---
void nsGALDeviceDX12::WaitForGPU()
{
  if (!m_pCommandQueue || !m_pFence)
    return;

  const uint64_t fenceValue = m_uiFenceValues[m_uiCurrentFrame];
  m_pCommandQueue->Signal(m_pFence.Get(), fenceValue);

  if (m_pFence->GetCompletedValue() < fenceValue)
  {
    m_pFence->SetEventOnCompletion(fenceValue, m_hFenceEvent);
    WaitForSingleObject(m_hFenceEvent, INFINITE);
  }

  m_uiFenceValues[m_uiCurrentFrame]++;
}

void nsGALDeviceDX12::MoveToNextFrame()
{
  const uint64_t currentFenceValue = m_uiFenceValues[m_uiCurrentFrame];
  m_pCommandQueue->Signal(m_pFence.Get(), currentFenceValue);

  m_uiCurrentFrame = (m_uiCurrentFrame + 1) % FrameCount;

  if (m_pFence->GetCompletedValue() < m_uiFenceValues[m_uiCurrentFrame])
  {
    m_pFence->SetEventOnCompletion(m_uiFenceValues[m_uiCurrentFrame], m_hFenceEvent);
    WaitForSingleObject(m_hFenceEvent, INFINITE);
  }

  m_uiFenceValues[m_uiCurrentFrame] = currentFenceValue + 1;
}

// --- Format Conversion ---
DXGI_FORMAT nsGALDeviceDX12::GetDXGIFormat(nsGALResourceFormat format) const
{
  switch (format)
  {
    case nsGALResourceFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case nsGALResourceFormat::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
    case nsGALResourceFormat::R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
    case nsGALResourceFormat::R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case nsGALResourceFormat::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
    case nsGALResourceFormat::B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    case nsGALResourceFormat::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case nsGALResourceFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case nsGALResourceFormat::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
    case nsGALResourceFormat::R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
    case nsGALResourceFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
    case nsGALResourceFormat::R32_UINT: return DXGI_FORMAT_R32_UINT;
    case nsGALResourceFormat::R32_SINT: return DXGI_FORMAT_R32_SINT;
    case nsGALResourceFormat::R16_UINT: return DXGI_FORMAT_R16_UINT;
    case nsGALResourceFormat::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
    case nsGALResourceFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case nsGALResourceFormat::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
    case nsGALResourceFormat::D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    case nsGALResourceFormat::BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
    case nsGALResourceFormat::BC1_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
    case nsGALResourceFormat::BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
    case nsGALResourceFormat::BC3_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
    case nsGALResourceFormat::BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
    case nsGALResourceFormat::BC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
    default: return DXGI_FORMAT_UNKNOWN;
  }
}

D3D12_RESOURCE_FLAGS nsGALDeviceDX12::GetResourceFlags(const nsGALTextureCreationDescription& desc) const
{
  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
  if (desc.m_bCreateRenderTarget)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  if (desc.m_bAllowUAV)
    flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  if (nsGALResourceFormatIsDepth(desc.m_Format))
    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  if (!desc.m_bAllowShaderResourceView && nsGALResourceFormatIsDepth(desc.m_Format))
    flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
  return flags;
}

void nsGALDeviceDX12::TransitionResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
  if (before == after)
    return;

  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = pResource;
  barrier.Transition.StateBefore = before;
  barrier.Transition.StateAfter = after;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  m_pCommandList->ResourceBarrier(1, &barrier);
}

// --- Resource Creation ---
nsGALTextureHandle nsGALDeviceDX12::CreateTexture(const nsGALTextureCreationDescription& desc)
{
  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension = desc.m_uiDepth > 1 ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  resourceDesc.Width = desc.m_uiWidth;
  resourceDesc.Height = desc.m_uiHeight;
  resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.m_uiDepth > 1 ? desc.m_uiDepth : desc.m_uiArraySize);
  resourceDesc.MipLevels = static_cast<UINT16>(desc.m_uiMipLevelCount);
  resourceDesc.Format = GetDXGIFormat(desc.m_Format);
  resourceDesc.SampleDesc.Count = desc.m_uiSampleCount;
  resourceDesc.SampleDesc.Quality = 0;
  resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  resourceDesc.Flags = GetResourceFlags(desc);

  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

  D3D12_CLEAR_VALUE clearValue = {};
  D3D12_CLEAR_VALUE* pClearValue = nullptr;

  if (desc.m_bCreateRenderTarget)
  {
    clearValue.Format = resourceDesc.Format;
    pClearValue = &clearValue;
  }
  else if (nsGALResourceFormatIsDepth(desc.m_Format))
  {
    clearValue.Format = resourceDesc.Format;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;
    pClearValue = &clearValue;
  }

  DX12Texture tex;
  tex.m_Desc = desc;
  tex.m_CurrentState = D3D12_RESOURCE_STATE_COMMON;

  if (FAILED(m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                  &resourceDesc, tex.m_CurrentState,
                                                  pClearValue, IID_PPV_ARGS(&tex.m_pResource))))
  {
    return nsGALTextureHandle();
  }

  uint64_t id = m_uiNextResourceID++;
  m_Textures[id] = std::move(tex);
  return nsGALTextureHandle(id);
}

void nsGALDeviceDX12::DestroyTexture(nsGALTextureHandle handle)
{
  m_Textures.erase(handle.m_uiInternalID);
}

nsGALBufferHandle nsGALDeviceDX12::CreateBuffer(const nsGALBufferCreationDescription& desc)
{
  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  resourceDesc.Width = desc.m_uiByteSize;
  resourceDesc.Height = 1;
  resourceDesc.DepthOrArraySize = 1;
  resourceDesc.MipLevels = 1;
  resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
  resourceDesc.SampleDesc.Count = 1;
  resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  resourceDesc.Flags = desc.m_bAllowUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = (desc.m_Usage == nsGALResourceUsage::Dynamic) ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;

  DX12Buffer buf;
  buf.m_Desc = desc;
  buf.m_CurrentState = (heapProps.Type == D3D12_HEAP_TYPE_UPLOAD) ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;

  if (FAILED(m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                  &resourceDesc, buf.m_CurrentState,
                                                  nullptr, IID_PPV_ARGS(&buf.m_pResource))))
  {
    return nsGALBufferHandle();
  }

  uint64_t id = m_uiNextResourceID++;
  m_Buffers[id] = std::move(buf);
  return nsGALBufferHandle(id);
}

void nsGALDeviceDX12::DestroyBuffer(nsGALBufferHandle handle)
{
  m_Buffers.erase(handle.m_uiInternalID);
}

// --- Swap Chain ---
nsGALSwapChainHandle nsGALDeviceDX12::CreateSwapChain(const nsGALSwapChainCreationDescription& desc)
{
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.Width = desc.m_uiWidth;
  swapChainDesc.Height = desc.m_uiHeight;
  swapChainDesc.Format = GetDXGIFormat(desc.m_BackBufferFormat);
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = desc.m_uiBackBufferCount;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  ComPtr<IDXGISwapChain1> swapChain1;
  if (FAILED(m_pFactory->CreateSwapChainForHwnd(m_pCommandQueue.Get(),
                                                 static_cast<HWND>(desc.m_pWindowHandle),
                                                 &swapChainDesc, nullptr, nullptr, &swapChain1)))
  {
    return nsGALSwapChainHandle();
  }

  DX12SwapChain sc;
  sc.m_Desc = desc;
  swapChain1.As(&sc.m_pSwapChain);

  // Create back buffer texture handles
  for (uint32_t i = 0; i < desc.m_uiBackBufferCount; ++i)
  {
    DX12Texture tex;
    sc.m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&tex.m_pResource));
    tex.m_CurrentState = D3D12_RESOURCE_STATE_PRESENT;
    tex.m_Desc.m_uiWidth = desc.m_uiWidth;
    tex.m_Desc.m_uiHeight = desc.m_uiHeight;
    tex.m_Desc.m_Format = desc.m_BackBufferFormat;

    uint64_t texId = m_uiNextResourceID++;
    m_Textures[texId] = std::move(tex);
    sc.m_BackBufferTextures.push_back(nsGALTextureHandle(texId));
  }

  uint64_t id = m_uiNextResourceID++;
  m_SwapChains[id] = std::move(sc);
  return nsGALSwapChainHandle(id);
}

void nsGALDeviceDX12::DestroySwapChain(nsGALSwapChainHandle handle)
{
  auto it = m_SwapChains.find(handle.m_uiInternalID);
  if (it != m_SwapChains.end())
  {
    for (auto& texHandle : it->second.m_BackBufferTextures)
      m_Textures.erase(texHandle.m_uiInternalID);
    m_SwapChains.erase(it);
  }
}

nsGALResult nsGALDeviceDX12::Present(nsGALSwapChainHandle handle)
{
  auto it = m_SwapChains.find(handle.m_uiInternalID);
  if (it == m_SwapChains.end())
    return nsGALResult::InvalidArgument;

  UINT syncInterval = it->second.m_Desc.m_bVSync ? 1 : 0;
  HRESULT hr = it->second.m_pSwapChain->Present(syncInterval, 0);
  return SUCCEEDED(hr) ? nsGALResult::Success : nsGALResult::DeviceLost;
}

nsGALResult nsGALDeviceDX12::ResizeSwapChain(nsGALSwapChainHandle handle, uint32_t width, uint32_t height)
{
  auto it = m_SwapChains.find(handle.m_uiInternalID);
  if (it == m_SwapChains.end())
    return nsGALResult::InvalidArgument;

  WaitForGPU();

  // Release back buffer references
  for (auto& texHandle : it->second.m_BackBufferTextures)
    m_Textures.erase(texHandle.m_uiInternalID);
  it->second.m_BackBufferTextures.clear();

  // Resize
  HRESULT hr = it->second.m_pSwapChain->ResizeBuffers(it->second.m_Desc.m_uiBackBufferCount,
                                                        width, height,
                                                        GetDXGIFormat(it->second.m_Desc.m_BackBufferFormat), 0);
  if (FAILED(hr))
    return nsGALResult::Failure;

  it->second.m_Desc.m_uiWidth = width;
  it->second.m_Desc.m_uiHeight = height;

  // Recreate back buffer handles
  for (uint32_t i = 0; i < it->second.m_Desc.m_uiBackBufferCount; ++i)
  {
    DX12Texture tex;
    it->second.m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&tex.m_pResource));
    tex.m_CurrentState = D3D12_RESOURCE_STATE_PRESENT;
    tex.m_Desc.m_uiWidth = width;
    tex.m_Desc.m_uiHeight = height;
    tex.m_Desc.m_Format = it->second.m_Desc.m_BackBufferFormat;

    uint64_t texId = m_uiNextResourceID++;
    m_Textures[texId] = std::move(tex);
    it->second.m_BackBufferTextures.push_back(nsGALTextureHandle(texId));
  }

  return nsGALResult::Success;
}

nsGALTextureHandle nsGALDeviceDX12::GetBackBufferTexture(nsGALSwapChainHandle handle)
{
  auto it = m_SwapChains.find(handle.m_uiInternalID);
  if (it == m_SwapChains.end())
    return nsGALTextureHandle();

  uint32_t idx = it->second.m_pSwapChain->GetCurrentBackBufferIndex();
  return it->second.m_BackBufferTextures[idx];
}

// --- State Objects (stored as descriptions for PSO compilation) ---
nsGALBlendStateHandle nsGALDeviceDX12::CreateBlendState(const nsGALBlendStateCreationDescription&)
{
  return nsGALBlendStateHandle(m_uiNextResourceID++);
}
void nsGALDeviceDX12::DestroyBlendState(nsGALBlendStateHandle) {}

nsGALDepthStencilStateHandle nsGALDeviceDX12::CreateDepthStencilState(const nsGALDepthStencilStateCreationDescription&)
{
  return nsGALDepthStencilStateHandle(m_uiNextResourceID++);
}
void nsGALDeviceDX12::DestroyDepthStencilState(nsGALDepthStencilStateHandle) {}

nsGALRasterizerStateHandle nsGALDeviceDX12::CreateRasterizerState(const nsGALRasterizerStateCreationDescription&)
{
  return nsGALRasterizerStateHandle(m_uiNextResourceID++);
}
void nsGALDeviceDX12::DestroyRasterizerState(nsGALRasterizerStateHandle) {}

nsGALSamplerStateHandle nsGALDeviceDX12::CreateSamplerState(const nsGALSamplerStateCreationDescription&)
{
  return nsGALSamplerStateHandle(m_uiNextResourceID++);
}
void nsGALDeviceDX12::DestroySamplerState(nsGALSamplerStateHandle) {}

// --- Command Recording ---
nsGALResult nsGALDeviceDX12::BeginFrame()
{
  HRESULT hr = m_pCommandAllocators[m_uiCurrentFrame]->Reset();
  if (FAILED(hr))
    return nsGALResult::Failure;

  hr = m_pCommandList->Reset(m_pCommandAllocators[m_uiCurrentFrame].Get(), nullptr);
  return SUCCEEDED(hr) ? nsGALResult::Success : nsGALResult::Failure;
}

nsGALResult nsGALDeviceDX12::EndFrame()
{
  HRESULT hr = m_pCommandList->Close();
  if (FAILED(hr))
    return nsGALResult::Failure;

  ID3D12CommandList* ppCommandLists[] = {m_pCommandList.Get()};
  m_pCommandQueue->ExecuteCommandLists(1, ppCommandLists);

  MoveToNextFrame();
  return nsGALResult::Success;
}

void nsGALDeviceDX12::BeginRenderPass(const nsGALRenderingSetup& setup)
{
  // Transition render targets and set them
  // Simplified — full implementation would track per-RT state
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRTVHeap->GetCPUDescriptorHandleForHeapStart();

  for (uint32_t i = 0; i < setup.m_uiRenderTargetCount; ++i)
  {
    auto it = m_Textures.find(setup.m_RenderTargetViews[i]);
    if (it != m_Textures.end())
    {
      TransitionResource(it->second.m_pResource.Get(), it->second.m_CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
      it->second.m_CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;

      D3D12_CPU_DESCRIPTOR_HANDLE rtv = {rtvHandle.ptr + i * m_uiRTVDescriptorSize};
      m_pDevice->CreateRenderTargetView(it->second.m_pResource.Get(), nullptr, rtv);

      if (setup.m_bClearColor)
        m_pCommandList->ClearRenderTargetView(rtv, setup.m_ClearColor, 0, nullptr);
    }
  }

  m_pCommandList->OMSetRenderTargets(setup.m_uiRenderTargetCount, &rtvHandle, FALSE, nullptr);
}

void nsGALDeviceDX12::EndRenderPass()
{
  // Render pass end — barriers handled in next BeginRenderPass or Present
}

void nsGALDeviceDX12::SetViewport(const nsGALViewport& viewport)
{
  D3D12_VIEWPORT vp;
  vp.TopLeftX = viewport.m_fX;
  vp.TopLeftY = viewport.m_fY;
  vp.Width = viewport.m_fWidth;
  vp.Height = viewport.m_fHeight;
  vp.MinDepth = viewport.m_fMinDepth;
  vp.MaxDepth = viewport.m_fMaxDepth;
  m_pCommandList->RSSetViewports(1, &vp);
}

void nsGALDeviceDX12::SetScissorRect(const nsGALScissorRect& rect)
{
  D3D12_RECT r;
  r.left = rect.m_iLeft;
  r.top = rect.m_iTop;
  r.right = rect.m_iRight;
  r.bottom = rect.m_iBottom;
  m_pCommandList->RSSetScissorRects(1, &r);
}

void nsGALDeviceDX12::SetBlendState(nsGALBlendStateHandle) { /* Stored in PSO */ }
void nsGALDeviceDX12::SetDepthStencilState(nsGALDepthStencilStateHandle) { /* Stored in PSO */ }
void nsGALDeviceDX12::SetRasterizerState(nsGALRasterizerStateHandle) { /* Stored in PSO */ }

void nsGALDeviceDX12::SetPrimitiveTopology(nsGALPrimitiveTopology topology)
{
  D3D_PRIMITIVE_TOPOLOGY d3dTopology;
  switch (topology)
  {
    case nsGALPrimitiveTopology::Points: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST; break;
    case nsGALPrimitiveTopology::Lines: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
    case nsGALPrimitiveTopology::LineStrip: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
    case nsGALPrimitiveTopology::Triangles: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
    case nsGALPrimitiveTopology::TriangleStrip: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
    default: d3dTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
  }
  m_pCommandList->IASetPrimitiveTopology(d3dTopology);
}

void nsGALDeviceDX12::SetVertexBuffer(uint32_t slot, nsGALBufferHandle handle, uint32_t stride, uint32_t offset)
{
  auto it = m_Buffers.find(handle.m_uiInternalID);
  if (it == m_Buffers.end())
    return;

  D3D12_VERTEX_BUFFER_VIEW vbv;
  vbv.BufferLocation = it->second.m_pResource->GetGPUVirtualAddress() + offset;
  vbv.SizeInBytes = it->second.m_Desc.m_uiByteSize - offset;
  vbv.StrideInBytes = stride;
  m_pCommandList->IASetVertexBuffers(slot, 1, &vbv);
}

void nsGALDeviceDX12::SetIndexBuffer(nsGALBufferHandle handle, bool b32Bit)
{
  auto it = m_Buffers.find(handle.m_uiInternalID);
  if (it == m_Buffers.end())
    return;

  D3D12_INDEX_BUFFER_VIEW ibv;
  ibv.BufferLocation = it->second.m_pResource->GetGPUVirtualAddress();
  ibv.SizeInBytes = it->second.m_Desc.m_uiByteSize;
  ibv.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
  m_pCommandList->IASetIndexBuffer(&ibv);
}

void nsGALDeviceDX12::Draw(uint32_t vertexCount, uint32_t startVertex)
{
  m_pCommandList->DrawInstanced(vertexCount, 1, startVertex, 0);
}

void nsGALDeviceDX12::DrawIndexed(uint32_t indexCount, uint32_t startIndex, int32_t baseVertex)
{
  m_pCommandList->DrawIndexedInstanced(indexCount, 1, startIndex, baseVertex, 0);
}

void nsGALDeviceDX12::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
  m_pCommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
}

// --- Readback ---
nsGALResult nsGALDeviceDX12::ReadbackTexture(nsGALTextureHandle handle, uint32_t subresource,
                                              void* pDestBuffer, uint32_t destBufferSize,
                                              uint32_t& outRowPitch)
{
  auto it = m_Textures.find(handle.m_uiInternalID);
  if (it == m_Textures.end())
    return nsGALResult::InvalidArgument;

  auto& tex = it->second;

  // Get texture layout info
  D3D12_RESOURCE_DESC texDesc = tex.m_pResource->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
  uint64_t totalBytes;
  m_pDevice->GetCopyableFootprints(&texDesc, subresource, 1, 0, &layout, nullptr, nullptr, &totalBytes);

  if (totalBytes > destBufferSize)
    return nsGALResult::OutOfMemory;

  // Create readback buffer
  D3D12_RESOURCE_DESC bufDesc = {};
  bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  bufDesc.Width = totalBytes;
  bufDesc.Height = 1;
  bufDesc.DepthOrArraySize = 1;
  bufDesc.MipLevels = 1;
  bufDesc.SampleDesc.Count = 1;
  bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  D3D12_HEAP_PROPERTIES readbackHeap = {};
  readbackHeap.Type = D3D12_HEAP_TYPE_READBACK;

  ComPtr<ID3D12Resource> readbackBuffer;
  if (FAILED(m_pDevice->CreateCommittedResource(&readbackHeap, D3D12_HEAP_FLAG_NONE,
                                                  &bufDesc, D3D12_RESOURCE_STATE_COPY_DEST,
                                                  nullptr, IID_PPV_ARGS(&readbackBuffer))))
  {
    return nsGALResult::Failure;
  }

  // Transition texture to copy source
  TransitionResource(tex.m_pResource.Get(), tex.m_CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE);

  // Copy texture to readback buffer
  D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
  srcLoc.pResource = tex.m_pResource.Get();
  srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  srcLoc.SubresourceIndex = subresource;

  D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
  dstLoc.pResource = readbackBuffer.Get();
  dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  dstLoc.PlacedFootprint = layout;

  m_pCommandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

  // Transition back
  TransitionResource(tex.m_pResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, tex.m_CurrentState);

  // Execute and wait
  m_pCommandList->Close();
  ID3D12CommandList* ppCommandLists[] = {m_pCommandList.Get()};
  m_pCommandQueue->ExecuteCommandLists(1, ppCommandLists);
  WaitForGPU();

  // Map and copy data
  void* pMappedData = nullptr;
  D3D12_RANGE readRange = {0, static_cast<SIZE_T>(totalBytes)};
  if (FAILED(readbackBuffer->Map(0, &readRange, &pMappedData)))
    return nsGALResult::Failure;

  memcpy(pDestBuffer, pMappedData, static_cast<size_t>(totalBytes));
  outRowPitch = layout.Footprint.RowPitch;

  D3D12_RANGE writeRange = {0, 0};
  readbackBuffer->Unmap(0, &writeRange);

  // Reset command list for further use
  m_pCommandAllocators[m_uiCurrentFrame]->Reset();
  m_pCommandList->Reset(m_pCommandAllocators[m_uiCurrentFrame].Get(), nullptr);

  return nsGALResult::Success;
}

// --- Debug ---
void nsGALDeviceDX12::PushDebugGroup(const char* szName)
{
  // PIX event — requires PIX headers for full implementation
  (void)szName;
}

void nsGALDeviceDX12::PopDebugGroup()
{
}

void nsGALDeviceDX12::ConsumeValidationMessages(std::vector<nsValidationMessage>& outMessages)
{
  if (!m_pInfoQueue)
    return;

  const uint64_t messageCount = m_pInfoQueue->GetNumStoredMessages();
  for (uint64_t i = m_uiConsumedInfoQueueMessages; i < messageCount; ++i)
  {
    SIZE_T messageLength = 0;
    if (FAILED(m_pInfoQueue->GetMessage(i, nullptr, &messageLength)) || messageLength == 0)
      continue;

    std::vector<char> storage(messageLength);
    auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
    if (FAILED(m_pInfoQueue->GetMessage(i, message, &messageLength)))
      continue;

    nsValidationMessage validation;
    validation.m_sSource = "D3D12";
    validation.m_sSeverity = ToValidationSeverity(message->Severity);
    validation.m_sMessage = message->pDescription ? message->pDescription : "";
    validation.m_sRecommendation =
      validation.m_sSeverity == "Fatal"
        ? "Treat D3D12 validation errors as fatal for this test; inspect the RHI call sequence and resource state transitions."
        : "Review the D3D12 validation warning and either fix the backend call sequence or explicitly document why it is expected.";
    outMessages.push_back(std::move(validation));
  }

  m_uiConsumedInfoQueueMessages = messageCount;
}
