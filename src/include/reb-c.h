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
#ifndef REB_C_H
#define REB_C_H

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

#if defined(__cplusplus) && __cplusplus >= 201103L
#include <type_traits> // used in CASTING MACROS
#endif

#ifdef __OBJC__
#define HAS_BOOL // don't redefine BOOL in objective-c code
#endif

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

#define I8_C(c)			INT8_C(c)
#define U8_C(c)			UINT8_C(c)

#define I16_C(c)		INT16_C(c)
#define U16_C(c)		UINT16_C(c)

#define I32_C(c)		INT32_C(c)
#define U32_C(c)		UINT32_C(c)

#define I64_C(c)		INT64_C(c)
#define U64_C(c)		UINT64_C(c)

#else
/* C-code types: C99 definitions unavailable, do it ourselves */

typedef char			i8;
typedef unsigned char	u8;
#define I8(c) 			c
#define U8(c) 			c

typedef short			i16;
typedef unsigned short	u16;
#define I16(c) 			c
#define U16(c) 			c

#ifdef __LP64__
typedef int				i32;
typedef unsigned int	u32;
#else
typedef long            i32;
typedef unsigned long	u32;
#endif
#define I32_C(c) c
#define U32_C(c) c ## U

#ifdef ODD_INT_64       // Windows VC6 nonstandard typing for 64 bits
typedef _int64          i64;
typedef unsigned _int64 u64;
#define I64_C(c) c ## I64
#define U64_C(c) c ## U64
#else
typedef long long       i64;
typedef unsigned long long u64;
#define I64_C(c) c ## LL
#define U64_C(c) c ## ULL
#endif
#ifdef __LLP64__
typedef long long		REBIPT;		// integral counterpart of void*
typedef unsigned long long	REBUPT;		// unsigned counterpart of void*
#else
typedef long			REBIPT;		// integral counterpart of void*
typedef unsigned long	REBUPT;		// unsigned counterpart of void*
#endif

#define MAX_I32 I32_C(0x7fffffff)
#define MIN_I32 ((i32)I32_C(0x80000000)) //compiler treats the hex literal as unsigned without casting
#define MAX_I64 I64_C(0x7fffffffffffffff)
#define MIN_I64 ((i64)I64_C(0x8000000000000000)) //compiler treats the hex literal as unsigned without casting

#endif
/* C-code types */

#define MAX_U32 U32_C(0xffffffff)
#define MAX_U64 U64_C(0xffffffffffffffff)

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

#define MAX_INT_LEN     21
#define MAX_HEX_LEN     16

#ifdef ITOA64           // Integer to ascii conversion
#define INT_TO_STR(n,s) _i64toa(n, s_cast(s), 10)
#else
#define INT_TO_STR(n,s) Form_Int_Len(s, n, MAX_INT_LEN)
#endif

#ifdef ATOI64           // Ascii to integer conversion
#define CHR_TO_INT(s)   _atoi64(cs_cast(s))
#else
#define CHR_TO_INT(s)   strtoll(cs_cast(s), 0, 10)
#endif

#define LDIV            lldiv
#define LDIV_T          lldiv_t

/***********************************************************************
**
**  Address and Function Pointers
**
***********************************************************************/

#ifdef TO_WINDOWS
typedef long (__stdcall *FUNCPTR)();
typedef void(__cdecl *CFUNC)(void *);
#else
typedef long (*FUNCPTR)(void);
typedef void(*CFUNC)(void *);
#endif

/***********************************************************************
**
**  Useful Macros
**
***********************************************************************/

#define UNUSED(x) (void)x;

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
#define MAKE_NEW(s)     MAKE_MEM(sizeof(s))
#define FREE_MEM(m)     free(m)
#define CLEAR(m, s)     memset((void*)(m), 0, s);
#define CLEAR_WTH(m, s, v)     memset((void*)(m), v, s);
#define CLEARS(m)       memset((void*)(m), 0, sizeof(*m));
#define COPY_MEM(t,f,l) memcpy((void*)(t), (void*)(f), l)
#define MOVE_MEM(t,f,l) memmove((void*)(t), (void*)(f), l)

// OS has wide char string interfaces:
#ifdef OS_WIDE_CHAR
#define OS_WIDE TRUE
#define TXT(s) (L##s)
#define COPY_STR(t,f,l) wcsncpy((wchar_t*)t,(const wchar_t*)f, l)
#define JOIN_STR(d,s,l) wcsncat((wchar_t*)d,(const wchar_t*)s,l)
#define FIND_STR(d,s)   (REBCHR*)wcsstr((const wchar_t*)d,s)
#define FIND_CHR(d,s)   (REBCHR*)wcschr((const wchar_t*)d,s)
#define LEN_STR(s)      wcslen((const wchar_t*)s)
#define TO_OS_STR(s1,s2,l)   mbstowcs(s1,s2,l)
#define FROM_OS_STR(s1,s2,l) wcstombs(s1,(const wchar_t*)s2,l)
#else
// OS has UTF-8 byte string interfaces:
#define OS_WIDE FALSE
#define TXT(s) (s)
#define COPY_STR(t,f,l) strncpy((char *)t, (const char *)f, l)
#define JOIN_STR(d,s,l) strncat((char *)d,(const char *)s,l)
#define FIND_STR(d,s)   strstr((const char*)d,s)
#define FIND_CHR(d,s)   (REBCHR*)strchr((const char*)d,s)
#define LEN_STR(s)      strlen((const char*)s)
#define TO_OS_STR(s1,s2,l)   strncpy(s1,s2,l)
#define FROM_OS_STR(s1,s2,l) strncpy(s1,(const char*)s2,l)
#endif

#define MAKE_STR(n) (REBCHR*)(MAKE_MEM((n) * sizeof(REBCHR)))  // OS chars!

#define ROUND_TO_INT(d) (REBINT)(floor((d) + 0.5))

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
// Note: 32bit, v must be > 0
#define U32_ROUND_UP_POWER_OF_2(v) \
                    v--;           \
                    v |= v >> 1;   \
                    v |= v >> 2;   \
                    v |= v >> 4;   \
                    v |= v >> 8;   \
                    v |= v >> 16;  \
                    v++;           \

/***********************************************************************
**
**  Color Macros
**
***********************************************************************/
//global pixelformat setup for REBOL image!, image loaders, color handling, tuple! conversions etc.
//the graphics compositor code should rely on this setting(and do specific conversions if needed)
//notes:
//TO_RGBA_COLOR always returns 32bit RGBA value, converts R,G,B,A components to native RGBA order
//TO_PIXEL_COLOR must match internal image! datatype byte order, converts R,G,B,A components to native image format
// C_R, C_G, C_B, C_A Maps color components to correct byte positions for image! datatype byte order

#ifdef ENDIAN_BIG

#define TO_RGBA_COLOR(r,g,b,a) (REBCNT)((r)<<24 | (g)<<16 | (b)<<8 |  (a))

//ARGB pixelformat used on big endian systems
#define C_A 0
#define C_R 1
#define C_G 2
#define C_B 3

#define TO_PIXEL_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (r)<<16 | (g)<<8 |  (b))

#else

#define TO_RGBA_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (b)<<16 | (g)<<8 |  (r))

//we use RGBA pixelformat on Android
#ifdef TO_ANDROID_ARM
#define C_R 0
#define C_G 1
#define C_B 2
#define C_A 3
#define TO_PIXEL_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (b)<<16 | (g)<<8 |  (r))
#else
//BGRA pixelformat is used on Windows
#define C_B 0
#define C_G 1
#define C_R 2
#define C_A 3
#define TO_PIXEL_COLOR(r,g,b,a) (REBCNT)((a)<<24 | (r)<<16 | (g)<<8 |  (b))
#endif

#endif


/***********************************************************************
**
**  Conversion Macros
**
***********************************************************************/

/* interpret four 8 bit unsigned integers as a 32 bit unsigned integer in little endian
   originaly named LE in ChaCha20 code and U8TO32 in Poly1305 code */
#define U8TO32_LE(p)                  \
    (((u32)((p)[0]))       |          \
     ((u32)((p)[1]) << 8)  |          \
     ((u32)((p)[2]) << 16) |          \
     ((u32)((p)[3]) << 24))

/* store a 32 bit unsigned integer as four 8 bit unsigned integers in little endian
   originaly named FROMLE in ChaCha20 code */
#define U32TO8_LE(p, v)               \
  do {                                \
    (p)[0] = (u8)((v));               \
    (p)[1] = (u8)((v) >> 8);          \
    (p)[2] = (u8)((v) >> 16);         \
    (p)[3] = (u8)((v) >> 24);         \
  } while (0)

/* interpret eight 8 bit unsigned integers as a 64 bit unsigned integer in little endian */
#define U8TO64_LE(p)                  \
    (((u64)((p)[0] & 0xff)      ) |   \
     ((u64)((p)[1] & 0xff) <<  8) |   \
     ((u64)((p)[2] & 0xff) << 16) |   \
     ((u64)((p)[3] & 0xff) << 24) |   \
     ((u64)((p)[4] & 0xff) << 32) |   \
     ((u64)((p)[5] & 0xff) << 40) |   \
     ((u64)((p)[6] & 0xff) << 48) |   \
     ((u64)((p)[7] & 0xff) << 56))


/* store a 64 bit unsigned integer as eight 8 bit unsigned integers in little endian */
#define U64TO8_LE(p, v)               \
  do {                                \
    (p)[0] = (u8)((v)      ) & 0xff;  \
    (p)[1] = (u8)((v) >>  8) & 0xff;  \
    (p)[2] = (u8)((v) >> 16) & 0xff;  \
    (p)[3] = (u8)((v) >> 24) & 0xff;  \
    (p)[4] = (u8)((v) >> 32) & 0xff;  \
    (p)[5] = (u8)((v) >> 40) & 0xff;  \
    (p)[6] = (u8)((v) >> 48) & 0xff;  \
    (p)[7] = (u8)((v) >> 56) & 0xff;  \
  } while (0)



//
// CASTING MACROS
//
// The following code and explanation is from "Casts for the Masses (in C)":
//
// http://blog.hostilefork.com/c-casts-for-the-masses/
//
// But debug builds don't inline functions--not even no-op ones whose sole
// purpose is static analysis.  This means the cast macros add a headache when
// stepping through the debugger, and also they consume a measurable amount
// of runtime.  Hence we sacrifice cast checking in the debug builds...and the
// release C++ builds on Travis are relied upon to do the proper optimizations
// as well as report any static analysis errors.
//

#if !defined(__cplusplus) || !defined(NDEBUG)
    /* These macros are easier-to-spot variants of the parentheses cast.
     * The 'm_cast' is when getting [M]utablity on a const is okay (RARELY!)
     * Plain 'cast' can do everything else (except remove volatile)
     * The 'c_cast' helper ensures you're ONLY adding [C]onst to a value
     */
    #define m_cast(t,v)     ((t)(v))
    #define cast(t,v)       ((t)(v))
    #define c_cast(t,v)     ((t)(v))
    /*
     * Q: Why divide roles?  A: Frequently, input to cast is const but you
     * "just forget" to include const in the result type, gaining mutable
     * access.  Stray writes to that can cause even time-traveling bugs, with
     * effects *before* that write is made...due to "undefined behavior".
     */
#elif defined(__cplusplus) /* for gcc -Wundef */ && (__cplusplus < 201103L)
    /* Well-intentioned macros aside, C has no way to enforce that you can't
     * cast away a const without m_cast. C++98 builds can do that, at least:
     */
    #define m_cast(t,v)     const_cast<t>(v)
    #define cast(t,v)       ((t)(v))
    #define c_cast(t,v)     const_cast<t>(v)
#else
    /* __cplusplus >= 201103L has C++11's type_traits, where we get some
     * actual power.  cast becomes a reinterpret_cast for pointers and a
     * static_cast otherwise.  We ensure c_cast added a const and m_cast
     * removed one, and that neither affected volatility.
     */
    template<typename T, typename V>
    T m_cast_helper(V v) {
        static_assert(!std::is_const<T>::value,
            "invalid m_cast() - requested a const type for output result");
        static_assert(std::is_volatile<T>::value == std::is_volatile<V>::value,
            "invalid m_cast() - input and output have mismatched volatility");
        return const_cast<T>(v);
    }
    /* reinterpret_cast for pointer to pointer casting (non-class source)*/
    template<typename T, typename V,
        typename std::enable_if<
            !std::is_class<V>::value
            && (std::is_pointer<V>::value || std::is_pointer<T>::value)
        >::type* = nullptr>
                T cast_helper(V v) { return reinterpret_cast<T>(v); }
    /* static_cast for non-pointer to non-pointer casting (non-class source) */
    template<typename T, typename V,
        typename std::enable_if<
            !std::is_class<V>::value
            && (!std::is_pointer<V>::value && !std::is_pointer<T>::value)
        >::type* = nullptr>
                T cast_helper(V v) { return static_cast<T>(v); }
    /* use static_cast on all classes, to go through their cast operators */
    template<typename T, typename V,
        typename std::enable_if<
            std::is_class<V>::value
        >::type* = nullptr>
                T cast_helper(V v) { return static_cast<T>(v); }
    template<typename T, typename V>
    T c_cast_helper(V v) {
        static_assert(!std::is_const<T>::value,
            "invalid c_cast() - did not request const type for output result");
        static_assert(std::is_volatile<T>::value == std::is_volatile<V>::value,
            "invalid c_cast() - input and output have mismatched volatility");
        return const_cast<T>(v);
    }
    #define m_cast(t, v)    m_cast_helper<t>(v)
    #define cast(t, v)      cast_helper<t>(v)
    #define c_cast(t, v)    c_cast_helper<t>(v)
#endif


//=//// BYTE STRINGS VS UNENCODED CHARACTER STRINGS ///////////////////////=//
//
// Use these when you semantically are talking about unsigned characters as
// bytes.  For instance: if you want to count unencoded chars in 'char *' us
// strlen(), and the reader will know that is a count of letters.  If you have
// something like UTF-8 with more than one byte per character, use LEN_BYTES.
// The casting macros are derived from "Casts for the Masses (in C)":
//
// http://blog.hostilefork.com/c-casts-for-the-masses/
//
// For APPEND_BYTES_LIMIT, m is the max-size allocated for d (dest)
//
#include <string.h> // for strlen() etc, but also defines `size_t`
#define strsize strlen
#if defined(NDEBUG)
    /* These [S]tring and [B]inary casts are for "flips" between a 'char *'
     * and 'unsigned char *' (or 'const char *' and 'const unsigned char *').
     * Being single-arity with no type passed in, they are succinct to use:
     */
    #define s_cast(b)       ((char *)(b))
    #define cs_cast(b)      ((const char *)(b))
    #define b_cast(s)       ((REBYTE *)(s))
    #define cb_cast(s)      ((const REBYTE *)(s))

    #define LEN_BYTES(s) \
        strlen((const char*)(s))

    #define COPY_BYTES(d,s,n) \
        strncpy((char*)(d), (const char*)(s), (n))

    #define CMP_BYTES(l,r) \
        strcmp((const char*)(l), (const char*)(r))

    inline static REBYTE *APPEND_BYTES(
        REBYTE *dest, const REBYTE *src, size_t max
    ){
        size_t len = LEN_BYTES(dest);
        return b_cast(strncat(
            s_cast(dest), cs_cast(src), MAX(max - len - 1, 0)
        ));
    }
#else
    /* We want to ensure the input type is what we thought we were flipping,
     * particularly not the already-flipped type.  Instead of type_traits, 4
     * functions check in both C and C++ (here only during Debug builds):
     */
    inline static REBYTE *b_cast(char *s)
        { return (REBYTE*)s; }

    inline static const REBYTE *cb_cast(const char *s)
        { return (const REBYTE*)s; }

    inline static char *s_cast(REBYTE *s)
        { return (char*)s; }

    inline static const char *cs_cast(const REBYTE *s)
        { return (const char*)s; }

    // Debug build uses inline functions to ensure you pass in unsigned char *
    //
    inline static unsigned char *COPY_BYTES(
        REBYTE *dest, const REBYTE *src, size_t count
    ){
        return b_cast(strncpy(s_cast(dest), cs_cast(src), count));
    }

    inline static size_t LEN_BYTES(const REBYTE *str)
        { return strlen(cs_cast(str)); }

    inline static int CMP_BYTES(
        const REBYTE *lhs, const REBYTE *rhs
    ){
        return strcmp(cs_cast(lhs), cs_cast(rhs));
    }

    inline static REBYTE *APPEND_BYTES(
        REBYTE *dest, const REBYTE *src, size_t max
    ){
        size_t len = LEN_BYTES(dest);
        return b_cast(strncat(
            s_cast(dest), cs_cast(src), MAX(max - len - 1, 0)
        ));
    }
#endif

#endif // REB_C_H
