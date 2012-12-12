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
**  Module:  t-char.c
**  Summary: character datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	REBINT CT_Char(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT num;
	
	if (mode >= 0) {
		if (mode < 2)
			num = LO_CASE(VAL_CHAR(a)) - LO_CASE(VAL_CHAR(b));
		else
			num = VAL_CHAR(a) - VAL_CHAR(b);
		return (num == 0);
	}

	num = VAL_CHAR(a) - VAL_CHAR(b);
	if (mode == -1) return (num >= 0);
	return (num > 0);
}


/***********************************************************************
**
*/	REBTYPE(Char)
/*
***********************************************************************/
{
	REBINT	chr = VAL_CHAR(D_ARG(1));
	REBINT	arg;
	REBVAL	*val;

	if (IS_BINARY_ACT(action)) {
		val = D_ARG(2);
		if (IS_CHAR(val))
			arg = VAL_CHAR(val);
		else if (IS_INTEGER(val))
			arg = VAL_INT32(val);
		else if (IS_DECIMAL(val))
			arg = (REBINT)VAL_DECIMAL(val);
		else
			Trap_Math_Args(REB_CHAR, action);
	}

	switch (action) {

	case A_ADD: chr += (REBUNI)arg; break;
	case A_SUBTRACT:
		chr -= (REBUNI)arg;
		if (IS_CHAR(D_ARG(2))) {
			DS_RET_INT(chr);
			return R_RET;
		}
		break;
	case A_MULTIPLY: chr *= arg; break;
	case A_DIVIDE:
		if (arg == 0) Trap0(RE_ZERO_DIVIDE);
		chr /= arg;
		break;
	case A_REMAINDER:
		if (arg == 0) Trap0(RE_ZERO_DIVIDE);
		chr %= arg;
		break;

	case A_AND: chr &= (REBUNI)arg; break;
	case A_OR:  chr |= (REBUNI)arg; break;
	case A_XOR: chr ^= (REBUNI)arg; break;

	case A_NEGATE: chr = (REBUNI)-chr; break;
	case A_COMPLEMENT: chr = (REBUNI)~chr; break;
	case A_EVENQ: chr = (REBUNI)~chr;
	case A_ODDQ: DECIDE(chr & 1);

	case A_RANDOM:	//!!! needs further definition ?  random/zero
		if (D_REF(2)) { // /seed
			Set_Random(chr);
			return R_UNSET;
		}
		if (chr == 0) break;
		chr = (REBUNI)(1 + ((REBCNT)Random_Int(D_REF(3)) % chr)); // /secure
		break;

	case A_MAKE:
	case A_TO:
		val = D_ARG(2);

		switch(VAL_TYPE(val)) {
		case REB_CHAR:
			chr = VAL_CHAR(val);
			break;

		case REB_INTEGER:
		case REB_DECIMAL:
			arg = Int32(val);
			if (arg > MAX_UNI || arg < 0) goto bad_make;
			chr = arg;
			break;
	
		case REB_BINARY:
		{
			REBYTE *bp = VAL_BIN(val);
			arg = VAL_LEN(val);
			if (arg == 0) goto bad_make;
			if (*bp > 0x80) {
				if (!Legal_UTF8_Char(bp, arg)) goto bad_make;
				chr = Decode_UTF8_Char(&bp, 0); // zero on error
				if (!chr) goto bad_make;
			}
			else
				chr = *bp;
		}
			break;

#ifdef removed
//		case REB_ISSUE:
			// Scan 8 or 16 bit hex str, will throw on error...
			arg = Scan_Hex_Value(VAL_DATA(val), VAL_LEN(val), (REBOOL)!VAL_BYTE_SIZE(val));
			if (arg > MAX_UNI || arg < 0) goto bad_make;
			chr = arg;
			break;
#endif

		case REB_STRING:
			if (VAL_INDEX(val) >= VAL_TAIL(val)) Trap_Make(REB_CHAR, val);
			chr = GET_ANY_CHAR(VAL_SERIES(val), VAL_INDEX(val));
			break;

		default:
bad_make:
		Trap_Make(REB_CHAR, val);
	}
		break;
		
	default:
		Trap_Action(REB_CHAR, action);
	}

	if ((chr >> 16) != 0 && (chr >> 16) != 0xffff) Trap1(RE_TYPE_LIMIT, Get_Type(REB_CHAR)); 
	SET_CHAR(DS_RETURN, chr);
	return R_RET;

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}

