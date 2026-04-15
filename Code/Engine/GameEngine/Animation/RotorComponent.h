#pragma once

#include <GameEngine/GameEngineDLL.h>

/// Rotates an object around an axis at a given speed.
class NS_GAMEENGINE_DLL nsRotorComponent
{
public:
  nsRotorComponent() = default;
  virtual ~nsRotorComponent() = default;

  float m_fSpeed = 1.0f;
};
