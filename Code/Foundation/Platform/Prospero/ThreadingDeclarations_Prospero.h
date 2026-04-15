#pragma once

// Deactivate Doxygen document generation for the following block.
/// \cond

#include <pthread.h>
#include <semaphore.h>
#include <kernel.h>

using nsThreadHandle = ScePthread;
using nsThreadID = int;
using nsMutexHandle = ScePthreadMutex;
using nsOSThreadEntryPoint = void* (*)(void* pThreadParameter);

struct nsSemaphoreHandle
{
  SceKernelSema* m_pNamedOrUnnamed = nullptr;
  SceKernelSema* m_pNamed = nullptr;
  SceKernelSema m_Unnamed;
};

#define NS_THREAD_CLASS_ENTRY_POINT void* nsThreadClassEntryPoint(void* pThreadParameter);

struct nsConditionVariableData
{
  ScePthreadCond m_ConditionVariable;
};


/// \endcond
