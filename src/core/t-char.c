/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
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
*/	REBINT PD_Char(REBPVS* pvs)
/*
***********************************************************************/
{
	if (pvs->setval) return PE_BAD_SET;
	if (IS_WORD(pvs->select)) {
		REBU32 len;
		switch (VAL_WORD_CANON(pvs->select)) {
		case SYM_WIDTH: len = UTF8_Width(VAL_CHAR(pvs->value)); break;
		case SYM_SIZE:  len = UTF8_Codepoint_Size(VAL_CHAR(pvs->value)); break;
		default: return PE_BAD_SELECT;
		}
		SET_INTEGER(pvs->store, len);
		return PE_USE;
	}
	else
		return PE_BAD_SELECT;

	return PE_OK;
}


/***********************************************************************
**
*/	REBTYPE(Char)
/*
***********************************************************************/
{
	REBU32	chr = VAL_CHAR(D_ARG(1));
	REBCNT	arg = 0;
	REBVAL	*val;

	if (IS_BINARY_ACT(action)) {
		val = D_ARG(2);
		if (IS_CHAR(val))
			arg = VAL_CHAR(val);
		else if (IS_INTEGER(val))
			arg = VAL_INT32(val);
		else if (IS_DECIMAL(val))
			arg = (REBINT)VAL_DECIMAL(val);
        else {
			Trap_Math_Args(REB_CHAR, action);
        }
	}

	switch (action) {

	case A_ADD: chr += arg; break;
	case A_SUBTRACT:
		chr -= arg;
		if (IS_CHAR(D_ARG(2))) {
			DS_RET_INT(chr);
			return R_RET;
		}
		break;
	case A_MULTIPLY: chr *= arg; break;
	case A_DIVIDE:
		if (arg == 0) Trap0(RE_ZERO_DIVIDE);
		else chr /= arg;
		break;
	case A_REMAINDER:
		if (arg == 0) Trap0(RE_ZERO_DIVIDE);
		else chr %= arg;
		break;

	case A_AND: chr &= arg; break;
	case A_OR:  chr |= arg; break;
	case A_XOR: chr ^= arg; break;

	//case A_NEGATE: chr = -chr; break;
	case A_COMPLEMENT: chr = ~chr; break;
	case A_EVENQ: chr = ~chr;
	case A_ODDQ: DECIDE(chr & 1);

	case A_RANDOM:	//!!! needs further definition ?  random/zero
		if (D_REF(2)) { // /seed
			Set_Random(chr);
			return R_UNSET;
		}
		if (chr == 0) break;
		do {
			chr = (REBINT)(1 + ((REBCNT)Random_Int(D_REF(3)) % chr)); // /secure
		} while (IS_INVALID_CHAR(chr));
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
			if (arg < 0) goto bad_make;
			chr = arg;
			break;
	
		case REB_BINARY:
		{
			const REBYTE *bp = VAL_BIN(val);
			arg = VAL_LEN(val);
			if (arg == 0) goto bad_make;
			if (*bp > 0x80) {
				chr = UTF8_Decode_Codepoint(&bp, &arg);
				if (chr == UNI_ERROR) goto bad_make;
			}
			else
				chr = *bp;
		}
			break;

		case REB_STRING:
			if (VAL_INDEX(val) >= VAL_TAIL(val)) Trap_Make(REB_CHAR, val);
			chr = GET_ANY_CHAR(VAL_SERIES(val), VAL_INDEX(val));
			break;

		case REB_ISSUE:
		{
			const REBYTE* bp = Get_Word_Name(val);
			REBCNT len = LEN_BYTES(bp);
			REBINT n = MIN(MAX_HEX_LEN, len);
			REBI64 num;
			// Scan 8 or 16 bit hex str, will throw on error...
			if (Scan_Hex(bp, &num, n, n) == 0) goto bad_make;
			if (num > MAX_UNI || num < 0) goto bad_make;
			chr = (REBU32)num;
			break;
		}

		default:
bad_make:
		Trap_Make(REB_CHAR, val);
	}
		break;
		
	default:
		Trap_Action(REB_CHAR, action);
	}
	if (IS_INVALID_CHAR(chr)) {
		SET_INTEGER(DS_RETURN, chr);
		Trap1(RE_INVALID_CHAR, DS_RETURN);
	}
	SET_CHAR(DS_RETURN, chr);
	return R_RET;

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}

