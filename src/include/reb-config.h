/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: General build configuration
**  Module:  reb-config.h
**  Author:  Carl Sassenrath
**  Notes:
**      This is the first file included.
**
***********************************************************************/

// The TO_<target> define comes from -DTO_<target> in gcc cmd line

/** Primary Configuration **********************************************

The primary target system is defined by:

	TO_target		- for example TO_WINDOWS or TO_LINUX

The default config builds an R3 HOST executable program.

To change the config, host-kit developers can define:

	REB_EXT			- build an extension module
					  * create a DLL, not a host executable
					  * do not export a host lib (OS_ lib)
					  * call r3lib via struct and macros

	REB_CORE		- build /core only, no graphics, windows, etc.

Special internal defines used by RT, not Host-Kit developers:

	REB_API			- build r3lib as API
					  * export r3lib functions
					  * build r3lib dispatch table
					  * call host lib (OS_) via struct and macros

	REB_EXE			- build r3 as a standalone executable

	REB_DEF			- special includes, symbols, and tables

These are now obsolete (as of A107) and should be removed:

	REB_LIB
	CORE_ONLY
	REBOL_ONLY
	FULL_DEFS
	AS_LIB

*/


//* Options ************************************************************

#if !defined(REBOL_OPTIONS_FILE)
#include "opt-config.h"
#else
#include REBOL_OPTIONS_FILE
#endif

#define MIN_DICT 16 // Maximum number of keys in a map withouth hashing

//* Common *************************************************************

#define THREADED				// enable threads

#ifdef REB_EXE					// standalone exe from RT
#define RL_API
#else
#ifdef REB_API					// r3lib dll from RT
#define RL_API API_EXPORT
#else
#define RL_API API_IMPORT		// for host exe (not used for extension dlls)
#endif
#endif

#define HAS_LL_CONSTS // compiler allows 1234LL constants;
                      // undef bellow for targets where not supported


//* MS Windows *********************************************************

#ifdef TO_WINDOWS				// Win32/Intel

#define	WIN32_LEAN_AND_MEAN		// trim down the Win32 headers
#define OS_WIDE_CHAR			// OS uses WIDE_CHAR API
#define OS_CRLF TRUE			// uses CRLF as line terminator
#define OS_DIR_SEP '\\'			// file path separator (Thanks Bill.)
#define HAS_ASYNC_DNS			// supports it
#define ATOI					// supports it
#define ATOI64					// supports it
#define ITOA64					// supports it
#define NO_TTY_ATTRIBUTES		// used in read-line.c
#define FINITE _finite			// name used for it
#define INLINE __inline			// name used for it

#ifdef REB_VIEW
#define HAS_WIDGET_GOB			// supports it
// use native image codecs only in VIEW version so far
#if !defined(INCLUDE_IMAGE_OS_CODEC)
#define INCLUDE_IMAGE_OS_CODEC
#undef	INCLUDE_BMP_CODEC
#undef	INCLUDE_PNG_CODEC
#undef	INCLUDE_GIF_CODEC
#undef	INCLUDE_JPG_CODEC
#endif
#endif

#ifdef THREADED
#ifndef __MINGW32__
#define THREAD __declspec(thread)
#endif
#endif

// Used when we build REBOL as a DLL:
#define API_EXPORT __declspec(dllexport)
#define API_IMPORT __declspec(dllimport)

// Use non-standard int64 declarations:
#if (defined(_MSC_VER) && (_MSC_VER <= 1200))
#define ODD_INT_64
#undef HAS_LL_CONSTS
#endif

// Disable various warnings
#pragma warning(disable : 4201)		// nameless unions
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4244)		// float conversion - temporary
//#pragma warning(disable : 4057)
//#pragma warning(disable : 4701)

#define AGG_WIN32_FONTS //use WIN32 api for font handling

#else //end of Windows section

//* Non Windows ********************************************************

#define MIN_OS					// not all devices are working
#define NO_GRAPHICS				// no graphics yet
#define AGG_FREETYPE            //use freetype2 library for fonts by default
#define INLINE

#ifdef TO_MACOS
#define FINITE isfinite
#else
#define FINITE finite
#endif

#ifndef TO_HAIKU
// Unsupported by gcc 2.95.3-haiku-121101
#define API_EXPORT __attribute__((visibility("default")))
#else
#define API_EXPORT
#define DEF_UINT
#endif 

#define API_IMPORT
#endif


#ifdef TO_LINUX
#undef INCLUDE_MIDI_DEVICE      // Not implemented!
#define USE_SETENV 
#endif

#ifdef TO_MACOS					// macOS
#define USE_SETENV 
#endif

#ifdef TO_OSX					// OSX/PPC
#define OLD_COMPILER
#define USE_SETENV 
#endif

#ifdef TO_OPENBSD				// OpenBSD
#undef INCLUDE_MIDI_DEVICE      // Not implemented!
#define USE_SETENV 
#endif

#ifdef TO_FREEBSD				// FreeBSD
#undef INCLUDE_MIDI_DEVICE      // Not implemented!
#define USE_SETENV 
#endif

#ifdef TO_AMIGA					// Target for OS4
#undef INCLUDE_MIDI_DEVICE      // Not implemented!
#define HAS_BOOL
#define HAS_SMART_CONSOLE
#define NO_DL_LIB
#endif

//* Defaults ***********************************************************

#ifndef THREAD
#define THREAD
#endif

#ifndef OS_DIR_SEP
#define OS_DIR_SEP '/'			// rest of the world uses it
#endif

#ifndef OS_CRLF
#define OS_CRLF FALSE
#endif

//* ZLIB ***************************************************************

/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
#  ifdef SYS16BIT
#    define MAX_MEM_LEVEL 8
#  else
#    define MAX_MEM_LEVEL 9
#  endif
#endif

/* Maximum value for windowBits in deflateInit2 and inflateInit2. */
#ifndef MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#endif
