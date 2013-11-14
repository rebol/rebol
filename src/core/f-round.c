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
**  Module:  f-round.c
**  Summary: special rounding math functions
**  Section: functional
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-deci-funcs.h"


// Rounding flags (passed as refinements to ROUND function):
enum {
	RF_TO,
	RF_EVEN,
	RF_DOWN,
	RF_HALF_DOWN,
	RF_FLOOR,
	RF_CEILING,
	RF_HALF_CEILING
};

#define RB_DFC (1 << RF_DOWN | 1 << RF_FLOOR | 1 << RF_CEILING)

/***********************************************************************
**
*/  REBCNT Get_Round_Flags(REBVAL *ds)
/*
**		1 n [number! money! time!] "The value to round"
**		2 /to "Return the nearest multiple of the scale parameter"
**		3	scale [number! money! time!] "Must be a non-zero value"
**		4 /even      "Halves round toward even results"
**		5 /down      "Round toward zero, ignoring discarded digits. (truncate)"
**		6 /half-down "Halves round toward zero"
**		7 /floor     "Round in negative direction"
**		8 /ceiling   "Round in positive direction"
**		9 /half-ceiling "Halves round in positive direction"
**
***********************************************************************/
{
	REBCNT flags = 0;

	if (D_REF(2)) SET_FLAG(flags, RF_TO);
	if (D_REF(4)) SET_FLAG(flags, RF_EVEN);
	if (D_REF(5)) SET_FLAG(flags, RF_DOWN);
	if (D_REF(6)) SET_FLAG(flags, RF_HALF_DOWN);
	if (D_REF(7)) SET_FLAG(flags, RF_FLOOR);
	if (D_REF(8)) SET_FLAG(flags, RF_CEILING);
	if (D_REF(9)) SET_FLAG(flags, RF_HALF_CEILING);

	return flags;
}


#define Dec_Trunc(x) (((x) < 0.0) ? -1.0 : 1.0) * floor(fabs(x))
#define Dec_Away(x) (((x) < 0.0) ? -1.0 : 1.0) * ceil(fabs(x))

/***********************************************************************
**
*/	REBDEC Round_Dec(REBDEC dec, REBCNT flags, REBDEC scale)
/*
**		Identical to ROUND mezzanine function.
**		Note: scale arg only valid if RF_TO is set
**
***********************************************************************/
{
	REBDEC r;
	int e;
	REBFLG v;
	union {REBDEC d; REBI64 i;} m;
	REBI64 j;

	if (GET_FLAG(flags, RF_TO)) {
		if (scale == 0.0) Trap0(RE_ZERO_DIVIDE);
		scale = fabs(scale);
	} else scale = 1.0;

	/* is scale negligible? */
	if (scale < ldexp(fabs(dec), -53)) return dec;

	if (v = scale >= 1.0) dec = dec / scale;
	else {
		r = frexp(scale, &e);
		if (e <= -1022) {
		    scale = r;
			dec = ldexp(dec, e);
		} else e = 0;
		scale = 1.0 / scale;
		dec = dec * scale;
	}
	if (flags & RB_DFC) {
	    if (GET_FLAG(flags, RF_FLOOR)) dec = floor(dec);
	    else if (GET_FLAG(flags, RF_DOWN)) dec = Dec_Trunc(dec);
		else dec = ceil(dec);
	} else {
	    /*	integer-compare fabs(dec) and floor(fabs(dec)) + 0.5,
	        which is equivalent to "tolerant comparison" of the
	        fractional part with 0.5								*/
		m.d = fabs(dec);
		j = m.i;
		m.d = floor(m.d) + 0.5;
		if (j - m.i < -10) dec = Dec_Trunc(dec);
		else if (j - m.i > 10) dec = Dec_Away(dec);
	    else if (GET_FLAG(flags, RF_EVEN)) {
			if (fmod(fabs(dec), 2.0) < 1.0) dec = Dec_Trunc(dec);
			else dec = Dec_Away(dec);
		}
		else if (GET_FLAG(flags, RF_HALF_DOWN)) dec = Dec_Trunc(dec);
		else if (GET_FLAG(flags, RF_HALF_CEILING)) dec = ceil(dec);
		else dec = Dec_Away(dec);
	}

	if (v) {
		if (fabs(dec = dec * scale) != HUGE_VAL) return dec;
		else Trap0(RE_OVERFLOW);
	}
	return ldexp(dec / scale, e);
}

#define Int_Abs(x) ((x) < 0) ? -(x) : (x)
#define Int_Trunc num = (num > 0) ? n - r : -(REBI64)(n - r)
#define Int_Floor	{\
    					if (num > 0) num = n - r;\
						else if ((m = n + s) <= (REBU64)1 << 63) num = -(REBI64)m;\
						else Trap0(RE_OVERFLOW);\
					}
#define Int_Ceil	{\
						if (num < 0) num = -(REBI64)(n - r);\
	        			else if ((m = n + s) < (REBU64)1 << 63) num = m;\
	        			else Trap0(RE_OVERFLOW);\
					}
#define Int_Away	if ((m = n + s) >= (REBU64)1 << 63)\
						if (num < 0 && m == (REBU64) 1 << 63) num = m;\
		    			else Trap0(RE_OVERFLOW);\
					else num = (num > 0) ? m : -(REBI64)m

/***********************************************************************
**
*/	REBI64 Round_Int(REBI64 num, REBCNT flags, REBI64 scale)
/*
**		Identical to ROUND mezzanine function.
**		Note: scale arg only valid if RF_TO is set
**
***********************************************************************/
{
	/* using safe unsigned arithmetic */
	REBU64 sc, n, r, m, s;

	if (GET_FLAG(flags, RF_TO)) {
		if (scale == 0) Trap0(RE_ZERO_DIVIDE);
		sc = Int_Abs(scale);
	}
	else sc = 1;

	n = Int_Abs(num);
	r = n % sc;
	s = sc - r;
	if (r == 0) return num;

	if (flags & RB_DFC) {
		if (GET_FLAG(flags, RF_DOWN)) {Int_Trunc; return num;}
		if (GET_FLAG(flags, RF_FLOOR)) {Int_Floor; return num;}
		Int_Ceil; return num;
	}

	/* "genuine" rounding */
	if (r < s) {Int_Trunc; return num;}
	else if (r > s) {Int_Away; return num;}

	/* half */
	if (GET_FLAG(flags, RF_EVEN)) {
	    if ((n / sc) & 1) {Int_Away; return num;}
		else {Int_Trunc; return num;}
	}
	if (GET_FLAG(flags, RF_HALF_DOWN)) {Int_Trunc; return num;}
	if (GET_FLAG(flags, RF_HALF_CEILING)) {Int_Ceil; return num;}

	Int_Away; return num; /* this is round_half_away */
}

/***********************************************************************
**
*/	REBDCI Round_Deci(REBDCI num, REBCNT flags, REBDCI scale)
/*
**		Identical to ROUND mezzanine function.
**		Note: scale arg only valid if RF_TO is set
**
***********************************************************************/
{
	REBDCI deci_one = {1u, 0u, 0u, 0u, 0};

	if (GET_FLAG(flags, RF_TO)) {
		if (deci_is_zero(scale)) Trap0(RE_ZERO_DIVIDE);
		scale = deci_abs(scale);
	}
	else scale = deci_one;

	if (GET_FLAG(flags, RF_EVEN)) return deci_half_even(num, scale);
	if (GET_FLAG(flags, RF_DOWN)) return deci_truncate(num, scale);
	if (GET_FLAG(flags, RF_HALF_DOWN)) return deci_half_truncate(num, scale);
	if (GET_FLAG(flags, RF_FLOOR)) return deci_floor(num, scale);
	if (GET_FLAG(flags, RF_CEILING)) return deci_ceil(num, scale);
	if (GET_FLAG(flags, RF_HALF_CEILING)) return deci_half_ceil(num, scale);

	return deci_half_away(num, scale);
}
