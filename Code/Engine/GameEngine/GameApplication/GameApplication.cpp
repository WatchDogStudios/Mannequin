#include <GameEngine/GameEnginePCH.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/GameState.h>

nsGameApplication::nsGameApplication(nsStringView sAppName, nsStringView sProjectPath)
  : nsApplication(sAppName)
  , m_sAppProjectPath(sProjectPath)
{
}

nsGameApplication::~nsGameApplication() = default;

nsResult nsGameApplication::BeforeCoreSystemsStartup()
{
  return nsApplication::BeforeCoreSystemsStartup();
}

void nsGameApplication::AfterCoreSystemsStartup()
{
  nsApplication::AfterCoreSystemsStartup();
}

void nsGameApplication::BeforeCoreSystemsShutdown()
{
  if (m_pActiveGameState)
  {
    m_pActiveGameState->OnDeactivation();
    m_pActiveGameState = nullptr;
  }

  nsApplication::BeforeCoreSystemsShutdown();
}

void nsGameApplication::Run()
{
  Run_InputUpdate();

  if (m_bQuitRequested || (m_pActiveGameState && m_pActiveGameState->WasQuitRequested()))
  {
    RequestApplicationQuit();
  }
}

void nsGameApplication::Run_InputUpdate()
{
  Run_ProcessApplicationInput();
}

void nsGameApplication::ExecuteInitFunctions()
{
  Init_LoadProjectPlugins();
  Init_SetupDefaultResources();
  Init_ConfigureInput();
  Init_ConfigureTags();
}

void nsGameApplication::ActivateGameState(nsGameState* pState, nsArrayPtr<const nsString> args, const nsTransform& startPos)
{
  if (m_pActiveGameState)
  {
    m_pActiveGameState->OnDeactivation();
  }

  m_pActiveGameState = pState;

  if (m_pActiveGameState)
  {
    m_pActiveGameState->OnActivation();
  }
}

nsGameState* nsGameApplication::GetActiveGameState() const
{
  return m_pActiveGameState;
}

void nsGameApplication::RequestApplicationQuit()
{
  m_bQuitRequested = true;
}
