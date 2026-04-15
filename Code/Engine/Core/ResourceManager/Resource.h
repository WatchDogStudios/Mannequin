#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Types.h>

/// \brief Resource priority levels.
struct nsResourcePriority
{
  enum Enum : nsUInt8
  {
    Lowest = 0,
    Low,
    Medium,
    High,
    Critical,
  };

  nsUInt8 m_Value = Enum::Medium;

  nsResourcePriority() = default;
  nsResourcePriority(Enum e) : m_Value(e) {}
};

/// \brief Resource flags.
struct nsResourceFlags
{
  using StorageType = nsUInt8;

  enum Enum : nsUInt8
  {
    None = 0,
    UpdateOnMainThread = NS_BIT(0),
    NoFileAccessRequired = NS_BIT(1),
    ResourceHasFallback = NS_BIT(2),
    ResourceHasTypeFallback = NS_BIT(3),
    Default = None
  };

  struct Bits
  {
    StorageType UpdateOnMainThread : 1;
    StorageType NoFileAccessRequired : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsResourceFlags);

/// \brief Resource states.
struct nsResourceState
{
  enum Enum : nsUInt8
  {
    Invalid,
    Unloaded,
    LoadedResourceMissing,
    Loaded,
  };
};

/// \brief Describes the loading state of a resource.
struct nsResourceLoadDesc
{
  nsResourceState::Enum m_State = nsResourceState::Invalid;
  nsUInt32 m_uiQualityLevelsDiscardable = 0;
  nsUInt32 m_uiQualityLevelsLoadable = 0;
};

/// \brief Base class for all resources.
class NS_CORE_DLL nsResource
{
public:
  struct MemoryUsage
  {
    nsUInt64 m_uiMemoryCPU = 0;
    nsUInt64 m_uiMemoryGPU = 0;
  };

  virtual ~nsResource() = default;

  const nsString& GetResourceID() const;
  const nsString& GetResourceDescription() const;
  nsResourcePriority GetPriority() const;
  nsBitflags<nsResourceFlags> GetBaseResourceFlags() const;
  nsResourceLoadDesc GetLoadingState() const;
  MemoryUsage GetMemoryUsage() const;

protected:
  nsString m_sResourceID;
};

/// \brief Typed resource handle.
template <typename ResourceType>
class nsTypedResourceHandle
{
public:
  nsTypedResourceHandle() = default;
  bool IsValid() const { return m_bValid; }
  void Invalidate() { m_bValid = false; }

private:
  bool m_bValid = false;
};
