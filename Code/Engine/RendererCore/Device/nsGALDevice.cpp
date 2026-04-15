// Copyright (c) WD Studios. All rights reserved.
// Graphics Abstraction Layer - Device Factory Implementation

#include "nsGALDevice.h"

#include <vector>

// Backend registration
struct nsGALBackendEntry
{
  nsGALGraphicsAPI m_API;
  std::unique_ptr<nsGALDevice> (*m_pfnCreate)();
};

static std::vector<nsGALBackendEntry>& GetBackendRegistry()
{
  static std::vector<nsGALBackendEntry> s_Backends;
  return s_Backends;
}

void nsGALRegisterBackend(nsGALGraphicsAPI api, std::unique_ptr<nsGALDevice> (*pfnCreate)())
{
  GetBackendRegistry().push_back({api, pfnCreate});
}

std::unique_ptr<nsGALDevice> nsGALDevice::CreateDevice(nsGALGraphicsAPI api)
{
  for (auto& entry : GetBackendRegistry())
  {
    if (entry.m_API == api)
      return entry.m_pfnCreate();
  }
  return nullptr;
}

std::vector<nsGALGraphicsAPI> nsGALDevice::GetAvailableAPIs()
{
  std::vector<nsGALGraphicsAPI> apis;
  for (auto& entry : GetBackendRegistry())
  {
    apis.push_back(entry.m_API);
  }
  return apis;
}

// Declaration format helpers
uint32_t nsGALResourceFormatGetBitsPerPixel(nsGALResourceFormat fmt)
{
  switch (fmt)
  {
    case nsGALResourceFormat::R8G8B8A8_UNORM:
    case nsGALResourceFormat::R8G8B8A8_SNORM:
    case nsGALResourceFormat::R8G8B8A8_UINT:
    case nsGALResourceFormat::R8G8B8A8_SRGB:
    case nsGALResourceFormat::B8G8R8A8_UNORM:
    case nsGALResourceFormat::B8G8R8A8_SRGB:
    case nsGALResourceFormat::R32_FLOAT:
    case nsGALResourceFormat::R32_UINT:
    case nsGALResourceFormat::R32_SINT:
      return 32;

    case nsGALResourceFormat::R16G16_FLOAT:
    case nsGALResourceFormat::R16_UINT:
    case nsGALResourceFormat::D16_UNORM:
      return 16;

    case nsGALResourceFormat::R16G16B16A16_FLOAT:
    case nsGALResourceFormat::D32_FLOAT_S8X24_UINT:
      return 64;

    case nsGALResourceFormat::R32G32B32A32_FLOAT:
      return 128;

    case nsGALResourceFormat::R32G32B32_FLOAT:
      return 96;

    case nsGALResourceFormat::D24_UNORM_S8_UINT:
    case nsGALResourceFormat::D32_FLOAT:
      return 32;

    case nsGALResourceFormat::BC1_UNORM:
    case nsGALResourceFormat::BC1_SRGB:
      return 4; // 4 bits per pixel compressed

    case nsGALResourceFormat::BC3_UNORM:
    case nsGALResourceFormat::BC3_SRGB:
    case nsGALResourceFormat::BC7_UNORM:
    case nsGALResourceFormat::BC7_SRGB:
      return 8; // 8 bits per pixel compressed

    default:
      return 0;
  }
}
