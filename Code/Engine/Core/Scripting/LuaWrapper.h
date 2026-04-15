#pragma once

#include <Core/CoreDLL.h>

struct lua_State;

/// \brief Wrapper around Lua scripting.
class NS_CORE_DLL nsLuaWrapper
{
public:
  nsLuaWrapper();
  nsLuaWrapper(lua_State* pState);
  ~nsLuaWrapper();

  lua_State* GetLuaState() const;
};
