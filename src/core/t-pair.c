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
**  Module:  t-pair.c
**  Summary: pair datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	REBINT CT_Pair(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode >= 0) return Cmp_Pair(a, b) == 0; // works for INTEGER=0 too (spans x y)
	if (IS_PAIR(b) && 0 == VAL_INT64(b)) { // for negative? and positive?
		if (mode == -1)
			return (VAL_PAIR_X(a) >= 0 || VAL_PAIR_Y(a) >= 0); // not LT
		return (VAL_PAIR_X(a) > 0 && VAL_PAIR_Y(a) > 0); // NOT LTE
	}
	return -1;
}


/***********************************************************************
**
*/	REBFLG MT_Pair(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBD32 x;
	REBD32 y;

	if (IS_PAIR(data)) {
		*out = *data;
		return TRUE;
	}

	if (!IS_BLOCK(data)) return FALSE;

	data = VAL_BLK_DATA(data);

	if (IS_INTEGER(data)) x = (REBD32)VAL_INT64(data);
	else if (IS_DECIMAL(data)) x = (REBD32)VAL_DECIMAL(data);
	else return FALSE;

	data++;
	if (IS_INTEGER(data)) y = (REBD32)VAL_INT64(data);
	else if (IS_DECIMAL(data)) y = (REBD32)VAL_DECIMAL(data);
	else return FALSE;

	VAL_SET(out, REB_PAIR);
	VAL_PAIR_X(out) = x;
	VAL_PAIR_Y(out) = y;
	return TRUE;
}


/***********************************************************************
**
*/	REBINT Cmp_Pair(REBVAL *t1, REBVAL *t2)
/*
**	Given two pairs, compare them.
**
***********************************************************************/
{
	REBD32	diff;

	if ((diff = VAL_PAIR_Y(t1) - VAL_PAIR_Y(t2)) == 0)
		diff = VAL_PAIR_X(t1) - VAL_PAIR_X(t2);
	return (diff > 0.0) ? 1 : ((diff < 0.0) ? -1 : 0);
}


/***********************************************************************
**
*/	REBINT Min_Max_Pair(REBVAL *ds, REBFLG maxed)
/*
***********************************************************************/
{
	REBXYF aa;
	REBXYF bb;
	REBXYF *cc;
	REBVAL *a = D_ARG(1);
	REBVAL *b = D_ARG(2);
	REBVAL *c = D_RET;

	if (IS_PAIR(a)) aa = VAL_PAIR(a);
	else if (IS_INTEGER(a)) aa.x = aa.y = (REBD32)VAL_INT64(a);
	else Trap_Arg(a);

	if (IS_PAIR(b)) bb = VAL_PAIR(b);
	else if (IS_INTEGER(b)) bb.x = bb.y = (REBD32)VAL_INT64(b);
	else Trap_Arg(b);

	cc = &VAL_PAIR(c);
	if (maxed) {
		cc->x = MAX(aa.x, bb.x);
		cc->y = MAX(aa.y, bb.y);
	}
	else {
		cc->x = MIN(aa.x, bb.x);
		cc->y = MIN(aa.y, bb.y);
	}
	SET_TYPE(c, REB_PAIR);

	return R_RET;
}


/***********************************************************************
**
*/	REBINT PD_Pair(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *sel;
	REBVAL *val;
	REBINT n = 0;
	REBD32 dec;

	if (IS_WORD(sel = pvs->select)) {
		if (VAL_WORD_CANON(sel) == SYM_X) n = 1;
		else if (VAL_WORD_CANON(sel) == SYM_Y) n = 2;
		else return PE_BAD_SELECT;
	}
	else if (IS_INTEGER(sel)) {
		n = Int32(sel);
		if (n != 1 && n !=2) return PE_BAD_SELECT;
	}
	else
		return PE_BAD_SELECT;

	if (NZ(val = pvs->setval)) {
		if (IS_INTEGER(val)) dec = (REBD32)VAL_INT64(val);
		else if (IS_DECIMAL(val)) dec = (REBD32)VAL_DECIMAL(val);
		else return PE_BAD_SET;
		if (n == 1) VAL_PAIR_X(pvs->value) = dec;
		else VAL_PAIR_Y(pvs->value) = dec;
	} else {
		dec = (n == 1 ? VAL_PAIR_X(pvs->value) : VAL_PAIR_Y(pvs->value));
		SET_DECIMAL(pvs->store, dec);
		return PE_USE;
	}

	return PE_OK;
}

/***********************************************************************
**
*/	REBTYPE(Pair)
/*
***********************************************************************/
{
	REBVAL *val;
	REBVAL *arg;
	REBINT n;
	REBD32 x1, x2;
	REBD32 y1, y2;

	val = D_ARG(1);
	x1 = VAL_PAIR_X(val);
	y1 = VAL_PAIR_Y(val);
	if (DS_ARGC > 1) arg = D_ARG(2);

	if (IS_BINARY_ACT(action)) {
		n = VAL_TYPE(arg);

		if (n == REB_PAIR) {		// handle PAIR - PAIR cases
			x2 = VAL_PAIR_X(arg);
			y2 = VAL_PAIR_Y(arg);
		}
		else if (n == REB_INTEGER) {
			x2 = y2 = (REBD32)VAL_INT64(arg);
		}
		else if (n == REB_DECIMAL || n == REB_PERCENT) {
			x2 = y2 = (REBD32)VAL_DECIMAL(arg);
		}
		else
			Trap_Math_Args(REB_PAIR, action);

		switch (action) {

		case A_ADD:
			x1 += x2;
			y1 += y2;
			goto setPair;

		case A_SUBTRACT:
			x1 -= x2;
			y1 -= y2;
			goto setPair;

		case A_MULTIPLY:
			x1 *= x2;
			y1 *= y2;
			goto setPair;

		case A_DIVIDE:
		case A_REMAINDER:
			if (x2 == 0 || y2 == 0) Trap0(RE_ZERO_DIVIDE);
			if (action == A_DIVIDE) {
				x1 /= x2;
				y1 /= y2;
			}
			else {
				x1 = (REBD32)fmod(x1, x2);
				y1 = (REBD32)fmod(y1, y2);
			}
			goto setPair;
		}
		Trap_Math_Args(REB_PAIR, action);
	}
	// Unary actions:
	else {
		switch(action) {

#ifdef temp
		case A_ODDQ:
			DECIDE((x1 & 1) && (y1 & 1));

		case A_EVENQ:
			DECIDE((x1 & 1) == 0 && (y1 & 1) == 0);
#endif
		case A_NEGATE:
			x1 = -x1;
			y1 = -y1;
			goto setPair;
#ifdef temp
		case A_COMPLEMENT:
			x1 = ~x1;
			y1 = ~y1;
			goto setPair;
#endif
		case A_ABSOLUTE:
			if (x1 < 0) x1 = -x1;
			if (y1 < 0) y1 = -y1;
			goto setPair;

		case A_ROUND:
			{
				REBDEC d64;
				n = Get_Round_Flags(ds);
				if (D_REF(2))
					d64 = Dec64(D_ARG(3));
				else {
					d64 = 1.0L;
					n |= 1;
				}
				x1 = (REBD32)Round_Dec(x1, n, d64);
				y1 = (REBD32)Round_Dec(y1, n, d64);
			}
			goto setPair;

		case A_REVERSE:
			x2 = x1;
			x1 = y1;
			y1 = x2;
			goto setPair;

		case A_RANDOM:
			if (D_REF(2)) Trap0(RE_BAD_REFINES); // seed
			x1 = (REBD32)Random_Range((REBINT)x1, (REBOOL)D_REF(3));
			y1 = (REBD32)Random_Range((REBINT)y1, (REBOOL)D_REF(3));
			goto setPair;

		case A_PICK:
			if (IS_WORD(arg)) {
				if (VAL_WORD_CANON(arg) == SYM_X) n = 0;
				else if (VAL_WORD_CANON(arg) == SYM_Y) n = 1;
				else Trap_Arg(arg);
			}
			else {
				n = Get_Num_Arg(arg);
				if (n < 1 || n > 2) Trap_Range(arg);
				n--;
			}
///		case A_POKE:
///			if (action == A_POKE) {
///				arg = D_ARG(3);
///				if (IS_INTEGER(arg)) {
///					if (index == 0) x1 = VAL_INT32(arg);
///					else y1 = VAL_INT32(arg);
///				}
///				else if (IS_DECIMAL(arg)) {
///					if (index == 0) x1 = (REBINT)VAL_DECIMAL(arg);
///					else y1 = (REBINT)VAL_DECIMAL(arg);
///				} else
///					Trap_Arg(arg);
///				goto setPair;
///			}
			SET_DECIMAL(DS_RETURN, n == 0 ? x1 : y1);
			return R_RET;

		case A_MAKE:
		case A_TO:
			val = D_ARG(2);
			x1 = y1 = 0;
//			if (IS_NONE(val)) goto setPair;
			if (IS_PAIR(val)) {
				*DS_RETURN = *val;
				return R_RET;
			}
			if (IS_STRING(val)) {
				REBYTE *bp;
				REBCNT len;
				// -1234567890x-1234567890
				bp = Qualify_String(val, 24, &len, FALSE);
				if (Scan_Pair(bp, len, DS_RETURN)) return R_RET;
			}
			if (IS_INTEGER(val)) {
				x1 = y1 = (REBD32)VAL_INT64(val);
				goto setPair;
			}
			if (IS_DECIMAL(val)) {
				x1 = y1 = (REBD32)VAL_DECIMAL(val);
				goto setPair;
			}
			if (ANY_BLOCK(val) && VAL_LEN(val) <= 2) {
				if (MT_Pair(D_RET, val, REB_PAIR))
					return R_RET;
			}
			Trap_Make(REB_PAIR, val);
		}
	}
	Trap_Action(REB_PAIR, action);

setPair:
	VAL_SET(DS_RETURN, REB_PAIR);
	VAL_PAIR_X(DS_RETURN) = x1;
	VAL_PAIR_Y(DS_RETURN) = y1;
	return R_RET;

//is_false:
//	return R_FALSE;

//is_true:
//	return R_TRUE;
}

