#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Strings/String.h>

/// \brief Event data for console events.
struct nsConsoleEvent
{
  enum class Type
  {
    OutputLine,
    AutoCompleteRequest,
  };

  Type m_Type;
  nsStringView m_sText;
};

/// \brief In-game console for executing commands and displaying output.
class NS_CORE_DLL nsConsole
{
public:
  nsConsole();
  ~nsConsole();

  void ExecuteCommand(nsStringView sCommand);
  void AddOutputLine(nsStringView sText, nsColor color = nsColor::White);

  const nsDeque<nsString>& GetOutputLines() const;

  nsEvent<const nsConsoleEvent&> m_Events;
};
