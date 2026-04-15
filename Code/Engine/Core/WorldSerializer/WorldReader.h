#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/IO/Stream.h>

class nsWorld;

/// \brief Reads world data from a stream.
class NS_CORE_DLL nsWorldReader
{
public:
  nsWorldReader();
  ~nsWorldReader();

  nsResult ReadWorldDescription(nsStreamReader& ref_stream);

  void InstantiateWorld(nsWorld& ref_world, const nsUInt16* pOverrideTeamID = nullptr);

  nsStreamReader& GetStream();

  template <typename T>
  T ReadValue();
};
