#pragma once

#include <Core/CoreDLL.h>
#include <Core/System/Window.h>

/// \brief A window for use by game states.
class NS_CORE_DLL nsGameStateWindow : public nsWindow
{
public:
  nsGameStateWindow(const nsWindowCreationDesc& desc);
  ~nsGameStateWindow();
};
