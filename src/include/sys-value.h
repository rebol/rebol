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
**  Summary: Value and Related Definitions
**  Module:  sys-value.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#ifndef VALUE_H
#define VALUE_H

/***********************************************************************
**
**	REBOL Value Type
**
**		This is used for all REBOL values. This is a forward
**		declaration. See end of this file for actual structure.
**
***********************************************************************/

#pragma pack(4)

typedef struct Reb_Header {
#ifdef ENDIAN_LITTLE
	unsigned type:8;	// datatype
	unsigned opts:8;	// special options
	unsigned exts:8;	// extensions to datatype
	unsigned resv:8;	// reserved for future
#else
	unsigned resv:8;	// reserved for future
	unsigned exts:8;	// extensions to datatype
	unsigned opts:8;	// special options
	unsigned type:8;	// datatype
#endif
} REBHED;

struct Reb_Value;
typedef struct Reb_Value REBVAL;
typedef struct Reb_Series REBSER;

// Value type identifier (generally, should be handled as integer):

#define VAL_TYPE(v)		((v)->flags.flags.type)			// get only type, not flags
#define SET_TYPE(v,t)	((v)->flags.flags.type = (t))	// set only type, not flags
#define VAL_SET(v,t)	((v)->flags.header = (t))		// set type, clear all flags
// Note: b-init.c verifies that lower 8 bits of header = flags.type

// Clear type identifier:
#define SET_END(v)			VAL_SET(v, 0)

// Value option flags:
enum {
	OPTS_LINE = 0,	// Line break occurs before this value
	OPTS_LOCK,		// Lock word from modification
	OPTS_REVAL,		// Reevaluate result value
	OPTS_UNWORD,	// Not a normal word
	OPTS_TEMP,		// Temporary flag - variety of uses
	OPTS_HIDE,		// Hide the word
};

#define VAL_OPTS(v)			((v)->flags.flags.opts)
#define VAL_SET_OPT(v,n)	SET_FLAG(VAL_OPTS(v), n)
#define VAL_GET_OPT(v,n)	GET_FLAG(VAL_OPTS(v), n)
#define VAL_CLR_OPT(v,n)	CLR_FLAG(VAL_OPTS(v), n)

#define VAL_GET_LINE(v)		VAL_GET_OPT((v), OPTS_LINE)
#define VAL_SET_LINE(v)		VAL_SET_OPT((v), OPTS_LINE)
#define VAL_CLR_LINE(v)		VAL_CLR_OPT((v), OPTS_LINE)

#define VAL_PROTECTED(v)	VAL_GET_OPT((v), OPTS_LOCK)

// Used for datatype-dependent data (e.g. op! stores action!)
#define VAL_GET_EXT(v)		((v)->flags.flags.exts)
#define VAL_SET_EXT(v,n)	((v)->flags.flags.exts = (n))

#define	IS_SET(v)			(VAL_TYPE(v) > REB_UNSET)
#define IS_SCALAR(v)		(VAL_TYPE(v) <= REB_DATE)


/***********************************************************************
**
**	DATATYPE - Datatype or pseudo-datatype
**
***********************************************************************/

typedef struct Reb_Type {
	REBINT	type;	// base type
	REBSER  *spec;
//	REBINT	min_type;
//	REBINT	max_type;
} REBTYP;

#define	VAL_DATATYPE(v)		((v)->data.datatype.type)
#define	VAL_TYPE_SPEC(v)	((v)->data.datatype.spec)

//#define	VAL_MIN_TYPE(v)	((v)->data.datatype.min_type)
//#define	VAL_MAX_TYPE(v)	((v)->data.datatype.max_type)
#define	IS_OF_DATATYPE(v,t) (IS_DATATYPE(v) && (VAL_DATATYPE(v) == (t)))
#define	NO_TYPE (0)


/***********************************************************************
**
**	NUMBERS - Integer and other simple scalars
**
***********************************************************************/

#define	SET_UNSET(v)	VAL_SET(v, REB_UNSET)

#define	SET_NONE(v)		VAL_SET(v, REB_NONE)
#define NONE_VALUE		ROOT_NONEVAL

#define VAL_INT32(v)	(REBINT)((v)->data.integer)
#define VAL_INT64(v)	((v)->data.integer)
#define VAL_UNT64(v)	((v)->data.unteger)
#define	SET_INTEGER(v,n) VAL_SET(v, REB_INTEGER), ((v)->data.integer) = (n)
#define	SET_INT32(v,n)  ((v)->data.integer) = (REBINT)(n)

#define MAX_CHAR		0xffff
#define VAL_CHAR(v)		((v)->data.uchar)
#define	SET_CHAR(v,n)	VAL_SET(v, REB_CHAR), VAL_CHAR(v) = (REBUNI)(n)

#define IS_NUMBER(v)	(VAL_TYPE(v) == REB_INTEGER || VAL_TYPE(v) == REB_DECIMAL)


/***********************************************************************
**
**	DECIMAL, MONEY -- Includes denomination and amount
**
***********************************************************************/

#define VAL_DECIMAL(v)	((v)->data.decimal)
#define	SET_DECIMAL(v,n) VAL_SET(v, REB_DECIMAL), VAL_DECIMAL(v) = (n)

typedef deci REBDCI;
#define VAL_DECI(v)		((v)->data.deci)
#define	SET_MONEY(v,n) VAL_SET(v, REB_MONEY), VAL_DECI(v) = (n)

#ifdef not_used
typedef struct Reb_Decimal {
	REBDEC number;
	REBYTE denom[4];
} REBMNY;
#define VAL_MONEY(v)	((v)->data.money)
#define	VAL_MONEY_DENOM(v)	//((v)->data.money.denom)
#define VAL_MONEY_AMOUNT(v)	((v)->data.money.amount)
#endif


/***********************************************************************
**
**	DATE and TIME
**
***********************************************************************/

typedef struct reb_ymdz {
#ifdef ENDIAN_LITTLE
	REBINT zone:7;  // +/-15:00 res: 0:15
	REBCNT day:5;
	REBCNT month:4;
	REBCNT year:16;
#else
	REBCNT year:16;
	REBCNT month:4;
	REBCNT day:5;
	REBINT zone:7;  // +/-15:00 res: 0:15
#endif
} REBYMD;

typedef union reb_date {
	REBYMD date;
	REBCNT bits;
} REBDAT;

typedef struct Reb_Time {
	REBI64 time;	// time in nanoseconds
	REBDAT date;
} REBTIM;

#define VAL_TIME(v)	((v)->data.time.time)
#define TIME_SEC(n)	((REBI64)(n) * 1000000000L)

#define MAX_SECONDS	(((i64)1<<31)-1)
#define MAX_HOUR	(MAX_SECONDS / 3600)
#define MAX_TIME	((REBI64)MAX_HOUR * HR_SEC)

#define NANO		1.0e-9
#define	SEC_SEC		((REBI64)1000000000L)
#define MIN_SEC		(60 * SEC_SEC)
#define HR_SEC		(60 * 60 * SEC_SEC)

#define SEC_TIME(n)  ((n) * SEC_SEC)
#define MIN_TIME(n)  ((n) * MIN_SEC)
#define HOUR_TIME(n) ((n) * HR_SEC)

#define SECS_IN(n) ((n) / SEC_SEC)
#define VAL_SECS(n) (VAL_TIME(n) / SEC_SEC)

#define DEC_TO_SECS(n) (i64)(((n) + 5.0e-10) * SEC_SEC)

#define SECS_IN_DAY 86400
#define TIME_IN_DAY (SEC_TIME((i64)SECS_IN_DAY))

#define NO_TIME		MIN_I64

#define	MAX_YEAR		0x3fff

#define VAL_DATE(v)		((v)->data.time.date)
#define VAL_YEAR(v)		((v)->data.time.date.date.year)
#define VAL_MONTH(v)	((v)->data.time.date.date.month)
#define VAL_DAY(v)		((v)->data.time.date.date.day)
#define VAL_ZONE(v)		((v)->data.time.date.date.zone)

#define ZONE_MINS 15
#define ZONE_SECS (ZONE_MINS*60)
#define MAX_ZONE (15 * (60/ZONE_MINS))


/***********************************************************************
**
**	TUPLE
**
***********************************************************************/

typedef struct Reb_Tuple {
	REBYTE tuple[12];
} REBTUP;

#define	VAL_TUPLE(v)	((v)->data.tuple.tuple+1)
#define	VAL_TUPLE_LEN(v) ((v)->data.tuple.tuple[0])
#define MAX_TUPLE 10


/***********************************************************************
**
**	PAIR
**
***********************************************************************/

#define	VAL_PAIR(v)		((v)->data.pair)
#define	VAL_PAIR_X(v)	((v)->data.pair.x)
#define	VAL_PAIR_Y(v) 	((v)->data.pair.y)
#define SET_PAIR(v,x,y)	(VAL_SET(v, REB_PAIR),VAL_PAIR_X(v)=(x),VAL_PAIR_Y(v)=(y))
#define	VAL_PAIR_X_INT(v) ROUND_TO_INT((v)->data.pair.x)
#define	VAL_PAIR_Y_INT(v) ROUND_TO_INT((v)->data.pair.y)


/***********************************************************************
**
**	EVENT
**
***********************************************************************/

#define	VAL_EVENT_TYPE(v)	((v)->data.event.type)  //(VAL_EVENT_INFO(v) & 0xff)
#define	VAL_EVENT_FLAGS(v)	((v)->data.event.flags) //((VAL_EVENT_INFO(v) >> 16) & 0xff)
#define	VAL_EVENT_WIN(v)	((v)->data.event.win)   //((VAL_EVENT_INFO(v) >> 24) & 0xff)
#define	VAL_EVENT_MODEL(v)	((v)->data.event.model)
#define	VAL_EVENT_DATA(v)	((v)->data.event.data)
#define	VAL_EVENT_TIME(v)	((v)->data.event.time)
#define	VAL_EVENT_REQ(v)	((v)->data.event.req)
#define	VAL_EVENT_SER(v)	((v)->data.event.ser)

#define IS_EVENT_MODEL(v,f)	(VAL_EVENT_MODEL(v) == (f))

#define SET_EVENT_INFO(val, type, flags, win) \
	VAL_EVENT_TYPE(val)=type, VAL_EVENT_FLAGS(val)=flags, VAL_EVENT_WIN(val)=win
	//VAL_EVENT_INFO(val) = (type | (flags << 16) | (win << 24))

#define	VAL_EVENT_X(v)		((REBINT) (short) (VAL_EVENT_DATA(v) & 0xffff))
#define	VAL_EVENT_Y(v) 		((REBINT) (short) ((VAL_EVENT_DATA(v) >> 16) & 0xffff))
#define VAL_EVENT_XY(v)		(VAL_EVENT_DATA(v))
#define SET_EVENT_XY(v,x,y) VAL_EVENT_DATA(v) = ((y << 16) | (x & 0xffff))

#define	VAL_EVENT_KEY(v)	(VAL_EVENT_DATA(v) & 0xffff)
#define	VAL_EVENT_KCODE(v)	((VAL_EVENT_DATA(v) >> 16) & 0xffff)
#define SET_EVENT_KEY(v,k,c) VAL_EVENT_DATA(v) = ((c << 16) + k)

#define	IS_KEY_EVENT(type)	0

#ifdef old_code
#define	TO_EVENT_XY(x,y)	(((y)<<16)|((x)&0xffff))
#define	SET_EVENT_INFO(v,t,k,c,w,f)	((VAL_FLAGS(v)=(VAL_FLAGS(v)&0x0f)|((f)&0xf0)),\
									(VAL_EVENT_INFO(v)=(((t)&0xff)|(((k)&0xff)<<8)|\
									(((c)&0xff)<<16)|(((w)&0xff)<<24))))
#endif


/***********************************************************************
**
*/	struct Reb_Series
/*
**		Series header points to data and keeps track of tail and size.
**		Additional fields can be used for attributes and GC. Every
**		string and block in REBOL uses one of these to permit GC
**		and compaction.
**
***********************************************************************/
{
	REBYTE	*data;		// series data head
	REBCNT	tail;		// one past end of useful data
	REBCNT	rest;		// total number of units from bias to end
	REBINT	info;		// holds width and flags
	union {
		REBCNT size;	// used for vectors and bitsets
		REBSER *series;	// MAP datatype uses this
		struct {
			REBCNT wide:16;
			REBCNT high:16;
		} area;
	};
#ifdef SERIES_LABELS
	REBYTE  *label;		// identify the series
#endif
};

#define SERIES_TAIL(s)	 ((s)->tail)
#define SERIES_REST(s)	 ((s)->rest)
#define	SERIES_LEN(s)    ((s)->tail + 1) // Includes terminator
#define	SERIES_FLAGS(s)	 ((s)->info)
#define	SERIES_WIDE(s)	 (((s)->info) & 0xff)
#define SERIES_DATA(s)   ((s)->data)
#define	SERIES_SKIP(s,i) (SERIES_DATA(s) + (SERIES_WIDE(s) * i))

#define END_FLAG 0x80000000  // Indicates end of block as an index (from DO_NEXT)

#ifdef SERIES_LABELS
#define SERIES_LABEL(s)  ((s)->label)
#define SET_SERIES_LABEL(s,l) (((s)->label) = (l))
#else
#define SERIES_LABEL(s)  "-"
#define SET_SERIES_LABEL(s,l)
#endif

// Flag: If wide field is not set, series is free (not used):
#define	SERIES_FREED(s)  (!SERIES_WIDE(s))

// Size in bytes of memory allocated (including bias area):
#define SERIES_TOTAL(s) ((SERIES_REST(s) + SERIES_BIAS(s)) * (REBCNT)SERIES_WIDE(s))
// Size in bytes of series (not including bias area):
#define	SERIES_SPACE(s) (SERIES_REST(s) * (REBCNT)SERIES_WIDE(s))
// Size in bytes being used, including terminator:
#define SERIES_USED(s) (SERIES_LEN(s) * SERIES_WIDE(s))

// Optimized expand when at tail (but, does not reterminate)
#define EXPAND_SERIES_TAIL(s,l) if (SERIES_FITS(s, l)) s->tail += l; else Expand_Series(s, AT_TAIL, l)
#define RESIZE_SERIES(s,l) s->tail = 0; if (!SERIES_FITS(s, l)) Expand_Series(s, AT_TAIL, l); s->tail = 0
#define RESET_SERIES(s) s->tail = 0; TERM_SERIES(s)
#define RESET_TAIL(s) s->tail = 0

// Clear all and clear to tail:
#define CLEAR_SERIES(s) CLEAR(SERIES_DATA(s), SERIES_SPACE(s))
#define ZERO_SERIES(s) memset(SERIES_DATA(s), 0, SERIES_USED(s))
#define TERM_SERIES(s) memset(SERIES_SKIP(s, SERIES_TAIL(s)), 0, SERIES_WIDE(s))

// Returns space that a series has available (less terminator):
#define SERIES_FULL(s) (SERIES_LEN(s) >= SERIES_REST(s))
#define SERIES_AVAIL(s) (SERIES_REST(s) - SERIES_LEN(s))
#define SERIES_FITS(s,n) ((SERIES_TAIL(s) + (REBCNT)(n) + 1) < SERIES_REST(s))

// Flag used for extending series at tail:
#define	AT_TAIL	((REBCNT)(~0))	// Extend series at tail

// Is it a byte-sized series? (this works because no other odd size allowed)
#define BYTE_SIZE(s) (((s)->info) & 1)
#define VAL_BYTE_SIZE(v) (BYTE_SIZE(VAL_SERIES(v)))
#define VAL_STR_IS_ASCII(v) (VAL_BYTE_SIZE(v) && !Is_Not_ASCII(VAL_BIN_DATA(v), VAL_LEN(v)))

// Bias is empty space in front of head:
#define	SERIES_BIAS(s)	   (REBCNT)(SERIES_FLAGS(s) >> 16)
#define MAX_SERIES_BIAS    0x1000
#define SERIES_SET_BIAS(s,b) (SERIES_FLAGS(s) = (SERIES_FLAGS(s) & 0xffff) | (b << 16))
#define SERIES_ADD_BIAS(s,b) (SERIES_FLAGS(s) += (b << 16))
#define SERIES_SUB_BIAS(s,b) (SERIES_FLAGS(s) -= (b << 16))

// Series Flags:
enum {
	SER_MARK = 1,		// Series was found during GC mark scan.
	SER_KEEP = 1<<1,	// Series is permanent, do not GC it.
	SER_LOCK = 1<<2,	// Series is locked, do not expand it
	SER_EXT  = 1<<3,	// Series is external (library), do not GC it.
	SER_FREE = 1<<4,	// mark series as removed
	SER_BARE = 1<<5,	// Series has no links to GC-able values
	SER_PROT = 1<<6,	// Series is protected from modification
	SER_MON  = 1<<7,	// Monitoring
};

#define SERIES_SET_FLAG(s, f) (SERIES_FLAGS(s) |= ((f) << 8))
#define SERIES_CLR_FLAG(s, f) (SERIES_FLAGS(s) &= ~((f) << 8))
#define SERIES_GET_FLAG(s, f) (SERIES_FLAGS(s) &  ((f) << 8))

#define	IS_FREEABLE(s)    !SERIES_GET_FLAG(s, SER_MARK|SER_KEEP|SER_EXT|SER_FREE)
#define MARK_SERIES(s)    SERIES_SET_FLAG(s, SER_MARK)
#define UNMARK_SERIES(s)  SERIES_CLR_FLAG(s, SER_MARK)
#define IS_MARK_SERIES(s) SERIES_GET_FLAG(s, SER_MARK)
#define KEEP_SERIES(s,l)  do {SERIES_SET_FLAG(s, SER_KEEP); LABEL_SERIES(s,l);} while(0)
#define IS_EXT_SERIES(s)  SERIES_GET_FLAG(s, SER_EXT)
#define LOCK_SERIES(s)    SERIES_SET_FLAG(s, SER_LOCK)
#define IS_LOCK_SERIES(s) SERIES_GET_FLAG(s, SER_LOCK)
#define BARE_SERIES(s)    SERIES_SET_FLAG(s, SER_BARE)
#define IS_BARE_SERIES(s) SERIES_GET_FLAG(s, SER_BARE)
#define PROTECT_SERIES(s) SERIES_SET_FLAG(s, SER_PROT)
#define UNPROTECT_SERIES(s)  SERIES_CLR_FLAG(s, SER_PROT)
#define IS_PROTECT_SERIES(s) SERIES_GET_FLAG(s, SER_PROT)

#define TRAP_PROTECT(s) if (IS_PROTECT_SERIES(s)) Trap0(RE_PROTECTED)

#ifdef SERIES_LABELS
#define LABEL_SERIES(s,l) s->label = (l)
#else
#define LABEL_SERIES(s,l)
#endif

#ifdef MEM_STRESS
#define FREE_SERIES(s)    SERIES_SET_FLAG(s, SER_FREE) // mark as removed
#define	CHECK_MARK(s,d) \
		if (SERIES_GET_FLAG(s, SER_FREE)) Choke(); \
		if (!IS_MARK_SERIES(s)) Mark_Series(s, d);
#else
#define FREE_SERIES(s)
#define	CHECK_MARK(s,d) if (!IS_MARK_SERIES(s)) Mark_Series(s, d);
#endif

//#define LABEL_SERIES(s,l) s->label = (l)
#define IS_BLOCK_SERIES(s) (SERIES_WIDE(s) == sizeof(REBVAL))

// !!! Remove if not used after port:
//#define	SERIES_SIDE(s)	 ((s)->link.side)
//#define	SERIES_FRAME(s)	 ((s)->link.frame)
//#define SERIES_NOT_REBOLS(s) SERIES_SET_FLAG(s, SER_XLIB)


/***********************************************************************
**
**	SERIES -- Generic series macros
**
***********************************************************************/

#pragma pack()
#include "reb-gob.h"
#pragma pack(4)

typedef struct Reb_Series_Ref
{
	REBSER	*series;
	REBCNT	index;
	union {
		REBSER	*side;		// lookaside block for lists/hashes/images
		REBINT  back;		// (Used in DO for stack back linking)
//		REBFRM	*frame;		// (may also be used as frame for binding blocks)
	} link;
} REBSRI;

#define VAL_SERIES(v)	    ((v)->data.series.series)
#define VAL_INDEX(v)	    ((v)->data.series.index)
#define	VAL_TAIL(v)		    (VAL_SERIES(v)->tail)
#define VAL_LEN(v)			(Val_Series_Len(v))

#define VAL_DATA(s)			(VAL_BIN_HEAD(s) + (VAL_INDEX(s) * VAL_SERIES_WIDTH(s)))

#define VAL_BACK(v)			((v)->data.series.link.back)
#define	VAL_SERIES_SIDE(v)  ((v)->data.series.link.side)
#define	VAL_SERIES_FRAME(v) ((v)->data.series.link.frame)
#define VAL_SERIES_WIDTH(v) (SERIES_WIDE(VAL_SERIES(v)))
#define VAL_LIMIT_SERIES(v) if (VAL_INDEX(v) > VAL_TAIL(v)) VAL_INDEX(v) = VAL_TAIL(v)

#define DIFF_PTRS(a,b) (REBCNT)((REBYTE*)a - (REBYTE*)b)


/***********************************************************************
**
**	STRINGS -- All string related values
**
***********************************************************************/

#define	SET_STRING(v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, REB_STRING)
#define	SET_BINARY(v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, REB_BINARY)
#define	SET_FILE(v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, REB_FILE)
#define	SET_STR_TYPE(t,v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, t)
#define SET_STR_END(s,n) (*STR_SKIP(s,n) = 0)

// Arg is a binary (byte) series:
#define	BIN_HEAD(s)		((REBYTE *)((s)->data))
#define BIN_DATA(s)		((REBYTE *)((s)->data))
#define	BIN_TAIL(s)		(REBYTE*)STR_TAIL(s)
#define BIN_SKIP(s, n)	(((REBYTE *)((s)->data))+(n))
#define	BIN_LEN(s)		(SERIES_TAIL(s))

// Arg is a unicode series:
#define UNI_HEAD(s)		((REBUNI *)((s)->data))
#define UNI_SKIP(s, n)	(((REBUNI *)((s)->data))+(n))
#define UNI_TAIL(s)		(((REBUNI *)((s)->data))+(s)->tail)
#define	UNI_LAST(s)		(((REBUNI *)((s)->data))+((s)->tail-1)) // make sure tail not zero
#define	UNI_LEN(s)		(SERIES_TAIL(s))
#define UNI_TERM(s)		(*UNI_TAIL(s) = 0)
#define UNI_RESET(s)	(UNI_HEAD(s)[(s)->tail = 0] = 0)

// Obsolete (remove after Unicode conversion):
#define STR_HEAD(s)		((REBYTE *)((s)->data))
#define STR_DATA(s)		((REBYTE *)((s)->data))
#define STR_SKIP(s, n)	(((REBYTE *)((s)->data))+(n))
#define STR_TAIL(s)		(((REBYTE *)((s)->data))+(s)->tail)
#define	STR_LAST(s)		(((REBYTE *)((s)->data))+((s)->tail-1)) // make sure tail not zero
#define	STR_LEN(s)		(SERIES_TAIL(s))
#define STR_TERM(s)		(*STR_TAIL(s) = 0)
#define STR_RESET(s)	(STR_HEAD(s)[(s)->tail = 0] = 0)

// Arg is a binary value:
#define VAL_BIN(v)		BIN_HEAD(VAL_SERIES(v))
#define VAL_BIN_HEAD(v)	BIN_HEAD(VAL_SERIES(v))
#define VAL_BIN_DATA(v)	BIN_SKIP(VAL_SERIES(v), VAL_INDEX(v))
#define VAL_BIN_SKIP(v,n) BIN_SKIP(VAL_SERIES(v), (n))
#define VAL_BIN_TAIL(v)	BIN_SKIP(VAL_SERIES(v), VAL_SERIES(v)->tail)

// Arg is a unicode value:
#define VAL_UNI(v)		UNI_HEAD(VAL_SERIES(v))
#define VAL_UNI_HEAD(v) UNI_HEAD(VAL_SERIES(v))
#define VAL_UNI_DATA(v) UNI_SKIP(VAL_SERIES(v), VAL_INDEX(v))

// Get a char, from either byte or unicode string:
#define GET_ANY_CHAR(s,n)   (REBUNI)(BYTE_SIZE(s) ? BIN_HEAD(s)[n] : UNI_HEAD(s)[n])
#define SET_ANY_CHAR(s,n,c) if BYTE_SIZE(s) BIN_HEAD(s)[n]=((REBYTE)c); else UNI_HEAD(s)[n]=((REBUNI)c)
#define GET_CHAR_UNI(f,p,i) (uni ? ((REBUNI*)p)[i] : ((REBYTE*)bp)[i])

#define VAL_ANY_CHAR(v) GET_ANY_CHAR(VAL_SERIES(v), VAL_INDEX(v))

//#define VAL_STR_LAST(v)	STR_LAST(VAL_SERIES(v))
//#define	VAL_MEM_LEN(v)	(VAL_TAIL(v) * VAL_SERIES_WIDTH(v))


/***********************************************************************
**
**	IMAGES, QUADS - RGBA
**
***********************************************************************/

//typedef struct Reb_ImageInfo
//{
//	REBCNT width;
//	REBCNT height;
//	REBINT transp;
//} REBIMI;

#define QUAD_HEAD(s)	((REBYTE *)((s)->data))
#define QUAD_SKIP(s,n)	(((REBYTE *)((s)->data))+(n * 4))
#define QUAD_TAIL(s)	(((REBYTE *)((s)->data))+((s)->tail * 4))
#define	QUAD_LEN(s)		(SERIES_TAIL(s))

#define	IMG_SIZE(s)		((s)->size)
#define	IMG_WIDE(s)		((s)->area.wide)
#define	IMG_HIGH(s)		((s)->area.high)
#define IMG_DATA(s)		((REBYTE *)((s)->data))

#define VAL_IMAGE_HEAD(v)	QUAD_HEAD(VAL_SERIES(v))
#define VAL_IMAGE_TAIL(v)	QUAD_SKIP(VAL_SERIES(v), VAL_SERIES(v)->tail)
#define VAL_IMAGE_DATA(v)	QUAD_SKIP(VAL_SERIES(v), VAL_INDEX(v))
#define VAL_IMAGE_BITS(v)	((REBCNT *)VAL_IMAGE_HEAD((v)))
#define	VAL_IMAGE_WIDE(v)	(IMG_WIDE(VAL_SERIES(v)))
#define	VAL_IMAGE_HIGH(v)	(IMG_HIGH(VAL_SERIES(v)))
#define	VAL_IMAGE_LEN(v)	VAL_LEN(v)

#define SET_IMAGE(v,s) VAL_SET(v, REB_IMAGE);VAL_SERIES(v)=s;VAL_INDEX(v) = 0;


//#define VAL_IMAGE_TRANSP(v) (VAL_IMAGE_INFO(v)->transp)
//#define VAL_IMAGE_TRANSP_TYPE(v) (VAL_IMAGE_TRANSP(v)&0xff000000)
//#define VITT_UNKNOWN	0x00000000
//#define VITT_NONE		0x01000000
//#define VITT_ALPHA		0x02000000
//#define	VAL_IMAGE_DEPTH(v)	((VAL_IMAGE_INFO(v)>>24)&0x3f)
//#define VAL_IMAGE_TYPE(v)		((VAL_IMAGE_INFO(v)>>30)&3)

// New Image Datatype defines:
#define TO_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (r)<<16 | (g)<<8 |  (b))

#define TO_COLOR_TUPLE(t) TO_COLOR(VAL_TUPLE(t)[0], VAL_TUPLE(t)[1], VAL_TUPLE(t)[2], \
							VAL_TUPLE_LEN(t) > 3 ? VAL_TUPLE(t)[3] : 0)

// Maps color components to correct byte positions for RGBA:
#ifdef ENDIAN_BIG
#define C_A 0
#define C_R 1
#define C_G 2
#define C_B 3
#else
#define C_B 0
#define C_G 1
#define C_R 2
#define C_A 3
#endif


/***********************************************************************
**
**	Logic and Logic Bits
**
***********************************************************************/

#define VAL_LOGIC(v)	((v)->data.logic)
#define	SET_LOGIC(v,n)	VAL_SET(v, REB_LOGIC), VAL_LOGIC(v) = ((n)!=0) //, VAL_LOGIC_WORDS(v)=0
#define SET_TRUE(v)		SET_LOGIC(v, TRUE)  // compound statement
#define SET_FALSE(v)	SET_LOGIC(v, FALSE) // compound statement
#define IS_FALSE(v)		(IS_NONE(v) || (IS_LOGIC(v) && !VAL_LOGIC(v)))
#define	IS_TRUE(v)		(!IS_FALSE(v))
#define VAL_I32(v)		((v)->data.logic)	// used for handles, etc.


/***********************************************************************
**
**	BIT_SET -- Bit sets
**
***********************************************************************/

#define	VAL_BITSET(v)	VAL_SERIES(v)

#define	VAL_BIT_DATA(v)	VAL_BIN(v)

#define	SET_BIT(d,n)	((d)[(n) >> 3] |= (1 << ((n) & 7)))
#define	CLR_BIT(d,n)	((d)[(n) >> 3] &= ~(1 << ((n) & 7)))
#define	IS_BIT(d,n)		((d)[(n) >> 3] & (1 << ((n) & 7)))


/***********************************************************************
**
**	BLOCKS -- Block is a terminated string of values
**
***********************************************************************/

#define NOT_END(v)		(!IS_END(v))

// Arg is a series:
#define BLK_HEAD(s)		((REBVAL *)((s)->data))
#define BLK_SKIP(s, n)	(((REBVAL *)((s)->data))+(n))
#define	BLK_TAIL(s)		(((REBVAL *)((s)->data))+(s)->tail)
#define	BLK_LAST(s)		(((REBVAL *)((s)->data))+((s)->tail-1)) // make sure tail not zero
#define	BLK_LEN(s)		(SERIES_TAIL(s))
#define BLK_TERM(s)		SET_END(BLK_TAIL(s))
#define BLK_RESET(b)	(b)->tail = 0, SET_END(BLK_HEAD(b))

// Arg is a value:
#define VAL_BLK(v)		BLK_HEAD(VAL_SERIES(v))
#define VAL_BLK_DATA(v)	BLK_SKIP(VAL_SERIES(v), VAL_INDEX(v))
#define VAL_BLK_SKIP(v,n)	BLK_SKIP(VAL_SERIES(v), (n))
#define VAL_BLK_TAIL(v)	BLK_SKIP(VAL_SERIES(v), VAL_SERIES(v)->tail)
#define	VAL_BLK_LEN(v)	VAL_LEN(v)
#define VAL_BLK_TERM(v)	BLK_TERM(VAL_SERIES(v))

#define	COPY_VALUES(f,t,l) memcpy(t, f, (l) * sizeof(REBVAL))

#define COPY_BLK_PART(d, s, n) memcpy((d)->data, s, (n) * sizeof(REBVAL)); SERIES_TAIL(d) = n; BLK_TERM(d)

#define IS_EMPTY(v)		(VAL_INDEX(v) >= VAL_TAIL(v))


/***********************************************************************
**
**	LIST & HASH Block Lookaside buffer
**
***********************************************************************/

typedef struct Reb_Side {		// lookaside series
	REBCNT	next;		// next element
	REBCNT	past;		// prior element
} REBLAB;

#define LIST_HEAD(s)	((REBLAB *)((s)->data))
#define LIST_SKIP(s,n)	(((REBLAB *)((s)->data))+(n))

#define	VAL_LIST(v)		LIST_HEAD(VAL_SERIES_SIDE(v))

#define	SET_LIST(v,s,l)	VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SER_LIST(v)=(l), VAL_SET(v, REB_LIST)


/***********************************************************************
**
**	SYMBOLS -- Used only for symbol tables
**
***********************************************************************/

typedef struct Reb_Symbol {
	REBCNT	canon;	// Index of the canonical (first) word
	REBCNT	alias;	// Index to next alias form
	REBCNT	name;	// Index into PG_Word_Names string
} REBSYM;

// Arg is value:
#define VAL_SYM_NINDEX(v)	((v)->data.symbol.name)
#define VAL_SYM_NAME(v)		(STR_HEAD(PG_Word_Names) + VAL_SYM_NINDEX(v))
#define VAL_SYM_CANON(v)	((v)->data.symbol.canon)
#define VAL_SYM_ALIAS(v)	((v)->data.symbol.alias)

// Return the CANON value for a symbol number:
#define SYMBOL_TO_CANON(sym) (VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, sym)))
// Return the CANON value for a word value:
#define WORD_TO_CANON(w) (VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, VAL_WORD_SYM(w))))

#define IS_STAR(v) (IS_WORD(v) && VAL_WORD_CANON(v) == SYM__P)


/***********************************************************************
**
**	WORDS -- All word related types
**
***********************************************************************/

typedef struct Reb_Word {
	REBCNT	sym;		// Index of the word's symbol
	REBINT	index;		// Index of the word in the frame
	REBSER	*frame;		// Frame in which the word is defined
} REBWRD;

typedef struct Reb_Word_Spec {
	REBCNT	sym;		// Index of the word's symbol (and pad for U64 alignment)
	REBU64	typeset;
} REBWRS;

#define IS_SAME_WORD(v, n)		(IS_WORD(v) && VAL_WORD_CANON(v) == n)

#define VAL_WORD_SYM(v)			((v)->data.word.sym)
#define VAL_WORD_INDEX(v)		((v)->data.word.index)
#define VAL_WORD_FRAME(v)		((v)->data.word.frame)
#define HAS_FRAME(v)			VAL_WORD_FRAME(v)

#define	UNBIND(v)				VAL_WORD_FRAME(v)=0, VAL_WORD_INDEX(v)=0

#define VAL_WORD_CANON(v)		VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, VAL_WORD_SYM(v)))
#define	VAL_WORD_NAME(v)		VAL_SYM_NAME(BLK_SKIP(PG_Word_Table.series, VAL_WORD_SYM(v)))
#define	VAL_WORD_NAME_STR(v)	STR_HEAD(VAL_WORD_NAME(v))

// When words are used in frame word lists, fields get a different meaning:
#define	VAL_BIND_SYM(v)			((v)->data.wordspec.sym)
#define VAL_BIND_CANON(v)		VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, VAL_BIND_SYM(v))) //((v)->data.wordspec.index)
#define VAL_BIND_TYPESET(v)		((v)->data.wordspec.typeset)
#define VAL_WORD_FRAME_WORDS(v) VAL_WORD_FRAME(v)->words
#define VAL_WORD_FRAME_VALUES(v) VAL_WORD_FRAME(v)->values

// Is it the same symbol? Quick check, then canon check:
#define SAME_SYM(a,b) (VAL_WORD_SYM(a)==VAL_BIND_SYM(b)||VAL_WORD_CANON(a)==VAL_BIND_CANON(b))

/***********************************************************************
**
**	Frame -- Used to bind words to values.
**
**		This type of value is used at the head of a frame block.
**		It should appear in no other place.
**
***********************************************************************/

typedef struct Reb_Frame {
	REBSER	*words;
	REBSER	*spec;
//	REBSER	*parent;
} REBFRM;

// Value to frame fields:
#define	VAL_FRM_WORDS(v)	((v)->data.frame.words)
#define	VAL_FRM_SPEC(v)		((v)->data.frame.spec)
//#define	VAL_FRM_PARENT(v)	((v)->data.frame.parent)

// Word number array (used by Bind_Table):
#define WORDS_HEAD(w)		((REBINT *)(w)->data)
#define WORDS_LAST(w)		(((REBINT *)(w)->data)+(w)->tail-1) // (tail never zero)

// Frame series to frame components:
#define FRM_WORD_SERIES(c)	VAL_FRM_WORDS(BLK_HEAD(c))
#define FRM_WORDS(c)		BLK_HEAD(FRM_WORD_SERIES(c))
#define FRM_VALUES(c)		BLK_HEAD(c)
#define FRM_VALUE(c,n)		BLK_SKIP(c,(n))
#define FRM_WORD(c,n)		BLK_SKIP(FRM_WORD_SERIES(c),(n))
#define FRM_WORD_SYM(c,n)	VAL_BIND_SYM(FRM_WORD(c,n))

#define VAL_FRM_WORD(v,n)	BLK_SKIP(FRM_WORD_SERIES(VAL_SERIES(v)),(n))

// Object field (series, index):
#define OFV(s,n)			BLK_SKIP(s,n)

#define SET_FRAME(v, s, w) \
	VAL_FRM_SPEC(v) = (s); \
	VAL_FRM_WORDS(v) = (w); \
	VAL_SET(v, REB_FRAME)

#define SET_SELFLESS(f) VAL_BIND_SYM(FRM_WORDS(f)) = 0
#define IS_SELFLESS(f) (!VAL_BIND_SYM(FRM_WORDS(f)))

/***********************************************************************
**
**	OBJECTS - Object Support
**
***********************************************************************/

typedef struct Reb_Object {
	REBSER	*frame;
	REBSER	*body;		// module body
//	REBSER	*spec;
//	REBCNT	num;		// shortcut for checking error number
} REBOBJ;

//#define	SET_OBJECT(v,s,f) VAL_OBJ_SPEC(v) = (s), VAL_OBJ_FRAME(v) = (f), VAL_SET(v, REB_OBJECT)
#define	SET_OBJECT(v,f)		VAL_OBJ_FRAME(v) = (f), VAL_SET(v, REB_OBJECT)
#define	SET_MODULE(v,f)		VAL_OBJ_FRAME(v) = (f), VAL_SET(v, REB_MODULE)

#define VAL_OBJ_FRAME(v)	((v)->data.object.frame)
#define VAL_OBJ_VALUES(v)	FRM_VALUES((v)->data.object.frame)
#define VAL_OBJ_VALUE(v,n)	FRM_VALUE((v)->data.object.frame, n)
#define VAL_OBJ_WORDS(v)	FRM_WORD_SERIES((v)->data.object.frame)
#define VAL_OBJ_WORD(v,n)	BLK_SKIP(VAL_OBJ_WORDS(v), (n))
//#define VAL_OBJ_SPEC(v)		((v)->data.object.spec)
#define	CLONE_OBJECT(c)		Copy_Series(c)

#define	VAL_MOD_FRAME(v)	((v)->data.object.frame)
#define VAL_MOD_BODY(v)		((v)->data.object.body)
#define VAL_MOD_SPEC(v)		VAL_FRM_SPEC(VAL_OBJ_VALUES(v))

#define SET_HANDLE(v,h)		VAL_SET(v, REB_HANDLE), VAL_HANDLE(v) = (void*)(h) // a place to put it.

/***********************************************************************
**
**	PORTS - External series interface
**
***********************************************************************/

#define	VAL_PORT(v)			VAL_OBJ_FRAME(v)
#define SET_PORT(v,s)		VAL_SET(v, REB_PORT), VAL_PORT(v) = s


/***********************************************************************
**
**	ERRORS - Error values
**
***********************************************************************/

typedef struct Reb_Error {
	union Reo {
		REBSER	*object;
		REBVAL	*value;		// RETURN value (also BREAK, THROW)
	} reo;
	REBCNT	num;			// (Determines value used below.)
	REBCNT  sym;			// THROW symbol
} REBERR;

// Value Accessors:
#define	VAL_ERR_NUM(v)		((v)->data.error.num)
#define VAL_ERR_OBJECT(v)	((v)->data.error.reo.object)
#define VAL_ERR_VALUE(v)	((v)->data.error.reo.value)
#define VAL_ERR_SYM(v)		((v)->data.error.sym)

#define	IS_THROW(v)			(VAL_ERR_NUM(v) < RE_THROW_MAX)
#define	IS_BREAK(v)			(VAL_ERR_NUM(v) == RE_BREAK)
#define	IS_RETURN(v)		(VAL_ERR_NUM(v) == RE_RETURN)
#define	IS_CONTINUE(v)		(VAL_ERR_NUM(v) == RE_CONTINUE)
#define THROWN(v)			(IS_ERROR(v) && IS_THROW(v))

#define	SET_ERROR(v,n,a)	VAL_SET(v, REB_ERROR), VAL_ERR_NUM(v)=n, VAL_ERR_OBJECT(v)=a, VAL_ERR_SYM(v)=0
#define	SET_THROW(v,n,a)	VAL_SET(v, REB_ERROR), VAL_ERR_NUM(v)=n, VAL_ERR_VALUE(v)=a, VAL_ERR_SYM(v)=0

#define VAL_ERR_VALUES(v)	((ERROR_OBJ *)(FRM_VALUES(VAL_ERR_OBJECT(v))))
#define	VAL_ERR_ARG1(v)		(&VAL_ERR_VALUES(v)->arg1)
#define	VAL_ERR_ARG2(v)		(&VAL_ERR_VALUES(v)->arg2)

// Error Object (frame) Accessors:
#define ERR_VALUES(frame)	((ERROR_OBJ *)FRM_VALUES(frame))
#define	ERR_NUM(frame)		VAL_INT32(&ERR_VALUES(frame)->code)


/***********************************************************************
**
**	GOBS - Graphic Objects
**
***********************************************************************/

typedef struct Reb_Gob {
	REBGOB *gob;
	REBCNT index;
} REBGBO;

#define	VAL_GOB(v)			((v)->data.gob.gob)
#define	VAL_GOB_INDEX(v)	((v)->data.gob.index)
#define SET_GOB(v,g)		VAL_SET(v, REB_GOB), VAL_GOB(v)=g, VAL_GOB_INDEX(v)=0


/***********************************************************************
**
**	FUNCTIONS - Natives, actions, operators, and user functions
**
***********************************************************************/

typedef int  (*REBFUN)(REBVAL *ds);				// Native function
typedef int  (*REBACT)(REBVAL *ds, REBCNT a);	// Action function
typedef void (*REBDOF)(REBVAL *ds);				// DO evaltype dispatch function
typedef int  (*REBPAF)(REBVAL *ds, REBSER *p, REBCNT a); // Port action func

typedef void (*ANYFUNC)(void *);
typedef void (*TRYFUNC)(void *);
typedef int  (*CMD_FUNC)(REBCNT n, REBSER *args);

#define REBNATIVE(n) int N_##n(REBVAL *ds)
#define REBTYPE(n)   int T_##n(REBVAL *ds, REBCNT action)
#define REBPACT(n)   int P_##n(REBVAL *ds)

typedef struct Reb_Function {
	REBSER	*spec;	// Spec block for function
	REBSER	*args;	// Block of Wordspecs (with typesets)
	union Reb_Func_Code {
		REBFUN	code;
		REBSER	*body;
		REBCNT	act;
	} func;
} REBFCN;

#define VAL_FUNC_SPEC(v)	  ((v)->data.func.spec)	// a series
#define VAL_FUNC_SPEC_BLK(v)  BLK_HEAD((v)->data.func.spec)
#define VAL_FUNC_ARGS(v)	  ((v)->data.func.args)
#define VAL_FUNC_WORDS(v)     VAL_FUNC_ARGS(v)
#define VAL_FUNC_CODE(v)	  ((v)->data.func.func.code)
#define VAL_FUNC_BODY(v)	  ((v)->data.func.func.body)
#define VAL_FUNC_ACT(v)       ((v)->data.func.func.act)
#define VAL_FUNC_ARGC(v)	  SERIES_TAIL((v)->data.func.args)

typedef struct Reb_Path_Value {
	REBVAL *value;	// modified
	REBVAL *select;	// modified
	REBVAL *path;	// modified
	REBVAL *store;  // modified (holds constructed values)
	REBVAL *setval;	// static
	REBVAL *orig;	// static
} REBPVS;

enum Path_Eval_Result {
	PE_OK,
	PE_SET,
	PE_USE,
	PE_NONE,
	PE_BAD_SELECT,
	PE_BAD_SET,
	PE_BAD_RANGE,
	PE_BAD_SET_TYPE
};

typedef REBINT (*REBPEF)(REBPVS *pvs); // Path evaluator function

typedef REBINT (*REBCTF)(REBVAL *a, REBVAL *b, REBINT s);

/***********************************************************************
**
**	HANDLE
**
***********************************************************************/

typedef struct Reb_Handle {
	ANYFUNC	code;
} REBHAN;

#define VAL_HANDLE(v)		((v)->data.handle.code)

/***********************************************************************
**
**	LIBRARY -- External library management structures
**
***********************************************************************/

typedef struct Reb_Library {
	long handle;        // ALPHA wants a long
	REBSER *name;
	REBCNT id;
} REBLIB;

#define VAL_LIBRARY(v)        (v->data.library)
#define VAL_LIBRARY_HANDLE(v) (v->data.library.handle)
#define VAL_LIBRARY_NAME(v)   (v->data.library.name)
#define VAL_LIBRARY_ID(v)     (v->data.library.id)


/***********************************************************************
**
**	ROUTINE -- External library routine structures
**
***********************************************************************/

typedef struct Reb_Routine {
	FUNCPTR funcptr;
	REBSER  *spec; // struct-ptr
	REBCNT  id;
} REBROT;

typedef struct Reb_Rot_Info {
	REBCNT call_idx;
	REBCNT pad1;
	REBCNT pad2;
} REBFRO;

#define VAL_ROUTINE(v)          (v->data.routine)
#define VAL_ROUTINE_FUNCPTR(v)  (v->data.routine.funcptr)
#define VAL_ROUTINE_SPEC_SER(v) (v->data.routine.spec)
#define VAL_ROUTINE_SPEC(v)     ((REBVAL *) (((REBFRO *)BLK_HEAD(VAL_ROUTINE_SPEC_SER(v))) + 1))
#define VAL_ROUTINE_INFO(v)	((REBFRO *) (((REBFRO *)BLK_HEAD(VAL_ROUTINE_SPEC_SER(v)))))
#define VAL_ROUTINE_ID(v)       (v->data.routine.id)

#define RFRO_CALLIDX(i) ((i)->call_idx)

typedef struct Reb_Typeset {
	REBCNT  pad;	// Allows us to overlay this type on WORD spec type
	REBU64  bits;
} REBTYS;

#define VAL_TYPESET(v)  ((v)->data.typeset.bits)
#define TYPE_CHECK(v,n) ((VAL_TYPESET(v) & ((REBU64)1 << (n))) != (REBU64)0)
#define TYPE_SET(v,n)   (VAL_TYPESET(v) |= ((REBU64)1 << (n)))
#define EQUAL_TYPESET(v,w) (VAL_TYPESET(v) == VAL_TYPESET(w))
#define TYPESET(n) ((REBU64)1 << (n))

//#define TYPE_CHECK(v,n) ((VAL_TYPESET(v)[(n)/32] & (1 << ((n)%32))) != 0)
//#define TYPE_SET(v,n)   (VAL_TYPESET(v)[(n)/32] |= (1 << ((n)%32)))
//#define EQUAL_TYPESET(v,n) (VAL_TYPESET(v)[0] == VAL_TYPESET(n)[0] && VAL_TYPESET(v)[1] == VAL_TYPESET(n)[1])

/***********************************************************************
**
**	STRUCT -- C Structures
**
***********************************************************************/

typedef struct Reb_Struct {
	REBSER	*spec;
	REBSER	*vals;
	REBSER	*data;
} REBSTU;

#define VAL_STRUCT(v)       (v->data.structure)
#define VAL_STRUCT_SPEC(v)  (v->data.structure.spec)
#define VAL_STRUCT_VALS(v)  (v->data.structure.vals)
#define VAL_STRUCT_DATA(v)  (v->data.structure.data)
#define VAL_STRUCT_DP(v)    (STR_HEAD(VAL_STRUCT_DATA(v)))
#define VAL_STRUCT_LEN(v)   (SERIES_TAIL(VAL_STRUCT_DATA(v)))

/***********************************************************************
**
**	UTYPE - User defined types
**
***********************************************************************/

typedef struct Reb_Utype {
	REBSER	*func;	// func object
	REBSER	*data;	// data object
} REBUDT;

#define VAL_UTYPE_FUNC(v)	((v)->data.utype.func)
#define VAL_UTYPE_DATA(v)	((v)->data.utype.data)

// All bits of value fields:
typedef struct Reb_All {
	REBCNT bits[3];
} REBALL;

#define VAL_ALL_BITS(v) ((v)->data.all.bits)


/***********************************************************************
**
*/	struct Reb_Value
/*
**		The structure/union for all REBOL values. Most efficient
**		if it fits into 16 bytes of memory (but not required).
**
***********************************************************************/
{
	union Reb_Val_Head {
		REBHED flags;
		REBCNT header;
	} flags;
	union Reb_Val_Data {
		REBWRD	word;
		REBSRI	series;
		REBCNT  logic;
		REBI64	integer;
		REBU64	unteger;
		REBDEC	decimal;
		REBUNI  uchar;
		REBERR	error;
		REBTYP	datatype;
		REBFRM	frame;
		REBWRS	wordspec;
		REBTYS  typeset;
		REBSYM	symbol;
		REBTIM	time;
		REBTUP	tuple;
		REBFCN	func;
		REBOBJ	object;
		REBXYF	pair;
		REBEVT	event;
		REBLIB  library;
		REBROT  routine;
		REBSTU  structure;
		REBGBO	gob;
		REBUDT  utype;
		REBDCI  deci;
		REBHAN  handle;
		REBALL  all;
	} data;
};

#define ANY_SERIES(v)		(VAL_TYPE(v) >= REB_BINARY && VAL_TYPE(v) <= REB_LIT_PATH)
#define ANY_STR(v)			(VAL_TYPE(v) >= REB_STRING && VAL_TYPE(v) <= REB_TAG)
#define ANY_BINSTR(v)		(VAL_TYPE(v) >= REB_BINARY && VAL_TYPE(v) <= REB_TAG)
#define ANY_BLOCK(v)		(VAL_TYPE(v) >= REB_BLOCK  && VAL_TYPE(v) <= REB_LIT_PATH)
#define	ANY_WORD(v)			(VAL_TYPE(v) >= REB_WORD   && VAL_TYPE(v) <= REB_ISSUE)
#define	ANY_PATH(v)			(VAL_TYPE(v) >= REB_PATH   && VAL_TYPE(v) <= REB_LIT_PATH)
#define ANY_FUNC(v)			(VAL_TYPE(v) >= REB_NATIVE && VAL_TYPE(v) <= REB_FUNCTION)
#define ANY_EVAL_BLOCK(v)	(VAL_TYPE(v) >= REB_BLOCK  && VAL_TYPE(v) <= REB_PAREN)
#define ANY_OBJECT(v)		(VAL_TYPE(v) >= REB_OBJECT && VAL_TYPE(v) <= REB_PORT)

#pragma pack()

#endif // value.h

