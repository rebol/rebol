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
**  Module:  t-decimal.c
**  Summary: decimal datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include <math.h>
#include <float.h>
#include "sys-deci-funcs.h"

#define COEF 0.0625 // Coefficient used for float comparision
#define EQ_RANGE 4

#ifdef NO_GCVT
static char *gcvt(double value, int digits, char *buffer)
{
	sprintf(buffer, "%.*g", digits, value);
	return buffer;
}
#endif

/*
	Purpose: {defines the almost_equal comparison function}
	Properties: {    
		since floating point numbers are ordered and there is only
		a finite quantity of floating point numbers, it is possible
		to assign an ordinal (integer) number to any floating point number so,
		that the ordinal numbers of neighbors differ by one
		
		the function compares floating point numbers based on
		the difference of their ordinal numbers in the ordering
		of floating point numbers
		
		difference of 0 means exact equality, difference of 1 means, that
		the numbers are neighbors.
	}
	Advantages: {
		the function detects approximate equality.
		
		the function is more strict in the zero neighborhood than
		absolute-error-based approaches

		as opposed to relative-error-based approaches the error can be
		precisely specified, max_diff = 0 meaning exact match, max_diff = 1
		meaning that neighbors are deemed equal, max_diff = 10 meaning, that
		the numbers are deemed equal if at most 9
		distinct floating point numbers can be found between them
		
		the max_diff value may be one of the system options specified in
		the system/options object allowing users to exactly define the
		strictness of equality checks
	}
	Differences: {
		The approximate comparison currently used in R3 corresponds to the
		almost_equal function using max_diff = 10 (according to my tests).
		
		The main differences between the currently used comparison and the
		one based on the ordinal number comparison are:
		-   the max_diff parameter can be adjusted, allowing
			the user to precisely specify the strictness of the comparison
		-   the difference rule holds for zero too, which means, that
			zero is deemed equal with totally max_diff distinct (tiny) numbers
	}
	Notes: {
		the max_diff parameter does not need to be a REBI64 number,
		a smaller range like REBCNT may suffice
	}
*/

REBOOL almost_equal(REBDEC a, REBDEC b, REBCNT max_diff) {
	union {REBDEC d; REBI64 i;} ua, ub;
	REBI64 int_diff;

	ua.d = a;
	ub.d = b;

	/* Make ua.i a twos-complement ordinal number */
	if (ua.i < 0) ua.i = MIN_I64 - ua.i;

	/* Make ub.i a twos-complement ordinal number */
	if (ub.i < 0) ub.i = MIN_I64 - ub.i;

	int_diff = ua.i - ub.i;
	if (int_diff < 0) int_diff = -int_diff;
	
	return ((REBU64) int_diff <= max_diff);
}


/***********************************************************************
**
*/  REBFLG MT_Decimal(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	if (!IS_END(data+1)) return FALSE;

	if (IS_DECIMAL(data))
		*out = *data;
	else if (IS_INTEGER(data)) {
		SET_DECIMAL(out, (REBDEC)VAL_INT64(data));
	}
	else return FALSE;

	SET_TYPE(out, type);
	return TRUE;
}


/***********************************************************************
**
*/  REBFLG Eq_Decimal(REBDEC a, REBDEC b)
/*
***********************************************************************/
{
	return almost_equal(a, b, 10);
#ifdef older
	REBDEC d = (COEF * a) - (COEF * b);
	static volatile REBDEC c, e;
	c = b + d; // These are stored in variables to avoid 80bit
	e = a - d; // intermediate math, which creates problems.
	if ((c - b) == 0.0 && (e - a) == 0.0) return TRUE;
	return FALSE;
#endif
}


/***********************************************************************
**
*/  REBFLG Eq_Decimal2(REBDEC a, REBDEC b)
/*
***********************************************************************/
{
	return almost_equal(a, b, 0);
#ifdef older
	REBI64 d;
	if (a == b) return TRUE;
	d = *(REBU64*)&a - *(REBU64*)&b;
	if (d < 0) d = ~d;
	if (d <= EQ_RANGE) return TRUE;
	return FALSE;
#endif
}

/***********************************************************************
**
*/	REBINT CT_Decimal(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode >= 0) {
		if (mode <= 1) return almost_equal(VAL_DECIMAL(a), VAL_DECIMAL(b), 10);
		if (mode == 2) return almost_equal(VAL_DECIMAL(a), VAL_DECIMAL(b), 0);
		return VAL_INT64(a) == VAL_INT64(b); // bits are identical
	}
	if (mode == -1) return VAL_DECIMAL(a) >= VAL_DECIMAL(b);
	return VAL_DECIMAL(a) > VAL_DECIMAL(b);
}

/***********************************************************************
**
*/  static void Check_Overflow(REBDEC dval)
/*
***********************************************************************/
{
	if (!FINITE(dval)) Trap0(RE_OVERFLOW);
}


/***********************************************************************
**
*/  static void Binary_To_Decimal(REBVAL *bin, REBVAL *dec)
/*
***********************************************************************/
{
	REBI64 n = 0;
	REBSER *ser = VAL_SERIES(bin);
	REBCNT idx = VAL_INDEX(bin);
	REBCNT len = VAL_LEN(bin);

	if (len > 8) len = 8;

	for (; len; len--, idx++) n = (n << 8) | (REBI64)(GET_ANY_CHAR(ser, idx));

	VAL_SET(dec, REB_DECIMAL);
	VAL_INT64(dec) = n; // aliasing the bits!
}


/***********************************************************************
**
*/  REBTYPE(Decimal)
/*
***********************************************************************/
{
	REBVAL  *val = D_ARG(1);
	REBDEC  d1 = VAL_DECIMAL(val);
	REBVAL  *arg;
	REBDEC  d2;
	REBINT  num;
	REBDEC  exp;
	REBINT  type = 0;

	// all binary actions
	if (IS_BINARY_ACT(action)) {

		arg = D_ARG(2);
		type = VAL_TYPE(arg);
		if (type != REB_DECIMAL && (
				type == REB_PAIR ||
				type == REB_TUPLE ||
				type == REB_MONEY ||
				type == REB_TIME
			) && (
				action == A_ADD ||
				action == A_MULTIPLY
			)
		){
			*D_RET = *D_ARG(2);
			*D_ARG(2) = *D_ARG(1);
			*D_ARG(1) = *D_RET;
			return Value_Dispatch[VAL_TYPE(D_ARG(1))](ds, action);
		}

		// If the type of the second arg is something we can handle:
		if (type == REB_DECIMAL
			|| type == REB_INTEGER
			|| type == REB_PERCENT
			|| type == REB_MONEY
			|| type == REB_CHAR
		){
			if (type == REB_DECIMAL) {
				d2 = VAL_DECIMAL(arg);
			} else if (type == REB_PERCENT) {
				d2 = VAL_DECIMAL(arg);
				if (action == A_DIVIDE) type = REB_DECIMAL;
				else if (!IS_PERCENT(val)) type = VAL_TYPE(val);
			} else if (type == REB_MONEY) {
				VAL_DECI(val) = decimal_to_deci(VAL_DECIMAL(val));
				VAL_SET(val, REB_MONEY);
				return T_Money(ds, action);
			} else if (type == REB_CHAR) {
				d2 = (REBDEC)VAL_CHAR(arg);
				type = REB_DECIMAL;
			} else {
				d2 = (REBDEC)VAL_INT64(arg);
				type = REB_DECIMAL;
			}

			switch (action) {

			case A_ADD:
				d1 += d2;
				goto setDec;

			case A_SUBTRACT:
				d1 -= d2;
				goto setDec;

			case A_MULTIPLY:
				d1 *= d2;
				goto setDec;

			case A_DIVIDE:
			case A_REMAINDER:
				if (d2 == 0.0) Trap0(RE_ZERO_DIVIDE);
				if (action == A_DIVIDE) d1 /= d2;
				else d1 = fmod(d1, d2);
				goto setDec;

			case A_POWER:
				if (d1 == 0) goto setDec;
				if (d2 == 0) {
					d1 = 1.0;
					goto setDec;
				}
				//if (d1 < 0 && d2 < 1 && d2 != -1)
				//  Trap0(RE_POSITIVE);
				d1 = pow(d1, d2);
				goto setDec;

			}
		}
		Trap_Math_Args(VAL_TYPE(val), action);
	}
	else {
		type = VAL_TYPE(val);

		// unary actions
		switch (action) {

		case A_NEGATE:
			d1 = -d1;
			goto setDec;
		case A_ABSOLUTE:
			if (d1 < 0) d1 = -d1;
			goto setDec;
		case A_EVENQ:
		case A_ODDQ:
			d1 = fabs(fmod(d1, 2.0));
			DECIDE((action != A_EVENQ) != ((d1 < 0.5) || (d1 >= 1.5)));

		case A_MAKE:
		case A_TO:
			// MAKE decimal! 2  and  MAKE 1.0 2  formats:
			if (IS_DATATYPE(val)) type = VAL_DATATYPE(val);
			else type = VAL_TYPE(val);

			val = D_ARG(2);

			switch (VAL_TYPE(val)) {

			case REB_DECIMAL:
				d1 = VAL_DECIMAL(val);
				goto setDec;

			case REB_PERCENT:
				d1 = VAL_DECIMAL(val);
				goto setDec;
			
			case REB_INTEGER:
				d1 = (REBDEC)VAL_INT64(val);
				goto setDec;
			
			case REB_MONEY:
				d1 = deci_to_decimal(VAL_DECI(val));
				goto setDec;
			
			case REB_LOGIC:
				d1 = VAL_LOGIC(val) ? 1.0 : 0.0;
				goto setDec;
			
			case REB_CHAR:
				d1 = (REBDEC)VAL_CHAR(val);
				goto setDec;
			
			case REB_TIME:
				d1 = VAL_TIME(val) * NANO;
				break;

			case REB_STRING:
			{
				REBYTE *bp;
				REBCNT len;
				bp = Qualify_String(val, 24, &len, FALSE);
				if (Scan_Decimal(bp, len, D_RET, type != REB_PERCENT)) {
					d1 = VAL_DECIMAL(D_RET);
					if (type == REB_PERCENT) break;
					goto setDec;
				}
				Trap_Make(type, val);
			}
			
			case REB_BINARY:
				Binary_To_Decimal(val, D_RET);
				d1 = VAL_DECIMAL(D_RET);
				break;
			
#ifdef removed
//			case REB_ISSUE:
			{
				REBYTE *bp;
				REBCNT len;
				bp = Qualify_String(val, MAX_HEX_LEN, &len, FALSE);
				if (Scan_Hex(bp, &VAL_INT64(D_RET), len, len) == 0)
					Trap_Make(REB_DECIMAL, val);
				d1 = VAL_DECIMAL(D_RET);
				break;
			}
#endif

			default:
				if (ANY_BLOCK(val) && VAL_BLK_LEN(val) == 2) {
					arg = VAL_BLK_DATA(val);
					if (IS_INTEGER(arg)) d1 = (REBDEC)VAL_INT64(arg);
					else if (IS_DECIMAL(arg) || IS_PERCENT(val)) d1 = VAL_DECIMAL(arg);
					else Trap_Make(REB_DECIMAL, arg);

					if (IS_INTEGER(++arg)) exp = (REBDEC)VAL_INT64(arg);
					else if (IS_DECIMAL(arg) || IS_PERCENT(val)) exp = VAL_DECIMAL(arg);
					else Trap_Make(REB_DECIMAL, arg);
					while (exp >= 1)            // funky. There must be a better way
						exp--, d1 *= 10.0, Check_Overflow(d1);
					while (exp <= -1)
						exp++, d1 /= 10.0;
				} else
					Trap_Make(type, val);
			}

			if (type == REB_PERCENT) d1 /= 100.0;
			goto setDec;

		case A_ROUND:
			arg = D_ARG(3);
			num = Get_Round_Flags(ds);
			if (D_REF(2)) { // to
				if (IS_MONEY(arg)) {
					VAL_DECI(D_RET) = Round_Deci(decimal_to_deci(d1), num, VAL_DECI(arg));
					SET_TYPE(D_RET, REB_MONEY);
					return R_RET;
				}
				if (IS_TIME(arg)) Trap_Arg(arg);

				d1 = Round_Dec(d1, num, Dec64(arg));
				if (IS_INTEGER(arg)) {
					VAL_INT64(D_RET) = (REBI64)d1;
					SET_TYPE(D_RET, REB_INTEGER);
					return R_RET;
				}
				if (IS_PERCENT(arg)) type = REB_PERCENT;
			}
			else
				d1 = Round_Dec(d1, num | 1, type == REB_PERCENT ? 0.01L : 1.0L); // /TO
			goto setDec;

		case A_RANDOM:
			if (D_REF(2)) {
				Set_Random(VAL_INT64(val)); // use IEEE bits
				return R_UNSET;
			}
#ifdef OLD_METHOD
			if (d1 > (double) (((unsigned long) -1)>>1))
				d1 = ((unsigned long) -1)>>1;
			i = (REBINT)d1;
			if (i == 0) goto setDec;
			if (i < 0)  d1 = -1.0 * (1.0 + (REBDEC)(Random_Int((REBOOL)D_REF(3)) % abs(i)));
			else d1 = 1.0 + (REBDEC)(Random_Int((REBOOL)D_REF(3)) % i);
#else
			d1 = Random_Dec(d1, D_REF(3));
#endif
			goto setDec;

		case A_COMPLEMENT:
			SET_INTEGER(D_RET, ~(REBINT)d1);
			return R_RET;
		}
	}
	Trap_Action(VAL_TYPE(val), action);

setDec:
	if (!FINITE(d1)) Trap0(RE_OVERFLOW);
#ifdef not_required
	if (type == REB_PERCENT) {
		// Keep percent in smaller range (not to use e notation).
		if (d1 != 0) {
			num = (REBINT)floor(log10(fabs(d1)));
			if (num > 12 || num < -6) Trap0(RE_OVERFLOW); // use gcvt
		}
	}
#endif
	VAL_SET(D_RET, type);
	VAL_DECIMAL(D_RET) = d1;
	///if (type == REB_MONEY) VAL_MONEY_DENOM(D_RET)[0] = 0;
	return R_RET;

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}
