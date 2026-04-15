#pragma once

#include <GameEngine/GameEngineDLL.h>
#include <Foundation/Strings/String.h>

/// Stub component that maps input actions to game logic.
class NS_GAMEENGINE_DLL nsInputComponent
{
public:
  nsInputComponent() = default;
  virtual ~nsInputComponent() = default;

  nsString m_sInputMappingName;
};
