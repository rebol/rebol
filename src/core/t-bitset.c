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
**  Module:  t-bitset.c
**  Summary: bitset datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#define MAX_BITSET 0x7fffffff

#define BITS_NOT(s) ((s)->size)

/***********************************************************************
**
*/	REBINT CT_Bitset(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode == 3) return VAL_SERIES(a) == VAL_SERIES(b);
	if (mode >= 0) return (
		BITS_NOT(VAL_SERIES(a)) == BITS_NOT(VAL_SERIES(b))
		&&
		Compare_Binary_Vals(a, b) == 0
	);
	return -1;
}


/***********************************************************************
**
*/	REBSER *Make_Bitset(REBCNT len)
/*
**		Return a bitset series (binary.
**
**		len: the # of bits in the bitset.
**
***********************************************************************/
{
	REBSER *ser;

	len = (len + 7) / 8;
	ser = Make_Binary(len);
	Clear_Series(ser);
	SERIES_TAIL(ser) = len;
	BITS_NOT(ser) = 0;

	return ser;
}


/***********************************************************************
**
*/	void Mold_Bitset(REBVAL *value, REB_MOLD *mold)
/*
***********************************************************************/
{
	REBSER *ser = VAL_SERIES(value);

	if (BITS_NOT(ser)) Append_Bytes(mold->series, "[not bits ");
	Mold_Binary(value, mold);
	if (BITS_NOT(ser)) Append_Byte(mold->series, ']');
}


/***********************************************************************
**
*/	REBFLG MT_Bitset(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBFLG is_not = 0;
	
	if (IS_BLOCK(data)) {
		REBINT len = Find_Max_Bit(data);
		REBSER *ser;
		if (len < 0 || len > 0xFFFFFF) Trap_Arg(data);
		ser = Make_Bitset(len);
		Set_Bits(ser, data, TRUE);
		Set_Series(REB_BITSET, out, ser);
		return TRUE;
	}

	if (!IS_BINARY(data)) return FALSE;
	Set_Series(REB_BITSET, out, Copy_Series_Value(data));
	BITS_NOT(VAL_SERIES(out)) = 0;
	return TRUE;
}


/***********************************************************************
**
*/	REBINT Find_Max_Bit(REBVAL *val)
/*
**		Return integer number for the maximum bit number defined by
**		the value. Used to determine how much space to allocate.
**
***********************************************************************/
{
	REBINT maxi = 0;
	REBINT n;

	switch (VAL_TYPE(val)) {

	case REB_CHAR:
		maxi = VAL_CHAR(val)+1;
		break;

	case REB_INTEGER:
		maxi = Int32s(val, 0);
		break;

	case REB_STRING:
	case REB_FILE:
	case REB_EMAIL:
	case REB_URL:
	case REB_TAG:
//	case REB_ISSUE:
		n = VAL_INDEX(val);
		if (VAL_BYTE_SIZE(val)) {
			REBYTE *bp = VAL_BIN(val);
			for (; n < (REBINT)VAL_TAIL(val); n++)
				if (bp[n] > maxi) maxi = bp[n];
		}
		else {
			REBUNI *up = VAL_UNI(val);
			for (; n < (REBINT)VAL_TAIL(val); n++)
				if (up[n] > maxi) maxi = up[n];
		}
		maxi++;
		break;

	case REB_BINARY:
		maxi = VAL_LEN(val) * 8 - 1;
		if (maxi < 0) maxi = 0;
		break;

	case REB_BLOCK:
		for (val = VAL_BLK_DATA(val); NOT_END(val); val++) {
			n = Find_Max_Bit(val);
			if (n > maxi) maxi = n;
		}
		//maxi++;
		break;

	case REB_NONE:
		maxi = 0;
		break;

	default:
		return -1;
	}

	return maxi;
}


/***********************************************************************
**
*/	REBFLG Check_Bit(REBSER *bset, REBCNT c, REBFLG uncased)
/*
**		Check bit indicated. Returns TRUE if set.
**		If uncased is TRUE, try to match either upper or lower case.
**
***********************************************************************/
{
	REBCNT i, n = c;
	REBCNT tail = SERIES_TAIL(bset);
	REBFLG flag = 0;

	if (uncased) {
		if (n >= UNICODE_CASES) uncased = FALSE; // no need to check
		else n = LO_CASE(c);
	}

	// Check lowercase char:
retry:
	i = n >> 3;
	if (i < tail)
		flag = (0 != (BIN_HEAD(bset)[i] & (1 << (7 - ((n) & 7)))));

	// Check uppercase if needed:
	if (uncased && !flag) {
		n = UP_CASE(c);
		uncased = FALSE;
		goto retry;
	}
		
	return (BITS_NOT(bset)) ? !flag : flag;
}


/***********************************************************************
**
*/	REBFLG Check_Bit_Str(REBSER *bset, REBVAL *val, REBFLG uncased)
/*
**		If uncased is TRUE, try to match either upper or lower case.
**
***********************************************************************/
{
	REBCNT n = VAL_INDEX(val);

	if (VAL_BYTE_SIZE(val)) {
		REBYTE *bp = VAL_BIN(val);
		for (; n < VAL_TAIL(val); n++)
			if (Check_Bit(bset, bp[n], uncased)) return TRUE;
	}
	else {
		REBUNI *up = VAL_UNI(val);
		for (; n < VAL_TAIL(val); n++)
			if (Check_Bit(bset, up[n], uncased)) return TRUE;
	}
	return FALSE;
}


/***********************************************************************
**
*/	void Set_Bit(REBSER *bset, REBCNT n, REBOOL set)
/*
**		Set/clear a single bit. Expand if needed.
**
***********************************************************************/
{
	REBCNT i = n >> 3;
	REBCNT tail = SERIES_TAIL(bset);
	REBYTE bit;

	// Expand if not enough room:
	if (i >= tail) {
		if (!set) return; // no need to expand
		Expand_Series(bset, tail, (i - tail) + 1);
		CLEAR(BIN_SKIP(bset, tail), (i - tail) + 1);
	}

	bit = 1 << (7 - ((n) & 7));
	if (set)
		BIN_HEAD(bset)[i] |= bit;
	else
		BIN_HEAD(bset)[i] &= ~bit;
}


/***********************************************************************
**
*/	void Set_Bit_Str(REBSER *bset, REBVAL *val, REBOOL set)
/*
***********************************************************************/
{
	REBCNT n = VAL_INDEX(val);

	if (VAL_BYTE_SIZE(val)) {
		REBYTE *bp = VAL_BIN(val);
		for (; n < VAL_TAIL(val); n++)
			Set_Bit(bset, bp[n], set);
	}
	else {
		REBUNI *up = VAL_UNI(val);
		for (; n < VAL_TAIL(val); n++)
			Set_Bit(bset, up[n], set);
	}
}


/***********************************************************************
**
*/	REBFLG Set_Bits(REBSER *bset, REBVAL *val, REBOOL set)
/*
**		Set/clear bits indicated by strings and chars and ranges.
**
***********************************************************************/
{
	REBCNT n;
	REBCNT c;

	if (IS_CHAR(val)) {
		Set_Bit(bset, VAL_CHAR(val), set);
		return TRUE;
	}

	if (IS_INTEGER(val)) {
		n = Int32s(val, 0);
		if (n > MAX_BITSET) return 0;
		Set_Bit(bset, n, set);
		return TRUE;
	}

	if (ANY_BINSTR(val)) {
		Set_Bit_Str(bset, val, set);
		return TRUE;
	}

	if (!ANY_BLOCK(val)) Trap_Type(val);

	val = VAL_BLK_DATA(val);
	if (IS_SAME_WORD(val, SYM_NOT)) {
		BITS_NOT(bset) = TRUE;
		val++;
	}

	// Loop through block of bit specs:
	for (; NOT_END(val); val++) {

		switch (VAL_TYPE(val)) {

		case REB_CHAR:
			c = VAL_CHAR(val);
			if (IS_SAME_WORD(val + 1, SYM__)) {
				val += 2;
				if (IS_CHAR(val)) {
					n = VAL_CHAR(val);
span_bits:
					if (n < c) Trap1(RE_PAST_END, val);
					for (; c <= n; c++) Set_Bit(bset, c, set);
				} else Trap_Arg(val);
			}
			else Set_Bit(bset, c, set);
			break;

		case REB_INTEGER:
			n = Int32s(val, 0);
			if (n > MAX_BITSET) return 0;
			if (IS_SAME_WORD(val + 1, SYM__)) {
				c = n;
				val += 2;
				if (IS_INTEGER(val)) {
					n = Int32s(val, 0);
					goto span_bits;
				} else Trap_Arg(val);
			}
			else Set_Bit(bset, n, set);
			break;

		case REB_BINARY:
		case REB_STRING:
		case REB_FILE:
		case REB_EMAIL:
		case REB_URL:
		case REB_TAG:
//		case REB_ISSUE:
			Set_Bit_Str(bset, val, set);
			break;

		case REB_WORD:
			// Special: BITS #{000...}
			if (!IS_SAME_WORD(val, SYM_BITS)) return 0;
			val++;
			if (!IS_BINARY(val)) return 0;
			n = VAL_LEN(val);
			c = bset->tail;
			if (n >= c) {
				Expand_Series(bset, c, (n - c));
				CLEAR(BIN_SKIP(bset, c), (n - c));
			}
			memcpy(BIN_HEAD(bset), VAL_BIN_DATA(val), n);
			break;

		default:
			return 0;
		}
	}

	return TRUE;
}

		
/***********************************************************************
**
*/	REBFLG Check_Bits(REBSER *bset, REBVAL *val, REBFLG uncased)
/*
**		Check bits indicated by strings and chars and ranges.
**		If uncased is TRUE, try to match either upper or lower case.
**
***********************************************************************/
{
	REBCNT n;
	REBUNI c;

	if (IS_CHAR(val))
		return Check_Bit(bset, VAL_CHAR(val), uncased);

	if (IS_INTEGER(val))
		return Check_Bit(bset, Int32s(val, 0), uncased);

	if (ANY_BINSTR(val))
		return Check_Bit_Str(bset, val, uncased);

	if (!ANY_BLOCK(val)) Trap_Type(val);

	// Loop through block of bit specs:
	for (val = VAL_BLK_DATA(val); NOT_END(val); val++) {

		switch (VAL_TYPE(val)) {

		case REB_CHAR:
			c = VAL_CHAR(val);
			if (IS_SAME_WORD(val + 1, SYM__)) {
				val += 2;
				if (IS_CHAR(val)) {
					n = VAL_CHAR(val);
scan_bits:
					if (n < c) Trap1(RE_PAST_END, val);
					for (; c <= n; c++)
						if (Check_Bit(bset, c, uncased)) goto found;
				} else Trap_Arg(val);
			}
			else
				if (Check_Bit(bset, c, uncased)) goto found;
			break;

		case REB_INTEGER:
			n = Int32s(val, 0);
			if (n > 0xffff) return 0;
			if (IS_SAME_WORD(val + 1, SYM__)) {
				c = n;
				val += 2;
				if (IS_INTEGER(val)) {
					n = Int32s(val, 0);
					goto scan_bits;
				} else Trap_Arg(val);
			}
			else
				if (Check_Bit(bset, n, uncased)) goto found;
			break;

		case REB_BINARY:
		case REB_STRING:
		case REB_FILE:
		case REB_EMAIL:
		case REB_URL:
		case REB_TAG:
//		case REB_ISSUE:
			if (Check_Bit_Str(bset, val, uncased)) goto found;
			break;

		default:
			Trap_Type(val);
		}
	}
	return FALSE;

found:
	return TRUE;
}


/***********************************************************************
**
*/	REBINT PD_Bitset(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *data = pvs->value;
	REBVAL *val = pvs->setval;
	REBSER *ser = VAL_SERIES(data);
	REBFLG t;

	if (val == 0) {
		if (Check_Bits(ser, pvs->select, 0)) {
			SET_TRUE(pvs->store);
			return PE_USE;
		}
		return PE_NONE;
	}

	t = IS_TRUE(val);
	if (BITS_NOT(ser)) t = !t;
	if (Set_Bits(ser, pvs->select, (REBOOL)t)) 
		return PE_OK;	

	return PE_BAD_SET;
}


/***********************************************************************
**
*/	void Trim_Tail_Zeros(REBSER *ser)
/*
**		Remove extra zero bytes from end of byte string.
**
***********************************************************************/
{
	REBCNT tail = SERIES_TAIL(ser);
	REBYTE *bp = BIN_HEAD(ser);

	for (; tail > 0 && !bp[tail]; tail--);

	if (bp[tail]) tail++;
	SERIES_TAIL(ser) = tail;
}


/***********************************************************************
**
*/	REBTYPE(Bitset)
/*
***********************************************************************/
{
	REBYTE *data = 0;
	REBVAL *value = D_ARG(1);
	REBVAL *arg = D_ARG(2);
	REBSER *ser;
	REBINT len;
	REBINT diff;

	if (action != A_MAKE && action != A_TO)
		data = VAL_BIT_DATA(value);

	// Check must be in this order (to avoid checking a non-series value);
	if (action >= A_TAKE && action <= A_SORT && IS_PROTECT_SERIES(VAL_SERIES(value)))
		Trap0(RE_PROTECTED);

	switch (action) {

	// Define PICK for BITSETS?  PICK's set bits and returns #?
	// Add AND, OR, XOR

	case A_PICK:
	case A_FIND:
		if (!Check_Bits(VAL_SERIES(value), arg, D_REF(ARG_FIND_CASE))) return R_NONE;
		return R_TRUE;

	case A_COMPLEMENT:
	case A_NEGATE:
		ser = Copy_Series(VAL_SERIES(value));
		BITS_NOT(ser) = !BITS_NOT(VAL_SERIES(value));
		Set_Series(REB_BITSET, value, ser);
		break;

	case A_MAKE:
	case A_TO:
		// Determine size of bitset. Returns -1 for errors.
		len = Find_Max_Bit(arg);
		if (len < 0 || len > 0x0FFFFFFF) Trap_Arg(arg);

		ser = Make_Bitset(len);
		Set_Series(REB_BITSET, value, ser);

		// Nothing more to do.
		if (IS_INTEGER(arg)) break;

		if (IS_BINARY(arg)) {
			memcpy(BIN_HEAD(ser), VAL_BIN_DATA(arg), len/8 + 1);
			break;
		}
		// FALL THRU...

	case A_APPEND:	// Accepts: #"a" "abc" [1 - 10] [#"a" - #"z"] etc.
	case A_INSERT:
		diff = TRUE;
		goto set_bits;

	case A_POKE:
		diff = Get_Logic_Arg(D_ARG(3));
set_bits:
		if (BITS_NOT(VAL_SERIES(value))) diff = !diff;
		if (Set_Bits(VAL_SERIES(value), arg, (REBOOL)diff)) break;
		Trap_Arg(arg);

	case A_REMOVE:	// #"a" "abc"  remove/part bs "abcd"  yuk: /part ?
		if (!D_REF(2)) Trap0(RE_MISSING_ARG); // /part required
		if (Set_Bits(VAL_SERIES(value), D_ARG(3), FALSE)) break;
		Trap_Arg(D_ARG(3));

	case A_COPY:
		VAL_SERIES(value) = Copy_Series_Value(value);
		break;

	case A_LENGTHQ:
		len = VAL_TAIL(value) * 8;
		SET_INTEGER(value, len);
		break;

	case A_TAILQ:
		// Necessary to make EMPTY? work:
		return (VAL_TAIL(value) == 0) ? R_TRUE : R_FALSE;

	case A_CLEAR:
		Clear_Series(VAL_SERIES(value));
		break;

	case A_AND:
	case A_OR:
	case A_XOR:
		if (!IS_BITSET(arg) && !IS_BINARY(arg))
			Trap_Math_Args(VAL_TYPE(arg), action);
		VAL_SERIES(value) = ser = Xandor_Binary(action, value, arg);
		Trim_Tail_Zeros(ser);
		break;

	default:
		Trap_Action(REB_BITSET, action);
	}

	DS_RET_VALUE(value);
	return R_RET;
}

