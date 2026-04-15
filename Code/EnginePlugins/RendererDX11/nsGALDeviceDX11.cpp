// Copyright (c) WD Studios. All rights reserved.
// DX11 Renderer Backend — Stub Implementation

#include "nsGALDeviceDX11.h"

// --- Auto-registration ---
static std::unique_ptr<nsGALDevice> CreateDX11Device()
{
  return std::make_unique<nsGALDeviceDX11>();
}

extern void nsGALRegisterBackend(nsGALGraphicsAPI api, std::unique_ptr<nsGALDevice> (*pfnCreate)());

struct DX11BackendRegistrar
{
  DX11BackendRegistrar() { nsGALRegisterBackend(nsGALGraphicsAPI::DX11, &CreateDX11Device); }
};
static DX11BackendRegistrar s_DX11Registrar;

// --- Stub implementations ---
nsGALDeviceDX11::nsGALDeviceDX11() = default;
nsGALDeviceDX11::~nsGALDeviceDX11() { Shutdown(); }

nsGALResult nsGALDeviceDX11::Init(const nsGALDeviceCreationDescription&)
{
  m_Caps.m_API = nsGALGraphicsAPI::DX11;
  m_Caps.m_sAdapterName = "DX11 (Not Yet Implemented)";
  // TODO: D3D11CreateDevice, enumerate adapters
  return nsGALResult::NotSupported;
}

void nsGALDeviceDX11::Shutdown() { /* TODO */ }

nsGALTextureHandle nsGALDeviceDX11::CreateTexture(const nsGALTextureCreationDescription&) { return {}; }
void nsGALDeviceDX11::DestroyTexture(nsGALTextureHandle) {}
nsGALBufferHandle nsGALDeviceDX11::CreateBuffer(const nsGALBufferCreationDescription&) { return {}; }
void nsGALDeviceDX11::DestroyBuffer(nsGALBufferHandle) {}

nsGALSwapChainHandle nsGALDeviceDX11::CreateSwapChain(const nsGALSwapChainCreationDescription&) { return {}; }
void nsGALDeviceDX11::DestroySwapChain(nsGALSwapChainHandle) {}
nsGALResult nsGALDeviceDX11::Present(nsGALSwapChainHandle) { return nsGALResult::NotSupported; }
nsGALResult nsGALDeviceDX11::ResizeSwapChain(nsGALSwapChainHandle, uint32_t, uint32_t) { return nsGALResult::NotSupported; }
nsGALTextureHandle nsGALDeviceDX11::GetBackBufferTexture(nsGALSwapChainHandle) { return {}; }

nsGALBlendStateHandle nsGALDeviceDX11::CreateBlendState(const nsGALBlendStateCreationDescription&) { return {}; }
void nsGALDeviceDX11::DestroyBlendState(nsGALBlendStateHandle) {}
nsGALDepthStencilStateHandle nsGALDeviceDX11::CreateDepthStencilState(const nsGALDepthStencilStateCreationDescription&) { return {}; }
void nsGALDeviceDX11::DestroyDepthStencilState(nsGALDepthStencilStateHandle) {}
nsGALRasterizerStateHandle nsGALDeviceDX11::CreateRasterizerState(const nsGALRasterizerStateCreationDescription&) { return {}; }
void nsGALDeviceDX11::DestroyRasterizerState(nsGALRasterizerStateHandle) {}
nsGALSamplerStateHandle nsGALDeviceDX11::CreateSamplerState(const nsGALSamplerStateCreationDescription&) { return {}; }
void nsGALDeviceDX11::DestroySamplerState(nsGALSamplerStateHandle) {}

nsGALResult nsGALDeviceDX11::BeginFrame() { return nsGALResult::NotSupported; }
nsGALResult nsGALDeviceDX11::EndFrame() { return nsGALResult::NotSupported; }
void nsGALDeviceDX11::BeginRenderPass(const nsGALRenderingSetup&) {}
void nsGALDeviceDX11::EndRenderPass() {}
void nsGALDeviceDX11::SetViewport(const nsGALViewport&) {}
void nsGALDeviceDX11::SetScissorRect(const nsGALScissorRect&) {}
void nsGALDeviceDX11::SetBlendState(nsGALBlendStateHandle) {}
void nsGALDeviceDX11::SetDepthStencilState(nsGALDepthStencilStateHandle) {}
void nsGALDeviceDX11::SetRasterizerState(nsGALRasterizerStateHandle) {}
void nsGALDeviceDX11::SetPrimitiveTopology(nsGALPrimitiveTopology) {}
void nsGALDeviceDX11::SetVertexBuffer(uint32_t, nsGALBufferHandle, uint32_t, uint32_t) {}
void nsGALDeviceDX11::SetIndexBuffer(nsGALBufferHandle, bool) {}
void nsGALDeviceDX11::Draw(uint32_t, uint32_t) {}
void nsGALDeviceDX11::DrawIndexed(uint32_t, uint32_t, int32_t) {}
void nsGALDeviceDX11::Dispatch(uint32_t, uint32_t, uint32_t) {}

nsGALResult nsGALDeviceDX11::ReadbackTexture(nsGALTextureHandle, uint32_t, void*, uint32_t, uint32_t&)
{
  return nsGALResult::NotSupported;
}

void nsGALDeviceDX11::PushDebugGroup(const char*) {}
void nsGALDeviceDX11::PopDebugGroup() {}
