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
**  Module:  n-math.c
**  Summary: native functions for math
**  Section: natives
**  Author:  Carl Sassenrath
**  Notes:   See also: the numeric datatypes
**
***********************************************************************/

#include "sys-core.h"
#include "gen-comptypes.h"
#include "sys-deci-funcs.h"

#include <math.h>
#include <float.h>

#define	LOG2	0.6931471805599453
#define	EPS		2.718281828459045235360287471

const double pi1 = 3.14159265358979323846;
const double pi2 = 2.0 * 3.14159265358979323846;

#ifndef DBL_EPSILON
#define DBL_EPSILON	2.2204460492503131E-16
#endif

enum {SINE, COSINE, TANGENT};


/***********************************************************************
**
*/	static REBDEC Trig_Value(REBVAL *ds, REBCNT which)
/*
**	Convert integer arg, if present, to decimal and convert to radians
**	if necessary.  Clip ranges for correct REBOL behavior.
**
***********************************************************************/
{
	REBDEC	dval;

	dval = AS_DECIMAL(D_ARG(1));

	if (!D_REF(2)) {
        /* get dval between -360.0 and 360.0 */
        dval = fmod (dval, 360.0);

        /* get dval between -180.0 and 180.0 */
        if (fabs (dval) > 180.0) dval += dval < 0.0 ? 360.0 : -360.0;
        if (which == TANGENT) {
            /* get dval between -90.0 and 90.0 */
            if (fabs (dval) > 90.0) dval += dval < 0.0 ? 180.0 : -180.0;
        } else if (which == SINE) {
            /* get dval between -90.0 and 90.0 */
            if (fabs (dval) > 90.0) dval = (dval < 0.0 ? -180.0 : 180.0) - dval;
        }
        dval = dval * pi1 / 180.0; // to radians
    }

	return dval;
}


/***********************************************************************
**
*/	static void Arc_Trans(REBVAL *ds, REBCNT kind)
/*
***********************************************************************/
{
	REBDEC	dval;

	dval = AS_DECIMAL(D_ARG(1));
#ifdef USE_NO_INFINITY
	if (kind != TANGENT && (dval < -1 || dval > 1)) Trap0(RE_OVERFLOW);
#endif

	if (kind == SINE) dval = asin(dval);
	else if (kind == COSINE) dval = acos(dval);
	else dval = atan(dval);

	if (!D_REF(2)) dval = dval * 180.0 / pi1; // to degrees

	SET_DECIMAL(D_RET, dval);
}


/***********************************************************************
**
*/	REBNATIVE(cosine)
/*
***********************************************************************/
{
	REBDEC dval = cos(Trig_Value(ds, COSINE));
	if (fabs(dval) < DBL_EPSILON) dval = 0.0;
	SET_DECIMAL(D_RET, dval);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(sine)
/*
***********************************************************************/
{
	REBDEC dval = sin(Trig_Value(ds, SINE));
	if (fabs(dval) < DBL_EPSILON) dval = 0.0;
	SET_DECIMAL(D_RET, dval);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(tangent)
/*
***********************************************************************/
{
	REBDEC dval = Trig_Value(ds, TANGENT);
	if (Eq_Decimal(fabs(dval), pi1 / 2.0)) {
#ifdef USE_NO_INFINITY
		Trap0(RE_OVERFLOW);
#else
		SET_DECIMAL(D_RET, (dval < 0) ? -INFINITY : INFINITY);
		return R_RET;
#endif
	}
	SET_DECIMAL(D_RET, tan(dval));
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(arccosine)
/*
***********************************************************************/
{
	Arc_Trans(ds, COSINE);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(arcsine)
/*
***********************************************************************/
{
	Arc_Trans(ds, SINE);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(arctangent)
/*
***********************************************************************/
{
	Arc_Trans(ds, TANGENT);
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(arctangent2)
/*
//	arctangent2: native [
//		{Returns the angle of the point, when measured counterclockwise from a circle's X axis (where 0x0 represents the center of the circle). The return value is in interval -180 to 180 degrees.}
//		point [pair!] "X/Y coordinate in space"
//		/radians "Result is in radians instead of degrees"
//	]
***********************************************************************/
{
	REBDEC dval = atan2(VAL_PAIR_Y(D_ARG(1)), VAL_PAIR_X(D_ARG(1)));
	if(!D_REF(2)) dval *= 180.0 / pi1; //to degrees
	SET_DECIMAL(D_RET, dval);
	return R_RET;
}


// Follows faster trigonometric functions (without conversions and bounds checks)
#ifndef USE_NO_INFINITY //use these functions only with INFINITY support (cos/sin may return 1.#NaN value)!

/***********************************************************************
**
*/	REBNATIVE(cos)
/*
//	cos: native [
//		{Returns the trigonometric cosine.}
//		value [decimal!] "In radians"
//	]
***********************************************************************/
{
	SET_DECIMAL(D_RET, cos(VAL_DECIMAL(D_ARG(1))));
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(sin)
/*
//	sin: native [
//		{Returns the trigonometric sine.}
//		value [decimal!] "In radians"
//	]
***********************************************************************/
{
	SET_DECIMAL(D_RET, sin(VAL_DECIMAL(D_ARG(1))));
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(tan)
/*
//	tan: native [
//		{Returns the trigonometric tangent.}
//		value [decimal!] "In radians"
//	]
***********************************************************************/
{
	SET_DECIMAL(D_RET, tan(VAL_DECIMAL(D_ARG(1))));
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(atan)
/*
//	atan: native [
//		{Returns the trigonometric arctangent.}
//		value [decimal!] "In radians"
//	]
***********************************************************************/
{
	SET_DECIMAL(D_RET, atan(VAL_DECIMAL(D_ARG(1))));
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(asin)
/*
//	asin: native [
//		{Returns the trigonometric arcsine.}
//		value [decimal!] "In radians"
//	]
***********************************************************************/
{
	SET_DECIMAL(D_RET, asin(VAL_DECIMAL(D_ARG(1))));
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(acos)
/*
//	acos: native [
//		{Returns the trigonometric arccosine.}
//		value [decimal!] "In radians"
//	]
***********************************************************************/
{
	SET_DECIMAL(D_RET, acos(VAL_DECIMAL(D_ARG(1))));
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(atan2)
/*
//	atan2: native [
//		{Returns the angle of the point y/x in the interval [-pi,+pi] radians.}
//		y [decimal!] "The proportion of the Y-coordinate"
//		x [decimal!] "The proportion of the X-coordinate"
//	]
***********************************************************************/
{
	SET_DECIMAL(D_RET, atan2(VAL_DECIMAL(D_ARG(1)), VAL_DECIMAL(D_ARG(2))));
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(sqrt)
/*
//	sqrt: native [
//		{Returns the square root of a number.}
//		value [decimal!]
//	]
***********************************************************************/
{
	SET_DECIMAL(D_RET, sqrt(VAL_DECIMAL(D_ARG(1))));
	return R_RET;
}

#endif //!USE_NO_INFINITY

/***********************************************************************
**
*/	REBNATIVE(numberq)
/*
//	number?: native [
//		{Returns TRUE if the value is any type of number and not a NaN. }
//		value
//	]
***********************************************************************/
{
	BOOL result = FALSE;
	switch(VAL_TYPE(D_ARG(1))) {
		case REB_DECIMAL:
			if (!isnan(VAL_DECIMAL(D_ARG(1)))) result = TRUE;
			break;
		case REB_INTEGER:
		case REB_MONEY:
		case REB_PERCENT:
			result = TRUE;
			break;
	}
	SET_LOGIC(D_RET, result);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(exp)
/*
***********************************************************************/
{
	REBDEC	dval = AS_DECIMAL(D_ARG(1));
	static REBDEC eps = EPS;

	dval = pow(eps, dval);
//!!!!	Check_Overflow(dval);
	SET_DECIMAL(D_RET, dval);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(log_10)
/*
***********************************************************************/
{
	REBDEC dval = AS_DECIMAL(D_ARG(1));
#ifdef USE_NO_INFINITY
	if (dval <= 0) Trap0(RE_POSITIVE);
#endif
	SET_DECIMAL(D_RET, log10(dval));
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(log_2)
/*
***********************************************************************/
{
	REBDEC dval = AS_DECIMAL(D_ARG(1));
#ifdef USE_NO_INFINITY
	if (dval <= 0) Trap0(RE_POSITIVE);
#endif
	SET_DECIMAL(D_RET, log(dval) / LOG2);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(log_e)
/*
***********************************************************************/
{
	REBDEC dval = AS_DECIMAL(D_ARG(1));
#ifdef USE_NO_INFINITY
	if (dval <= 0) Trap0(RE_POSITIVE);
#endif
	SET_DECIMAL(D_RET, log(dval));
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(square_root)
/*
***********************************************************************/
{
	REBDEC dval = AS_DECIMAL(D_ARG(1));
#ifdef USE_NO_INFINITY
	if (dval < 0) Trap0(RE_POSITIVE);
#endif
	SET_DECIMAL(D_RET, sqrt(dval));
	return R_RET;
}

#if defined(_MSC_VER) && _MSC_VER > 1800
#pragma warning (disable : 4146)
#endif


/***********************************************************************
**
*/	REBNATIVE(shift)
/*
**		shift int bits arithmetic or logical
**
***********************************************************************/
{
	REBI64 b = VAL_INT64(D_ARG(2));
	REBVAL *a = D_ARG(1);
	REBU64 c, d;

	if (b < 0) {
		// this is defined:
		c = -(REBU64)b;
		if (c >= 64) {
			if (D_REF(3)) VAL_INT64(a) = 0;
			else VAL_INT64(a) >>= 63;
		} else {
			if (D_REF(3)) VAL_UNT64(a) >>= c;
			else VAL_INT64(a) >>= (REBI64)c;
		}
	} else {
		if (b >= 64) {
			if (D_REF(3)) VAL_INT64(a) = 0;
			else if (VAL_INT64(a)) Trap0(RE_OVERFLOW);
		} else
			if (D_REF(3)) VAL_UNT64(a) <<= b;
			else {
				c = (REBU64)MIN_I64 >> b;
				d = VAL_INT64(a) < 0 ? -VAL_UNT64(a) : VAL_UNT64(a);
				if (c <= d)
					if ((c < d) || (VAL_INT64(a) >= 0)) Trap0(RE_OVERFLOW);
					else VAL_INT64(a) = MIN_I64;
				else
					VAL_INT64(a) <<= b;
			}
	}
	return R_ARG1;
}

// See above for the temporary disablement and reasoning.
//
#if defined(_MSC_VER) && _MSC_VER > 1800
#pragma warning (default : 4146)
#endif

/***********************************************************************
**
*/	REBNATIVE(shift_left)
/*
//  shift-left: native [
//		"Shift bits to the left (unsigned)."
//		data [integer!]
//		bits [integer!]
//  ]
***********************************************************************/
{
	REBVAL *a = D_ARG(1);
	REBVAL *b = D_ARG(2);

	if (!IS_INTEGER(a)) Trap2(RE_EXPECT_VAL, Get_Type_Word(REB_INTEGER), a);
	if (!IS_INTEGER(b)) Trap2(RE_EXPECT_VAL, Get_Type_Word(REB_INTEGER), b);
	if (VAL_INT64(b) > (uint)0L) VAL_INT64(a) <<= VAL_INT64(b);
	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(shift_right)
/*
//  shift-right: native [
//		"Shift bits to the right (unsigned)."
//		data [integer!]
//		bits [integer!]
//  ]
***********************************************************************/
{
	REBVAL *a = D_ARG(1);
	REBVAL *b = D_ARG(2);
	
	if (!IS_INTEGER(a)) Trap2(RE_EXPECT_VAL, Get_Type_Word(REB_INTEGER), a);
	if (!IS_INTEGER(b)) Trap2(RE_EXPECT_VAL, Get_Type_Word(REB_INTEGER), b);
	if (VAL_INT64(b) > (uint)0L) VAL_INT64(a) >>= VAL_INT64(b);
	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(to_radians)
/*
//	to-radians: native [
//		"Converts degrees to radians"
//		degrees [integer! decimal!] "Degrees to convert"
//	]
***********************************************************************/
{
	REBDEC degrees = AS_DECIMAL(D_ARG(1));
	SET_DECIMAL(D_RET, degrees * pi1 / 180.0 );
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(to_degrees)
/*
//	to-degrees: native [
//		"Converts radians to degrees"
//		radians [integer! decimal!] "Radians to convert"
//	]
***********************************************************************/
{
	REBDEC radians = AS_DECIMAL(D_ARG(1));
	SET_DECIMAL(D_RET, radians * 180.0 / pi1 );
	return R_RET;
}

/***********************************************************************
**
*/	REBINT Compare_Values(REBVAL *a, REBVAL *b, REBINT strictness)
/*
**		Compare 2 values depending on level of strictness.
**		NOTE: MODIFIES a and b args.
**
**		Strictness:
**			0 - coersed equality
**			1 - equivalence
**			2 - strict equality
**			3 - same (identical bits)
**
**		   -1 - greater or equal
**		   -2 - greater
**
***********************************************************************/
{
	REBCNT ta = VAL_TYPE(a);
	REBCNT tb = VAL_TYPE(b);
	REBCTF code;
	REBINT result;

	if (ta != tb) {
		if (strictness > 1) return FALSE;

		switch (ta) {
		case REB_INTEGER:
			if (tb == REB_DECIMAL || tb == REB_PERCENT) {
				SET_DECIMAL(a, (REBDEC)VAL_INT64(a));
				goto compare;
			}
			else if (tb == REB_MONEY) {
				SET_MONEY(a, int_to_deci(VAL_INT64(a)));
				goto compare;
			}
			else if (tb == REB_INTEGER || tb == REB_CHAR) // special negative?, zero?, ...
				goto compare;
			break;

		case REB_DECIMAL:
		case REB_PERCENT:
			if (tb == REB_INTEGER) {
				SET_DECIMAL(b, (REBDEC)VAL_INT64(b));
				goto compare;
			}
			else if (tb == REB_MONEY) {
				SET_MONEY(a, decimal_to_deci(VAL_DECIMAL(a)));
				goto compare;
			}
			else if (tb == REB_DECIMAL || tb == REB_PERCENT) // equivalent types
				goto compare;
			break;

		case REB_MONEY:
			if (tb == REB_INTEGER) {
				SET_MONEY(b, int_to_deci(VAL_INT64(b)));
				goto compare;
			}
			if (tb == REB_DECIMAL || tb == REB_PERCENT) {
				SET_MONEY(b, decimal_to_deci(VAL_DECIMAL(b)));
				goto compare;
			}
			break;

		case REB_WORD:
		case REB_SET_WORD:
		case REB_GET_WORD:
		case REB_LIT_WORD:
		case REB_REFINEMENT:
		case REB_ISSUE:
			if (ANY_WORD(b)) goto compare;
			break;

		case REB_STRING:
		case REB_FILE:
		case REB_EMAIL:
		case REB_URL:
		case REB_TAG:
			if (ANY_STR(b)) goto compare;
			break;

		case REB_CHAR:
			//o: using sepparated case as I don't want to allow comparison
			//o: with not integer numbers (money, decimal, percent)
			if (tb == REB_INTEGER)
				goto compare;
		}
		if (strictness == 0 || strictness == 1) return FALSE;
		//if (strictness >= 2)
		Trap2(RE_INVALID_COMPARE, Of_Type(a), Of_Type(b));
	}

compare:
	// At this point, both args are of the same datatype.
	if (!(code = Compare_Types[VAL_TYPE(a)])) return FALSE;
	result = code(a, b, strictness);
	if (result < 0) Trap2(RE_INVALID_COMPARE, Of_Type(a), Of_Type(b));
	return result;
}


//	EQUAL? < EQUIV? < STRICT-EQUAL? < SAME?

/***********************************************************************
**
*/	REBNATIVE(equalq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), 0)) return R_TRUE;
	return R_FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(not_equalq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), 0)) return R_FALSE;
	return R_TRUE;
}

/***********************************************************************
**
*/	REBNATIVE(equivq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), 1)) return R_TRUE;
	return R_FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(not_equivq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), 1)) return R_FALSE;
	return R_TRUE;
}

/***********************************************************************
**
*/	REBNATIVE(strict_equalq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), 2)) return R_TRUE;
	return R_FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(strict_not_equalq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), 2)) return R_FALSE;
	return R_TRUE;
}

/***********************************************************************
**
*/	REBNATIVE(sameq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), 3)) return R_TRUE;
	return R_FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(lesserq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), -1)) return R_FALSE;
	return R_TRUE;
}

/***********************************************************************
**
*/	REBNATIVE(lesser_or_equalq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), -2)) return R_FALSE;
	return R_TRUE;
}

/***********************************************************************
**
*/	REBNATIVE(greaterq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), -2)) return R_TRUE;
	return R_FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(greater_or_equalq)
/*
***********************************************************************/
{
	if (Compare_Values(D_ARG(1), D_ARG(2), -1)) return R_TRUE;
	return R_FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(maximum)
/*
***********************************************************************/
{
	REBVAL a, b;

	if (IS_PAIR(D_ARG(1)) || IS_PAIR(D_ARG(2)))
		return Min_Max_Pair(ds, 1);

	a = *D_ARG(1);
	b = *D_ARG(2);
	if (Compare_Values(&a, &b, -1)) return R_ARG1;
	return R_ARG2;
}

/***********************************************************************
**
*/	REBNATIVE(minimum)
/*
***********************************************************************/
{
	REBVAL a, b;

	if (IS_PAIR(D_ARG(1)) || IS_PAIR(D_ARG(2)))
		return Min_Max_Pair(ds, 0);

	a = *D_ARG(1);
	b = *D_ARG(2);
	if (Compare_Values(&a, &b, -1)) return R_ARG2;
	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(negativeq)
/*
***********************************************************************/
{
	REBVAL *val = &DS_Base[++DSP];
	CLEARS(val);
	VAL_SET(val, VAL_TYPE(D_ARG(1)));
	if (Compare_Values(D_ARG(1), D_ARG(2), -1)) return R_FALSE;
	return R_TRUE;
}

/***********************************************************************
**
*/	REBNATIVE(positiveq)
/*
***********************************************************************/
{
	REBVAL *val = &DS_Base[++DSP];
	CLEARS(val);
	VAL_SET(val, VAL_TYPE(D_ARG(1)));
	if (Compare_Values(D_ARG(1), D_ARG(2), -2)) return R_TRUE;
	return R_FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(zeroq)
/*
***********************************************************************/
{
	REBCNT type = VAL_TYPE(D_ARG(1));

	if (type >= REB_INTEGER && type <= REB_TIME) {
		REBVAL *val = &DS_Base[++DSP];
		CLEARS(val);
		VAL_SET(val, type);
		if (Compare_Values(D_ARG(1), D_ARG(2), 1)) return R_TRUE;
	}
	else if (type == REB_BITSET && Is_Zero_Bitset(VAL_SERIES(D_ARG(1)))) {
		return R_TRUE;
	}
		
	return R_FALSE;
}
