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
**  Module:  n-sets.c
**  Summary: native functions for data sets
**  Section: natives
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

enum {
	SOP_BOTH,		// combine and interate over both series
	SOP_CHECK,		// check other series for value existence
	SOP_INVERT,		// invert the result of the search
};

#define SET_OP_UNIQUE		0
#define SET_OP_UNION		FLAGIT(SOP_BOTH)
#define SET_OP_INTERSECT	FLAGIT(SOP_CHECK)
#define SET_OP_EXCLUDE		(FLAGIT(SOP_CHECK) | FLAGIT(SOP_INVERT))
#define SET_OP_DIFFERENCE	(FLAGIT(SOP_BOTH) | FLAGIT(SOP_CHECK) | FLAGIT(SOP_INVERT))


/***********************************************************************
**
*/	static REBINT Do_Set_Operation(REBVAL *ds, REBCNT flags)
/*
**		Do set operations on a series.
**
***********************************************************************/
{
	REBVAL *val;
	REBVAL *val1;
	REBVAL *val2 = 0;
	REBSER *ser;
	REBSER *hser = 0;	// hash table for series
	REBSER *retser;		// return series
	REBSER *hret;		// hash table for return series
	REBCNT i;
	REBINT h = TRUE;
	REBCNT skip = 1;	// record size
	REBCNT cased = 0;	// case sensitive when TRUE

	SET_NONE(D_RET);
	val1 = D_ARG(1);
	i = 2;

	// Check for second series argument:
	if (flags != SET_OP_UNIQUE) {
		val2 = D_ARG(i++);
		if (VAL_TYPE(val1) != VAL_TYPE(val2)) {
			Trap_Types(RE_EXPECT_VAL, VAL_TYPE(val1), VAL_TYPE(val2));
		}
	}

	// Refinements /case and /skip N
	cased = D_REF(i++); // cased
	if (D_REF(i++)) skip = Int32s(D_ARG(i), 1);

	switch (VAL_TYPE(val1)) {

	case REB_BLOCK:
		i = VAL_LEN(val1);
		// Setup result block:
		if (GET_FLAG(flags, SOP_BOTH)) i += VAL_LEN(val2);
		retser = BUF_EMIT;			// use preallocated shared block
		Resize_Series(retser, i);
		hret = Make_Hash_Array(i);	// allocated

		// Optimization note: !!
		// This code could be optimized for small blocks by not hashing them
		// and extending Find_Key to do a FIND on the value itself w/o the hash.

		do {
			// Check what is in series1 but not in series2:
			if (GET_FLAG(flags, SOP_CHECK))
				hser = Hash_Block(val2, cased);

			// Iterate over first series:
			ser = VAL_SERIES(val1);
			i = VAL_INDEX(val1);
			FOR_SER(ser, val, i, skip) {
				if (GET_FLAG(flags, SOP_CHECK)) {
					h = Find_Key(VAL_SERIES(val2), hser, val, skip, cased, 1) >= 0;
					if (GET_FLAG(flags, SOP_INVERT)) h = !h;
				}
				if (h) Find_Key(retser, hret, val, skip, cased, 2);
			}

			// Iterate over second series?
			if (NZ(i = GET_FLAG(flags, SOP_BOTH))) {
				val = val1;
				val1 = val2;
				val2 = val;
				CLR_FLAG(flags, SOP_BOTH);
			}
		} while (i);

		Set_Block(D_RET, Copy_Series(retser));
		RESET_TAIL(retser); // required - allow reuse

		break;

	case REB_BINARY:
		cased = TRUE;
		SET_TYPE(D_RET, REB_BINARY);
	case REB_STRING:
		i = VAL_LEN(val1);
		// Setup result block:
		if (GET_FLAG(flags, SOP_BOTH)) i += VAL_LEN(val2);

		retser = BUF_MOLD;
		Reset_Buffer(retser, i);
		RESET_TAIL(retser);

		do {
			REBUNI uc;

			cased = cased ? AM_FIND_CASE : 0;

			// Iterate over first series:
			ser = VAL_SERIES(val1);
			i = VAL_INDEX(val1);
			FOR_SER(ser, val, i, skip) {
				uc = GET_ANY_CHAR(ser, i);
				if (GET_FLAG(flags, SOP_CHECK)) {
					h = Find_Str_Char(VAL_SERIES(val2), 0, VAL_INDEX(val2), VAL_TAIL(val2), skip, uc, cased) != NOT_FOUND;
					if (GET_FLAG(flags, SOP_INVERT)) h = !h;
				}
				if (h && (Find_Str_Char(retser, 0, 0, SERIES_TAIL(retser), skip, uc, cased) == NOT_FOUND)) {
					Append_String(retser, ser, i, skip);
				}
			}

			// Iterate over second series?
			if (NZ(i = GET_FLAG(flags, SOP_BOTH))) {
				val = val1;
				val1 = val2;
				val2 = val;
				CLR_FLAG(flags, SOP_BOTH);
			}
		} while (i);

		ser = Copy_String(retser, 0, -1);
		if (IS_BINARY(D_RET))
			Set_Binary(D_RET, ser);
		else
			Set_String(D_RET, ser);
		break;

	case REB_BITSET:
		switch (flags) {
		case SET_OP_UNIQUE:
			return R_ARG1;
		case SET_OP_UNION:
			i = A_OR;
			break;
		case SET_OP_INTERSECT:
			i = A_AND;
			break;
		case SET_OP_DIFFERENCE:
			i = A_XOR;
			break;
		case SET_OP_EXCLUDE:
			i = 0; // special case
			break;
		}
		ser = Xandor_Binary(i, val1, val2);
		Set_Series(REB_BITSET, D_RET, ser);
		break;

	case REB_TYPESET:
		switch (flags) {
		case SET_OP_UNIQUE:
			break;
		case SET_OP_UNION:
			VAL_TYPESET(val1) |= VAL_TYPESET(val2);
			break;
		case SET_OP_INTERSECT:
			VAL_TYPESET(val1) &= VAL_TYPESET(val2);
			break;
		case SET_OP_DIFFERENCE:
			VAL_TYPESET(val1) ^= VAL_TYPESET(val2);
			break;
		case SET_OP_EXCLUDE:
			VAL_TYPESET(val1) &= ~VAL_TYPESET(val2);
			break;
		}
		return R_ARG1;

	default:
		Trap_Arg(val1);
	}

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(difference)
/*
**	Set functions use this arg pattern:
**
**		set1 [ series! bitset! date! ] "first set"
**		set2 [ series! bitset! date! ] "second set"
**		/case "case sensitive"
**		/skip "treat the series as records of fixed size"
**		size [integer!]
**
***********************************************************************/
{
	REBVAL *val1, *val2;

	val1 = D_ARG(1);
	val2 = D_ARG(2);

	if (IS_DATE(val1) || IS_DATE(val2)) {
		if (!IS_DATE(val1)) Trap_Arg(val1);
		if (!IS_DATE(val2)) Trap_Arg(val2);
		Subtract_Date(val1, val2, D_RET);
		return R_RET;
	}

	return Do_Set_Operation(ds, SET_OP_DIFFERENCE);
}


/***********************************************************************
**
*/	REBNATIVE(exclude)
/*
***********************************************************************/
{
	return Do_Set_Operation(ds, SET_OP_EXCLUDE);
}


/***********************************************************************
**
*/	REBNATIVE(intersect)
/*
***********************************************************************/
{
	return Do_Set_Operation(ds, SET_OP_INTERSECT);
}


/***********************************************************************
**
*/	REBNATIVE(union)
/*
***********************************************************************/
{
	return Do_Set_Operation(ds, SET_OP_UNION);
}


/***********************************************************************
**
*/	REBNATIVE(unique)
/*
***********************************************************************/
{
	return Do_Set_Operation(ds, SET_OP_UNIQUE);
}


