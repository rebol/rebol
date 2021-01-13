/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
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
**  Summary: Optional (default) build configuration
**  Module:  opt-config.h
**  Author:  Oldes
**  Notes:
**      This file may be used to specify, which parts should be used or excluded
**
***********************************************************************/

#ifndef REBOL_OPTIONS_H

// native codecs ******************************************************/
#define INCLUDE_BMP_CODEC     // used in u-bmp.c file
#define INCLUDE_PNG_CODEC     // used in u-png.c file
#define INCLUDE_JPG_CODEC     // used in u-jpg.c file
#define INCLUDE_GIF_CODEC     // used in u-gif.c file

// native WAV codec was just a prove of concept, don't use it
// there is more feature full Rebol implementation instead
//#define INCLUDE_WAV_CODEC   // used in u-wav.c file


// optional compression & encoding *************************************/
#define	INCLUDE_LZMA          // used in u-lzma.c, n-string.c and u-compress.c files
#define INCLUDE_BASE85        // adds support for enbase/debase with base 85 (ASCII85)


// optional devices ***************************************************/
#define INCLUDE_MIDI_DEVICE


// optional natives ***************************************************/
#define INCLUDE_IMAGE_NATIVES


// otional checksums **************************************************/
#define INCLUDE_MBEDTLS       // used for checksum implementation so far (for crypt later)
#define INCLUDE_MD4           // checksum: MD4 (unsecure)
#define INCLUDE_RIPEMD160     // checksum: RIPE-MD-160 (requires USE_MBEDTLS)
#define INCLUDE_SHA224
#define INCLUDE_SHA384


// unfinished features ************************************************/
//#define INCLUDE_TASK        // tasks are not implemented yet, so include it only on demand



// other options ******************************************************/

//#define HAS_WIDGET_GOB  // used in t-gob.c

//#define EXCLUDE_CHACHA20POLY1305    // don't include chacha20 and poly1305 cipher/authentication code

//#define USE_EMPTY_HASH_AS_NONE      // a single # means NONE, else error; Used in l-scan.c file

//#define DO_NOT_NORMALIZE_MAP_KEYS
//	 with above define you would get:
//		[a b:]     = keys-of make map! [a 1 b: 2]
//		[a 1 b: 2] = body-of make map! [a 1 b: 2]
//	
//	 else:
//		[a b]       = keys-of make map! [a 1 b: 2]
//		[a: 1 b: 2] = body-of make map! [a 1 b: 2]

//#define FORCE_ANSI_ESC_EMULATION_ON_WINDOWS  // would not try to use MS' built-in VIRTUAL_TERMINAL_PROCESSING
//#define EXCLUDE_VECTOR_MATH  // don't include vector math support (like: 3 * #[vector! integer! 8 3 [1 2 3]]); Used in t-vector.c file
//#define WRITE_ANY_VALUE_TO_CLIPBOARD // https://github.com/Oldes/Rebol-issues/issues/1619


//#define SERIES_LABELS               // used for special debug purposes
//#define SHOW_SIZEOFS                // for debugging ports to some systems
//#define NDEBUG                      // removes some asserts



//**************************************************************//
#include "opt-dependencies.h" // checks for above options

#endif //REBOL_OPTIONS_H