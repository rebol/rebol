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

#ifndef REB_DEF
typedef void(*ANYFUNC)(void *);
typedef struct RL_Reb_Series {
	void *data;
	u32 tail;
} REBSER;
typedef void *REBOBJ;
typedef struct Reb_Handle {
	REBCNT	sym;    // Index of the word's symbol. Used as a handle's type!
	REBFLG  flags;  // Handle_Flags
	union {
		ANYFUNC	code;
		REBSER *data;
		REBINT  index;
	};
} REBHAN;
#endif

/* These used for access-os native function */
#define OS_ENA    -1
#define OS_EINVAL -2
#define OS_EPERM  -3
#define OS_ESRCH  -4

#pragma pack(4)

// X/Y coordinate pair as floats:
typedef struct rebol_xy_float {
	float x;
	float y;
} REBXYF;

// X/Y coordinate pair as integers:
typedef struct rebol_xy_int {
	int x;
	int y;
} REBXYI;

#define REBPAR REBXYI  // temporary until all sources are converted

// Standard date and time:
typedef struct rebol_dat {
	int year;
	int month;
	int day;
	int time;
	int nano;
	int zone;
} REBOL_DAT;  // not same as REBDAT

// OS metrics: (not used as of A100!)
typedef struct rebol_met {
	int len; // # entries in this table
	REBPAR screen_size;
	REBPAR title_size;
	REBPAR border_size;
	REBPAR border_fixed;
	REBPAR work_origin;
	REBPAR work_size;
} X_REBOL_OS_METRICS;

typedef int	cmp_t(const void *, const void *);
void reb_qsort(void *a, size_t n, size_t es, cmp_t *cmp);

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

#endif
