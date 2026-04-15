#pragma once

#include <GameEngine/GameEngineDLL.h>
#include <GameEngine/GameState/GameState.h>
#include <Foundation/Application/Application.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Math/Transform.h>

/// Base class for game applications with full engine integration.
class NS_GAMEENGINE_DLL nsGameApplication : public nsApplication
{
public:
  nsGameApplication(nsStringView sAppName, nsStringView sProjectPath);
  virtual ~nsGameApplication();

  // Application lifecycle
  virtual nsResult BeforeCoreSystemsStartup() override;
  virtual void AfterCoreSystemsStartup() override;
  virtual void BeforeCoreSystemsShutdown() override;
  virtual void Run() override;

  // Input processing
  virtual void Run_InputUpdate();
  virtual bool Run_ProcessApplicationInput() { return false; }

  // Initialization hooks (can be overridden to disable)
  virtual void Init_LoadProjectPlugins() {}
  virtual void Init_SetupDefaultResources() {}
  virtual void Init_ConfigureInput() {}
  virtual void Init_ConfigureTags() {}

  // Execution helpers
  void ExecuteInitFunctions();

  // Game state management
  void ActivateGameState(nsGameState* pState, nsArrayPtr<const nsString> args = {}, const nsTransform& startPos = nsTransform::MakeIdentity());
  nsGameState* GetActiveGameState() const;

  // Application quit
  void RequestApplicationQuit();

protected:
  nsString m_sAppProjectPath;

private:
  nsGameState* m_pActiveGameState = nullptr;
  bool m_bQuitRequested = false;
};
