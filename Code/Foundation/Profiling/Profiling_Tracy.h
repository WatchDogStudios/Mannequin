#include <Foundation/Basics.h>
#include <Foundation/Strings/HashedString.h>

#if NS_ENABLED(NS_USE_PROFILING) && TRACY_ENABLE && !NS_DOCS

#  include <tracy/tracy/Tracy.hpp>

NS_ALWAYS_INLINE nsUInt32 __tracyEzStringLength(const char* szString)
{
  return nsStringUtils::GetStringElementCount(szString);
}

NS_ALWAYS_INLINE nsUInt32 __tracyEzStringLength(nsStringView sString)
{
  return sString.GetElementCount();
}

NS_ALWAYS_INLINE nsUInt32 __tracyEzStringLength(const nsString& sString)
{
  return sString.GetElementCount();
}

NS_ALWAYS_INLINE nsUInt32 __tracyEzStringLength(const nsStringBuilder& sString)
{
  return sString.GetElementCount();
}

NS_ALWAYS_INLINE nsUInt32 __tracyEzStringLength(const nsHashedString& sString)
{
  return sString.GetView().GetElementCount();
}

NS_ALWAYS_INLINE const char* __tracyEzStringToConstChar(const nsString& sString)
{
  return sString.GetData();
}

NS_ALWAYS_INLINE const char* __tracyEzStringToConstChar(const nsStringBuilder& sString)
{
  return sString.GetData();
}

NS_ALWAYS_INLINE const char* __tracyEzStringToConstChar(const nsHashedString& sString)
{
  return sString.GetData();
}

NS_ALWAYS_INLINE const char* __tracyEzStringToConstChar(const nsStringView& sString)
{
  // can just return the string views start pointer, because this is used together with __tracyEzStringLength
  return sString.GetStartPointer();
}

NS_ALWAYS_INLINE const char* __tracyEzStringToConstChar(const char* szString)
{
  return szString;
}

/// \brief Similar to NS_PROFILE_SCOPE, but only forwards to Tracy
#  define NS_TRACY_PROFILE_SCOPE(ScopeName) \
    ZoneScoped;                             \
    ZoneName(__tracyEzStringToConstChar(ScopeName), __tracyEzStringLength(ScopeName))

// Override the standard EZ profiling macros and inject Tracy profiling scopes

#  undef NS_PROFILE_SCOPE
#  define NS_PROFILE_SCOPE(ScopeName)                                                                                    \
    nsProfilingScope NS_PP_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ScopeName, NS_SOURCE_FUNCTION, nsTime::MakeZero()); \
    NS_TRACY_PROFILE_SCOPE(ScopeName)

#  undef NS_PROFILE_SCOPE_WITH_TIMEOUT
#  define NS_PROFILE_SCOPE_WITH_TIMEOUT(ScopeName, Timeout)                                                   \
    nsProfilingScope NS_PP_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ScopeName, NS_SOURCE_FUNCTION, Timeout); \
    NS_TRACY_PROFILE_SCOPE(ScopeName);

#  undef NS_PROFILE_LIST_SCOPE
#  define NS_PROFILE_LIST_SCOPE(ListName, FirstSectionName)                                                               \
    nsProfilingListScope NS_PP_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ListName, FirstSectionName, NS_SOURCE_FUNCTION); \
    NS_TRACY_PROFILE_SCOPE(ScopeName);

#  undef NS_PROFILER_FRAME_MARKER
#  define NS_PROFILER_FRAME_MARKER() FrameMark

#else

/// \brief Similar to NS_PROFILE_SCOPE, but only forwards to Tracy
#  define NS_TRACY_PROFILE_SCOPE(ScopeName)

#endif
