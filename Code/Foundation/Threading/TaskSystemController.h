/*
This code is part of WDFoundation - wdtier0

Copyright (c) 2020-2025 WD Studios Corp. and/or its licensors. All rights reserved in all
media. The coded instructions, statements, computer programs, and/or related material
(collectively the "Data") in these files contain confidential and unpublished information
proprietary WD Studios and/or its licensors, which is protected by United States of
America federal copyright law and by international treaties.

This software or source code is supplied under the terms of a license
agreement and nondisclosure agreement with WD Studios Corp. and may
not be copied, disclosed, or exploited except in accordance with the
terms of that agreement. The Data may not be disclosed or distributed to
third parties, in whole or in part, without the prior written consent of
WD Studios Corp..

WD STUDIOS MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
SOURCE CODE FOR ANY PURPOSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER, ITS AFFILIATES,
PARENT COMPANIES, LICENSORS, SUPPLIERS, OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OR PERFORMANCE OF THIS SOFTWARE OR SOURCE CODE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/List.h>
#include <Foundation/Threading/Implementation/Task.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Basics.h>

class nsTaskSystem;
/// TODO: If a application flag (e.g. NoTaskSystemController) is provided, then let the
/// tasksystem allocate its threads as needed.

/// \brief This class is responsible for managing the task system. It is used to
/// initialize and shutdown the task system.
///
/// This class was mainly created for APHTML, to allow the sdk to control how many threads
/// are spawned.
class NS_FOUNDATION_DLL nsTaskSystemController
{
  friend class nsTaskSystem;

public:
  nsTaskSystemController() = default;
  ~nsTaskSystemController() = default;

  /// \brief Initializes the task system with the given number of worker threads.
  /// \param numWorkerThreads The number of worker threads to use. This should be less
  /// than or equal to the number of CPU cores. \param numLongWorkerThreads The number of
  /// long worker threads to use. This should be less than or equal to the number of CPU
  /// cores.
  /// \warning CALL THIS BEFORE nsStartup!
  static void InitializeController(nsUInt8 numWorkerThreads, nsUInt8 numLongWorkerThreads);

  static nsUInt8 GetWorkerThreads();
  static nsUInt8 GetLongWorkerThreads();

  /// \brief Requests a new worker thread to be created if the current number of worker
  /// threads is less than the maximum allowed. \param priority The priority of the task
  /// that requires a new worker thread. \param numThreads The number of new worker
  /// threads to request. \return true if a new worker thread was created, false
  /// otherwise. \warning Will return false if too many threads are created (e.g. We are
  /// hitting the cpu's max threads count).
  bool RequestNewWorkerThread(nsWorkerThreadType::Enum priority, nsUInt8 numThreads = 1);

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, TaskSystemController);

  static void Startup();
  /// \brief Shuts down the task system and waits for all tasks to finish.
  static void Shutdown();

  // NOTE(Mikael): TaskSystem has two types of threads, short and long threads.
  // The Developer who is accessing the controller from aperture should allow 2 long task
  // threads, one for UI, one for JavaScript. It should also be known, that unless
  // notified from a SDK call, that no more than half the amount of the users cpu threads
  // should be used within Aperture.
  static inline bool m_bIsInitialized;
  static inline nsUInt8 m_uimaxWorkerThreads;
  static inline nsUInt8 m_uimaxLongWorkerThreads;
};
