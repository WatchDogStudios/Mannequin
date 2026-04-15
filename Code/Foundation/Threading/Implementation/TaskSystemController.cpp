#include <Foundation/System/SystemInformation.h>
#include <Foundation/Threading/TaskSystemController.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Logging/Log.h>

NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, TaskSystemController)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ThreadUtils", "Time"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    if (nsStartup::HasApplicationTag("NoTaskSystem"))
      return;

    nsTaskSystemController::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsTaskSystemController::Shutdown();
  }
NS_END_SUBSYSTEM_DECLARATION;

void nsTaskSystemController::InitializeController(
  nsUInt8 numWorkerThreads, nsUInt8 numLongWorkerThreads)
{
  m_uimaxWorkerThreads = numWorkerThreads;
  m_uimaxLongWorkerThreads = numLongWorkerThreads;
}

nsUInt8 nsTaskSystemController::GetWorkerThreads()
{
  return m_uimaxWorkerThreads;
}

nsUInt8 nsTaskSystemController::GetLongWorkerThreads()
{
  return m_uimaxLongWorkerThreads;
}

bool nsTaskSystemController::RequestNewWorkerThread(
  nsWorkerThreadType::Enum priority, nsUInt8 numThreads)
{
  nsSystemInformation info = nsSystemInformation::Get();
  const nsInt32 iCpuCores = info.GetCPUCoreCount();
  if ((m_uimaxWorkerThreads + m_uimaxLongWorkerThreads)
    >= static_cast<nsUInt8>(iCpuCores))
  {
    nsLog::Warning(
      "Cannot create new worker thread. Maximum number of worker threads reached.");
    return false;
  }
  nsTaskSystem::AllocateThreads(priority, numThreads);
  return true;
}

void nsTaskSystemController::Startup()
{
  nsTaskSystem::Startup();
  m_bIsInitialized = true;
}

void nsTaskSystemController::Shutdown()
{
  nsTaskSystem::Shutdown();
  m_bIsInitialized = false;
}
