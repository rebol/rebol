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
**  Module:  t-typeset.c
**  Summary: typeset datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	const REBU64 Typesets[] =
/*
**		Order of symbols is important- used below for Make_Typeset().
**
************************************************************************/
{
	1, 0, // First (0th) typeset is not valid
	SYM_ANY_TYPEX,     ((REBU64)1<<REB_MAX)-2, // do not include END!
   	SYM_ANY_WORDX,     TS_WORD,
	SYM_ANY_PATHX,     TS_PATH,
   	SYM_ANY_FUNCTIONX, TS_FUNCTION,
   	SYM_NUMBERX,       TS_NUMBER,
   	SYM_SCALARX,       TS_SCALAR,
   	SYM_SERIESX,       TS_SERIES,
   	SYM_ANY_STRINGX,   TS_STRING,
   	SYM_ANY_OBJECTX,   TS_OBJECT,
   	SYM_ANY_BLOCKX,    TS_BLOCK,
	0, 0
};


/***********************************************************************
**
*/	REBINT CT_Typeset(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode < 0) return -1;
	return EQUAL_TYPESET(a, b);
}


/***********************************************************************
**
*/	void Init_Typesets(void)
/*
**		Create typeset variables that are defined above.
**		For example: NUMBER is both integer and decimal.
**		Add the new variables to the system context.
**
***********************************************************************/
{
	REBVAL *value;
	REBINT n;
	
	Set_Root_Series(ROOT_TYPESETS, Make_Block(40), "typeset presets");

	for (n = 0; Typesets[n]; n += 2) {
		value = Append_Value(VAL_SERIES(ROOT_TYPESETS));
		VAL_SET(value, REB_TYPESET);
		VAL_TYPESET(value) = Typesets[n+1];
		if (Typesets[n] > 1)
			*Append_Frame(Lib_Context, 0, (REBCNT)(Typesets[n])) = *value;
	}
}


/***********************************************************************
**
*/	REBFLG Make_Typeset(REBVAL *block, REBVAL *value, REBFLG load)
/*
**		block - block of datatypes (datatype words ok too)
**		value - value to hold result (can be word-spec type too)
**
***********************************************************************/
{
	REBVAL *val;
	REBCNT sym;
	REBSER *types = VAL_SERIES(ROOT_TYPESETS);

	VAL_TYPESET(value) = 0;

	for (; NOT_END(block); block++) {
		val = 0;
		if (IS_WORD(block)) {
			//Print("word: %s", Get_Word_Name(block));
			sym = VAL_WORD_SYM(block);
			if (VAL_WORD_FRAME(block)) { // Get word value
				val = Get_Var(block);
			} else if (sym < REB_MAX) { // Accept datatype word
				TYPE_SET(value, VAL_WORD_SYM(block)-1);
				continue;
			} // Special typeset symbols:
			else if (sym >= SYM_ANY_TYPEX && sym <= SYM_ANY_BLOCKX)
				val = BLK_SKIP(types, sym - SYM_ANY_TYPEX + 1);
		}
		if (!val) val = block;
		if (IS_DATATYPE(val)) {
			TYPE_SET(value, VAL_DATATYPE(val));
		} else if (IS_TYPESET(val)) {
			VAL_TYPESET(value) |= VAL_TYPESET(val);
		} else {
			if (load) return FALSE;
			Trap_Arg(block);
		}
	}

	return TRUE;
}


/***********************************************************************
**
*/	REBFLG MT_Typeset(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	if (!IS_BLOCK(data)) return FALSE;

	if (!Make_Typeset(VAL_BLK(data), out, TRUE)) return FALSE;
	VAL_SET(out, REB_TYPESET);

	return TRUE;
}


/***********************************************************************
**
*/	REBINT Find_Typeset(REBVAL *block)
/*
***********************************************************************/
{
	REBVAL value;
	REBVAL *val;
	REBINT n;

	VAL_SET(&value, REB_TYPESET);
	Make_Typeset(block, &value, 0);

	val = VAL_BLK_SKIP(ROOT_TYPESETS, 1);

	for (n = 1; NOT_END(val); val++, n++) {
		if (EQUAL_TYPESET(&value, val)){
			//Print("FTS: %d", n);
			return n;
		}
	}

//	Print("Size Typesets: %d", VAL_TAIL(ROOT_TYPESETS));
	Append_Val(VAL_SERIES(ROOT_TYPESETS), &value);
	return n;
}


/***********************************************************************
**
*/	REBSER *Typeset_To_Block(REBVAL *tset)
/*
**		Converts typeset value to a block of datatypes.
**		No order is specified.
**
***********************************************************************/
{
	REBSER *block;
	REBVAL *value;
	REBINT n;
	REBINT size = 0;

	for (n = 0; n < REB_MAX; n++) {
		if (TYPE_CHECK(tset, n)) size++;
	}

	block = Make_Block(size);

	// Convert bits to types:
	for (n = 0; n < REB_MAX; n++) {
		if (TYPE_CHECK(tset, n)) {
			value = Append_Value(block);
			Set_Datatype(value, n);
		}
	}
	return block;
}


/***********************************************************************
**
*/	REBTYPE(Typeset)
/*
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBVAL *arg = D_ARG(2);

	switch (action) {

	case A_FIND:
		if (IS_DATATYPE(arg)) {
			DECIDE(TYPE_CHECK(val, VAL_DATATYPE(arg)));
		}
		Trap_Arg(arg);

	case A_MAKE:
	case A_TO:
		if (IS_BLOCK(arg)) {
			VAL_SET(D_RET, REB_TYPESET);
			Make_Typeset(VAL_BLK_DATA(arg), D_RET, 0);
			return R_RET;
		}
	//	if (IS_NONE(arg)) {
	//		VAL_SET(arg, REB_TYPESET);
	//		VAL_TYPESET(arg) = 0L;
	//		return R_ARG2;
	//	}
		if (IS_TYPESET(arg)) return R_ARG2;
		Trap_Make(REB_TYPESET, arg);

	case A_AND:
	case A_OR:
	case A_XOR:
		if (IS_DATATYPE(arg)) VAL_TYPESET(arg) = TYPESET(VAL_DATATYPE(arg));
		else if (!IS_TYPESET(arg)) Trap_Arg(arg);

		if (action == A_OR) VAL_TYPESET(val) |= VAL_TYPESET(arg);
		else if (action == A_AND) VAL_TYPESET(val) &= VAL_TYPESET(arg);
		else VAL_TYPESET(val) ^= VAL_TYPESET(arg);
		return R_ARG1;

	case A_COMPLEMENT:
		VAL_TYPESET(val) = ~VAL_TYPESET(val);
		return R_ARG1;

	default:
		Trap_Action(REB_TYPESET, action);
	}

is_true:
	return R_TRUE;

is_false:
	return R_FALSE;
}
