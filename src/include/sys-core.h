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
**  Summary: System Core Include
**  Module:  sys-core.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "reb-config.h"

// Set as compiler symbol flags:
//#define UNICODE				// enable unicode OS API (windows)

// Internal configuration:
#define REB_DEF					// kernel definitions and structs
#define ASSERTIONS				// special run-time checks
//#define DEBUGGING				// debug output and debugger assistance
//#define SERIES_LABELS			// enable identifier labels for series
//#define MUNGWALL				// memory allocation bounds checking
#define STACK_MIN   4000		// data stack increment size
#define STACK_LIMIT 400000		// data stack max (6.4MB)
#define MIN_COMMON 10000		// min size of common buffer
#define MAX_COMMON 100000		// max size of common buffer (shrink trigger)
#define	MAX_NUM_LEN 64			// As many numeric digits we will accept on input
#define MAX_SAFE_SERIES 5		// quanitity of most recent series to not GC.
#define MAX_EXPAND_LIST 5		// number of series-1 in Prior_Expand list
#define USE_UNICODE 1			// scanner uses unicode
#define UNICODE_CASES 0x2E00	// size of unicode folding table
#define HAS_SHA1				// allow it
#define HAS_MD5					// allow it

// External system includes:
#include <stdlib.h>
#include <stdarg.h>		// For var-arg Print functions
#include <string.h>
#include <setjmp.h>
#include <math.h>

// Special OS-specific definitions:
#ifdef OS_DEFS
	#ifdef TO_WIN32
	#include <windows.h>
	#undef IS_ERROR
	#endif
	//#error The target platform must be specified (TO_* define)
#endif

#ifdef OS_IO
	#include <stdio.h>
	#include <stdarg.h>
#endif

// Local includes:
#include "reb-c.h"
#include "reb-defs.h"
#include "reb-args.h"
#include "tmp-bootdefs.h"
#define PORT_ACTIONS A_CREATE  // port actions begin here

#include "reb-device.h"
#include "reb-types.h"
#include "reb-event.h"

#include "sys-deci.h"

#include "sys-value.h"
#include "tmp-strings.h"
#include "tmp-funcargs.h"

//-- Port actions (for native port schemes):
typedef struct rebol_port_action_map {
	const REBCNT action;
	const REBPAF func;
} PORT_ACTION;

typedef struct rebol_mold {
	REBSER *series;		// destination series (uni)
	REBCNT opts;		// special option flags
	REBINT indent;		// indentation amount
//	REBYTE space;		// ?
	REBYTE period;		// for decimal point
	REBYTE dash;		// for date fields
	REBYTE digits;		// decimal digits
} REB_MOLD;

#include "reb-file.h"
#include "reb-filereq.h"
#include "reb-math.h"
#include "reb-codec.h"

#include "tmp-sysobj.h"
#include "tmp-sysctx.h"

//#include "reb-net.h"
#include "sys-panics.h"
#include "tmp-boot.h"
#include "sys-mem.h"
#include "tmp-errnums.h"
#include "host-lib.h"
#include "sys-stack.h"

/***********************************************************************
**
**	Constants
**
***********************************************************************/

enum Boot_Phases {
	BOOT_START = 0,
	BOOT_LOADED,
	BOOT_ERRORS,
	BOOT_MEZZ,
	BOOT_DONE
};

enum Boot_Levels {
	BOOT_LEVEL_BASE,
	BOOT_LEVEL_SYS,
	BOOT_LEVEL_MODS,
	BOOT_LEVEL_FULL
};

// Modes allowed by Copy_Block function:
enum {
	COPY_SHALLOW = 0,
	COPY_DEEP,			// recurse into blocks
	COPY_STRINGS,		// copy strings in blocks
	COPY_ALL,			// both deep, strings (3)
//	COPY_IGNORE = 4,	// ignore tail position (used for stack args)
	COPY_OBJECT = 8,	// copy an object
	COPY_SAME = 16,
};

#define CP_DEEP TYPESET(63)

#define TS_NOT_COPIED (TYPESET(REB_IMAGE) | TYPESET(REB_VECTOR) | TYPESET(REB_TASK) | TYPESET(REB_PORT))
#define TS_STD_SERIES (TS_SERIES & ~TS_NOT_COPIED)
#define TS_SERIES_OBJ ((TS_SERIES | TS_OBJECT) & ~TS_NOT_COPIED)
#define TS_BLOCKS_OBJ ((TS_BLOCK | TS_OBJECT) & ~TS_NOT_COPIED)

#define TS_CODE ((CP_DEEP | TS_SERIES) & ~TS_NOT_COPIED)

#define TS_FUNCLOS (TYPESET(REB_FUNCTION) | TYPESET(REB_CLOSURE))
#define TS_CLONE ((CP_DEEP | TS_SERIES | TS_FUNCLOS) & ~TS_NOT_COPIED)

// Modes allowed by Bind related functions:
enum {
	BIND_ONLY = 0,		// Only bind the words found in the context.
	BIND_SET,			// Add set-words to the context during the bind.
	BIND_ALL,			// Add words to the context during the bind.
	BIND_DEEP = 4,		// Recurse into sub-blocks.
	BIND_GET = 8,		// Lookup :word and use its word value
	BIND_NO_DUP = 16,	// Do not allow dups during word collection (for specs)
	BIND_FUNC = 32,		// Recurse into functions.
	BIND_NO_SELF = 64,	// Do not bind SELF (in closures)
};

// Modes for Rebind_Block:
enum {
	REBIND_TYPE = 1,	// Change frame type when rebinding
	REBIND_FUNC	= 2,	// Rebind function and closure bodies
	REBIND_TABLE = 4,	// Use bind table when rebinding
};

// Mold and form options:
enum REB_Mold_Opts {
	MOPT_MOLD_ALL,		// Output lexical types in #[type...] format
	MOPT_COMMA_PT,		// Decimal point is a comma.
	MOPT_SLASH_DATE,	// Date as 1/1/2000
//	MOPT_MOLD_VALS,		// Value parts are molded (strings are kept as is)
	MOPT_FILE,			// Molding %file
	MOPT_INDENT,		// Indentation
	MOPT_TIGHT,			// No space between block values
	MOPT_NO_NONE,		// Do not output UNSET or NONE object vars
	MOPT_EMAIL,
	MOPT_ONLY,			// Mold/only - no outer block []
	MOPT_LINES,			// add a linefeed between each value
};

#define GET_MOPT(v, f) GET_FLAG(v->opts, f)

// Special flags for decimal formatting:
#define DEC_MOLD_PERCENT 1  // follow num with %
#define DEC_MOLD_MINIMAL 2  // allow decimal to be integer

// Temporary:
#define MOPT_ANSI_ONLY	MOPT_MOLD_ALL	// Non ANSI chars are ^() escaped

// Reflector words (words-of, body-of, etc.)
enum Reb_Reflectors {
	OF_BASE,
	OF_WORDS, // to be compatible with R2
	OF_BODY,
	OF_SPEC,
	OF_VALUES,
	OF_TYPES,
	OF_TITLE,
};

// Load option flags:
enum {
	LOAD_ALL = 0,		// Returns header along with script if present
	LOAD_HEADER,		// Converts header to object, checks values
	LOAD_NEXT,			// Load next value
	LOAD_NORMAL,		// Convert header, load script
	LOAD_REQUIRE		// Header is required, else error
};

// General constants:
#define NOT_FOUND ((REBCNT)-1)
#define UNKNOWN   ((REBCNT)-1)
#define LF 10
#define CR 13
#define TAB '\t'
#define CRLF "\r\n"
#define TAB_SIZE 4

// Move this:
enum Insert_Arg_Nums {
	AN_SERIES = 1,
	AN_VALUE,
	AN_PART,
	AN_LENGTH,
	AN_ONLY,
	AN_DUP,
	AN_COUNT
};

enum rebol_signals {
	SIG_RECYCLE,
	SIG_ESCAPE,
	SIG_EVENT_PORT,
};

// Security flags:
enum {
	SEC_ALLOW,
	SEC_ASK,
	SEC_THROW,
	SEC_QUIT,
};

// Security policy byte offsets:
enum {
	POL_READ,
	POL_WRITE,
	POL_EXEC,
};

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

/***********************************************************************
**
**	Macros
**
***********************************************************************/

// Generic defines:
#define NZ(c) ((c) != 0)

#define FREE(m, s)  free(m)
#define ALIGN(s, a) (((s) + (a)-1) & ~((a)-1))

#define ALEVEL 2

#define ASSERT(c,m) if (!(c)) Crash(m);		// (breakpoint in Crash() to see why)
#if (ALEVEL>0)
#define ASSERT1(c,m) if (!(c)) Crash(m);	// Not in beta releases
#if (ALEVEL>1)
#define ASSERT2(c,m) if (!(c)) Crash(m);	// Not in any releases
#endif
#endif
#define MEM_CARE 5				// Lower number for more frequent checks


#define LOOP(n) for (; n > 0; n--)
#define	FOREACH(n, limit) for (n = 0; n < limit; n++)
#define	FOR_BLK(b, v, t) for (v = VAL_BLK_DATA(b), t = VAL_BLK_TAIL(b); v != t; v++)
#define	FOR_SER(b, v, i, s) for (; v = BLK_SKIP(b, i), i < SERIES_TAIL(b); i += skip)

#define UP_CASE(c) Upper_Cases[c]
#define LO_CASE(c) Lower_Cases[c]
#define IS_WHITE(c) ((c) <= 32 && (White_Chars[c]&1) != 0)
#define IS_SPACE(c) ((c) <= 32 && (White_Chars[c]&2) != 0)

#define SET_SIGNAL(f) SET_FLAG(Eval_Signals, f)
#define GET_SIGNAL(f) GET_FLAG(Eval_Signals, f)
#define CLR_SIGNAL(f) CLR_FLAG(Eval_Signals, f)

#define	DECIDE(cond) if (cond) goto is_true; else goto is_false
#define REM2(a, b) ((b)!=-1 ? (a) % (b) : 0)
//#define DO_BLOCK(v) Do_Block(VAL_SERIES(v), VAL_INDEX(v))
#define DO_BLK(v) Do_Blk(VAL_SERIES(v), VAL_INDEX(v))

#define DEAD_END	return 0	// makes compiler happy (for never used return case)

#define	NO_RESULT	((REBCNT)(-1))
#define	ALL_BITS	((REBCNT)(-1))
#ifdef HAS_LL_CONSTS
#define	ALL_64		((REBU64)0xffffffffffffffffLL)
#else
#define	ALL_64		((REBU64)0xffffffffffffffffL)
#endif

#define BOOT_STR(c,i) PG_Boot_Strs[(c)+(i)]

//-- Temporary Buffers
//   These are reused for cases for appending, when length cannot be known.
#define BUF_EMIT  VAL_SERIES(TASK_BUF_EMIT)
#define BUF_WORDS VAL_SERIES(TASK_BUF_WORDS)
#define BUF_PRINT VAL_SERIES(TASK_BUF_PRINT)
#define BUF_FORM  VAL_SERIES(TASK_BUF_FORM)
#define BUF_MOLD  VAL_SERIES(TASK_BUF_MOLD)
#define BUF_UTF8  VAL_SERIES(TASK_BUF_UTF8)
#define MOLD_LOOP VAL_SERIES(TASK_MOLD_LOOP)

#ifdef OS_WIDE_CHAR
#define BUF_OS_STR BUF_MOLD
#else
#define BUF_OS_STR BUF_FORM
#endif

// Save/Unsave Macros:
#define	SAVE_SERIES(s)		Save_Series(s)
#ifdef ASSERTIONS
#define	UNSAVE_SERIES(s)	GC_Protect->tail--;\
	ASSERT(((REBSER **)GC_Protect->data)[GC_Protect->tail] == s, RP_HOLD_SERIES_MALIGN)
#else
#define	UNSAVE_SERIES(s)	GC_Protect->tail--
#endif

#ifdef OS_STACK_GROWS_UP
#define CHECK_STACK(v) if ((REBCNT)(v) >= Stack_Limit) Trap_Stack();
#else
#define CHECK_STACK(v) if ((REBCNT)(v) <= Stack_Limit) Trap_Stack();
#endif
#define STACK_BOUNDS (4*1024*1000) // note: need a better way to set it !!
// Also: made somewhat smaller than linker setting to allow trapping it


/***********************************************************************
**
**	Structures
**
***********************************************************************/

// Word Table Structure - used to manage hashed word tables (symbol tables).
typedef struct rebol_word_table
{
	REBSER	*series;	// Global block of words
	REBSER	*hashes;	// Hash table
//	REBCNT	count;		// Number of units used in hash table
} WORD_TABLE;

//-- Measurement Variables:
typedef struct rebol_stats {
	REBI64	Series_Memory;
	REBCNT	Series_Made;
	REBCNT	Series_Freed;
	REBCNT	Series_Expanded;
	REBCNT	Recycle_Counter;
	REBCNT	Recycle_Series_Total;
	REBCNT	Recycle_Series;
	REBI64  Recycle_Prior_Eval;
	REBCNT	Mark_Count;
	REBCNT	Free_List_Checked;
	REBCNT	Blocks;
	REBCNT	Objects;
} REB_STATS;

//-- Options of various kinds:
typedef struct rebol_opts {
	REBFLG	watch_obj_copy;
	REBFLG	watch_recycle;
	REBFLG	watch_series;
	REBFLG	watch_expand;
	REBFLG	crash_dump;
} REB_OPTS;

typedef struct rebol_time_fields {
	REBCNT h;
	REBCNT m;
	REBCNT s;
	REBCNT n;
} REB_TIMEF;

/***********************************************************************
**
**	Thread Shared Variables
**
**		Set by main boot and not changed after that.
**
***********************************************************************/

extern const REBACT Value_Dispatch[];
//extern const REBYTE Upper_Case[];
//extern const REBYTE Lower_Case[];


#include "tmp-funcs.h"


/***********************************************************************
**
**	Threaded Global Variables
**
***********************************************************************/

#define PVAR extern
#define TVAR extern THREAD

#include "sys-globals.h"
