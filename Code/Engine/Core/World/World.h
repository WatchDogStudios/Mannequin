#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Math/Declarations.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Status.h>

class nsWorldReader;
class nsWorldWriter;

/// \brief Description for creating an nsWorld.
struct NS_CORE_DLL nsWorldDesc
{
  nsWorldDesc(nsStringView sName)
    : m_sName(sName)
  {
  }

  nsString m_sName;
  nsUInt32 m_uiRandomNumberGeneratorSeed = 0;
};

/// \brief Handle type for game objects.
struct nsGameObjectHandle
{
  bool IsInvalidated() const { return !m_bValid; }
  void Invalidate() { m_bValid = false; }
  bool operator==(const nsGameObjectHandle& rhs) const { return m_bValid == rhs.m_bValid; }
  bool operator!=(const nsGameObjectHandle& rhs) const { return m_bValid != rhs.m_bValid; }

private:
  bool m_bValid = false;
};

/// \brief Handle type for components.
struct nsComponentHandle
{
  bool IsInvalidated() const { return !m_bValid; }
  void Invalidate() { m_bValid = false; }

private:
  bool m_bValid = false;
};

/// \brief Description for creating game objects.
struct nsGameObjectDesc
{
  nsHashedString m_sName;
  nsGameObjectHandle m_hParent;
  bool m_bDynamic = false;
  bool m_bActive = true;
  nsVec3 m_LocalPosition = nsVec3::MakeZero();
  nsQuat m_LocalRotation = nsQuat::MakeIdentity();
  nsVec3 m_LocalScaling = nsVec3(1.0f);
  float m_LocalUniformScaling = 1.0f;
};

class nsWorld;

/// \brief Iterator for child objects.
class NS_CORE_DLL nsGameObjectChildIterator
{
public:
  bool IsValid() const { return m_pObject != nullptr; }
  void Next();
  class nsGameObject& operator*() const { return *m_pObject; }
  void operator++() { Next(); }

private:
  friend class nsGameObject;
  class nsGameObject* m_pObject = nullptr;
};

/// \brief A game object in the world.
class NS_CORE_DLL nsGameObject
{
public:
  nsGameObjectHandle GetHandle() const;
  const nsHashedString& GetName() const;
  void SetName(nsStringView sName);

  nsGameObject* GetParent() const;
  nsGameObjectChildIterator GetChildren();
  nsUInt32 GetChildCount() const;

  bool IsActive() const;
  bool IsDynamic() const;
  void SetActiveFlag(bool bActive);

  nsVec3 GetGlobalPosition() const;
  nsQuat GetGlobalRotation() const;
  nsVec3 GetGlobalScaling() const;

  void SetLocalPosition(nsVec3 vPosition);
  void SetLocalRotation(nsQuat qRotation);
  void SetLocalScaling(nsVec3 vScaling);

  nsVec3 GetLocalPosition() const;
  nsQuat GetLocalRotation() const;
  nsVec3 GetLocalScaling() const;

  nsTransform GetGlobalTransform() const;
  void SetGlobalTransform(const nsTransform& transform);

  void UpdateLocalBounds();

  template <typename T>
  bool TryGetComponentOfBaseType(T*& out_pComponent);

  template <typename T>
  bool TryGetComponentOfBaseType(const T*& out_pComponent) const;

  nsArrayPtr<nsComponentHandle> GetComponents();

  nsWorld* GetWorld() const;

  void SendMessage(nsMessage& ref_msg);
  void PostMessage(const nsMessage& msg, nsTime delay, nsObjectMsgQueueType::Enum queueType = nsObjectMsgQueueType::NextFrame) const;

private:
  friend class nsWorld;
};

/// \brief Enum for message queue types.
struct nsObjectMsgQueueType
{
  enum Enum
  {
    NextFrame,
    PostAsync,
    PostTransform,
    AfterInitialized,
  };
};

/// \brief Base class for all world modules.
class NS_CORE_DLL nsWorldModule
{
public:
  struct UpdateContext
  {
    nsUInt32 m_uiFirstComponentIndex = 0;
    nsUInt32 m_uiComponentCount = 0;
  };

  nsWorld* GetWorld() const;
};

/// \brief Base class for all component managers.
class NS_CORE_DLL nsComponentManagerBase : public nsWorldModule
{
};

/// \brief Base class for all components.
class NS_CORE_DLL nsComponent : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsComponent, nsReflectedClass);

public:
  virtual ~nsComponent() = default;

  virtual void Initialize() {}
  virtual void Deinitialize() {}
  virtual void OnActivated() {}
  virtual void OnDeactivated() {}
  virtual void OnSimulationStarted() {}

  nsGameObject* GetOwner() const;
  nsWorld* GetWorld() const;
  nsComponentHandle GetHandle() const;
  bool IsActive() const;
  bool IsActiveAndInitialized() const;
  void SetActiveFlag(bool bActive);

  virtual void SerializeComponent(nsWorldWriter& inout_stream) const;
  virtual void DeserializeComponent(nsWorldReader& inout_stream);
};

/// \brief Typed component manager template.
template <typename ComponentType, typename StorageType>
class nsComponentManager : public nsComponentManagerBase
{
};

/// \brief Simple component manager template.
template <typename ComponentType>
class nsComponentManagerSimple : public nsComponentManagerBase
{
};

/// \brief Block storage type enum.
enum class nsBlockStorageType
{
  FreeList,
  Compact
};

/// \brief Component update type.
enum class nsComponentUpdateType
{
  Always,
  WhenSimulating
};

/// \brief Component mode.
enum class nsComponentMode
{
  Static,
  Dynamic
};

/// \brief A world containing game objects and components.
class NS_CORE_DLL nsWorld
{
public:
  nsWorld(nsWorldDesc& ref_desc);
  ~nsWorld();

  nsStringView GetName() const;

  nsGameObjectHandle CreateObject(const nsGameObjectDesc& desc);
  nsGameObjectHandle CreateObject(const nsGameObjectDesc& desc, nsGameObject*& out_pObject);

  bool TryGetObject(nsGameObjectHandle hObject, nsGameObject*& out_pObject);
  bool TryGetObject(nsGameObjectHandle hObject, const nsGameObject*& out_pObject) const;

  void DeleteObjectNow(nsGameObjectHandle hObject);
  void DeleteObjectDelayed(nsGameObjectHandle hObject);

  template <typename ManagerType>
  ManagerType* GetOrCreateComponentManager();

  template <typename ManagerType>
  ManagerType* GetComponentManager();

  template <typename ManagerType>
  const ManagerType* GetComponentManager() const;

  template <typename ManagerType>
  void DeleteComponentManager();

  nsComponentManagerBase* GetManagerForComponentType(const nsRTTI* pRtti);

  void SetWorldSimulationEnabled(bool bEnable);
  bool GetWorldSimulationEnabled() const;

  void Update();

  nsRandom& GetRandomNumberGenerator();

  nsUInt32 GetIndex() const;
};
