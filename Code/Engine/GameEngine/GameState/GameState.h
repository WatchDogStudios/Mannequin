#pragma once

#include <GameEngine/GameEngineDLL.h>
#include <Foundation/Strings/String.h>

class NS_GAMEENGINE_DLL nsGameState
{
public:
  virtual ~nsGameState() = default;

  virtual void OnActivation() {}
  virtual void OnDeactivation() {}
  virtual bool WasQuitRequested() const { return m_bQuitRequested; }

  void RequestQuit() { m_bQuitRequested = true; }

protected:
  bool m_bQuitRequested = false;
};
