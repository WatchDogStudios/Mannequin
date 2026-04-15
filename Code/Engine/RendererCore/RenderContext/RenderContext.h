#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <Core/ResourceManager/Resource.h>

/// \brief Texture filter settings.
enum class nsTextureFilterSetting
{
  FixedNearest,
  FixedBilinear,
  FixedTrilinear,
  FixedAnisotropic2x,
  FixedAnisotropic4x,
  FixedAnisotropic8x,
  FixedAnisotropic16x,
  LowestQuality,
  LowQuality,
  DefaultQuality,
  HighQuality,
  HighestQuality,
};

/// \brief Shader bind flags.
struct nsShaderBindFlags
{
  using StorageType = nsUInt32;

  enum Enum : nsUInt32
  {
    None = 0,
    ForceRebind = NS_BIT(0),
    NoRasterizerState = NS_BIT(1),
    NoBlendState = NS_BIT(2),
    NoDepthStencilState = NS_BIT(3),
    Default = None
  };

  struct Bits
  {
    StorageType ForceRebind : 1;
    StorageType NoRasterizerState : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsShaderBindFlags);

/// \brief Manages rendering state and shader binding.
class NS_RENDERERCORE_DLL nsRenderContext
{
public:
  static nsRenderContext* GetDefaultInstance();

  void SetDefaultTextureFilter(nsTextureFilterSetting setting);
  nsTextureFilterSetting GetDefaultTextureFilter() const;

  void BindShader(const nsShaderResourceHandle& hShader, nsBitflags<nsShaderBindFlags> flags = nsShaderBindFlags::Default);

private:
  nsTextureFilterSetting m_DefaultTextureFilter = nsTextureFilterSetting::DefaultQuality;
};

/// \brief Handle to constant buffer storage.
struct nsConstantBufferStorageHandle
{
};

class nsShaderResource;
using nsShaderResourceHandle = nsTypedResourceHandle<nsShaderResource>;
