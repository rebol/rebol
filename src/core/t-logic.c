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
**  Module:  t-logic.c
**  Summary: logic datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
/*
**  Symbolic bit logic was experimental - but proved not to add much
**  value because the overhead of access offset the savings of storage.
**  It would be better to add a general purpose bit parsing dialect,
**  somewhat similar to R2's struct datatype.
*/

#include "sys-core.h"
#include "sys-deci-funcs.h"


/***********************************************************************
**
*/	REBINT CT_Logic(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode >= 0)  return (VAL_LOGIC(a) == VAL_LOGIC(b));
	return -1;
}


/***********************************************************************
**
*/	REBFLG MT_Logic(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	if (!IS_INTEGER(data)) return FALSE;
	SET_LOGIC(out, VAL_INT64(data) != 0);
	return TRUE;
}

#ifdef removed
static int find_word(REBVAL *val, REBVAL *word)
{
	REBCNT sym = VAL_WORD_CANON(word);
	REBINT n;

	for (n = 0; NOT_END(val+n); n++) {
		if (sym == VAL_WORD_CANON(val+n)) return 1<<n;
	}

	return 0;
}

/***********************************************************************
**
*/	REBINT PD_Logic(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *sel;
	REBVAL *val;
	REBINT i;
	REBINT bit;

	if (IS_WORD(sel = pvs->select)) {
		if (!VAL_LOGIC_WORDS(pvs->value) ||
			!(bit = find_word(BLK_HEAD(VAL_LOGIC_WORDS(pvs->value)), sel)))
			return PE_BAD_SELECT;
	}
	else if (IS_INTEGER(sel)) {
		bit = Int32(sel);
		if (bit < 0 || bit > 31) return PE_BAD_SELECT;
		bit = 1 << bit;
	}
	else
		return PE_BAD_SELECT;

	if (NZ(val = pvs->setval)) {
		if (IS_LOGIC(val)) i = VAL_LOGIC(val);
		else if (IS_INTEGER(val)) i = Int32(val);
		else return PE_BAD_SET;
		if (i) VAL_LOGIC(pvs->value) |= bit;
		else   VAL_LOGIC(pvs->value) &= ~bit;
		return PE_OK;
	} else {
		SET_LOGIC(pvs->store, VAL_LOGIC(pvs->value) & bit);
		return PE_USE;
	}
}
#endif

/***********************************************************************
**
*/	REBTYPE(Logic)
/*
***********************************************************************/
{
	REBCNT val1 = VAL_LOGIC(D_ARG(1));
	REBCNT val2;
	REBVAL *arg = D_ARG(2);

	if (IS_BINARY_ACT(action)) {
		if (IS_LOGIC(arg)) val2 = VAL_LOGIC(arg);
		else if (IS_NONE(arg)) val2 = FALSE;
		else Trap_Types(RE_EXPECT_VAL, REB_LOGIC, VAL_TYPE(arg));
	}

	switch (action) {

	case A_AND: val1 &= val2; break;
	case A_OR:  val1 |= val2; break;
	case A_XOR:  val1 ^= val2; break;
	case A_COMPLEMENT: val1 = 1 & ~val1; break;

	case A_RANDOM:
		if (D_REF(2)) { // /seed
			// random/seed false restarts; true randomizes
			Set_Random(val1 ? (REBINT)OS_DELTA_TIME(0, 0) : 1);
			return R_UNSET;
		}
		DECIDE(Random_Int(D_REF(3)) & 1);  // /secure

	case A_MAKE:
	case A_TO:
		// As a "Rebol conversion", TO falls in line with the rest of the
		// interpreter canon that all non-none non-logic values are
		// considered effectively "truth".  As a construction routine,
		// MAKE takes more liberties in the meaning of its parameters,
		// so it lets zero values be false.
		if (IS_NONE(arg) ||
			(IS_LOGIC(arg) && !VAL_LOGIC(arg)) ||
			(IS_INTEGER(arg) && (action == A_MAKE && VAL_INT64(arg) == 0)) ||
			((IS_DECIMAL(arg) || IS_PERCENT(arg)) && (action == A_MAKE && VAL_DECIMAL(arg) == 0.0)) ||
			(IS_MONEY(arg) && (action == A_MAKE && deci_is_zero(VAL_DECI(arg))))
		) goto is_false;
		goto is_true;

#ifdef removed
	case A_CHANGE:
		if (IS_NONE(arg)) val1 = 0;
		else if (IS_INTEGER(arg)) val1 = Int32(arg);
		else if (IS_LOGIC(arg)) val1 = TRUE;
		else Trap_Arg(arg);
		break;
#endif

	default:
		Trap_Action(REB_LOGIC, action);
	}

	// Keep other fields AS IS!
	VAL_LOGIC(D_ARG(1)) = val1;
	return R_ARG1;

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}
