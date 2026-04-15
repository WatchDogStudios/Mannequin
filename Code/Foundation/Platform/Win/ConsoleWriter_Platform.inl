#include <Foundation/Platform/Win/Utils/IncludeWindows.h>

void SetConsoleColor(nsUInt8 ui)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)ui);
}
