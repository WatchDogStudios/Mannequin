/*-----------------------------------------------------------------------------
 * wdcorpdeprecatedbase.h
 *
 * Copyright (c) Electronic Arts Inc. & WD Studios Corp. All rights reserved.
 *---------------------------------------------------------------------------*/

#pragma once
#ifndef INCLUDED_eadeprecated_H
#  define INCLUDED_eadeprecated_H

#  include <WDBase/config/wdcorpcompilertraits.h>


// ------------------------------------------------------------------------
// Documentation on deprecated attribute:
// https://en.cppreference.com/w/cpp/language/attributes/deprecated Documentation on
// SimVer version numbers: http://simver.org/
//
// These macros provide a structured formatting to C++ deprecated annotation messages.
// This ensures
//  that the required information is presented in a standard format for developers and
//  tools.
//
// Example usage:
//
// Current package version : current_ver
// Future version for code removal : major_ver, minor_ver, change_ver
// Deprecation comment : ""
//
// 	WDSC_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated function") 	void TestFunc() {}
//
// 	WDSC_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated typedef") 	typedef int TestTypedef;
//
// 	WDSC_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated variable") 	int TestVariable;
//
// 	WDSC_DEPRECATED_STRUCT(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated struct") 	TestStruct {};
//
// 	WDSC_DEPRECATED_CLASS(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated class") 	TestClass {};
//
// 	union TestUnion
// 	{
// 		WDSC_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated data member") int n;
// 	};
//
// 	WDSC_DEPRECATED_ENUM(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated enumeration") 	TestEnumeration { TestEnumeration_Value1,
// TestEnumeration_Value2 };
//
// 	enum TestEnumerator
// 	{
// 		TestEnumerator_Value1 WDSC_DEPRECATED_ENUMVALUE(current_ver, major_ver, minor_ver,
// change_ver, tag, "Do not use deprecated enum value") = 5, 		TestEnumerator_Value2 = 4
// 	};
//
// 	WDSC_DISABLE_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Suppress
// the deprecated warning until the given Release") 	void TestFunc() {}
// 	WDSC_RESTORE_DEPRECATED()
//

// ------------------------------------------------------------------------
// Create an integer version number which can be compared with numerical operators
#  define WDSC_CREATE_VERSION(MAJOR, MINOR, PATCH) \
    (((MAJOR) * 1000000) + (((MINOR) + 1) * 10000) + (((PATCH) + 1) * 100))

// ------------------------------------------------------------------------
// INTERNAL MACROS - DO NOT USE DIRECTLY
//
// When WDSC_DEPRECATED_API_EXPIRED_IS_ERROR this macro produce a static asset on code
// that is past the expiry date.

#  if defined(WDSC_DEPRECATED_API_EXPIRED_IS_ERROR) \
    && WDSC_DEPRECATED_API_EXPIRED_IS_ERROR
#    define WDSC_DEPRECATED_BEFORETYPE(                                            \
      _moduleVersion, _major_version, _minor_version, _patch_version, _annotation) \
      static_assert(_moduleVersion                                                 \
          < WDSC_CREATE_VERSION(_major_version, _minor_version, _patch_version),   \
        "This API has been deprecated and needs to be removed");
#  else
#    define WDSC_DEPRECATED_BEFORETYPE( \
      _moduleVersion, _major_version, _minor_version, _patch_version, _annotation)
#  endif


// ------------------------------------------------------------------------
// INTERNAL MACROS - DO NOT USE DIRECTLY
//
// When WDSC_IGNORE_DEPRECATION is set deprecation annotation will not be produced

#  if defined(WDSC_IGNORE_DEPRECATION) && WDSC_IGNORE_DEPRECATION
#    define WDSC_DEPRECATED_AFTERTYPE( \
      _major_version, _minor_version, _patch_version, _annotation, _msg)
#  else
#    define WDSC_DEPRECATED_AFTERTYPE(                                   \
      _major_version, _minor_version, _patch_version, _annotation, _msg) \
      WDSC_DEPRECATED_MESSAGE(_msg.This API will be removed in           \
          _major_version._minor_version._patch_version _annotation)
#  endif

// ------------------------------------------------------------------------
// INTERNAL MACROS - DO NOT USE DIRECTLY
//
// Simple case

#  define WDSC_DEPRECATED_SIMPLE(                                                      \
    _moduleVersion, _major_version, _minor_version, _patch_version, _annotation, _msg) \
    WDSC_DEPRECATED_BEFORETYPE(                                                        \
      _moduleVersion, _major_version, _minor_version, _patch_version, _annotation)     \
    WDSC_DEPRECATED_AFTERTYPE(                                                         \
      _major_version, _minor_version, _patch_version, _annotation, _msg)


// ------------------------------------------------------------------------
// INTERNAL MACROS - DO NOT USE DIRECTLY
//
// Macro which inserts the keyword to correctly format the deprecation annotation
#  define WDSC_DEPRECATED_TYPE(_moduleVersion, _major_version, _minor_version,     \
    _patch_version, _annotation, _msg, _keyword)                                   \
    WDSC_DEPRECATED_BEFORETYPE(                                                    \
      _moduleVersion, _major_version, _minor_version, _patch_version, _annotation) \
    _keyword WDSC_DEPRECATED_AFTERTYPE(                                            \
      _major_version, _minor_version, _patch_version, _annotation, _msg)



// ------------------------------------------------------------------------
// PUBLIC MACROS
// See file header comment for example usage.

// ------------------------------------------------------------------------
// 	WDSC_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated function") 	void TestFunc() {}
//
// 	WDSC_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated typedef") 	typedef int TestTypedef;
//
// 	WDSC_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated variable") 	int TestVariable;

#  define WDSC_DEPRECATED_API(                                                         \
    _moduleVersion, _major_version, _minor_version, _patch_version, _annotation, _msg) \
    WDSC_STRINGIFY(WDSC_DEPRECATED_SIMPLE(_moduleVersion, _major_version,              \
      _minor_version, _patch_version, _annotation, _msg))


// ------------------------------------------------------------------------
// 	WDSC_DEPRECATED_STRUCT(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated struct") 	TestStruct {};

#  define WDSC_DEPRECATED_STRUCT(                                                       \
    _moduleVersion, _major_version, _minor_version, _patch_version, _annotation, _msg)  \
    WDSC_STRINGIFY(WDSC_DEPRECATED_TYPE(_moduleVersion, _major_version, _minor_version, \
      _patch_version, _annotation, _msg, struct))


// ------------------------------------------------------------------------
// 	WDSC_DEPRECATED_CLASS(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated class") 	TestClass {};

#  define WDSC_DEPRECATED_CLASS(                                                        \
    _moduleVersion, _major_version, _minor_version, _patch_version, _annotation, _msg)  \
    WDSC_STRINGIFY(WDSC_DEPRECATED_TYPE(_moduleVersion, _major_version, _minor_version, \
      _patch_version, _annotation, _msg, class))


// ------------------------------------------------------------------------
// 	WDSC_DEPRECATED_ENUM(current_ver, major_ver, minor_ver, change_ver, tag, "Do not use
// deprecated enumeration") 	TestEnumeration { TestEnumeration_Value1,
// TestEnumeration_Value2 };

#  define WDSC_DEPRECATED_ENUM(                                                         \
    _moduleVersion, _major_version, _minor_version, _patch_version, _annotation, _msg)  \
    WDSC_STRINGIFY(WDSC_DEPRECATED_TYPE(_moduleVersion, _major_version, _minor_version, \
      _patch_version, _annotation, _msg, enum))


// ------------------------------------------------------------------------
// 	enum TestEnumerator
// 	{
// 		TestEnumerator_Value1 WDSC_DEPRECATED_ENUMVALUE(current_ver, major_ver, minor_ver,
// change_ver, tag, "Do not use deprecated enum value") = 5, 		TestEnumerator_Value2 = 4
// 	};
#  define WDSC_DEPRECATED_ENUMVALUE(_value, _moduleVersion, _major_version, \
    _minor_version, _patch_version, _annotation, _msg)                      \
    _value WDSC_STRINGIFY(WDSC_DEPRECATED_AFTERTYPE(                        \
      _moduleVersion, _major_version, _minor_version, _patch_version, _annotation))


// ------------------------------------------------------------------------
// Suppress deprecated warnings around a block of code, see file comment for full usage.
//  WDSC_DISABLE_DEPRECATED(current_ver, major_ver, minor_ver, change_ver, tag, "Suppress
//  the deprecated warning until the given Release")

#  define WDSC_DISABLE_DEPRECATED(                                                     \
    _moduleVersion, _major_version, _minor_version, _patch_version, _annotation, _msg) \
    WDSC_STRINGIFY(WDSC_DEPRECATED_BEFORETYPE(                                         \
      _moduleVersion, _major_version, _minor_version, _patch_version, _annotation))    \
    WDSC_DISABLE_VC_WARNING(4996);                                                     \
    WDSC_DISABLE_CLANG_WARNING(-Wdeprecated - declarations);

// ------------------------------------------------------------------------
// Restore the compiler warnings
//  WDSC_RESTORE_DEPRECATED()

#  define WDSC_RESTORE_DEPRECATED() \
    WDSC_RESTORE_CLANG_WARNING();   \
    WDSC_RESTORE_VC_WARNING();

#endif // Header include guard