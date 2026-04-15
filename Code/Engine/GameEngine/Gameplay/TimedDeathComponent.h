#pragma once

#include <GameEngine/GameEngineDLL.h>

/// Stub component that destroys its owner after a set time.
class NS_GAMEENGINE_DLL nsTimedDeathComponent
{
public:
  nsTimedDeathComponent() = default;
  virtual ~nsTimedDeathComponent() = default;

  float m_fTimeToLive = 5.0f;
};
