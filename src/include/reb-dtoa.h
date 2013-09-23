/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  REBOL is a trademark of REBOL Technologies
**
**  Copyright 2013 Saphirion AG
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
**  Title:  Settings for the f-dtoa.c file
**  Author: Ladislav Mecir
**  Notes:
**
************************************************************************
**
**  NOTE to PROGRAMMERS:
**
**    1. Keep code clear and simple.
**    2. Document unusual code, reasoning, or gotchas.
**    3. Use same style for code, vars, indent(4), comments, etc.
**    4. Keep in mind Linux, OS X, BSD, big/little endian CPUs.
**    5. Test everything, then test it again.
**
***********************************************************************/

/* dtoa needs float.h */
#include <float.h>

/* Rebol 3 definitions */
#include "sys-core.h"

/* this renames the strtod function to suppress possible conflicts
 * with some overly aggressive definitions in stdlib.h */
#undef strtod
#define strtod STRTOD

/* #define IEEE_8087 for IEEE-arithmetic machines where the least
 *	significant byte has the lowest address.
 * #define IEEE_MC68k for IEEE-arithmetic machines where the most
 *	significant byte has the lowest address. */
#ifdef ENDIAN_LITTLE
#define IEEE_8087
#else
#define IEEE_MC68k
#endif

/* #define Long int on machines with 32-bit ints and 64-bit longs. */
#define Long REBINT
#define ULong REBCNT

/* #define IBM for IBM mainframe-style floating-point arithmetic. */

/* #define VAX for VAX-style floating-point arithmetic (D_floating). */

/* #define No_leftright to omit left-right logic in fast floating-point
 *	computation of dtoa.  This will cause dtoa modes 4 and 5 to be
 *	treated the same as modes 2 and 3 for some inputs. */

/* #define Honor_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3
 *	and strtod and dtoa should round accordingly.  Unless Trust_FLT_ROUNDS
 *	is also #defined, fegetround() will be queried for the rounding mode.
 *	Note that both FLT_ROUNDS and fegetround() are specified by the C99
 *	standard (and are specified to be consistent, with fesetround()
 *	affecting the value of FLT_ROUNDS), but that some (Linux) systems
 *	do not work correctly in this regard, so using fegetround() is more
 *	portable than using FLT_ROUNDS directly. */

/* #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3
 *	and Honor_FLT_ROUNDS is not #defined. */

/* #define RND_PRODQUOT to use rnd_prod and rnd_quot (assembly routines
 *	that use extended-precision instructions to compute rounded
 *	products and quotients) with IBM. */

/* #define ROUND_BIASED for IEEE-format with biased rounding and arithmetic
 *	that rounds toward +Infinity. */

/* #define ROUND_BIASED_without_Round_Up for IEEE-format with biased
 *	rounding when the underlying floating-point arithmetic uses
 *	unbiased rounding.  This prevent using ordinary floating-point
 *	arithmetic when the result could be computed with one rounding error. */

/* #define Inaccurate_Divide for IEEE-format with correctly rounded
 *	products but inaccurate quotients, e.g., for Intel i860. */

/* #define NO_LONG_LONG on machines that do not have a "long long"
 *	integer type (of >= 64 bits).  On such machines, you can
 *	#define Just_16 to store 16 bits per 32-bit Long when doing
 *	high-precision integer arithmetic.  Whether this speeds things
 *	up or slows things down depends on the machine and the number
 *	being converted.  If long long is available and the name is
 *	something other than "long long", #define Llong to be the name,
 *	and if "unsigned Llong" does not work as an unsigned version of
 *	Llong, #define #ULLong to be the corresponding unsigned type. */
#define Llong REBI64
#define ULLong REBU64

/* #define KR_headers for old-style C function headers. */

/* #define Bad_float_h if your system lacks a float.h or if it does not
 *	define some or all of DBL_DIG, DBL_MAX_10_EXP, DBL_MAX_EXP,
 *	FLT_RADIX, FLT_ROUNDS, and DBL_MAX. */
#if !defined DBL_DIG || !defined DBL_MAX_10_EXP || !defined DBL_MAX_EXP || \
	! defined FLT_RADIX || !defined FLT_ROUNDS || !defined DBL_MAX
#define Bad_float_h
#endif

/* #define MALLOC your_malloc, where your_malloc(n) acts like malloc(n)
 *	if memory is available and otherwise does something you deem
 *	appropriate.  If MALLOC is undefined, malloc will be invoked
 *	directly -- and assumed always to succeed.  Similarly, if you
 *	want something other than the system's free() to be called to
 *	recycle memory acquired from MALLOC, #define FREE to be the
 *	name of the alternate routine.  (FREE or free is only called in
 *	pathological cases, e.g., in a dtoa call after a dtoa return in
 *	mode 3 with thousands of digits requested.) */
#undef FREE

/* #define Omit_Private_Memory to omit logic (added Jan. 1998) for making
 *	memory allocations from a private pool of memory when possible.
 *	When used, the private pool is PRIVATE_MEM bytes long:  2304 bytes,
 *	unless #defined to be a different length.  This default length
 *	suffices to get rid of MALLOC calls except for unusual cases,
 *	such as decimal-to-binary conversion of a very long string of
 *	digits.  The longest string dtoa can return is about 751 bytes
 *	long.  For conversions by strtod of strings of 800 digits and
 *	all dtoa conversions in single-threaded executions with 8-byte
 *	pointers, PRIVATE_MEM >= 7400 appears to suffice; with 4-byte
 *	pointers, PRIVATE_MEM >= 7112 appears adequate. */

/* #define NO_INFNAN_CHECK if you do not wish to have INFNAN_CHECK
 *	#defined automatically on IEEE systems.  On such systems,
 *	when INFNAN_CHECK is #defined, strtod checks
 *	for Infinity and NaN (case insensitively).  On some systems
 *	(e.g., some HP systems), it may be necessary to #define NAN_WORD0
 *	appropriately -- to the most significant word of a quiet NaN.
 *	(On HP Series 700/800 machines, -DNAN_WORD0=0x7ff40000 works.)
 *	When INFNAN_CHECK is #defined and No_Hex_NaN is not #defined,
 *	strtod also accepts (case insensitively) strings of the form
 *	NaN(x), where x is a string of hexadecimal digits and spaces;
 *	if there is only one string of hexadecimal digits, it is taken
 *	for the 52 fraction bits of the resulting NaN; if there are two
 *	or more strings of hex digits, the first is for the high 20 bits,
 *	the second and subsequent for the low 32 bits, with intervening
 *	white space ignored; but if this results in none of the 52
 *	fraction bits being on (an IEEE Infinity symbol), then NAN_WORD0
 *	and NAN_WORD1 are used instead. */

/* #define MULTIPLE_THREADS if the system offers preemptively scheduled
 *	multiple threads.  In this case, you must provide (or suitably
 *	#define) two locks, acquired by ACQUIRE_DTOA_LOCK(n) and freed
 *	by FREE_DTOA_LOCK(n) for n = 0 or 1.  (The second lock, accessed
 *	in pow5mult, ensures lazy evaluation of only one copy of high
 *	powers of 5; omitting this lock would introduce a small
 *	probability of wasting memory, but would otherwise be harmless.)
 *	You must also invoke freedtoa(s) to free the value s returned by
 *	dtoa.  You may do so whether or not MULTIPLE_THREADS is #defined. */

/* #define NO_IEEE_Scale to disable new (Feb. 1997) logic in strtod that
 *	avoids underflows on inputs whose result does not underflow.
 *	If you #define NO_IEEE_Scale on a machine that uses IEEE-format
 *	floating-point numbers and flushes underflows to zero rather
 *	than implementing gradual underflow, then you must also #define
 *	Sudden_Underflow. */

/* #define USE_LOCALE to use the current locale's decimal_point value. */

/* #define SET_INEXACT if IEEE arithmetic is being used and extra
 *	computation should be done to set the inexact flag when the
 *	result is inexact and avoid setting inexact when the result
 *	is exact.  In this case, dtoa.c must be compiled in
 *	an environment, perhaps provided by #include "dtoa.c" in a
 *	suitable wrapper, that defines two functions,
 *		int get_inexact(void);
 *		void clear_inexact(void);
 *	such that get_inexact() returns a nonzero value if the
 *	inexact bit is already set, and clear_inexact() sets the
 *	inexact bit to 0.  When SET_INEXACT is #defined, strtod
 *	also does extra computations to set the underflow and overflow
 *	flags when appropriate (i.e., when the result is tiny and
 *	inexact or when it is a numeric value rounded to +-infinity). */

/* #define NO_ERRNO if strtod should not assign errno = ERANGE when
 *	the result overflows to +-Infinity or underflows to 0. */

/* #define NO_HEX_FP to omit recognition of hexadecimal floating-point
 *	values by strtod. */

/* #define NO_STRTOD_BIGCOMP (on IEEE-arithmetic systems only for now)
 *	to disable logic for "fast" testing of very long input strings
 *	to strtod.  This testing proceeds by initially truncating the
 *	input string, then if necessary comparing the whole string with
 *	a decimal expansion to decide close cases. This logic is only
 *	used for input more than STRTOD_DIGLIM digits long (default 40).
 */
