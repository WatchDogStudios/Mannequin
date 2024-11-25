#include <Player/Player.h>

#include <Core/Input/InputManager.h>
#include <Core/World/World.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Utilities/AssetFileHeader.h>
#include <Foundation/Utilities/CommandLineOptions.h>
#include <GameEngine/Animation/RotorComponent.h>
#include <GameEngine/Animation/SliderComponent.h>
#include <GameEngine/Gameplay/InputComponent.h>
#include <GameEngine/Gameplay/SpawnComponent.h>
#include <GameEngine/Gameplay/TimedDeathComponent.h>
#include <RendererCore/Components/CameraComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponent.h>

// this injects the main function
NS_APPLICATION_ENTRY_POINT(nsPlayerApplication);

// these command line options may not all be directly used in nsPlayer, but the nsFallbackGameState reads those options to determine which scene to load
nsCommandLineOptionString opt_Project("_Player", "-project", "Path to the project folder.\nUsually an absolute path, though relative paths will work for projects that are located inside the NS SDK directory.", "");
nsCommandLineOptionString opt_Scene("_Player", "-scene", "Path to a scene file.\nUsually given relative to the corresponding project data directory where it resides, but can also be given as an absolute path.", "");

nsPlayerApplication::nsPlayerApplication()
  : nsGameApplication("nsPlayer", nullptr) // we don't have a fixed project path in this app, so we need to pass that in a bit later
{
}

nsResult nsPlayerApplication::BeforeCoreSystemsStartup()
{
  // show the command line options, if help is requested
  {
    // since this is a GUI application (not a console app), printf has no effect
    // therefore we have to show the command line options with a message box

    nsStringBuilder cmdHelp;
    if (nsCommandLineOption::LogAvailableOptionsToBuffer(cmdHelp, nsCommandLineOption::LogAvailableModes::IfHelpRequested))
    {
      nsLog::OsMessageBox(cmdHelp);
      SetReturnCode(-1);
      return NS_FAILURE;
    }
  }

  nsStartup::AddApplicationTag("player");

  NS_SUCCEED_OR_RETURN(SUPER::BeforeCoreSystemsStartup());

  DetermineProjectPath();

  return NS_SUCCESS;
}


void nsPlayerApplication::AfterCoreSystemsStartup()
{
  ExecuteInitFunctions();

  nsStartup::StartupHighLevelSystems();

  // we need a game state to do anything
  // if no custom game state is available, nsFallbackGameState will be used
  // the game state is also responsible for either creating a world, or loading it
  // the nsFallbackGameState inspects the command line to figure out which scene to load
  ActivateGameState(nullptr, {}, nsTransform::MakeIdentity());
}

void nsPlayerApplication::Run_InputUpdate()
{
  SUPER::Run_InputUpdate();

  if (auto pGameState = GetActiveGameState())
  {
    if (pGameState->WasQuitRequested())
    {
      RequestApplicationQuit();
    }
  }
}

void nsPlayerApplication::DetermineProjectPath()
{
  nsStringBuilder sProjectPath = opt_Project.GetOptionValue(nsCommandLineOption::LogMode::FirstTime);

#if NS_DISABLED(NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS)
  // We can't specify command line arguments on many platforms so the project must be defined by nsFileserve.
  // nsFileserve must be started with the project special dir set. For example:
  // -specialdirs project ".../nsEngine/Data/Samples/Testing Chambers

  if (sProjectPath.IsEmpty())
  {
    m_sAppProjectPath = ">project";
    return;
  }
#endif

  if (sProjectPath.IsEmpty())
  {
    const nsStringBuilder sScenePath = opt_Scene.GetOptionValue(nsCommandLineOption::LogMode::FirstTime);

    // project path is empty, need to extract it from the scene path

    if (!sScenePath.IsAbsolutePath())
    {
      // scene path is not absolute -> can't extract project path
      m_sAppProjectPath = nsFileSystem::GetSdkRootDirectory();
      SetReturnCode(1);
      return;
    }

    if (nsFileSystem::FindFolderWithSubPath(sProjectPath, sScenePath, "nsProject", "nsSdkRoot.txt").Failed())
    {
      // couldn't find the 'nsProject' file in any parent folder of the scene
      m_sAppProjectPath = nsFileSystem::GetSdkRootDirectory();
      SetReturnCode(1);
      return;
    }
  }
  else if (!nsPathUtils::IsAbsolutePath(sProjectPath))
  {
    // project path is not absolute, so must be relative to the SDK directory
    sProjectPath.Prepend(nsFileSystem::GetSdkRootDirectory(), "/");
  }

  sProjectPath.MakeCleanPath();
  sProjectPath.TrimWordEnd("/nsProject");

  if (sProjectPath.IsEmpty())
  {
    m_sAppProjectPath = nsFileSystem::GetSdkRootDirectory();
    SetReturnCode(1);
    return;
  }

  // store it now, even if it fails, for error reporting
  m_sAppProjectPath = sProjectPath;
}
