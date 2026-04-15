#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Types.h>

/// \brief Flags for input slots.
struct nsInputSlotFlags
{
  using StorageType = nsUInt16;

  enum Enum : nsUInt16
  {
    None = 0,
    IsButton = NS_BIT(0),
    IsAnalogStick = NS_BIT(1),
    IsMouseWheel = NS_BIT(2),
    IsTouchPoint = NS_BIT(3),
    IsTouchPosition = NS_BIT(4),
    IsAnalogTrigger = NS_BIT(5),
    Default = None
  };

  struct Bits
  {
    StorageType IsButton : 1;
    StorageType IsAnalogStick : 1;
    StorageType IsMouseWheel : 1;
    StorageType IsTouchPoint : 1;
  };
};

/// \brief Key states.
struct nsKeyState
{
  enum Enum
  {
    Up,
    Pressed,
    Down,
    Released,
  };
};

/// \brief Configuration for an input action.
struct NS_CORE_DLL nsInputActionConfig
{
  static constexpr int MaxInputSlotAlternatives = 3;

  bool m_bApplyTimeScaling = true;
  float m_fFilteredPriority = 0.0f;
  float m_fFilterXMinValue = 0.0f;
  float m_fFilterXMaxValue = 0.0f;
  float m_fFilterYMinValue = 0.0f;
  float m_fFilterYMaxValue = 0.0f;

  nsString m_sInputSlotTrigger[MaxInputSlotAlternatives];
  float m_fInputSlotScale[MaxInputSlotAlternatives] = {1.0f, 1.0f, 1.0f};
  nsString m_sFilterByInputSlotX[MaxInputSlotAlternatives];
  nsString m_sFilterByInputSlotY[MaxInputSlotAlternatives];
  nsHashedString m_OnEnterArea;
  nsHashedString m_OnLeaveArea;

  bool operator==(const nsInputActionConfig& rhs) const;
  bool operator!=(const nsInputActionConfig& rhs) const { return !(*this == rhs); }
};

/// \brief Central input manager. Provides static methods for input queries.
class NS_CORE_DLL nsInputManager
{
public:
  static void Update(nsTime timeDelta);

  static nsKeyState::Enum GetInputSlotState(nsStringView sSlot, float* pValue = nullptr);
  static nsKeyState::Enum GetInputActionState(nsStringView sSet, nsStringView sAction, float* pValue = nullptr, nsInt8* pSlot = nullptr);

  static void SetInputSlotDisplayName(nsStringView sSlot, nsStringView sDisplayName);
  static const char* GetInputSlotDisplayName(nsStringView sSlot);

  static void SetInputSlotDeadZone(nsStringView sSlot, float fDeadZone);
  static float GetInputSlotDeadZone(nsStringView sSlot);

  static void SetInputActionConfig(nsStringView sSet, nsStringView sAction, const nsInputActionConfig& config, bool bClearPreviousInputMappings);
  static const nsInputActionConfig& GetInputActionConfig(nsStringView sSet, nsStringView sAction);

  static void RemoveInputAction(nsStringView sSet, nsStringView sAction);

  static void SetActionDisplayName(nsStringView sAction, nsStringView sDisplayName);
  static const char* GetActionDisplayName(nsStringView sAction);

  static void GetAllInputSets(nsDynamicArray<nsString>& out_sets);
  static void GetAllInputActions(nsStringView sSet, nsHybridArray<nsString, 24>& out_actions);

  static nsStringView GetPressedInputSlot(nsInputSlotFlags::Enum mustHave, nsInputSlotFlags::Enum mustNotHave);
  static nsUInt32 RetrieveLastCharacter(bool bConsume = true);

  static void InjectInputSlotValue(nsStringView sSlot, float fValue);
  static void ClearInputMapping(nsStringView sSet, nsStringView sSlot);
};
