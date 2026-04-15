#pragma once

#include <Core/CoreDLL.h>
#include <Core/ResourceManager/Resource.h>
#include <Foundation/Strings/String.h>

/// \brief Resource acquire modes.
enum class nsResourceAcquireMode
{
  PointerOnly,
  AllowLoadingFallback,
  AllowLoadingFallback_NeverFail,
  BlockTillLoaded,
  BlockTillLoaded_NeverFail,
  NoFallback,
};

/// \brief Resource acquire result.
enum class nsResourceAcquireResult
{
  None,
  MissingFallback,
  LoadingFallback,
  Final,
};

/// \brief Lock that provides access to a resource.
template <typename T>
class nsResourceLock
{
public:
  nsResourceLock(const nsTypedResourceHandle<T>& hResource, nsResourceAcquireMode mode, const nsTypedResourceHandle<T>& hFallback = {});

  const T* GetPointer() const { return m_pResource; }
  const T* operator->() const { return m_pResource; }
  operator const T*() const { return m_pResource; }

  nsResourceAcquireResult GetAcquireResult() const { return m_Result; }

private:
  const T* m_pResource = nullptr;
  nsResourceAcquireResult m_Result = nsResourceAcquireResult::None;
};

/// \brief Central resource manager.
class NS_CORE_DLL nsResourceManager
{
public:
  template <typename ResourceType>
  static nsTypedResourceHandle<ResourceType> LoadResource(nsStringView sResourceID);

  template <typename ResourceType>
  static nsTypedResourceHandle<ResourceType> LoadResource(nsStringView sResourceID, nsTypedResourceHandle<ResourceType> hFallback);

  static void FreeAllUnusedResources();
};
