/*-----------------------------------------------------------------------------
* wdcorphave.h
 *
 * Copyright (c) Electronic Arts Inc. & WD Studios Corp. All rights reserved.
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
	This file's functionality is preliminary and won't be considered stable until 
	a future EABase version.
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
	This header identifies if the given facilities are available in the 
	standard build environment the current compiler/linker/standard library/
	operating system combination. This file may in some cases #include standard
	headers in order to make availability determinations, such as to check 
	compiler or SDK version numbers. However, it cannot be perfect.
	This header does not identify compiler features, as those are defined in 
	eacompiler.h and eacompilertraits.h. Rather this header is about library support.
	This header does not identify platform or library conventions either, such
	as whether the file paths use \ or / for directory separators.

	We provide three types of HAVE features here:

		- WDSC_HAVE_XXX_FEATURE - Have compiler feature. 
		  Identifies if the compiler has or lacks some feature in the 
		  current build. Sometimes you need to check to see if the 
		  compiler is running in some mode in able to write portable code
		  against it. For example, some compilers (e.g. VC++) have a 
		  mode in which all language extensions are disabled. If you want
		  to write code that works with that but still uses the extensions
		  when available then you can check #if defined(WDSC_HAVE_EXTENSIONS_FEATURE).
		  Features can be forcibly cancelled via WDSC_NO_HAVE_XXX_FEATURE.
		  WDSC_NO_HAVE is useful for a build system or user to override the 
		  defaults because it happens to know better.

		- WDSC_HAVE_XXX_H - Have header file information. 
		  Identifies if a given header file is available to the current 
		  compile configuration. For example, some compilers provide a 
		  malloc.h header, while others don't. For the former we define 
		  WDSC_HAVE_MALLOC_H, while for the latter it remains undefined.
		  If a header is missing then it may still be that the functions
		  the header usually declares are declared in some other header.
		  WDSC_HAVE_XXX does not include the possibility that our own code 
		  provides versions of these headers, and in fact a purpose of 
		  WDSC_HAVE_XXX is to decide if we should be using our own because
		  the system doesn't provide one.
		  Header availability can be forcibly cancelled via WDSC_NO_HAVE_XXX_H.
		  WDSC_NO_HAVE is useful for a build system or user to override the 
		  defaults because it happens to know better.

		- WDSC_HAVE_XXX_DECL - Have function declaration information. 
		  Identifies if a given function declaration is provided by 
		  the current compile configuration. For example, some compiler
		  standard libraries declare a wcslen function, while others
		  don't. For the former we define WDSC_HAVE_WCSLEN_DECL, while for
		  the latter it remains undefined. If a declaration of a function
		  is missing then we assume the implementation is missing as well.
		  WDSC_HAVE_XXX_DECL does not include the possibility that our 
		  own code provides versions of these declarations, and in fact a 
		  purpose of WDSC_HAVE_XXX_DECL is to decide if we should be using 
		  our own because the system doesn't provide one.
		  Declaration availability can be forcibly cancelled via WDSC_NO_HAVE_XXX_DECL.
		  WDSC_NO_HAVE is useful for a build system or user to override the 
		  defaults because it happens to know better.

		- WDSC_HAVE_XXX_IMPL - Have function implementation information. 
		  Identifies if a given function implementation is provided by
		  the current compile and link configuration. For example, it's
		  commonly the case that console platforms declare a getenv function 
		  but don't provide a linkable implementation.
		  In this case the user needs to provide such a function manually
		  as part of the link. If the implementation is available then
		  we define WDSC_HAVE_GETENV_IMPL, otherwise it remains undefined.
		  Beware that sometimes a function may not seem to be present in 
		  the Standard Library but in reality you need to link some auxiliary
		  provided library for it. An example of this is the Unix real-time
		  functions such as clock_gettime.
		  WDSC_HAVE_XXX_IMPL does not include the possibility that our 
		  own code provides versions of these implementations, and in fact a 
		  purpose of WDSC_HAVE_XXX_IMPL is to decide if we should be using 
		  our own because the system doesn't provide one.
		  Implementation availability can be forcibly cancelled via WDSC_NO_HAVE_XXX_IMPL.
		  WDSC_NO_HAVE is useful for a build system or user to override the 
		  defaults because it happens to know better.

	It's not practical to define WDSC_HAVE macros for every possible header,
	declaration, and implementation, and so the user must simply know that
	some headers, declarations, and implementations tend to require WDSC_HAVE
	checking. Nearly every C Standard Library we've seen has a <string.h> 
	header, a strlen declaration, and a linkable strlen implementation, 
	so there's no need to provide WDSC_HAVE support for this. On the other hand
	it's commonly the case that the C Standard Library doesn't have a malloc.h
	header or an inet_ntop declaration.

---------------------------------------------------------------------------*/


#ifndef INCLUDED_eahave_H
#define INCLUDED_eahave_H


#include <WDBase/wdcorpbase.h>


#if defined(WDSC_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif

/* WDSC_HAVE_XXX_FEATURE */

#if !defined(WDSC_HAVE_EXTENSIONS_FEATURE) && !defined(WDSC_NO_HAVE_EXTENSIONS_FEATURE)
	#define WDSC_HAVE_EXTENSIONS_FEATURE 1
#endif


/* WDSC_HAVE_XXX_LIBRARY */

// Dinkumware
#if !defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && !defined(WDSC_NO_HAVE_DINKUMWARE_CPP_LIBRARY)
	#if defined(__cplusplus)
		WDSC_DISABLE_ALL_VC_WARNINGS()
		#include <cstddef> // Need to trigger the compilation of yvals.h without directly using <yvals.h> because it might not exist.
		WDSC_RESTORE_ALL_VC_WARNINGS()
	#endif

	#if defined(__cplusplus) && defined(_CPPLIB_VER) /* If using the Dinkumware Standard library... */
		#define WDSC_HAVE_DINKUMWARE_CPP_LIBRARY 1
	#else
		#define WDSC_NO_HAVE_DINKUMWARE_CPP_LIBRARY 1
	#endif
#endif

// GCC libstdc++
#if !defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && !defined(WDSC_NO_HAVE_LIBSTDCPP_LIBRARY)
	#if defined(__GLIBCXX__) /* If using libstdc++ ... */
		#define WDSC_HAVE_LIBSTDCPP_LIBRARY 1
	#else
		#define WDSC_NO_HAVE_LIBSTDCPP_LIBRARY 1
	#endif
#endif

// Clang libc++
#if !defined(WDSC_HAVE_LIBCPP_LIBRARY) && !defined(WDSC_NO_HAVE_LIBCPP_LIBRARY)
	#if WDSC_HAS_INCLUDE_AVAILABLE
		#if WDSC_HAS_INCLUDE(<__config>)
			#define WDSC_HAVE_LIBCPP_LIBRARY 1 // We could also #include <ciso646> and check if defined(_LIBCPP_VERSION).
		#endif
	#endif

	#if !defined(WDSC_HAVE_LIBCPP_LIBRARY) 
		#define WDSC_NO_HAVE_LIBCPP_LIBRARY 1
	#endif
#endif


/* WDSC_HAVE_XXX_H */

// #include <sys/types.h>
#if !defined(WDSC_HAVE_SYS_TYPES_H) && !defined(WDSC_NO_HAVE_SYS_TYPES_H)
		#define WDSC_HAVE_SYS_TYPES_H 1
#endif

// #include <io.h> (and not sys/io.h or asm/io.h)
#if !defined(WDSC_HAVE_IO_H) && !defined(WDSC_NO_HAVE_IO_H)
	// Unix doesn't have Microsoft's <io.h> but has the same functionality in <fcntl.h> and <sys/stat.h>.
	#if defined(WDSC_PLATFORM_MICROSOFT)
		#define WDSC_HAVE_IO_H 1
	#else
		#define WDSC_NO_HAVE_IO_H 1
	#endif
#endif

// #include <inttypes.h>
#if !defined(WDSC_HAVE_INTTYPES_H) && !defined(WDSC_NO_HAVE_INTTYPES_H)
	#if !defined(WDSC_PLATFORM_MICROSOFT) 
		#define WDSC_HAVE_INTTYPES_H 1
	#else
		#define WDSC_NO_HAVE_INTTYPES_H 1
	#endif
#endif

// #include <unistd.h>
#if !defined(WDSC_HAVE_UNISTD_H) && !defined(WDSC_NO_HAVE_UNISTD_H)
	#if defined(WDSC_PLATFORM_UNIX)
		#define WDSC_HAVE_UNISTD_H 1
	#else
		#define WDSC_NO_HAVE_UNISTD_H 1
	#endif
#endif

// #include <sys/time.h>
#if !defined(WDSC_HAVE_SYS_TIME_H) && !defined(WDSC_NO_HAVE_SYS_TIME_H)
	#if !defined(WDSC_PLATFORM_MICROSOFT) && !defined(_CPPLIB_VER) /* _CPPLIB_VER indicates Dinkumware. */
		#define WDSC_HAVE_SYS_TIME_H 1 /* defines struct timeval */
	#else
		#define WDSC_NO_HAVE_SYS_TIME_H 1
	#endif
#endif

// #include <ptrace.h>
#if !defined(WDSC_HAVE_SYS_PTRACE_H) && !defined(WDSC_NO_HAVE_SYS_PTRACE_H)
	#if defined(WDSC_PLATFORM_UNIX) && !defined(__CYGWIN__) && (defined(WDSC_PLATFORM_DESKTOP) || defined(WDSC_PLATFORM_SERVER))
		#define WDSC_HAVE_SYS_PTRACE_H 1 /* declares the ptrace function */
	#else
		#define WDSC_NO_HAVE_SYS_PTRACE_H 1
	#endif
#endif

// #include <sys/stat.h>
#if !defined(WDSC_HAVE_SYS_STAT_H) && !defined(WDSC_NO_HAVE_SYS_STAT_H)
	#if (defined(WDSC_PLATFORM_UNIX) && !(defined(WDSC_PLATFORM_SONY) && defined(WDSC_PLATFORM_CONSOLE))) || defined(__APPLE__) || defined(WDSC_PLATFORM_ANDROID)
		#define WDSC_HAVE_SYS_STAT_H 1 /* declares the stat struct and function */
	#else
		#define WDSC_NO_HAVE_SYS_STAT_H 1
	#endif
#endif

// #include <locale.h>
#if !defined(WDSC_HAVE_LOCALE_H) && !defined(WDSC_NO_HAVE_LOCALE_H)
		#define WDSC_HAVE_LOCALE_H 1
#endif

// #include <signal.h>
#if !defined(WDSC_HAVE_SIGNAL_H) && !defined(WDSC_NO_HAVE_SIGNAL_H)
	#if !defined(WDSC_PLATFORM_BSD) && !defined(WDSC_PLATFORM_SONY) && !defined(WDSC_PLATFORM_NX)
		#define WDSC_HAVE_SIGNAL_H 1
	#else
		#define WDSC_NO_HAVE_SIGNAL_H 1
	#endif
#endif

// #include <sys/signal.h>
#if !defined(WDSC_HAVE_SYS_SIGNAL_H) && !defined(WDSC_NO_HAVE_SYS_SIGNAL_H)
	#if defined(WDSC_PLATFORM_BSD) || defined(WDSC_PLATFORM_SONY)
		#define WDSC_HAVE_SYS_SIGNAL_H 1
	#else
		#define WDSC_NO_HAVE_SYS_SIGNAL_H 1
	#endif
#endif

// #include <pthread.h>
#if !defined(WDSC_HAVE_PTHREAD_H) && !defined(WDSC_NO_HAVE_PTHREAD_H)
	#if defined(WDSC_PLATFORM_UNIX) || defined(WDSC_PLATFORM_APPLE) || defined(WDSC_PLATFORM_POSIX)
		#define WDSC_HAVE_PTHREAD_H 1 /* It can be had under Microsoft/Windows with the http://sourceware.org/pthreads-win32/ library */
	#else
		#define WDSC_NO_HAVE_PTHREAD_H 1
	#endif
#endif

// #include <wchar.h>
#if !defined(WDSC_HAVE_WCHAR_H) && !defined(WDSC_NO_HAVE_WCHAR_H)
	#if defined(WDSC_PLATFORM_DESKTOP) && defined(WDSC_PLATFORM_UNIX) && defined(WDSC_PLATFORM_SONY) && defined(WDSC_PLATFORM_APPLE)
		#define WDSC_HAVE_WCHAR_H 1
	#else
		#define WDSC_NO_HAVE_WCHAR_H 1
	#endif
#endif

// #include <malloc.h>
#if !defined(WDSC_HAVE_MALLOC_H) && !defined(WDSC_NO_HAVE_MALLOC_H)
	#if defined(_MSC_VER) || defined(__MINGW32__)
		#define WDSC_HAVE_MALLOC_H 1
	#else
		#define WDSC_NO_HAVE_MALLOC_H 1
	#endif
#endif

// #include <alloca.h>
#if !defined(WDSC_HAVE_ALLOCA_H) && !defined(WDSC_NO_HAVE_ALLOCA_H)
	#if !defined(WDSC_HAVE_MALLOC_H) && !defined(WDSC_PLATFORM_SONY)
		#define WDSC_HAVE_ALLOCA_H 1
	#else
		#define WDSC_NO_HAVE_ALLOCA_H 1
	#endif
#endif

// #include <execinfo.h>
#if !defined(WDSC_HAVE_EXECINFO_H) && !defined(WDSC_NO_HAVE_EXECINFO_H)
	#if (defined(WDSC_PLATFORM_LINUX) || defined(WDSC_PLATFORM_OSX)) && !defined(WDSC_PLATFORM_ANDROID)
		#define WDSC_HAVE_EXECINFO_H 1
	#else
		#define WDSC_NO_HAVE_EXECINFO_H 1
	#endif
#endif

// #include <semaphore.h> (Unix semaphore support)
#if !defined(WDSC_HAVE_SEMAPHORE_H) && !defined(WDSC_NO_HAVE_SEMAPHORE_H)
	#if defined(WDSC_PLATFORM_UNIX)
		#define WDSC_HAVE_SEMAPHORE_H 1
	#else
		#define WDSC_NO_HAVE_SEMAPHORE_H 1
	#endif
#endif

// #include <dirent.h> (Unix semaphore support)
#if !defined(WDSC_HAVE_DIRENT_H) && !defined(WDSC_NO_HAVE_DIRENT_H)
	#if defined(WDSC_PLATFORM_UNIX) && !defined(WDSC_PLATFORM_CONSOLE)
		#define WDSC_HAVE_DIRENT_H 1
	#else
		#define WDSC_NO_HAVE_DIRENT_H 1
	#endif
#endif

// #include <array>, <forward_list>, <ununordered_set>, <unordered_map>
#if !defined(WDSC_HAVE_CPP11_CONTAINERS) && !defined(WDSC_NO_HAVE_CPP11_CONTAINERS)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_CONTAINERS 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4004) // Actually GCC 4.3 supports array and unordered_
		#define WDSC_HAVE_CPP11_CONTAINERS 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_CONTAINERS 1
	#else
		#define WDSC_NO_HAVE_CPP11_CONTAINERS 1
	#endif
#endif

// #include <atomic>
#if !defined(WDSC_HAVE_CPP11_ATOMIC) && !defined(WDSC_NO_HAVE_CPP11_ATOMIC)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_ATOMIC 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4007)
		#define WDSC_HAVE_CPP11_ATOMIC 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_ATOMIC 1
	#else
		#define WDSC_NO_HAVE_CPP11_ATOMIC 1
	#endif
#endif

// #include <condition_variable>
#if !defined(WDSC_HAVE_CPP11_CONDITION_VARIABLE) && !defined(WDSC_NO_HAVE_CPP11_CONDITION_VARIABLE)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_CONDITION_VARIABLE 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4007)
		#define WDSC_HAVE_CPP11_CONDITION_VARIABLE 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_CONDITION_VARIABLE 1
	#else
		#define WDSC_NO_HAVE_CPP11_CONDITION_VARIABLE 1
	#endif
#endif

// #include <mutex>
#if !defined(WDSC_HAVE_CPP11_MUTEX) && !defined(WDSC_NO_HAVE_CPP11_MUTEX)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_MUTEX 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4007)
		#define WDSC_HAVE_CPP11_MUTEX 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_MUTEX 1
	#else
		#define WDSC_NO_HAVE_CPP11_MUTEX 1
	#endif
#endif

// #include <thread>
#if !defined(WDSC_HAVE_CPP11_THREAD) && !defined(WDSC_NO_HAVE_CPP11_THREAD)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_THREAD 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4007)
		#define WDSC_HAVE_CPP11_THREAD 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_THREAD 1
	#else
		#define WDSC_NO_HAVE_CPP11_THREAD 1
	#endif
#endif

// #include <future>
#if !defined(WDSC_HAVE_CPP11_FUTURE) && !defined(WDSC_NO_HAVE_CPP11_FUTURE)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_FUTURE 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4005)
		#define WDSC_HAVE_CPP11_FUTURE 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_FUTURE 1
	#else
		#define WDSC_NO_HAVE_CPP11_FUTURE 1
	#endif
#endif


// #include <type_traits>
#if !defined(WDSC_HAVE_CPP11_TYPE_TRAITS) && !defined(WDSC_NO_HAVE_CPP11_TYPE_TRAITS)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_TYPE_TRAITS 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4007) // Prior versions of libstdc++ have incomplete support for C++11 type traits.
		#define WDSC_HAVE_CPP11_TYPE_TRAITS 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_TYPE_TRAITS 1
	#else
		#define WDSC_NO_HAVE_CPP11_TYPE_TRAITS 1
	#endif
#endif

// #include <tuple>
#if !defined(WDSC_HAVE_CPP11_TUPLES) && !defined(WDSC_NO_HAVE_CPP11_TUPLES)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_TUPLES 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4003)
		#define WDSC_HAVE_CPP11_TUPLES 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_TUPLES 1
	#else
		#define WDSC_NO_HAVE_CPP11_TUPLES 1
	#endif
#endif

// #include <regex>
#if !defined(WDSC_HAVE_CPP11_REGEX) && !defined(WDSC_NO_HAVE_CPP11_REGEX)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) && (defined(_HAS_EXCEPTIONS) && _HAS_EXCEPTIONS) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_REGEX 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4003)
		#define WDSC_HAVE_CPP11_REGEX 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_REGEX 1
	#else
		#define WDSC_NO_HAVE_CPP11_REGEX 1
	#endif
#endif

// #include <random>
#if !defined(WDSC_HAVE_CPP11_RANDOM) && !defined(WDSC_NO_HAVE_CPP11_RANDOM)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_RANDOM 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4005)
		#define WDSC_HAVE_CPP11_RANDOM 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_RANDOM 1
	#else
		#define WDSC_NO_HAVE_CPP11_RANDOM 1
	#endif
#endif

// #include <chrono> 
#if !defined(WDSC_HAVE_CPP11_CHRONO) && !defined(WDSC_NO_HAVE_CPP11_CHRONO)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_CHRONO 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4007) // chrono was broken in glibc prior to 4.7.
		#define WDSC_HAVE_CPP11_CHRONO 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_CHRONO 1
	#else
		#define WDSC_NO_HAVE_CPP11_CHRONO 1
	#endif
#endif

// #include <scoped_allocator> 
#if !defined(WDSC_HAVE_CPP11_SCOPED_ALLOCATOR) && !defined(WDSC_NO_HAVE_CPP11_SCOPED_ALLOCATOR)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 540) // Dinkumware. VS2012+
		#define WDSC_HAVE_CPP11_SCOPED_ALLOCATOR 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4007)
		#define WDSC_HAVE_CPP11_SCOPED_ALLOCATOR 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_SCOPED_ALLOCATOR 1
	#else
		#define WDSC_NO_HAVE_CPP11_SCOPED_ALLOCATOR 1
	#endif
#endif

// #include <initializer_list> 
#if !defined(WDSC_HAVE_CPP11_INITIALIZER_LIST) && !defined(WDSC_NO_HAVE_CPP11_INITIALIZER_LIST)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) && !defined(WDSC_COMPILER_NO_INITIALIZER_LISTS) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_INITIALIZER_LIST 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_CLANG) && (WDSC_COMPILER_VERSION >= 301) && !defined(WDSC_COMPILER_NO_INITIALIZER_LISTS) && !defined(WDSC_PLATFORM_APPLE)
		#define WDSC_HAVE_CPP11_INITIALIZER_LIST 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBCPP_LIBRARY) && defined(WDSC_COMPILER_CLANG) && (WDSC_COMPILER_VERSION >= 301) && !defined(WDSC_COMPILER_NO_INITIALIZER_LISTS) && !defined(WDSC_PLATFORM_APPLE)
		#define WDSC_HAVE_CPP11_INITIALIZER_LIST 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4004) && !defined(WDSC_COMPILER_NO_INITIALIZER_LISTS) && !defined(WDSC_PLATFORM_APPLE)
		#define WDSC_HAVE_CPP11_INITIALIZER_LIST 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1) && !defined(WDSC_COMPILER_NO_INITIALIZER_LISTS)
		#define WDSC_HAVE_CPP11_INITIALIZER_LIST 1
	#else
		#define WDSC_NO_HAVE_CPP11_INITIALIZER_LIST 1
	#endif
#endif

// #include <system_error> 
#if !defined(WDSC_HAVE_CPP11_SYSTEM_ERROR) && !defined(WDSC_NO_HAVE_CPP11_SYSTEM_ERROR)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) && !(defined(_HAS_CPP0X) && _HAS_CPP0X) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_SYSTEM_ERROR 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_CLANG) && (WDSC_COMPILER_VERSION >= 301) && !defined(WDSC_PLATFORM_APPLE)
		#define WDSC_HAVE_CPP11_SYSTEM_ERROR 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4004) && !defined(WDSC_PLATFORM_APPLE)
		#define WDSC_HAVE_CPP11_SYSTEM_ERROR 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_SYSTEM_ERROR 1
	#else
		#define WDSC_NO_HAVE_CPP11_SYSTEM_ERROR 1
	#endif
#endif

// #include <codecvt> 
#if !defined(WDSC_HAVE_CPP11_CODECVT) && !defined(WDSC_NO_HAVE_CPP11_CODECVT)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_CODECVT 1
	// Future versions of libc++ may support this header.  However, at the moment there isn't
	// a reliable way of detecting if this header is available.
	//#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4008)
	//    #define WDSC_HAVE_CPP11_CODECVT 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_CODECVT 1
	#else
		#define WDSC_NO_HAVE_CPP11_CODECVT 1
	#endif
#endif

// #include <typeindex> 
#if !defined(WDSC_HAVE_CPP11_TYPEINDEX) && !defined(WDSC_NO_HAVE_CPP11_TYPEINDEX)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_TYPEINDEX 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4006)
		#define WDSC_HAVE_CPP11_TYPEINDEX 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_TYPEINDEX 1
	#else
		#define WDSC_NO_HAVE_CPP11_TYPEINDEX 1
	#endif
#endif




/* WDSC_HAVE_XXX_DECL */

#if !defined(WDSC_HAVE_mkstemps_DECL) && !defined(WDSC_NO_HAVE_mkstemps_DECL)
	#if defined(WDSC_PLATFORM_APPLE) || defined(WDSC_PLATFORM_SUN)
		#define WDSC_HAVE_mkstemps_DECL 1
	#else
		#define WDSC_NO_HAVE_mkstemps_DECL 1
	#endif
#endif

#if !defined(WDSC_HAVE_gettimeofday_DECL) && !defined(WDSC_NO_HAVE_gettimeofday_DECL)
	#if defined(WDSC_PLATFORM_POSIX) /* Posix means Linux, Unix, and Macintosh OSX, among others (including Linux-based mobile platforms). */
		#define WDSC_HAVE_gettimeofday_DECL 1
	#else
		#define WDSC_NO_HAVE_gettimeofday_DECL 1
	#endif
#endif

#if !defined(WDSC_HAVE_strcasecmp_DECL) && !defined(WDSC_NO_HAVE_strcasecmp_DECL)
	#if !defined(WDSC_PLATFORM_MICROSOFT)
		#define WDSC_HAVE_strcasecmp_DECL  1     /* This is found as stricmp when not found as strcasecmp */
		#define WDSC_HAVE_strncasecmp_DECL 1
	#else
		#define WDSC_HAVE_stricmp_DECL  1
		#define WDSC_HAVE_strnicmp_DECL 1
	#endif
#endif

#if !defined(WDSC_HAVE_mmap_DECL) && !defined(WDSC_NO_HAVE_mmap_DECL)
	#if defined(WDSC_PLATFORM_POSIX)
		#define WDSC_HAVE_mmap_DECL 1 /* mmap functionality varies significantly between systems. */
	#else
		#define WDSC_NO_HAVE_mmap_DECL 1
	#endif
#endif

#if !defined(WDSC_HAVE_fopen_DECL) && !defined(WDSC_NO_HAVE_fopen_DECL)
		#define WDSC_HAVE_fopen_DECL 1 /* C FILE functionality such as fopen */
#endif

#if !defined(WDSC_HAVE_ISNAN) && !defined(WDSC_NO_HAVE_ISNAN)
	#if defined(WDSC_PLATFORM_MICROSOFT) && !defined(WDSC_PLATFORM_MINGW)
		#define WDSC_HAVE_ISNAN(x)  _isnan(x)          /* declared in <math.h> */
		#define WDSC_HAVE_ISINF(x)  !_finite(x)
	#elif defined(WDSC_PLATFORM_APPLE)
		#define WDSC_HAVE_ISNAN(x)  std::isnan(x)      /* declared in <cmath> */
		#define WDSC_HAVE_ISINF(x)  std::isinf(x)
	#elif defined(WDSC_PLATFORM_ANDROID)
		#define WDSC_HAVE_ISNAN(x)  __builtin_isnan(x) /* There are a number of standard libraries for Android and it's hard to tell them apart, so just go with builtins */
		#define WDSC_HAVE_ISINF(x)  __builtin_isinf(x)
	#elif defined(__GNUC__) && defined(__CYGWIN__)
		#define WDSC_HAVE_ISNAN(x)  __isnand(x)        /* declared nowhere, it seems. */
		#define WDSC_HAVE_ISINF(x)  __isinfd(x)
	#else
		#define WDSC_HAVE_ISNAN(x)  std::isnan(x)      /* declared in <cmath> */
		#define WDSC_HAVE_ISINF(x)  std::isinf(x)
	#endif
#endif

#if !defined(WDSC_HAVE_itoa_DECL) && !defined(WDSC_NO_HAVE_itoa_DECL)
	#if defined(WDSC_COMPILER_MSVC)
		#define WDSC_HAVE_itoa_DECL 1
	#else
		#define WDSC_NO_HAVE_itoa_DECL 1
	#endif
#endif

#if !defined(WDSC_HAVE_nanosleep_DECL) && !defined(WDSC_NO_HAVE_nanosleep_DECL)
	#if (defined(WDSC_PLATFORM_UNIX) && !defined(WDSC_PLATFORM_SONY)) || defined(WDSC_PLATFORM_IPHONE) || defined(WDSC_PLATFORM_OSX) || defined(WDSC_PLATFORM_SONY) || defined(WDSC_PLATFORM_NX)
		#define WDSC_HAVE_nanosleep_DECL 1
	#else
		#define WDSC_NO_HAVE_nanosleep_DECL 1
	#endif
#endif

#if !defined(WDSC_HAVE_utime_DECL) && !defined(WDSC_NO_HAVE_utime_DECL)
	#if defined(WDSC_PLATFORM_MICROSOFT)
		#define WDSC_HAVE_utime_DECL _utime
	#elif WDSC_PLATFORM_UNIX
		#define WDSC_HAVE_utime_DECL utime
	#else
		#define WDSC_NO_HAVE_utime_DECL 1
	#endif
#endif

#if !defined(WDSC_HAVE_ftruncate_DECL) && !defined(WDSC_NO_HAVE_ftruncate_DECL)
	#if !defined(__MINGW32__)
		#define WDSC_HAVE_ftruncate_DECL 1
	#else
		#define WDSC_NO_HAVE_ftruncate_DECL 1
	#endif
#endif

#if !defined(WDSC_HAVE_localtime_DECL) && !defined(WDSC_NO_HAVE_localtime_DECL)
		#define WDSC_HAVE_localtime_DECL 1
#endif

#if !defined(WDSC_HAVE_pthread_getattr_np_DECL) && !defined(WDSC_NO_HAVE_pthread_getattr_np_DECL)
	#if defined(WDSC_PLATFORM_LINUX)
		#define WDSC_HAVE_pthread_getattr_np_DECL 1
	#else
		#define WDSC_NO_HAVE_pthread_getattr_np_DECL 1
	#endif
#endif



/* WDSC_HAVE_XXX_IMPL*/

#if !defined(WDSC_HAVE_WCHAR_IMPL) && !defined(WDSC_NO_HAVE_WCHAR_IMPL)
	#if defined(WDSC_PLATFORM_DESKTOP)
		#define WDSC_HAVE_WCHAR_IMPL 1      /* Specifies if wchar_t string functions are provided, such as wcslen, wprintf, etc. Implies WDSC_HAVE_WCHAR_H */
	#else
		#define WDSC_NO_HAVE_WCHAR_IMPL 1
	#endif
#endif

#if !defined(WDSC_HAVE_getenv_IMPL) && !defined(WDSC_NO_HAVE_getenv_IMPL)
	#if (defined(WDSC_PLATFORM_DESKTOP) || defined(WDSC_PLATFORM_UNIX)) && !defined(WDSC_PLATFORM_WINRT)
		#define WDSC_HAVE_getenv_IMPL 1
	#else
		#define WDSC_NO_HAVE_getenv_IMPL 1
	#endif
#endif

#if !defined(WDSC_HAVE_setenv_IMPL) && !defined(WDSC_NO_HAVE_setenv_IMPL)
	#if defined(WDSC_PLATFORM_UNIX) && defined(WDSC_PLATFORM_POSIX)
		#define WDSC_HAVE_setenv_IMPL 1
	#else
		#define WDSC_NO_HAVE_setenv_IMPL 1
	#endif
#endif

#if !defined(WDSC_HAVE_unsetenv_IMPL) && !defined(WDSC_NO_HAVE_unsetenv_IMPL)
	#if defined(WDSC_PLATFORM_UNIX) && defined(WDSC_PLATFORM_POSIX)
		#define WDSC_HAVE_unsetenv_IMPL 1
	#else
		#define WDSC_NO_HAVE_unsetenv_IMPL 1
	#endif
#endif

#if !defined(WDSC_HAVE_putenv_IMPL) && !defined(WDSC_NO_HAVE_putenv_IMPL)
	#if (defined(WDSC_PLATFORM_DESKTOP) || defined(WDSC_PLATFORM_UNIX)) && !defined(WDSC_PLATFORM_WINRT)
		#define WDSC_HAVE_putenv_IMPL 1        /* With Microsoft compilers you may need to use _putenv, as they have deprecated putenv. */
	#else
		#define WDSC_NO_HAVE_putenv_IMPL 1
	#endif
#endif

#if !defined(WDSC_HAVE_time_IMPL) && !defined(WDSC_NO_HAVE_time_IMPL)
	#if defined(WDSC_PLATFORM_NX)
		#define WDSC_NO_HAVE_time_IMPL 1
	#else
		#define WDSC_HAVE_time_IMPL 1
		#define WDSC_HAVE_clock_IMPL 1
	#endif
#endif

// <cstdio> fopen()
#if !defined(WDSC_HAVE_fopen_IMPL) && !defined(WDSC_NO_HAVE_fopen_IMPL)
		#define WDSC_HAVE_fopen_IMPL 1  /* C FILE functionality such as fopen */
#endif

// <arpa/inet.h> inet_ntop()
#if !defined(WDSC_HAVE_inet_ntop_IMPL) && !defined(WDSC_NO_HAVE_inet_ntop_IMPL)
	#if (defined(WDSC_PLATFORM_UNIX) || defined(WDSC_PLATFORM_POSIX)) && !defined(WDSC_PLATFORM_SONY) && !defined(WDSC_PLATFORM_NX) 
		#define WDSC_HAVE_inet_ntop_IMPL 1  /* This doesn't identify if the platform SDK has some alternative function that does the same thing; */
		#define WDSC_HAVE_inet_pton_IMPL 1  /* it identifies strictly the <arpa/inet.h> inet_ntop and inet_pton functions. For example, Microsoft has InetNtop in <Ws2tcpip.h> */
	#else
		#define WDSC_NO_HAVE_inet_ntop_IMPL 1
		#define WDSC_NO_HAVE_inet_pton_IMPL 1
	#endif
#endif

// <time.h> clock_gettime()
#if !defined(WDSC_HAVE_clock_gettime_IMPL) && !defined(WDSC_NO_HAVE_clock_gettime_IMPL)
	#if defined(WDSC_PLATFORM_LINUX) || defined(__CYGWIN__) || (defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)) || (defined(WDSC_PLATFORM_POSIX) && defined(_CPPLIB_VER) /*Dinkumware*/)
		#define WDSC_HAVE_clock_gettime_IMPL 1 /* You need to link the 'rt' library to get this */
	#else
		#define WDSC_NO_HAVE_clock_gettime_IMPL 1
	#endif
#endif

#if !defined(WDSC_HAVE_getcwd_IMPL) && !defined(WDSC_NO_HAVE_getcwd_IMPL)
	#if (defined(WDSC_PLATFORM_DESKTOP) || defined(WDSC_PLATFORM_UNIX)) && !defined(WDSC_PLATFORM_ANDROID) && !defined(WDSC_PLATFORM_WINRT)
		#define WDSC_HAVE_getcwd_IMPL 1       /* With Microsoft compilers you may need to use _getcwd, as they have deprecated getcwd. And in any case it's present at <direct.h> */
	#else
		#define WDSC_NO_HAVE_getcwd_IMPL 1
	#endif
#endif

#if !defined(WDSC_HAVE_tmpnam_IMPL) && !defined(WDSC_NO_HAVE_tmpnam_IMPL)
	#if (defined(WDSC_PLATFORM_DESKTOP) || defined(WDSC_PLATFORM_UNIX)) && !defined(WDSC_PLATFORM_ANDROID)
		#define WDSC_HAVE_tmpnam_IMPL 1
	#else
		#define WDSC_NO_HAVE_tmpnam_IMPL 1
	#endif
#endif

// nullptr, the built-in C++11 type.
// This WDSC_HAVE is deprecated, as WDSC_COMPILER_NO_NULLPTR is more appropriate, given that nullptr is a compiler-level feature and not a library feature.
#if !defined(WDSC_HAVE_nullptr_IMPL) && !defined(WDSC_NO_HAVE_nullptr_IMPL)
	#if defined(WDSC_COMPILER_NO_NULLPTR)
		#define WDSC_NO_HAVE_nullptr_IMPL 1
	#else
		#define WDSC_HAVE_nullptr_IMPL 1
	#endif
#endif

// <cstddef> std::nullptr_t
// Note that <EABase/nullptr.h> implements a portable nullptr implementation, but this 
// WDSC_HAVE specifically refers to std::nullptr_t from the standard libraries.
#if !defined(WDSC_HAVE_nullptr_t_IMPL) && !defined(WDSC_NO_HAVE_nullptr_t_IMPL)
	#if defined(WDSC_COMPILER_CPP11_ENABLED)
		// VS2010+ with its default Dinkumware standard library.
		#if defined(_MSC_VER) && (_MSC_VER >= 1600) && defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY)
			#define WDSC_HAVE_nullptr_t_IMPL 1

		#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) // clang/llvm libc++
			#define WDSC_HAVE_nullptr_t_IMPL 1

		#elif defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) // GNU libstdc++
			// Unfortunately __GLIBCXX__ date values don't go strictly in version ordering.
			#if (__GLIBCXX__ >= 20110325) && (__GLIBCXX__ != 20120702) && (__GLIBCXX__ != 20110428)
				#define WDSC_HAVE_nullptr_t_IMPL 1
			#else
				#define WDSC_NO_HAVE_nullptr_t_IMPL 1
			#endif
			
		// We simply assume that the standard library (e.g. Dinkumware) provides std::nullptr_t.
		#elif defined(__clang__)
			#define WDSC_HAVE_nullptr_t_IMPL 1

		// With GCC compiler >= 4.6, std::nullptr_t is always defined in <cstddef>, in practice.
		#elif defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4006)
			#define WDSC_HAVE_nullptr_t_IMPL 1

		// The EDG compiler provides nullptr, but uses an older standard library that doesn't support std::nullptr_t.
		#elif defined(__EDG_VERSION__) && (__EDG_VERSION__ >= 403) 
			#define WDSC_HAVE_nullptr_t_IMPL 1
			
		#else
			#define WDSC_NO_HAVE_nullptr_t_IMPL 1
		#endif
	#else
		#define WDSC_NO_HAVE_nullptr_t_IMPL 1
	#endif
#endif

// <exception> std::terminate
#if !defined(WDSC_HAVE_std_terminate_IMPL) && !defined(WDSC_NO_HAVE_std_terminate_IMPL)
	#if !defined(WDSC_PLATFORM_IPHONE) && !defined(WDSC_PLATFORM_ANDROID)
		#define WDSC_HAVE_std_terminate_IMPL 1 /* iOS doesn't appear to provide an implementation for std::terminate under the armv6 target. */
	#else
		#define WDSC_NO_HAVE_std_terminate_IMPL 1
	#endif
#endif

// <iterator>: std::begin, std::end, std::prev, std::next, std::move_iterator.
#if !defined(WDSC_HAVE_CPP11_ITERATOR_IMPL) && !defined(WDSC_NO_HAVE_CPP11_ITERATOR_IMPL)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) && !(defined(_HAS_CPP0X) && _HAS_CPP0X) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_ITERATOR_IMPL 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4006)
		#define WDSC_HAVE_CPP11_ITERATOR_IMPL 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_ITERATOR_IMPL 1
	#else
		#define WDSC_NO_HAVE_CPP11_ITERATOR_IMPL 1
	#endif
#endif

// <memory>: std::weak_ptr, std::shared_ptr, std::unique_ptr, std::bad_weak_ptr, std::owner_less
#if !defined(WDSC_HAVE_CPP11_SMART_POINTER_IMPL) && !defined(WDSC_NO_HAVE_CPP11_SMART_POINTER_IMPL)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) && !(defined(_HAS_CPP0X) && _HAS_CPP0X) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_SMART_POINTER_IMPL 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4004)
		#define WDSC_HAVE_CPP11_SMART_POINTER_IMPL 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_SMART_POINTER_IMPL 1
	#else
		#define WDSC_NO_HAVE_CPP11_SMART_POINTER_IMPL 1
	#endif
#endif

// <functional>: std::function, std::mem_fn, std::bad_function_call, std::is_bind_expression, std::is_placeholder, std::reference_wrapper, std::hash, std::bind, std::ref, std::cref.
#if !defined(WDSC_HAVE_CPP11_FUNCTIONAL_IMPL) && !defined(WDSC_NO_HAVE_CPP11_FUNCTIONAL_IMPL)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) && !(defined(_HAS_CPP0X) && _HAS_CPP0X) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_FUNCTIONAL_IMPL 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4004)
		#define WDSC_HAVE_CPP11_FUNCTIONAL_IMPL 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_FUNCTIONAL_IMPL 1
	#else
		#define WDSC_NO_HAVE_CPP11_FUNCTIONAL_IMPL 1
	#endif
#endif

// <exception> std::current_exception, std::rethrow_exception, std::exception_ptr, std::make_exception_ptr
#if !defined(WDSC_HAVE_CPP11_EXCEPTION_IMPL) && !defined(WDSC_NO_HAVE_CPP11_EXCEPTION_IMPL)
	#if defined(WDSC_HAVE_DINKUMWARE_CPP_LIBRARY) && (_CPPLIB_VER >= 520) && !(defined(_HAS_CPP0X) && _HAS_CPP0X) // Dinkumware. VS2010+
		#define WDSC_HAVE_CPP11_EXCEPTION_IMPL 1
	#elif defined(WDSC_COMPILER_CPP11_ENABLED) && defined(WDSC_HAVE_LIBSTDCPP_LIBRARY) && defined(WDSC_COMPILER_GNUC) && (WDSC_COMPILER_VERSION >= 4004)
		#define WDSC_HAVE_CPP11_EXCEPTION_IMPL 1
	#elif defined(WDSC_HAVE_LIBCPP_LIBRARY) && (_LIBCPP_VERSION >= 1)
		#define WDSC_HAVE_CPP11_EXCEPTION_IMPL 1
	#else
		#define WDSC_NO_HAVE_CPP11_EXCEPTION_IMPL 1
	#endif
#endif




/* Implementations that all platforms seem to have: */
/*
	alloca
	malloc
	calloc
	strtoll
	strtoull
	vsprintf
	vsnprintf
*/

/* Implementations that we don't care about: */
/*
	bcopy   -- Just use memmove or some customized equivalent. bcopy offers no practical benefit.
	strlcpy -- So few platforms have this built-in that we get no benefit from using it. Use EA::StdC::Strlcpy instead.
	strlcat -- "
*/



/*-----------------------------------------------------------------------------
	EABASE_USER_HAVE_HEADER
	
	This allows the user to define a header file to be #included after the 
	eahave.h's contents are compiled. A primary use of this is to override
	the contents of this header file. You can define the overhead header 
	file name in-code or define it globally as part of your build file.
	
	Example usage:
	   #define EABASE_USER_HAVE_HEADER "MyHaveOverrides.h" 
	   #include <WDBase/wdcorphave.h>
---------------------------------------------------------------------------*/

#ifdef EABASE_USER_HAVE_HEADER
	#include EABASE_USER_HAVE_HEADER
#endif


#endif /* Header include guard */



