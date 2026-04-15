// Copyright (c) WD Studios. All rights reserved.
// Mannequin Visual Test Pipeline - Image Capture Implementation

#include "nsImageCapture.h"

// stb_image_write for PNG output
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// stb_image for PNG input
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void nsCapturedImage::GetPixelFloat(uint32_t x, uint32_t y, float outRGBA[4]) const
{
  outRGBA[0] = outRGBA[1] = outRGBA[2] = outRGBA[3] = 0.0f;

  if (x >= m_uiWidth || y >= m_uiHeight || m_Data.empty())
    return;

  if (m_Format == nsGALResourceFormat::R8G8B8A8_UNORM || m_Format == nsGALResourceFormat::R8G8B8A8_SRGB)
  {
    const uint8_t* pPixel = m_Data.data() + y * m_uiRowPitch + x * 4;
    outRGBA[0] = pPixel[0] / 255.0f;
    outRGBA[1] = pPixel[1] / 255.0f;
    outRGBA[2] = pPixel[2] / 255.0f;
    outRGBA[3] = pPixel[3] / 255.0f;
  }
  else if (m_Format == nsGALResourceFormat::R32G32B32A32_FLOAT)
  {
    const float* pPixel = reinterpret_cast<const float*>(m_Data.data() + y * m_uiRowPitch + x * 16);
    outRGBA[0] = pPixel[0];
    outRGBA[1] = pPixel[1];
    outRGBA[2] = pPixel[2];
    outRGBA[3] = pPixel[3];
  }
}

bool nsCapturedImage::SavePNG(const std::string& path) const
{
  if (!IsValid())
    return false;

  // For PNG we need RGBA8 data
  if (m_Format == nsGALResourceFormat::R8G8B8A8_UNORM || m_Format == nsGALResourceFormat::R8G8B8A8_SRGB)
  {
    return stbi_write_png(path.c_str(), m_uiWidth, m_uiHeight, 4, m_Data.data(), m_uiRowPitch) != 0;
  }

  // Convert float data to RGBA8
  if (m_Format == nsGALResourceFormat::R32G32B32A32_FLOAT)
  {
    std::vector<uint8_t> rgba8(m_uiWidth * m_uiHeight * 4);
    for (uint32_t y = 0; y < m_uiHeight; ++y)
    {
      for (uint32_t x = 0; x < m_uiWidth; ++x)
      {
        float pixel[4];
        GetPixelFloat(x, y, pixel);
        uint8_t* dst = rgba8.data() + (y * m_uiWidth + x) * 4;
        for (int c = 0; c < 4; ++c)
        {
          float v = pixel[c] < 0.0f ? 0.0f : (pixel[c] > 1.0f ? 1.0f : pixel[c]);
          dst[c] = static_cast<uint8_t>(v * 255.0f + 0.5f);
        }
      }
    }
    return stbi_write_png(path.c_str(), m_uiWidth, m_uiHeight, 4, rgba8.data(), m_uiWidth * 4) != 0;
  }

  return false;
}

bool nsCapturedImage::SaveEXR(const std::string& /* path */) const
{
  // TODO: Implement EXR saving via tinyexr
  return false;
}

bool nsCapturedImage::LoadPNG(const std::string& path)
{
  int w, h, channels;
  uint8_t* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
  if (!data)
    return false;

  m_uiWidth = static_cast<uint32_t>(w);
  m_uiHeight = static_cast<uint32_t>(h);
  m_uiRowPitch = m_uiWidth * 4;
  m_Format = nsGALResourceFormat::R8G8B8A8_UNORM;
  m_Data.assign(data, data + m_uiRowPitch * m_uiHeight);

  stbi_image_free(data);
  return true;
}

nsGALResult nsImageCapture::CaptureTexture(nsGALDevice* pDevice, nsGALTextureHandle texture,
                                            uint32_t subresource, nsCapturedImage& outImage)
{
  if (!pDevice || !texture.IsValid())
    return nsGALResult::InvalidArgument;

  // Allocate buffer — caller must know texture dimensions beforehand
  // For now, use a generous allocation; the readback will fill in the actual data
  constexpr uint32_t kMaxReadbackSize = 3840 * 2160 * 16; // 4K RGBA32F
  outImage.m_Data.resize(kMaxReadbackSize);

  uint32_t rowPitch = 0;
  nsGALResult result = pDevice->ReadbackTexture(texture, subresource,
                                                 outImage.m_Data.data(),
                                                 static_cast<uint32_t>(outImage.m_Data.size()),
                                                 rowPitch);

  if (NS_GAL_FAILED(result))
  {
    outImage.m_Data.clear();
    return result;
  }

  outImage.m_uiRowPitch = rowPitch;
  return nsGALResult::Success;
}

nsGALResult nsImageCapture::CaptureBackBuffer(nsGALDevice* pDevice, nsGALSwapChainHandle swapChain,
                                               nsCapturedImage& outImage)
{
  if (!pDevice || !swapChain.IsValid())
    return nsGALResult::InvalidArgument;

  nsGALTextureHandle backBuffer = pDevice->GetBackBufferTexture(swapChain);
  if (!backBuffer.IsValid())
    return nsGALResult::Failure;

  return CaptureTexture(pDevice, backBuffer, 0, outImage);
}
