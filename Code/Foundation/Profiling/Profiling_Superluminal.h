#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/HashedString.h>
#include <cstring>

#if NS_ENABLED(NS_USE_PROFILING) || defined(NS_DOCS)
#ifdef NS_SENTRY_SUPPORT
#include <Foundation/System/CrashHandlers/SentrySystem.h>
#endif
#  if defined(SUPERLUMINALAPI) && NS_DISABLED(NS_PLATFORM_PLAYSTATION_5)
#    include <PerformanceAPI.h>
#    if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#      include <Foundation/Platform/Win/Utils/IncludeWindows.h>

inline PerformanceAPI_Functions* Perf;
/// TODO: Abstract this into a overridable function that could be called anywhere.
/// NOTE: szPerformanceName is not used on Windows.
NS_ALWAYS_INLINE void LoadPerformanceLibrary(const char* szPerformanceName = "")
{
  // Superluminal requires windows, so we will be using Windows specific API's.
  HMODULE module = LoadLibraryW(L"PerformanceAPI.dll");
  if (module == NULL)
  {
    nsLog::Error("Failed to Load Superluminal Performance DLL. (is the dll misplaced?)");
    return;
  }
  else
  {
    PerformanceAPI_GetAPI_Func getAPI = (PerformanceAPI_GetAPI_Func)((void*)GetProcAddress(module, "PerformanceAPI_GetAPI"));
    if (getAPI == NULL || getAPI(PERFORMANCEAPI_VERSION, Perf))
    {
      nsLog::Error("Failed to Get Superluminal Performance API.");
      FreeLibrary(module);
      return;
    }
  }
}

NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(const char* szString)
{
  return nsStringUtils::GetStringElementCount(szString);
}

NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(nsStringView szString)
{
  return szString.GetElementCount();
}

NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(const nsString& szString)
{
  return szString.GetElementCount();
}
NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(const nsStringBuilder& szString)
{
  return szString.GetElementCount();
}
NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(const nsHashedString& szString)
{
  return szString.GetView().GetElementCount();
}
NS_ALWAYS_INLINE const char* __convertnsStringToConstChar(const nsString& szString)
{
  return szString.GetData();
}
NS_ALWAYS_INLINE const char* __convertnsStringToConstChar(const nsStringBuilder& szString)
{
  return szString.GetData();
}
NS_ALWAYS_INLINE const char* __convertnsStringToConstChar(const nsHashedString& szString)
{
  return szString.GetData();
}
NS_ALWAYS_INLINE const char* __convertnsStringToConstChar(const nsStringView& szString)
{
  // Use thread_local to ensure the buffer outlives the function call
  // and is valid for the duration of the profiling scope
  thread_local char s_profilingBuffer[256];
  const nsUInt32 len = szString.GetElementCount();
  const nsUInt32 copyLen = (len < sizeof(s_profilingBuffer) - 1) ? len : (sizeof(s_profilingBuffer) - 1);
  if (copyLen > 0)
  {
    std::memcpy(s_profilingBuffer, szString.GetStartPointer(), copyLen);
  }
  s_profilingBuffer[copyLen] = '\0';
  return s_profilingBuffer;
}
#      define PERFORMANCE_PROFILE_SCOPE_DYNAMIC(szScopeName) \
        PERFORMANCEAPI_INSTRUMENT(szScopeName) 
    //SentryInstrument perfinst("Profiler", szScopeName)

/// \brief Profiles the current scope using the given name.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingScope
/// \sa NS_PROFILE_LIST_SCOPE
#      define NS_PROFILE_SCOPE(szScopeName) \
        PERFORMANCE_PROFILE_SCOPE_DYNAMIC(__convertnsStringToConstChar((const nsString&)szScopeName))

/// \brief Same as NS_PROFILE_SCOPE but if the scope takes longer than 'Timeout', the nsProfilingSystem's timeout callback is executed.
///
/// This can be used to log an error or save a callstack, etc. when a scope exceeds an expected amount of time.
///
/// \sa nsProfilingSystem::SetScopeTimeoutCallback()
#      define NS_PROFILE_SCOPE_WITH_TIMEOUT(szScopeName, Timeout) \
        PERFORMANCE_PROFILE_SCOPE_DYNAMIC(__convertnsStringToConstChar((const nsString&)szScopeName))
/// \brief Profiles the current scope using the given name as the overall list scope name and the section name for the first section in the list.
///
/// Use NS_PROFILE_LIST_NEXT_SECTION to start a new section in the list scope.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_NEXT_SECTION
#      define NS_PROFILE_LIST_SCOPE(szListName, szFirstSectionName)                                   \
        PERFORMANCE_PROFILE_SCOPE_DYNAMIC(__convertnsStringToConstChar((const nsString&)szScopeName)) \
        PerformanceAPI::BeginEvent(__convertnsStringToConstChar((const nsString&)szFirstSectionName))

/// \brief Starts a new section in a NS_PROFILE_LIST_SCOPE
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_SCOPE
#      define NS_PROFILE_LIST_NEXT_SECTION(szNextSectionName) \
        PerformanceAPI::BeginEvent(szNextSectionName)

#      define NS_PROFILER_END_FRAME \
        PerformanceAPI::EndEvent()

#    endif
#    define NS_PROFILER_FRAME_MARKER()
#  elif NS_ENABLED(NS_PLATFORM_PLAYSTATION_5)
NS_ALWAYS_INLINE void LoadPerformanceLibrary(const char* szPerformanceName = "libSuperluminalPS5.prx")
{
  int returnValue = -1;
  sceKernelLoadStartModule(szPerformanceName, 0, 0, 0, NULL, &returnValue);
  if (returnValue != 0)
  {
    nsLog::Error("Failed to Load Superluminal Performance Module.");
  }
}
// TODO: Clean this up.
/// \brief Profiles the current scope using the given name.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingScope
/// \sa NS_PROFILE_LIST_SCOPE
#    define NS_PROFILE_SCOPE(szScopeName) 

/// \brief Same as NS_PROFILE_SCOPE but if the scope takes longer than 'Timeout', the nsProfilingSystem's timeout callback is executed.
///
/// This can be used to log an error or save a callstack, etc. when a scope exceeds an expected amount of time.
///
/// \sa nsProfilingSystem::SetScopeTimeoutCallback()
#    define NS_PROFILE_SCOPE_WITH_TIMEOUT(szScopeName, Timeout) 

/// \brief Profiles the current scope using the given name as the overall list scope name and the section name for the first section in the list.
///
/// Use NS_PROFILE_LIST_NEXT_SECTION to start a new section in the list scope.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_NEXT_SECTION
#    define NS_PROFILE_LIST_SCOPE(szListName, szFirstSectionName) 

/// \brief Starts a new section in a NS_PROFILE_LIST_SCOPE
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_SCOPE
#    define NS_PROFILE_LIST_NEXT_SECTION(szNextSectionName) 
// NOTE/TODO: OBSOLETE.
#    define NS_PROFILER_END_FRAME
#    define NS_PROFILER_FRAME_MARKER()
#  endif
#endif
