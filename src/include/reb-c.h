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
**  Summary: General C definitions and constants
**  Module:  reb-c.h
**  Author:  Carl Sassenrath, Ladislav Mecir
**  Notes:
**      Various configuration defines (from reb-config.h):
**
**      HAS_LL_CONSTS - compiler allows 1234LL constants
**      ODD_INT_64 - old MSVC typedef for 64 bit int
**      OS_WIDE_CHAR - the OS uses wide chars (not UTF-8)
**
***********************************************************************/

#ifndef FALSE
#define FALSE 0
#define TRUE (!0)
#endif

/***********************************************************************
**
**  C-Code Types
**
**      One of the biggest flaws in the C language was not
**      to indicate bitranges of integers. So, we do that here.
**      You cannot "abstractly remove" the range of a number.
**      It is a critical part of its definition.
**
***********************************************************************/

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* C-code types: use C99 */

#include <stdint.h>

typedef int8_t			i8;
typedef uint8_t			u8;
typedef int16_t			i16;
typedef uint16_t		u16;
typedef int32_t			i32;
typedef uint32_t		u32;
typedef int64_t			i64;
typedef uint64_t		u64;
typedef intptr_t		REBIPT;		// integral counterpart of void*
typedef uintptr_t		REBUPT;		// unsigned counterpart of void*

#define MAX_I32 INT32_MAX
#define MIN_I32 INT32_MIN
#define MAX_I64 INT64_MAX
#define MIN_I64 INT64_MIN

#else
/* C-code types: C99 definitions unavailable, do it ourselves */

typedef char			i8;
typedef unsigned char	u8;
typedef short			i16;
typedef unsigned short	u16;
#ifdef __LP64__
typedef int				i32;
typedef unsigned int	u32;
#else
typedef long				i32;
typedef unsigned long	u32;
 #endif
#ifdef ODD_INT_64       // Windows VC6 nonstandard typing for 64 bits
typedef _int64          i64;
typedef unsigned _int64 u64;
#else
typedef long long       i64;
typedef unsigned long long u64;
#endif
#ifdef __LLP64__
typedef long long		REBIPT;		// integral counterpart of void*
typedef unsigned long long	REBUPT;		// unsigned counterpart of void*
#else
typedef long			REBIPT;		// integral counterpart of void*
typedef unsigned long	REBUPT;		// unsigned counterpart of void*
#endif

#define MAX_I32 ((i32)0x7fffffff)
#define MIN_I32 ((i32)0x80000000)
#ifdef HAS_LL_CONSTS
#define MAX_I64 ((i64)0x7fffffffffffffffLL)
#define MIN_I64 ((i64)0x8000000000000000LL)
#else
#define MAX_I64 ((i64)0x7fffffffffffffffI64)
#define MIN_I64 ((i64)0x8000000000000000I64)
#endif

#endif
/* C-code types */

#ifndef DEF_UINT		// some systems define it, don't define it again
typedef unsigned int    uint;
#endif

// Some systems define a cpu-optimal BOOL already. It is assumed that the
// R3 lib will use that same definition (so sizeof() is identical.)
// (Of course, all of this should have been built into C in 1970.)
#ifndef HAS_BOOL
typedef int BOOL;       // (int is used for speed in modern CPUs)
#endif

// Used for cases where we need 64 bits, even in 32 bit mode.
// (Note: compatible with FILETIME used in Windows)
#pragma pack(4)
typedef struct sInt64 {
	i32 l;
	i32 h;
} I64;
#pragma pack()

/***********************************************************************
**
**  REBOL Code Types
**
***********************************************************************/

typedef i32				REBINT;     // 32 bit (64 bit defined below)
typedef u32				REBCNT;     // 32 bit (counting number)
typedef i64				REBI64;     // 64 bit integer
typedef u64				REBU64;     // 64 bit unsigned integer
typedef i8				REBOOL;     // 8  bit flag (for struct usage)
typedef u32				REBFLG;     // 32 bit flag (for cpu efficiency)
typedef float			REBD32;     // 32 bit decimal
typedef double			REBDEC;     // 64 bit decimal

typedef unsigned char	REBYTE;     // unsigned byte data
typedef u16				REBUNI;     // unicode char

// REBCHR - only to refer to OS char strings (not internal strings)
#ifdef OS_WIDE_CHAR
typedef REBUNI          REBCHR;
#else
typedef REBYTE          REBCHR;
#endif

#define MAX_UNI ((1 << (8*sizeof(REBUNI))) - 1)

#define MIN_D64 ((double)-9.2233720368547758e18)
#define MAX_D64 ((double) 9.2233720368547758e18)

// Useful char constants:
enum {
	BEL =   7,
	BS  =   8,
	LF  =  10,
	CR  =  13,
	ESC =  27,
	DEL = 127
};

// Used for MOLDing:
#define MAX_DIGITS 17   // number of digits
#define MAX_NUMCHR 32   // space for digits and -.e+000%

/***********************************************************************
**
**  64 Bit Integers - Now supported in REBOL 3.0
**
***********************************************************************/

#define MAX_INT_LEN     20
#define MAX_HEX_LEN     16

#ifdef ITOA64           // Integer to ascii conversion
#define INT_TO_STR(n,s) _i64toa(n, s, 10)
#else
#define INT_TO_STR(n,s) Form_Int_Len(s, n, MAX_INT_LEN)
#endif

#ifdef ATOI64           // Ascii to integer conversion
#define CHR_TO_INT(s)   _atoi64(s)
#else
#define CHR_TO_INT(s)   strtoll(s, 0, 10)
#endif

#define LDIV            lldiv
#define LDIV_T          lldiv_t

/***********************************************************************
**
**  Address and Function Pointers
**
***********************************************************************/

#ifdef TO_WIN32
typedef long (__stdcall *FUNCPTR)();
typedef void(__cdecl *CFUNC)(void *);
#else
typedef long (*FUNCPTR)();
typedef void(*CFUNC)(void *);
#endif

/***********************************************************************
**
**  Useful Macros
**
***********************************************************************/

#define FLAGIT(f)           (1<<(f))
#define GET_FLAG(v,f)       (((v) & (1<<(f))) != 0)
#define GET_FLAGS(v,f,g)    (((v) & ((1<<(f)) | (1<<(g)))) != 0)
#define SET_FLAG(v,f)       ((v) |= (1<<(f)))
#define CLR_FLAG(v,f)       ((v) &= ~(1<<(f)))
#define CLR_FLAGS(v,f,g)    ((v) &= ~((1<<(f)) | (1<<(g))))

#ifdef min
#define MIN(a,b) min(a,b)
#define MAX(a,b) max(a,b)
#else
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

// Memory related functions:
#define MAKE_MEM(n)     malloc(n)
#define MAKE_NEW(s)     malloc(sizeof(s))
#define FREE_MEM(m)     free(m)
#define CLEAR(m, s)     memset((void*)(m), 0, s);
#define CLEARS(m)       memset((void*)(m), 0, sizeof(*m));
#define COPY_MEM(t,f,l) memcpy((void*)(t), (void*)(f), l)
#define MOVE_MEM(t,f,l) memmove((void*)(t), (void*)(f), l)

// Byte string functions:
#define COPY_BYTES(t,f,l)   strncpy((char*)t, (char*)f, l)
// For APPEND_BYTES, l is the max-size allocated for t (dest)
#define APPEND_BYTES(t,f,l) strncat((char*)t, (char*)f, MAX((l)-strlen(t)-1, 0))
#define LEN_BYTES(s)        strlen((char*)s)
#define CMP_BYTES(s,t)      strcmp((char*)s, (char*)t)
#define BYTES(s) (REBYTE*)(s)

// OS has wide char string interfaces:
#ifdef OS_WIDE_CHAR
#define OS_WIDE TRUE
#define TXT(s) (L##s)
#define COPY_STR(t,f,l) wcsncpy(t, f, l)
#define JOIN_STR(d,s,l) wcsncat(d,s,l)
#define FIND_STR(d,s)   wcsstr(d,s)
#define FIND_CHR(d,s)   wcschr(d,s)
#define LEN_STR(s)      wcslen(s)
#define TO_OS_STR(s1,s2,l)   mbstowcs(s1,s2,l)
#define FROM_OS_STR(s1,s2,l) wcstombs(s1,s2,l)
#else
// OS has UTF-8 byte string interfaces:
#define OS_WIDE FALSE
#define TXT(s) (s)
#define COPY_STR(t,f,l) strncpy(t, f, l)
#define JOIN_STR(d,s,l) strncat(d,s,l)
#define FIND_STR(d,s)   strstr(d,s)
#define FIND_CHR(d,s)   strchr(d,s)
#define LEN_STR(s)      strlen(s)
#define TO_OS_STR(s1,s2,l)   strncpy(s1,s2,l)
#define FROM_OS_STR(s1,s2,l) strncpy(s1,s2,l)
#endif

#define MAKE_STR(n) (REBCHR*)(malloc((n) * sizeof(REBCHR)))  // OS chars!

#define ROUND_TO_INT(d) (REBINT)(floor((d) + 0.5))
