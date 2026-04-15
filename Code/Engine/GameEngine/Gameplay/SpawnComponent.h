#pragma once

#include <GameEngine/GameEngineDLL.h>
#include <Foundation/Strings/String.h>

/// Stub component that spawns a prefab at this location.
class NS_GAMEENGINE_DLL nsSpawnComponent
{
public:
  nsSpawnComponent() = default;
  virtual ~nsSpawnComponent() = default;

  nsString m_sPrefabReference;
};
