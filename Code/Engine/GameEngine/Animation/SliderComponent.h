#pragma once

#include <GameEngine/GameEngineDLL.h>

/// Moves an object back and forth along an axis.
class NS_GAMEENGINE_DLL nsSliderComponent
{
public:
  nsSliderComponent() = default;
  virtual ~nsSliderComponent() = default;

  float m_fDistance = 1.0f;
  float m_fSpeed = 1.0f;
};
