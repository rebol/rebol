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
**  Module:  t-integer.c
**  Summary: integer datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-deci-funcs.h"


/***********************************************************************
**
*/	REBINT CT_Integer(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode >= 0)  return (VAL_INT64(a) == VAL_INT64(b));
	if (mode == -1) return (VAL_INT64(a) >= VAL_INT64(b));
	return (VAL_INT64(a) > VAL_INT64(b));
}


/***********************************************************************
**
*/	REBTYPE(Integer)
/*
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBVAL *val2 = D_ARG(2);
	REBI64 num;
	REBI64 arg;
	REBINT n;

	REBU64 p, a, b; // for overflow detection
	REBCNT a1, a0, b1, b0;
	REBFLG sgn;
	REBI64 anum;

	num = VAL_INT64(val);

	if (IS_BINARY_ACT(action)) {

		if (IS_INTEGER(val2)) arg = VAL_INT64(val2);
		else if (IS_CHAR(val2)) arg = VAL_CHAR(val2);
		else {
			// Decimal or other numeric second argument:
			n = 0; // use to flag special case
			switch(action) {
			// Anything added to an integer is same as adding the integer:
			case A_ADD:
			case A_MULTIPLY:
				// Swap parameter order:
				*D_RET = *val2;  // Use as temp workspace
				*val2 = *val;
				*val = *D_RET;
				return Value_Dispatch[VAL_TYPE(val)](ds, action);

			// Only type valid to subtract from, divide into, is decimal/money:
			case A_SUBTRACT:
				n = 1;
			case A_DIVIDE:
			case A_REMAINDER:
			case A_POWER:
				if (IS_DECIMAL(val2) | IS_PERCENT(val2)) {
					SET_DECIMAL(val, (REBDEC)num); // convert main arg
					return T_Decimal(ds, action);
				}
				if (IS_MONEY(val2)) {
					VAL_DECI(val) = int_to_deci(VAL_INT64(val));
					VAL_SET(val, REB_MONEY);
					return T_Money(ds, action);
				}
				if (n > 0) {
					if (IS_TIME(val2)) {
						VAL_TIME(val) = SEC_TIME(VAL_INT64(val));
						SET_TYPE(val, REB_TIME);
						return T_Time(ds, action);
					}
					if (IS_DATE(val2)) return T_Date(ds, action);
				}
			}
			Trap_Math_Args(REB_INTEGER, action);
		}
	}

	switch (action) {

	case A_ADD:
		anum = (REBU64)num + (REBU64)arg;
		if (
			((num < 0) == (arg < 0)) && ((num < 0) != (anum < 0))
		) Trap0(RE_OVERFLOW);
		num = anum;
		break;

	case A_SUBTRACT:
		anum = (REBU64)num - (REBU64)arg;
		if (
			((num < 0) != (arg < 0)) && ((num < 0) != (anum < 0))
		) Trap0(RE_OVERFLOW);
		num = anum;
		break;

	case A_MULTIPLY:
		a = num;
		sgn = (num < 0);
		if (sgn) a = -a;
		b = arg;
		if (arg < 0) {
			sgn = !sgn;
			b = -b;
		}
		p = a * b;
		a1 = a>>32;
		a0 = a;
		b1 = b>>32;
		b0 = b;
		if (
			(a1 && b1)
			|| ((REBU64)a0 * b1 + (REBU64)a1 * b0 > p >> 32)
			|| ((p > (REBU64)MAX_I64) && (!sgn || (p > -(REBU64)MIN_I64)))
		) Trap0(RE_OVERFLOW);
		num = sgn ? -p : p;
		break;

	case A_DIVIDE:
		if (arg == 0) Trap0(RE_ZERO_DIVIDE);
		if (num == MIN_I64 && arg == -1) Trap0(RE_OVERFLOW);
		if (num % arg == 0) {
			num = num / arg;
			break;
		}
		// Fall thru

	case A_POWER:
		SET_DECIMAL(val, (REBDEC)num);
		SET_DECIMAL(val2, (REBDEC)arg);
		return T_Decimal(ds, action);

	case A_REMAINDER:
		if (arg == 0) Trap0(RE_ZERO_DIVIDE);
		num = REM2(num, arg);
		break;

	case A_AND: num &= arg; break;
	case A_OR:  num |= arg; break;
	case A_XOR: num ^= arg; break;

	case A_NEGATE:
		if (num == MIN_I64) Trap0(RE_OVERFLOW);
		num = -num;
		break;

	case A_COMPLEMENT: num = ~num; break;

	case A_ABSOLUTE: 
		if (num == MIN_I64) Trap0(RE_OVERFLOW);
		if (num < 0) num = -num;
		break;

	case A_EVENQ: num = ~num;
	case A_ODDQ: DECIDE(num & 1);

	case A_ROUND:
		val2 = D_ARG(3);
		n = Get_Round_Flags(ds);
		if (D_REF(2)) { // to
			if (IS_MONEY(val2)) {
				VAL_DECI(D_RET) = Round_Deci(int_to_deci(num), n, VAL_DECI(val2));
				SET_TYPE(D_RET, REB_MONEY);
				return R_RET;
			}
			if (IS_DECIMAL(val2) || IS_PERCENT(val2)) {
				VAL_DECIMAL(D_RET) = Round_Dec((REBDEC)num, n, VAL_DECIMAL(val2));
				SET_TYPE(D_RET, VAL_TYPE(val2));
				return R_RET;
			}
			if (IS_TIME(val2)) Trap_Arg(val2);
			arg = VAL_INT64(val2);
		}
		else arg = 0L;
		num = Round_Int(num, n, arg);
		break;

	case A_RANDOM:
		if (D_REF(2)) { // seed
			Set_Random(num);
			return R_UNSET;
		}
		if (num == 0) break;
		num = Random_Range(num, (REBOOL)D_REF(3));	//!!! 64 bits
#ifdef OLD_METHOD
		if (num < 0)  num = -(1 + (REBI64)(arg % -num));
		else	      num =   1 + (REBI64)(arg % num);
#endif
		break;

	case A_MAKE:
	case A_TO:
		val = D_ARG(2);
		if (IS_DECIMAL(val) || IS_PERCENT(val)) {
			if (VAL_DECIMAL(val) < MIN_D64 || VAL_DECIMAL(val) >= MAX_D64)
				Trap0(RE_OVERFLOW);
			num = (REBI64)VAL_DECIMAL(val);
		}
		else if (IS_INTEGER(val))
			num = VAL_INT64(val);
		else if (IS_MONEY(val))
			num = deci_to_int(VAL_DECI(val));
		else if (IS_ISSUE(val)) {
			REBYTE *bp;
			REBCNT len;
			bp = Get_Word_Name(val);
			len = strlen(bp);
			n = MIN(MAX_HEX_LEN, len);
			if (Scan_Hex(bp, &num, n, n) == 0) goto is_bad;
		}
		else if (IS_BINARY(val)) { // must be before STRING!
			REBYTE	*bp;
			n = VAL_LEN(val);
			if (n > sizeof(REBI64)) n = sizeof(REBI64);
			num = 0;
			for (bp = VAL_BIN_DATA(val); n; n--, bp++)
				num = (num << 8) | *bp;
		}
		else if (ANY_STR(val)) {
			REBYTE *bp;
			REBCNT len;
			bp = Qualify_String(val, MAX_INT_LEN, &len, FALSE);
			if (memchr(bp, '.', len)) {
				if (Scan_Decimal(bp, len, DS_RETURN, TRUE)) {
					num = (REBINT)VAL_DECIMAL(DS_RETURN);
					break;
				}
			}
			if (Scan_Integer(bp, len, DS_RETURN))
				return R_RET;
			goto is_bad;
		}
		else if (IS_LOGIC(val)) {
			// No integer is uniquely representative of true, so TO conversions reject
			// integer-to-logic conversions.  MAKE is more liberal and constructs true
			// to 1 and false to 0.
			if (action != A_MAKE) goto is_bad;
			num = VAL_LOGIC(val) ? 1 : 0;
		}
		else if (IS_CHAR(val))
			num = VAL_CHAR(val);
		// else if (IS_NONE(val)) num = 0;
		else if (IS_TIME (val)) num = SECS_IN(VAL_TIME(val));
		else goto is_bad;
		break;

	default:
		Trap_Action(REB_INTEGER, action);
	}

	SET_INTEGER(DS_RETURN, num);
	return R_RET;

is_bad:
	Trap_Make(REB_INTEGER, val);

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}
