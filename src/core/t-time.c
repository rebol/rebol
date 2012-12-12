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
**  Module:  t-time.c
**  Summary: time datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

/***********************************************************************
**
*/	void Split_Time(REBI64 t, REB_TIMEF *tf)
/*
***********************************************************************/
{
	// note: negative sign will be lost.
	REBI64 h, m, s, n, i;

	if (t < 0) t = -t;

	h = t / HR_SEC;
	i = t - (h * HR_SEC);
	m = i / MIN_SEC;
	i = i - (m * MIN_SEC);
	s = i / SEC_SEC;
	n = i - (s * SEC_SEC);

	tf->h = (REBCNT)h;
	tf->m = (REBCNT)m;
	tf->s = (REBCNT)s;
	tf->n = (REBCNT)n;
}

/***********************************************************************
**
*/	REBI64 Join_Time(REB_TIMEF *tf)
/*
***********************************************************************/
{
	REBFLG neg = tf->h < 0;
	REBI64 t;

	t = tf->h * HR_SEC + tf->m * MIN_SEC + tf->s * SEC_SEC + tf->n;
	return (neg ? -t : t);
}

/***********************************************************************
**
*/	REBYTE *Scan_Time(REBYTE *cp, REBCNT len, REBVAL *value)
/*
**		Scan string and convert to time.  Return zero if error.
**
***********************************************************************/
{
	REBYTE  *sp;
	REBYTE	merid = FALSE;
	REBOOL	neg = FALSE;
	REBINT	part1, part2, part3 = -1;
	REBINT	part4 = -1;

	if (*cp == '-') cp++, neg = TRUE;
	else if (*cp == '+') cp++;

	if (*cp == '-' || *cp == '+') return 0; // small hole: --1:23

	// Can be:
	//    HH:MM       as part1:part2
	//    HH:MM:SS    as part1:part2:part3
	//    HH:MM:SS.DD as part1:part2:part3.part4
	//    MM:SS.DD    as part1:part2.part4
	cp = Grab_Int(cp, &part1);
	if (part1 > MAX_HOUR) return 0;
	if (*cp++ != ':') return 0;
	sp = Grab_Int(cp, &part2);
	if (part2 < 0 || sp == cp) return 0;
	cp = sp;
	if (*cp == ':') {	// optional seconds
		sp = cp + 1;
		cp = Grab_Int(sp, &part3);
		if (part3 < 0 || cp == sp) return 0;  //part3 = -1;
	}
	if (*cp == '.' || *cp == ',') {
		sp = ++cp;
		cp = Grab_Int_Scale(sp, &part4, 9);
		if (part4 == 0) part4 = -1;
	}
	if ((UP_CASE(*cp) == 'A' || UP_CASE(*cp) == 'P') && (UP_CASE(cp[1]) == 'M')) {
		merid = (REBYTE)UP_CASE(*cp);
		cp += 2;
	}

	if (part3 >= 0 || part4 < 0) {	// HH:MM mode
		if (merid) {
			if (part1 > 12) return 0;
			if (part1 == 12) part1 = 0;
			if (merid == 'P') part1 += 12;
		}
		if (part3 < 0) part3 = 0;
		VAL_TIME(value) = HOUR_TIME(part1) + MIN_TIME(part2) + SEC_TIME(part3);
	} else {						// MM:SS mode
		if (merid) return 0;		// no AM/PM for minutes
		VAL_TIME(value) = MIN_TIME(part1) + SEC_TIME(part2);
	}

	if (part4 > 0) VAL_TIME(value) += part4;

	if (neg) VAL_TIME(value) = -VAL_TIME(value);
	VAL_SET(value, REB_TIME);

	return cp;
}


/***********************************************************************
**
*/  void Emit_Time(REB_MOLD *mold, REBVAL *value)
/*
***********************************************************************/
{
	REB_TIMEF tf;
	REBYTE *fmt;

	Split_Time(VAL_TIME(value), &tf); // loses sign

	if (tf.s == 0 && tf.n == 0) fmt = "I:2";
	else fmt = "I:2:2";

	if (VAL_TIME(value) < (REBI64)0) Append_Byte(mold->series, '-');
	Emit(mold, fmt, tf.h, tf.m, tf.s, 0);

	if (tf.n > 0) Emit(mold, ".i", tf.n);
}


/***********************************************************************
**
*/	REBINT CT_Time(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT num = Cmp_Time(a, b);
	if (mode >= 0)  return (num == 0);
	if (mode == -1) return (num >= 0);
	return (num > 0);
}


/***********************************************************************
**
*/  REBI64 Make_Time(REBVAL *val)
/*
**		Returns NO_TIME if error.
**
***********************************************************************/
{
	REBI64 secs = 0;

	if (IS_TIME(val)) {
		secs = VAL_TIME(val);
	}
	else if (IS_STRING(val)) {
		REBYTE *bp;
		REBCNT len;
		bp = Qualify_String(val, 30, &len, FALSE); // can trap, ret diff str
		if (!Scan_Time(bp, len, val)) goto no_time;
		secs = VAL_TIME(val);
	}
	else if (IS_INTEGER(val)) {
		if (VAL_INT64(val) < -MAX_SECONDS || VAL_INT64(val) > MAX_SECONDS)
			Trap_Range(val);
		secs = VAL_INT64(val) * SEC_SEC;
	}
	else if (IS_DECIMAL(val)) {
		if (VAL_DECIMAL(val) < (REBDEC)(-MAX_SECONDS) || VAL_DECIMAL(val) > (REBDEC)MAX_SECONDS)
			Trap_Range(val);
		secs = DEC_TO_SECS(VAL_DECIMAL(val));
	}
	else if (ANY_BLOCK(val) && VAL_BLK_LEN(val) <= 3) {
		REBFLG neg = FALSE;
		REBINT i;

		val = VAL_BLK_DATA(val);
		if (!IS_INTEGER(val)) goto no_time;
		i = Int32(val);
		if (i < 0) i = -i, neg = TRUE;
		secs = i * 3600;
		if (secs > MAX_SECONDS) goto no_time;

		if (NOT_END(++val)) {
			if (!IS_INTEGER(val)) goto no_time;
			if ((i = Int32(val)) < 0) goto no_time;
			secs += i * 60;
			if (secs > MAX_SECONDS) goto no_time;
	
			if (NOT_END(++val)) {
				if (IS_INTEGER(val)) {
					if ((i = Int32(val)) < 0) goto no_time;
					secs += i;
					if (secs > MAX_SECONDS) goto no_time;
				}
				else if (IS_DECIMAL(val)) {
					if (secs + (REBI64)VAL_DECIMAL(val) + 1 > MAX_SECONDS) goto no_time;
					// added in below
				}
				else goto no_time;
			}
		}
		secs *= SEC_SEC;
		if (IS_DECIMAL(val)) secs += DEC_TO_SECS(VAL_DECIMAL(val));
		if (neg) secs = -secs;
	}
	else
		no_time: return NO_TIME;

	return secs;
}


/***********************************************************************
**
*/	REBFLG MT_Time(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBI64 secs = Make_Time(data);

	if (secs == NO_TIME) return FALSE;

	VAL_SET(out, REB_TIME);
	VAL_TIME(out) = secs;
	VAL_DATE(out).bits = 0;

	return TRUE;
}


/***********************************************************************
**
*/	REBINT Cmp_Time(REBVAL *v1, REBVAL *v2)
/*
**	Given two times, compare them.
**
***********************************************************************/
{
	REBI64 t1 = VAL_TIME(v1);
	REBI64 t2 = VAL_TIME(v2);

	if (t1 == NO_TIME) t1 = 0L;
	if (t2 == NO_TIME) t2 = 0L;
	if (t2 == t1) return 0;
	if (t1 > t2) return 1;
	return -1;
}


/***********************************************************************
**
*/	REBINT PD_Time(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *val;
	REBINT i;
	REBINT n;
	REBDEC f;
	REB_TIMEF tf;

	if (IS_WORD(pvs->select)) {
		switch (VAL_WORD_CANON(pvs->select)) {
		case SYM_HOUR:   i = 0; break;
		case SYM_MINUTE: i = 1; break;
		case SYM_SECOND: i = 2; break;
		default: return PE_BAD_SELECT;
		}
	}
	else if (IS_INTEGER(pvs->select))
		i = VAL_INT32(pvs->select) - 1;
	else
		return PE_BAD_SELECT;

	Split_Time(VAL_TIME(pvs->value), &tf); // loses sign

	if (!(val = pvs->setval)) {
		val = pvs->store;
		switch(i) {
		case 0: // hours
			SET_INTEGER(val, tf.h);
			break;
		case 1:
			SET_INTEGER(val, tf.m);
			break;
		case 2:
			if (tf.n == 0)
				SET_INTEGER(val, tf.s);
			else
				SET_DECIMAL(val, (REBDEC)tf.s + (tf.n * NANO));
			break;
		default:
			return PE_NONE;
		}
		return PE_USE;

	} else {
		if (IS_INTEGER(val) || IS_DECIMAL(val)) n = Int32s(val, 0);
		else if (IS_NONE(val)) n = 0;
		else return PE_BAD_SET;

		switch(i) {
		case 0:
			tf.h = n;
			break;
		case 1:
			tf.m = n;
			break;
		case 2:
			if (IS_DECIMAL(val)) {
				f = VAL_DECIMAL(val);
				if (f < 0.0) Trap_Range(val);
				tf.s = (REBINT)f;
				tf.n = (REBINT)((f - tf.s) * SEC_SEC);
			}
			else {
				tf.s = n;
				tf.n = 0;
			}
			break;
		default:
			return PE_BAD_SELECT;
		}

		VAL_TIME(pvs->value) = Join_Time(&tf);
		return PE_OK;
	}
}


/***********************************************************************
**
*/	REBTYPE(Time)
/*
***********************************************************************/
{
	REBI64	secs;
	REBVAL	*val;
	REBVAL	*arg;
	REBI64	num;

	val = D_ARG(1);

	secs = VAL_TIME(val); // note: not always valid REB_TIME (e.g. MAKE)

	if (DS_ARGC > 1) arg = D_ARG(2);

	if (IS_BINARY_ACT(action)) {
		REBINT	type = VAL_TYPE(arg);

		if (type == REB_TIME) {		// handle TIME - TIME cases
			REBI64	secs2 = VAL_TIME(arg);
			REBINT	diff;

			diff = Cmp_Time(val, arg);
			switch (action) {

			case A_ADD:
				secs = Add_Max(REB_TIME, secs, secs2, MAX_TIME);
				goto fixTime;

			case A_SUBTRACT:
				secs = Add_Max(REB_TIME, secs, -secs2, MAX_TIME);
				goto fixTime;

			case A_DIVIDE:
				if (secs2 == 0) Trap0(RE_ZERO_DIVIDE);
				//secs /= secs2;
				VAL_SET(DS_RETURN, REB_DECIMAL);
				VAL_DECIMAL(DS_RETURN) = (REBDEC)secs / (REBDEC)secs2;
				return R_RET;

			case A_REMAINDER:
				if (secs2 == 0) Trap0(RE_ZERO_DIVIDE);
				secs %= secs2;
				goto setTime;
			}
		}
		else if (type == REB_INTEGER) {		// handle TIME - INTEGER cases

			num = VAL_INT64(arg);

			switch(action) {
			case A_ADD:
				secs = Add_Max(REB_TIME, secs, num * SEC_SEC, MAX_TIME);
				goto fixTime;

			case A_SUBTRACT:
				secs = Add_Max(REB_TIME, secs, num * -SEC_SEC, MAX_TIME);
				goto fixTime;

			case A_MULTIPLY:
				secs *= num;
				if (secs < -MAX_TIME || secs > MAX_TIME)
					Trap1(RE_TYPE_LIMIT, Get_Type(REB_TIME));
				goto setTime;

			case A_DIVIDE:
				if (num == 0) Trap0(RE_ZERO_DIVIDE);
				secs /= num;
				DS_RET_INT(secs);
				goto setTime;

			case A_REMAINDER:
				if (num == 0) Trap0(RE_ZERO_DIVIDE);
				secs %= num;
				goto setTime;
			}
		}
		else if (type == REB_DECIMAL) {		// handle TIME - DECIMAL cases
			REBDEC dec = VAL_DECIMAL(arg);

			switch(action) {
			case A_ADD:
				secs = Add_Max(REB_TIME, secs, (i64)(dec * SEC_SEC), MAX_TIME);
				goto fixTime;

			case A_SUBTRACT:
				secs = Add_Max(REB_TIME, secs, (i64)(dec * -SEC_SEC), MAX_TIME);
				goto fixTime;

			case A_MULTIPLY:
				secs = (REBI64)(secs * dec);
				goto setTime;

			case A_DIVIDE:
				if (dec == 0.0) Trap0(RE_ZERO_DIVIDE);
				secs = (REBI64)(secs / dec);
				goto setTime;

//			case A_REMAINDER:
//				ld = fmod(ld, VAL_DECIMAL(arg));
//				goto decTime;
			}
		}
		else if (type == REB_DATE && action == A_ADD) {	// TIME + DATE case
			// Swap args and call DATE datatupe:
			*D_ARG(3) = *val; // (temporary location for swap)
			*D_ARG(1) = *arg;
			*D_ARG(2) = *D_ARG(3);
			T_Date(ds, action);
			return R_RET;
		}
		Trap_Math_Args(REB_TIME, action);
	}
	else {
		// unary actions
		switch(action) {

		case A_ODDQ:
			DECIDE((SECS_IN(secs) & 1) != 0);

		case A_EVENQ:
			DECIDE((SECS_IN(secs) & 1) == 0);

		case A_NEGATE:
			secs = -secs;
			goto setTime;

		case A_ABSOLUTE:
			if (secs < 0) secs = -secs;
			goto setTime;

		case A_ROUND:
			if (D_REF(2)) {
				arg = D_ARG(3);
				if (IS_TIME(arg)) {
					secs = Round_Int(secs, Get_Round_Flags(ds), VAL_TIME(arg));
				}
				else if (IS_DECIMAL(arg)) {
					VAL_DECIMAL(arg) = Round_Dec((REBDEC)secs, Get_Round_Flags(ds), Dec64(arg) * SEC_SEC) / SEC_SEC;
					VAL_SET(arg, REB_DECIMAL);
					return R_ARG3;
				}
				else if (IS_INTEGER(arg)) {
					VAL_INT64(arg) = Round_Int(secs, 1, Int32(arg) * SEC_SEC) / SEC_SEC;
					VAL_SET(arg, REB_INTEGER);
					return R_ARG3;
				}
				else Trap_Arg(arg);
			}
			else {
				secs = Round_Int(secs, Get_Round_Flags(ds) | 1, SEC_SEC);
			}
			goto fixTime;

		case A_RANDOM:
			if (D_REF(2)) {
				Set_Random(secs);
				return R_UNSET;
			}
			secs = Random_Range(secs / SEC_SEC, D_REF(3)) * SEC_SEC;
			goto fixTime;

		case A_PICK:
			Pick_Path(val, arg, 0);
			return R_TOS;

///		case A_POKE:
///			Pick_Path(val, arg, D_ARG(3));
///			return R_ARG3;

		case A_MAKE:
		case A_TO:
			secs = Make_Time(arg);
			if (secs == NO_TIME) Trap_Make(REB_TIME, arg);
			goto setTime;
		}
	}
	Trap_Action(REB_TIME, action);

fixTime:
setTime:
	VAL_TIME(D_RET) = secs;
	VAL_SET(D_RET, REB_TIME);
	return R_RET;

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}
