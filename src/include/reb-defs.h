/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
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
**  Summary: Miscellaneous structures and definitions
**  Module:  reb-defs.h
**  Author:  Carl Sassenrath
**  Notes:
**      This file is used by internal and external C code. It
**      should not depend on many other header files prior to it.
**
***********************************************************************/

#ifndef REB_DEFS_H  // due to sequences within the lib build itself
#define REB_DEFS_H

/* These used for access-os native function */
#define OS_ENA    -1
#define OS_EINVAL -2
#define OS_EPERM  -3
#define OS_ESRCH  -4


#ifdef ENDIAN_LITTLE
#define OS_LITTLE_ENDIAN TRUE 
#define WRITE_BE_2(cp, bp)  cp[0] = bp[1]; cp[1] = bp[0];
#define WRITE_BE_3(cp, bp)  cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#define WRITE_BE_4(cp, bp)  cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#define WRITE_BE_8(cp, bp)  cp[0] = bp[7]; cp[1] = bp[6]; cp[2] = bp[5]; cp[3] = bp[4]; \
							cp[4] = bp[3]; cp[5] = bp[2]; cp[6] = bp[1]; cp[7] = bp[0];
#define WRITE_LE_2(cp, bp)  memcpy(cp, bp, 2);
#define WRITE_LE_3(cp, bp)  memcpy(cp, bp, 3);
#define WRITE_LE_4(cp, bp)  memcpy(cp, bp, 4);
#define WRITE_LE_8(cp, bp)  memcpy(cp, bp, 8);
#else
#define OS_LITTLE_ENDIAN FALSE 
#define WRITE_BE_2(cp, bp)  memcpy(cp, bp, 2);
#define WRITE_BE_3(cp, bp)  memcpy(cp, bp, 3);
#define WRITE_BE_4(cp, bp)  memcpy(cp, bp, 4);
#define WRITE_BE_8(cp, bp)  memcpy(cp, bp, 8);
#define WRITE_LE_2(cp, bp)  cp[0] = bp[1]; cp[1] = bp[0];
#define WRITE_LE_3(cp, bp)  cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#define WRITE_LE_4(cp, bp)  cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#define WRITE_LE_8(cp, bp)  cp[0] = bp[7]; cp[1] = bp[6]; cp[2] = bp[5]; cp[3] = bp[4]; \
							cp[4] = bp[3]; cp[5] = bp[2]; cp[6] = bp[1]; cp[7] = bp[0];
#endif


#pragma pack(4)

// Standard date and time:
typedef struct rebol_dat {
	int year;
	int month;
	int day;
	int time;
	int nano;
	int zone;
} REBOL_DAT;  // not same as REBDAT

typedef int	cmp_t(const void *, const void *);
void unstable_sort(void* a, size_t n, size_t es, cmp_t* cmp);
void   stable_sort(void* a, size_t n, size_t es, cmp_t* cmp);
#define SORT_FLAG_REVERSE 1
#define SORT_FLAG_WIDE    2
#define SORT_FLAG_CASE    3
#define SORT_FLAG_ALL     4
#define SORT_FLAG_BINARY  5 // used with the custom sort function


// Encoding_opts was originally in sys-core.h, but I moved it here so it can
// be used also while makking external extensions. (oldes)

// Encoding options:
enum encoding_opts {
	ENC_OPT_BIG,		// big endian (not little)
	ENC_OPT_UTF8,		// UTF-8
	ENC_OPT_UTF16,		// UTF-16
	ENC_OPT_UTF32,		// UTF-32
	ENC_OPT_BOM,		// byte order marker
	ENC_OPT_CRLF,		// CR line termination
	ENC_OPT_NO_COPY,	// do not copy if ASCII
};

#define ENCF_NO_COPY (1<<ENC_OPT_NO_COPY)
#if OS_CRLF
#define ENCF_OS_CRLF (1<<ENC_OPT_CRLF)
#else
#define ENCF_OS_CRLF 0
#endif

#pragma pack()

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12
#define IS_SURROGATE(c) (c >= 0xD800 && c <= 0xDFFF)
#define IS_INVALID_CHAR(c) (c == UNKNOWN || c > MAX_UNI || IS_SURROGATE(c))

#endif
