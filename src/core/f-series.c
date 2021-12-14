/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2021 Rebol Open Source Contributors
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
**  Module:  f-series.c
**  Summary: common series handling functions
**  Section: functional
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#define THE_SIGN(v) ((v < 0) ? -1 : (v > 0) ? 1 : 0)

/***********************************************************************
**
*/	REBINT Do_Series_Action(REBCNT action, REBVAL *value, REBVAL *arg)
/*
**		Common series functions.
**
***********************************************************************/
{
	REBINT	index;
	REBINT	tail;
	REBINT	len = 0;

	// Common setup code for all actions:
	if (action != A_MAKE && action != A_TO) {
		index = (REBINT)VAL_INDEX(value);
		tail  = (REBINT)VAL_TAIL(value);
	} else return -1;

	switch (action) {

	//-- Navigation:

	case A_HEAD:
		VAL_INDEX(value) = 0;
		break;

	case A_TAIL:
		VAL_INDEX(value) = (REBCNT)tail;
		break;

	case A_HEADQ:
		DECIDE(index == 0);

	case A_TAILQ:
		DECIDE(index >= tail);

	case A_PASTQ:
		DECIDE(index > tail);

	case A_NEXT:
		if (index < tail) VAL_INDEX(value)++;
		break;

	case A_BACK:
		if (index > 0) VAL_INDEX(value)--;
		break;

	case A_SKIP:
	case A_AT:
	case A_ATZ:
		len = Get_Num_Arg(arg);
		{
			REBI64 i = (REBI64)index + (REBI64)len;
			if (action == A_SKIP) {
				if (IS_LOGIC(arg)) i--;
			} else if (action == A_AT) {
				if (len > 0) i--;
			}
			if (i > (REBI64)tail) i = (REBI64)tail;
			else if (i < 0) i = 0;
			VAL_INDEX(value) = (REBCNT)i;
		}
		break;

	case A_INDEXQ:
		SET_INTEGER(DS_RETURN, ((REBI64)index) + 1);
		return R_RET;

	case A_INDEXZQ:
		SET_INTEGER(DS_RETURN, ((REBI64)index));
		return R_RET;

	case A_LENGTHQ:
		SET_INTEGER(DS_RETURN, tail > index ? tail - index : 0);
		return R_RET;

	case A_REMOVE:
		// /PART length
		TRAP_PROTECT(VAL_SERIES(value));
		if (DS_REF(ARG_REMOVE_KEY)) {
			if (ANY_BLOCK(value)) {
				len = 2;
				index = Find_Block(VAL_SERIES(value), VAL_INDEX(value), VAL_TAIL(value), DS_ARG(ARG_REMOVE_KEY_ARG), VAL_LEN(value), AM_FIND_CASE, 2);
			}
			else Trap0(RE_FEATURE_NA);
		} else {
			len = DS_REF(2) ? Partial(value, 0, DS_ARG(3), 0) : 1;
			index = (REBINT)VAL_INDEX(value);
		}
		if (index < tail && len != 0)
			Remove_Series(VAL_SERIES(value), (REBCNT)index, len);
		break;

	case A_ADD:			// Join_Strings(value, arg);
	case A_SUBTRACT:	// "test this" - 10
	case A_MULTIPLY:	// "t" * 4 = "tttt"
	case A_DIVIDE:
		if (IS_VECTOR(value)) return -1; // allow vector for actions above
		//continue...
	case A_REMAINDER:
	case A_POWER:
	case A_ODDQ:
	case A_EVENQ:
	case A_ABSOLUTE:
		Trap_Action(VAL_TYPE(value), action);

	default:
		return -1;
	}

	DS_RET_VALUE(value);
	return R_RET;

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}


/***********************************************************************
**
*/	REBINT Cmp_Block(REBVAL *sval, REBVAL *tval, REBFLG is_case)
/*
**		Compare two blocks and return the difference of the first
**		non-matching value.
**
***********************************************************************/
{
	REBVAL	*s;
	REBVAL	*t;
	REBINT	diff;

	if ((VAL_SERIES(sval)==VAL_SERIES(tval))&&
	 (VAL_INDEX(sval)==VAL_INDEX(tval)))
		 return 0;

	// make sure that none of block is past tail
	// https://github.com/Oldes/Rebol-issues/issues/2439
	s = VAL_BLK_DATA_SAFE(sval);
	t = VAL_BLK_DATA_SAFE(tval);

	CHECK_STACK(&s);

	while (!IS_END(s) && (VAL_TYPE(s) == VAL_TYPE(t) ||
					(IS_NUMBER(s) && IS_NUMBER(t)))) {
		if ((diff = Cmp_Value(s, t, is_case)) != 0)
			return diff;
		s++, t++;
	}
	return VAL_TYPE(s) - VAL_TYPE(t);
}


/***********************************************************************
**
*/	REBINT Cmp_Value(REBVAL *s, REBVAL *t, REBFLG is_case)
/*
**		Compare two values and return the difference.
**
**		is_case TRUE for case sensitive compare
**
***********************************************************************/
{
	REBDEC	d1, d2;

	if (VAL_TYPE(t) != VAL_TYPE(s) && !(IS_NUMBER(s) && IS_NUMBER(t)))
		return VAL_TYPE(s) - VAL_TYPE(t);

	switch(VAL_TYPE(s)) {

	case REB_INTEGER:
		if (IS_DECIMAL(t)) {
			d1 = (REBDEC)VAL_INT64(s);
			d2 = VAL_DECIMAL(t);
			goto chkDecimal;
		}
		return THE_SIGN(VAL_INT64(s) - VAL_INT64(t));

	case REB_LOGIC:
		return VAL_LOGIC(s) - VAL_LOGIC(t);

	case REB_CHAR:
		if (is_case) return THE_SIGN(VAL_CHAR(s) - VAL_CHAR(t));
		return THE_SIGN((REBINT)(UP_CASE(VAL_CHAR(s)) - UP_CASE(VAL_CHAR(t))));

	case REB_DECIMAL:
	case REB_MONEY:
			d1 = VAL_DECIMAL(s);
		if (IS_INTEGER(t))
			d2 = (REBDEC)VAL_INT64(t);
		else
			d2 = VAL_DECIMAL(t);
chkDecimal:
		if (Eq_Decimal(d1, d2))
			return 0;
		if (d1 < d2)
			return -1;
		return 1;

	case REB_PAIR:
		return Cmp_Pair(s, t);

	case REB_EVENT:
		return Cmp_Event(s, t);

	case REB_GOB:
		return Cmp_Gob(s, t);

	case REB_TUPLE:
		return Cmp_Tuple(s, t);

	case REB_TIME:
		return Cmp_Time(s, t);

	case REB_DATE:
		return Cmp_Date(s, t);

	case REB_BLOCK:
	case REB_PAREN:
	case REB_MAP:
	case REB_PATH:
	case REB_SET_PATH:
	case REB_GET_PATH:
	case REB_LIT_PATH:
		return Cmp_Block(s, t, is_case);

	case REB_STRING:
	case REB_FILE:
	case REB_EMAIL:
	case REB_URL:
	case REB_TAG:
	case REB_REF:
		return Compare_String_Vals(s, t, (REBOOL)!is_case);

	case REB_BITSET:
	case REB_BINARY:
	case REB_IMAGE:
		return Compare_Binary_Vals(s, t);

	case REB_VECTOR:
		return Compare_Vector(s, t);

	case REB_DATATYPE:
		return VAL_DATATYPE(s) - VAL_DATATYPE(t);

	case REB_WORD:
	case REB_SET_WORD:
	case REB_GET_WORD:
	case REB_LIT_WORD:
	case REB_REFINEMENT:
	case REB_ISSUE:
		return Compare_Word(s,t,is_case);

	case REB_ERROR:
		return VAL_ERR_NUM(s) - VAL_ERR_NUM(s);

	case REB_OBJECT:
	case REB_MODULE:
	case REB_PORT:
		//return VAL_OBJ_FRAME(s) - VAL_OBJ_FRAME(t); // strict variant
		return !CT_Object(s, t, 1); // equality used

	case REB_NATIVE:
		return &VAL_FUNC_CODE(s) - &VAL_FUNC_CODE(t);

	case REB_ACTION:
	case REB_COMMAND:
	case REB_OP:
	case REB_FUNCTION:
		return VAL_FUNC_BODY(s) - VAL_FUNC_BODY(t);

	case REB_STRUCT:
		return Cmp_Struct(s, t);

	case REB_HANDLE:
		return Cmp_Handle(s, t);

	case REB_NONE:
	case REB_UNSET:
	case REB_END:
	default:
		break;

	}
	return 0;
}


/***********************************************************************
**
*/	REBCNT Find_Block_Simple(REBSER *series, REBCNT index, REBVAL *target)
/*
**		Simple search for a value in a block. Return the index of
**		the value or the TAIL index if not found.
**
***********************************************************************/
{
	REBVAL *value = BLK_HEAD(series);

	for (; index < SERIES_TAIL(series); index++) {
		if (0 == Cmp_Value(value+index, target, FALSE)) return index;
	}

	return SERIES_TAIL(series);
}

/***********************************************************************
**
*/	REBNATIVE(pickz)
/*
//	pickz: native [
//		{Returns the value at the specified position. (0-based wrapper over PICK action)}
//		aggregate [series! bitset! tuple!]
//		index [integer!] "Zero based"
]
***********************************************************************/
{
	if(VAL_INT64(D_ARG(2))>=0) VAL_INT64(D_ARG(2)) += 1;
	Do_Act(D_RET, VAL_TYPE(D_ARG(1)), A_PICK);
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(pokez)
/*
//	pokez: native [
//		{Replaces an element at a given position. (0-based wrapper over POKE action)}
//		series [series! bitset! tuple!] "(modified)"
//		index  [integer!]  "Zero based"
//		value  [any-type!] "The new value (returned)"
]
***********************************************************************/
{
	if (VAL_INT64(D_ARG(2)) >= 0) VAL_INT64(D_ARG(2)) += 1;
	Do_Act(D_RET, VAL_TYPE(D_ARG(1)), A_POKE);
	return R_RET;
}
