// Copyright (c) WD Studios. All rights reserved.
// Vulkan Renderer Backend — Stub Implementation
// This file provides the skeleton for Vulkan integration.
// Full implementation requires Vulkan SDK and platform surface extensions.

#include "nsGALDeviceVulkan.h"

// --- Auto-registration ---
static std::unique_ptr<nsGALDevice> CreateVulkanDevice()
{
  return std::make_unique<nsGALDeviceVulkan>();
}

extern void nsGALRegisterBackend(nsGALGraphicsAPI api, std::unique_ptr<nsGALDevice> (*pfnCreate)());

struct VulkanBackendRegistrar
{
  VulkanBackendRegistrar() { nsGALRegisterBackend(nsGALGraphicsAPI::Vulkan, &CreateVulkanDevice); }
};
static VulkanBackendRegistrar s_VulkanRegistrar;

// --- Stub implementations ---
nsGALDeviceVulkan::nsGALDeviceVulkan() = default;
nsGALDeviceVulkan::~nsGALDeviceVulkan() { Shutdown(); }

nsGALResult nsGALDeviceVulkan::Init(const nsGALDeviceCreationDescription&)
{
  m_Caps.m_API = nsGALGraphicsAPI::Vulkan;
  m_Caps.m_sAdapterName = "Vulkan (Not Yet Implemented)";
  // TODO: vkCreateInstance, vkEnumeratePhysicalDevices, vkCreateDevice
  return nsGALResult::NotSupported;
}

void nsGALDeviceVulkan::Shutdown() { /* TODO */ }

nsGALTextureHandle nsGALDeviceVulkan::CreateTexture(const nsGALTextureCreationDescription&) { return {}; }
void nsGALDeviceVulkan::DestroyTexture(nsGALTextureHandle) {}
nsGALBufferHandle nsGALDeviceVulkan::CreateBuffer(const nsGALBufferCreationDescription&) { return {}; }
void nsGALDeviceVulkan::DestroyBuffer(nsGALBufferHandle) {}

nsGALSwapChainHandle nsGALDeviceVulkan::CreateSwapChain(const nsGALSwapChainCreationDescription&) { return {}; }
void nsGALDeviceVulkan::DestroySwapChain(nsGALSwapChainHandle) {}
nsGALResult nsGALDeviceVulkan::Present(nsGALSwapChainHandle) { return nsGALResult::NotSupported; }
nsGALResult nsGALDeviceVulkan::ResizeSwapChain(nsGALSwapChainHandle, uint32_t, uint32_t) { return nsGALResult::NotSupported; }
nsGALTextureHandle nsGALDeviceVulkan::GetBackBufferTexture(nsGALSwapChainHandle) { return {}; }

nsGALBlendStateHandle nsGALDeviceVulkan::CreateBlendState(const nsGALBlendStateCreationDescription&) { return {}; }
void nsGALDeviceVulkan::DestroyBlendState(nsGALBlendStateHandle) {}
nsGALDepthStencilStateHandle nsGALDeviceVulkan::CreateDepthStencilState(const nsGALDepthStencilStateCreationDescription&) { return {}; }
void nsGALDeviceVulkan::DestroyDepthStencilState(nsGALDepthStencilStateHandle) {}
nsGALRasterizerStateHandle nsGALDeviceVulkan::CreateRasterizerState(const nsGALRasterizerStateCreationDescription&) { return {}; }
void nsGALDeviceVulkan::DestroyRasterizerState(nsGALRasterizerStateHandle) {}
nsGALSamplerStateHandle nsGALDeviceVulkan::CreateSamplerState(const nsGALSamplerStateCreationDescription&) { return {}; }
void nsGALDeviceVulkan::DestroySamplerState(nsGALSamplerStateHandle) {}

nsGALResult nsGALDeviceVulkan::BeginFrame() { return nsGALResult::NotSupported; }
nsGALResult nsGALDeviceVulkan::EndFrame() { return nsGALResult::NotSupported; }
void nsGALDeviceVulkan::BeginRenderPass(const nsGALRenderingSetup&) {}
void nsGALDeviceVulkan::EndRenderPass() {}
void nsGALDeviceVulkan::SetViewport(const nsGALViewport&) {}
void nsGALDeviceVulkan::SetScissorRect(const nsGALScissorRect&) {}
void nsGALDeviceVulkan::SetBlendState(nsGALBlendStateHandle) {}
void nsGALDeviceVulkan::SetDepthStencilState(nsGALDepthStencilStateHandle) {}
void nsGALDeviceVulkan::SetRasterizerState(nsGALRasterizerStateHandle) {}
void nsGALDeviceVulkan::SetPrimitiveTopology(nsGALPrimitiveTopology) {}
void nsGALDeviceVulkan::SetVertexBuffer(uint32_t, nsGALBufferHandle, uint32_t, uint32_t) {}
void nsGALDeviceVulkan::SetIndexBuffer(nsGALBufferHandle, bool) {}
void nsGALDeviceVulkan::Draw(uint32_t, uint32_t) {}
void nsGALDeviceVulkan::DrawIndexed(uint32_t, uint32_t, int32_t) {}
void nsGALDeviceVulkan::Dispatch(uint32_t, uint32_t, uint32_t) {}

nsGALResult nsGALDeviceVulkan::ReadbackTexture(nsGALTextureHandle, uint32_t, void*, uint32_t, uint32_t&)
{
  return nsGALResult::NotSupported;
}

void nsGALDeviceVulkan::PushDebugGroup(const char*) {}
void nsGALDeviceVulkan::PopDebugGroup() {}
