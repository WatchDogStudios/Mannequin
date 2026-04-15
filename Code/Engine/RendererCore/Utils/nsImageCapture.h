#pragma once

// Copyright (c) WD Studios. All rights reserved.
// Mannequin Visual Test Pipeline - Image Capture Utility
// Captures GPU render target contents for comparison testing.

#include "../RendererCore/Device/nsGALDevice.h"

#include <vector>
#include <string>
#include <cstdint>

/// Raw CPU-side image data captured from a render target.
struct nsCapturedImage
{
  std::vector<uint8_t> m_Data;
  uint32_t m_uiWidth = 0;
  uint32_t m_uiHeight = 0;
  uint32_t m_uiRowPitch = 0;
  nsGALResourceFormat m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;

  bool IsValid() const { return !m_Data.empty() && m_uiWidth > 0 && m_uiHeight > 0; }

  /// Get a pixel value as float RGBA (normalized). Returns {0,0,0,0} if out of bounds.
  void GetPixelFloat(uint32_t x, uint32_t y, float outRGBA[4]) const;

  /// Save to a PNG file. Returns true on success.
  bool SavePNG(const std::string& path) const;

  /// Save to an EXR file (HDR). Returns true on success.
  bool SaveEXR(const std::string& path) const;

  /// Load from a PNG file. Returns true on success.
  bool LoadPNG(const std::string& path);
};

/// Captures the contents of a render target from the GPU.
class nsImageCapture
{
public:
  /// Capture a texture from the GPU device into CPU memory.
  static nsGALResult CaptureTexture(nsGALDevice* pDevice, nsGALTextureHandle texture,
                                     uint32_t subresource, nsCapturedImage& outImage);

  /// Capture the current back buffer of a swap chain.
  static nsGALResult CaptureBackBuffer(nsGALDevice* pDevice, nsGALSwapChainHandle swapChain,
                                        nsCapturedImage& outImage);
};
