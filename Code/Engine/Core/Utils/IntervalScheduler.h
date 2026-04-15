#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Time/Time.h>

/// \brief Schedules work items at fixed intervals.
template <typename T>
class nsIntervalScheduler
{
public:
  nsIntervalScheduler();
  ~nsIntervalScheduler();

  void AddOrUpdateWork(T work, nsTime interval);
  void RemoveWork(T work);
  nsTime GetInterval(T work) const;

  template <typename Callback>
  void Update(nsTime timeDelta, Callback callback);
};
