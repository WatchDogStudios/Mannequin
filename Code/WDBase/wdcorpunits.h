/*-----------------------------------------------------------------------------
* wdcorpunits.h
 *
 * Copyright (c) Electronic Arts Inc. & WD Studios Corp. All rights reserved.
 *---------------------------------------------------------------------------*/


#ifndef INCLUDED_eaunits_h
#define INCLUDED_eaunits_h

#include <WDBase/wdcorpbase.h>

#if defined(WDSC_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif

// Defining common SI unit macros.
//
// The mebibyte is a multiple of the unit byte for digital information. Technically a
// megabyte (MB) is a power of ten, while a mebibyte (MiB) is a power of two,
// appropriate for binary machines. Many Linux distributions use the unit, but it is
// not widely acknowledged within the industry or media.
// Reference: https://en.wikipedia.org/wiki/Mebibyte
//
// Examples:
// 	auto size1 = WDSC_KILOBYTE(16);
// 	auto size2 = WDSC_MEGABYTE(128);
// 	auto size3 = WDSC_MEBIBYTE(8);
// 	auto size4 = WDSC_GIBIBYTE(8);

// define byte for completeness
#define WDSC_BYTE(x) (x)

// Decimal SI units
#define WDSC_KILOBYTE(x) (size_t(x) * 1000)
#define WDSC_MEGABYTE(x) (size_t(x) * 1000 * 1000)
#define WDSC_GIGABYTE(x) (size_t(x) * 1000 * 1000 * 1000)
#define WDSC_TERABYTE(x) (size_t(x) * 1000 * 1000 * 1000 * 1000)
#define WDSC_PETABYTE(x) (size_t(x) * 1000 * 1000 * 1000 * 1000 * 1000)
#define WDSC_EXABYTE(x)  (size_t(x) * 1000 * 1000 * 1000 * 1000 * 1000 * 1000)

// Binary SI units
#define WDSC_KIBIBYTE(x) (size_t(x) * 1024)
#define WDSC_MEBIBYTE(x) (size_t(x) * 1024 * 1024)
#define WDSC_GIBIBYTE(x) (size_t(x) * 1024 * 1024 * 1024)
#define WDSC_TEBIBYTE(x) (size_t(x) * 1024 * 1024 * 1024 * 1024)
#define WDSC_PEBIBYTE(x) (size_t(x) * 1024 * 1024 * 1024 * 1024 * 1024)
#define WDSC_EXBIBYTE(x) (size_t(x) * 1024 * 1024 * 1024 * 1024 * 1024 * 1024)

#endif // INCLUDED_earesult_H




