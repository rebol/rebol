/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
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
**  Module:  t-string.c
**  Summary: string related datatypes
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-scan.h"
#include "sys-deci-funcs.h"
#include "sys-int-funcs.h"

/***********************************************************************
**
*/	REBINT CT_String(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT num;

	if (mode == 3)
		return VAL_SERIES(a) == VAL_SERIES(b) && VAL_INDEX(a) == VAL_INDEX(b);

	num = Compare_String_Vals(a, b, (REBOOL) !(mode > 1));
	if (mode >= 0) return (num == 0);
	if (mode == -1) return (num >= 0);
	return (num > 0);
}


/***********************************************************************
**
**	Local Utility Functions
**
***********************************************************************/

static void str_to_char(REBVAL *out, REBVAL *val, REBCNT idx)
{
	// STRING value to CHAR value (save some code space)
	SET_CHAR(out, GET_ANY_CHAR(VAL_SERIES(val), idx));
}

static void swap_chars(REBVAL *val1, REBVAL *val2)
{
	REBUNI c1;
	REBUNI c2;
	REBSER *s1 = VAL_SERIES(val1);
	REBSER *s2 = VAL_SERIES(val2);

	c1 = GET_ANY_CHAR(s1, VAL_INDEX(val1));
	c2 = GET_ANY_CHAR(s2, VAL_INDEX(val2));

	if (BYTE_SIZE(s1) && c2 > 0xff) Widen_String(s1);
	SET_ANY_CHAR(s1, VAL_INDEX(val1), c2);
	
	if (BYTE_SIZE(s2) && c1 > 0xff) Widen_String(s2);
	SET_ANY_CHAR(s2, VAL_INDEX(val2), c1);
}

static void reverse_string(REBVAL *value, REBCNT len)
{
	REBCNT n;
	REBCNT m;
	REBUNI c;

	if (VAL_BYTE_SIZE(value)) {
		REBYTE *bp = VAL_BIN_DATA(value);

		for (n = 0, m = len-1; n < len / 2; n++, m--) {
			c = bp[n];
			bp[n] = bp[m];
			bp[m] = (REBYTE)c;
		}
	}
	else {
		REBUNI *up = VAL_UNI_DATA(value);

		for (n = 0, m = len-1; n < len / 2; n++, m--) {
			c = up[n];
			up[n] = up[m];
			up[m] = c;
		}
	}
}

static REBCNT find_string(REBSER *series, REBCNT index, REBCNT end, REBVAL *target, REBCNT len, REBCNT flags, REBINT skip, REBVAL *wild)
{
	REBCNT start = index;

	if (flags & (AM_FIND_REVERSE | AM_FIND_LAST)) {
		skip = -1;
		start = 0;
		if (flags & AM_FIND_LAST) index = end - len;
		else index--;
	}

	if (flags & AM_FIND_SAME) flags |= AM_FIND_CASE; // /SAME has same functionality as /CASE for any-string!

	//O: not using ANY_BINSTR as TAG is now handled separately
	if (VAL_TYPE(target) >= REB_BINARY && VAL_TYPE(target) < REB_TAG) {
		// Do the optimal search or the general search?
		if (BYTE_SIZE(series) && VAL_BYTE_SIZE(target) && !(flags & ~(AM_FIND_CASE|AM_FIND_MATCH))) {
			return Find_Byte_Str(series, start, VAL_BIN_DATA(target), len, !GET_FLAG(flags, ARG_FIND_CASE-1), GET_FLAG(flags, ARG_FIND_MATCH-1));
		} else if (flags & AM_FIND_ANY) {
			return Find_Str_Str_Any(series, start, index, end, skip, VAL_SERIES(target), VAL_INDEX(target), len, flags, wild);
		} else {
			return Find_Str_Str(series, start, index, end, skip, VAL_SERIES(target), VAL_INDEX(target), len, flags & (AM_FIND_MATCH | AM_FIND_CASE | AM_FIND_TAIL));
		}
	}
	else if (IS_TAG(target)) {
		return Find_Str_Tag(series, start, index, end, skip, VAL_SERIES(target), VAL_INDEX(target), len, flags & (AM_FIND_MATCH | AM_FIND_CASE | AM_FIND_TAIL));
	}
	//O: next condition is always false! It could be removed. 
	else if (IS_BINARY(target)) {
		return Find_Byte_Str(series, start, VAL_BIN_DATA(target), len, 0, GET_FLAG(flags, ARG_FIND_MATCH-1));
	}
	else if (IS_CHAR(target)) {
		return Find_Str_Char(series, start, index, end, skip, VAL_CHAR(target), flags);
	}
	else if (IS_INTEGER(target)) {
		return Find_Str_Char(series, start, index, end, skip, (REBUNI)VAL_INT32(target), flags);
	}
	else if (IS_BITSET(target)) {
		return Find_Str_Bitset(series, start, index, end, skip, VAL_SERIES(target), flags);
	}

	return NOT_FOUND;
}

static REBSER *make_string(REBVAL *arg, REBOOL make)
{
	REBSER *ser = 0;

	// MAKE <type> 123
	if (make && (IS_INTEGER(arg) || IS_DECIMAL(arg))) {
		ser = Make_Binary(Int32s(arg, 0));
	}
	// MAKE/TO <type> <binary!>
	else if (IS_BINARY(arg)) {
		ser = Decode_UTF_String(VAL_BIN_DATA(arg), VAL_LEN(arg), -1, FALSE, FALSE);
	}
	// MAKE/TO <type> <any-string>
	else if (ANY_BINSTR(arg)) {
		ser = Copy_String(VAL_SERIES(arg), VAL_INDEX(arg), VAL_LEN(arg));
	}
	// MAKE/TO <type> <any-word>
	else if (ANY_WORD(arg) || ANY_PATH(arg)) {
		ser = Copy_Form_Value(arg, TRUE);
		//ser = Append_UTF8(0, Get_Word_Name(arg), -1);
	}
	// MAKE/TO <type> #"A"
	else if (IS_CHAR(arg)) {
		ser = (VAL_CHAR(arg) > 0xff) ? Make_Unicode(2) : Make_Binary(2);
		Append_Byte(ser, VAL_CHAR(arg));
	}
	// MAKE/TO <type> <any-value>
//	else if (IS_NONE(arg)) {
//		ser = Make_Binary(0);
//	}
	else
		ser = Copy_Form_Value(arg, 1<<MOPT_TIGHT);

	return ser;
}

static REBSER *Make_Binary_BE64(REBVAL *arg)
{
	REBSER *ser = Make_Binary(9);
	REBI64 n = VAL_INT64(arg);
	REBINT count;
	REBYTE *bp = BIN_HEAD(ser);

	for (count = 7; count >= 0; count--) {
		bp[count] = (REBYTE)(n & 0xff);
		n >>= 8;
	}
	bp[8] = 0;
	ser->tail = 8;

	return ser;
}

static REBSER *make_binary(REBVAL *arg, REBOOL make)
{
	REBSER *ser;

	// MAKE BINARY! 123
	switch (VAL_TYPE(arg)) {
	case REB_INTEGER:
	case REB_DECIMAL:
		if (make) ser = Make_Binary(Int32s(arg, 0));
		else ser = Make_Binary_BE64(arg);
		break;

	// MAKE/TO BINARY! BINARY!
	case REB_BINARY:
		ser = Copy_Bytes(VAL_BIN_DATA(arg), VAL_LEN(arg));
		break;

	// MAKE/TO BINARY! <any-string>
	case REB_STRING:
	case REB_FILE:
	case REB_EMAIL:
	case REB_URL:
	case REB_TAG:
	case REB_REF:
//	case REB_ISSUE:
		ser = Encode_UTF8_Value(arg, VAL_LEN(arg), 0);
		break;

	// MAKE/TO BINARY! <vector!>
	case REB_VECTOR:
		// result is in little-endian!
		ser = Copy_Bytes(VAL_DATA(arg), VAL_LEN(arg) * VAL_VEC_WIDTH(arg));
		break;

	case REB_BLOCK:
		// Join_Binary returns a shared buffer, so produce a copy:
		ser = Copy_Series(Join_Binary(arg));
		break;

	// MAKE/TO BINARY! <tuple!>
	case REB_TUPLE:
		ser = Copy_Bytes(VAL_TUPLE(arg), VAL_TUPLE_LEN(arg));
		break;

	// MAKE/TO BINARY! <char!>
	case REB_CHAR:
		ser = Make_Binary(6);
		ser->tail = Encode_UTF8_Char(BIN_HEAD(ser), VAL_CHAR(arg));
		break;

	// MAKE/TO BINARY! <bitset!>
	case REB_BITSET:
		if(VAL_BITSET_NOT(arg)) {
			ser = Complement_Binary(arg);
		} else {
			ser = Copy_Bytes(VAL_BIN(arg), VAL_TAIL(arg));
		}
		break;

	// MAKE/TO BINARY! <image!>
	case REB_IMAGE:
	  	ser = Make_Image_Binary(arg);
		break;

	case REB_MONEY:
		ser = Make_Binary(12);
		ser->tail = 12;
		deci_to_binary(ser->data, VAL_DECI(arg));
		ser->data[12] = 0;
		break;

	case REB_STRUCT:
		ser = Copy_Series_Part(VAL_STRUCT_DATA_BIN(arg), VAL_STRUCT_OFFSET(arg), VAL_STRUCT_LEN(arg));
		break;

	default:
		ser = 0;
	}

	return ser;
}

/***********************************************************************
**
*/	REBFLG MT_String(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBCNT i;
	// allow only #[string! "data"] or #[string! "data" index]
	if (!(ANY_BINSTR(data) && (IS_END(data+1) || (IS_INTEGER(data+1) && IS_END(data+2)))))
		return FALSE;
	*out = *data++;
	VAL_SET(out, type);
	i = IS_INTEGER(data) ? Int32(data) - 1 : 0;
	if (i > VAL_TAIL(out)) i = VAL_TAIL(out); // clip it
	VAL_INDEX(out) = i;
	return TRUE;
}


/***********************************************************************
**
*/	static int Compare_Chr_Cased(const void *v1, const void *v2)
/*
***********************************************************************/
{
	return ((int)*(REBYTE*)v1) - ((int)*(REBYTE*)v2);
}


/***********************************************************************
**
*/	static int Compare_Chr_Cased_Rev(const void *v1, const void *v2)
/*
***********************************************************************/
{
	return ((int)*(REBYTE*)v2) - ((int)*(REBYTE*)v1);
}

/***********************************************************************
**
*/	static int Compare_Chr_Uncased(const void *v1, const void *v2)
/*
***********************************************************************/
{
	return ((int)LO_CASE(*(REBYTE*)v1)) - ((int)LO_CASE(*(REBYTE*)v2));
}


/***********************************************************************
**
*/	static int Compare_Chr_Uncased_Rev(const void *v1, const void *v2)
/*
***********************************************************************/
{
	return ((int)LO_CASE(*(REBYTE*)v2)) - ((int)LO_CASE(*(REBYTE*)v1));
}

// WARNING! Not re-entrant. !!!  Must find a way to push it on stack?
static struct {
	REBFLG cased;
	REBFLG reverse;
	REBCNT offset;
	REBVAL *compare;
	REBFLG wide;
} sort_flags = {0};

/***********************************************************************
**
*/	static int Compare_Call(const void *p1, const void *p2)
/*
***********************************************************************/
{
	REBVAL *v1;
	REBVAL *v2;
	REBVAL *val;
	REBVAL *tmp;

	// O: is there better way how to temporary use 2 values?
	DS_SKIP; v1 = DS_TOP;
	DS_SKIP; v2 = DS_TOP;

	if (sort_flags.wide) {
		SET_CHAR(v1, (int)(*(REBUNI*)p2));
		SET_CHAR(v2, (int)(*(REBUNI*)p1));
	} else {
		SET_CHAR(v1, (int)(*(REBYTE*)p2));
		SET_CHAR(v2, (int)(*(REBYTE*)p1));
	}
	
	if (sort_flags.reverse) {
		tmp = v1;
		v1 = v2;
		v2 = tmp;
	}

	val = Apply_Func(0, sort_flags.compare, v1, v2, 0);

	// v1 and v2 no more needed...
	DS_POP;
	DS_POP;

	if (IS_LOGIC(val)) {
		if (IS_TRUE(val)) return 1;
		return -1;
	}
	if (IS_INTEGER(val)) {
		if (VAL_INT64(val) < 0) return 1;
		if (VAL_INT64(val) == 0) return 0;
		return -1;
	}
	if (IS_DECIMAL(val)) {
		if (VAL_DECIMAL(val) < 0) return 1;
		if (VAL_DECIMAL(val) == 0) return 0;
		return -1;
	}
	return -1;
}


/***********************************************************************
**
*/	static void Sort_String(REBVAL *string, REBFLG ccase, REBVAL *skipv, REBVAL *compv, REBVAL *part, REBFLG all, REBFLG rev)
/*
***********************************************************************/
{
	REBCNT len;
	REBCNT skip = 1;
	REBCNT size = 1;
	REBSER *args;
	int (*sfunc)(const void *v1, const void *v2);

	// Determine length of sort:
	len = Partial(string, 0, part, 0);
	if (len <= 1) return;

	// Skip factor:
	if (!IS_NONE(skipv)) {
		skip = Get_Num_Arg(skipv);
		if (skip <= 0 || len % skip != 0 || skip > len)
			Trap_Arg(skipv);
	}

	// Use fast quicksort library function:
	if (skip > 1) len /= skip, size *= skip;

	if (ANY_FUNC(compv)) {
		// Check argument types of comparator function.
		args = VAL_FUNC_ARGS(compv);
		if (BLK_LEN(args) > 1 && !TYPE_CHECK(BLK_SKIP(args, 1), REB_CHAR))
			Trap3(RE_EXPECT_ARG, Of_Type(compv), BLK_SKIP(args, 1), Get_Type_Word(REB_CHAR));
		if (BLK_LEN(args) > 2 && !TYPE_CHECK(BLK_SKIP(args, 2), REB_CHAR))
			Trap3(RE_EXPECT_ARG, Of_Type(compv), BLK_SKIP(args, 2), Get_Type_Word(REB_CHAR));
		sort_flags.cased = ccase;
		sort_flags.reverse = rev;
		sort_flags.compare = 0;
		sort_flags.offset = 0;
		sort_flags.compare = compv;
		sort_flags.wide = 1 < SERIES_WIDE(VAL_SERIES(string));
		sfunc = Compare_Call;

	} else if (ccase) {
		sfunc = rev ? Compare_Chr_Cased_Rev : Compare_Chr_Cased;
	} else {
		sfunc = rev ? Compare_Chr_Uncased_Rev : Compare_Chr_Uncased;
	}

	//!!uni - needs to compare wide chars too
	reb_qsort((void *)VAL_DATA(string), len, size * SERIES_WIDE(VAL_SERIES(string)), sfunc);
}


/***********************************************************************
**
*/	REBINT PD_String(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *data = pvs->value;
	REBVAL *val = pvs->setval;
	REBINT n = 0;
	REBINT i;
	REBINT c;
	REBSER *ser = VAL_SERIES(data);

	if (IS_INTEGER(pvs->select) || IS_DECIMAL(pvs->select)) {
		i = Int32(pvs->select);
		if (i == 0) return PE_NONE; // like in case: path/0
		if (i < 0) i++;
		n = i + VAL_INDEX(data) - 1;
	}
	else return PE_BAD_SELECT;

	if (val == 0) {
		if (n < 0 || (REBCNT)n >= SERIES_TAIL(ser)) return PE_NONE;
		if (IS_BINARY(data)) {
			SET_INTEGER(pvs->store, *BIN_SKIP(ser, n));
		} else {
			SET_CHAR(pvs->store, GET_ANY_CHAR(ser, n));
		}
		return PE_USE;
	}

	if (n < 0 || (REBCNT)n >= SERIES_TAIL(ser)) return PE_BAD_RANGE;

	if (IS_CHAR(val)) {
		c = VAL_CHAR(val);
		if (c > MAX_CHAR) return PE_BAD_SET;
	}
	else if (IS_INTEGER(val)) {
		c = Int32(val);
		if (c > MAX_CHAR || c < 0) return PE_BAD_SET;
		if (IS_BINARY(data)) { // special case for binary
			if (c > 0xff) Trap_Range(val);
			BIN_HEAD(ser)[n] = (REBYTE)c;
			return PE_OK;
		}
	}
	else if (ANY_BINSTR(val)) {
		// for example: s: "abc" s/2: "xyz" s == "axc"
		if (VAL_INDEX(val) >= VAL_TAIL(val)) return PE_BAD_SET;
		c = GET_ANY_CHAR(VAL_SERIES(val), VAL_INDEX(val));
	}
	else
		return PE_BAD_SELECT;

	TRAP_PROTECT(ser);

	if (BYTE_SIZE(ser) && c > 0xff) Widen_String(ser);
	SET_ANY_CHAR(ser, n, c);

	return PE_OK;
}


/***********************************************************************
**
*/	REBINT PD_File(REBPVS *pvs)
/*
***********************************************************************/
{
	REBSER *ser;
	REB_MOLD mo = {0};
	REBCNT n;
	REBUNI c = 0;
	REBSER *arg;

	if (pvs->setval) return PE_BAD_SET;

	ser = Copy_Series_Value(pvs->value);

	n = SERIES_TAIL(ser);
	if (n > 0) c = GET_ANY_CHAR(ser, n-1);
	if (n == 0 || c != '/') Append_Byte(ser, '/');

	if (ANY_STR(pvs->select))
		arg = VAL_SERIES(pvs->select);
	else {
		Reset_Mold(&mo);
		Mold_Value(&mo, pvs->select, 0);
		arg = mo.series;
	}

	c = GET_ANY_CHAR(arg, 0);
	n = (c == '/' || c == '\\') ? 1 : 0;
	Append_String(ser, arg, n, arg->tail-n);

	Set_Series(VAL_TYPE(pvs->value), pvs->store, ser);

	return PE_USE;
}


/***********************************************************************
**
*/	REBTYPE(String)
/*
***********************************************************************/
{
	REBVAL	*value = D_ARG(1);
	REBVAL  *arg = D_ARG(2);
	REBINT	index = 0;
	REBINT	tail = 0;
	REBINT	len;
	REBSER  *ser;
	REBCNT  type;
	REBCNT	args;
	REBCNT	ret;

	if ((IS_FILE(value) || IS_URL(value)) && action >= PORT_ACTIONS) {
		return T_Port(ds, action);
	}

	len = Do_Series_Action(action, value, arg);
	if (len >= 0) return len;

	// Common setup code for all actions:
	if (action != A_MAKE && action != A_TO) {
		index = (REBINT)VAL_INDEX(value);
		tail  = (REBINT)VAL_TAIL(value);
		if (index > tail)
			VAL_INDEX(value) = index = tail;
	}

	// Check must be in this order (to avoid checking a non-series value);
	if (action >= A_TAKE && action <= A_SORT && IS_PROTECT_SERIES(VAL_SERIES(value)))
		Trap0(RE_PROTECTED);

	switch (action) {

	//-- Modification:
	case A_APPEND:
	case A_INSERT:
	case A_CHANGE:
		//Modify_String(action, value, arg);
		// Length of target (may modify index): (arg can be anything)
		len = Partial1((action == A_CHANGE) ? value : arg, DS_ARG(AN_LENGTH));
		index = VAL_INDEX(value);
		args = 0;
		if (IS_BINARY(value)) SET_FLAG(args, AN_SERIES); // special purpose
		if (DS_REF(AN_PART)) SET_FLAG(args, AN_PART);
		index = Modify_String(action, VAL_SERIES(value), index, arg, args, len, DS_REF(AN_DUP) ? Int32(DS_ARG(AN_COUNT)) : 1);
		VAL_INDEX(value) = index;
		break;

	//-- Search:
	case A_SELECT:
		ret = ALL_SELECT_REFS;
		goto find;
	case A_FIND:
		ret = ALL_FIND_REFS;
find:
		args = Find_Refines(ds, ret);

		if (IS_BINARY(value)) {
			args |= AM_FIND_CASE;
			if (!IS_BINARY(arg) && !IS_INTEGER(arg) && !IS_BITSET(arg) && !IS_CHAR(arg)) Trap0(RE_NOT_SAME_TYPE);
			if (IS_INTEGER(arg)) {
				if (VAL_INT64(arg) < 0 || VAL_INT64(arg) > 255) Trap_Range(arg);
				len = 1;
			}
		}
		else {
			if (IS_CHAR(arg) || IS_BITSET(arg)) len = 1;
			else if (!ANY_STR(arg)) {
				Set_String(arg, Copy_Form_Value(arg, 0));
			}
		}

		if (ANY_BINSTR(arg)) len = VAL_LEN(arg);

		if (args & AM_FIND_PART) tail = index + Partial(value, 0, D_ARG(ARG_FIND_RANGE), 0);
		ret = 1; // skip size
		if (args & AM_FIND_SKIP) {
			ret = Partial(value, 0, D_ARG(ARG_FIND_SIZE), 0);
			if(!ret) goto is_none;
		}

		if (action == A_SELECT) args |= AM_FIND_TAIL;

		ret = find_string(VAL_SERIES(value), index, tail, arg, len, args, ret, D_ARG(ARG_FIND_WILD));

		if (ret > (REBCNT)tail) goto is_none;
		if (args & AM_FIND_ONLY) len = 1;

		if (action == A_FIND) {
			VAL_INDEX(value) = ret;
		}
		else {
			if (ret >= (REBCNT)tail) goto is_none;
			if (IS_BINARY(value)) {
				SET_INTEGER(value, *BIN_SKIP(VAL_SERIES(value), ret));
			}
			else
				str_to_char(value, value, ret);
		}
		break;

	//-- Picking:
	case A_PICK:
	case A_POKE:
		len = Get_Num_Arg(arg); // Position
		if (len < 0) REB_I32_ADD_OF(index, 1, &index);
		if (len == 0
			|| REB_I32_SUB_OF(len, 1, &len)
			|| REB_I32_ADD_OF(index, len, &index)
			|| index < 0 || index >= tail) {
			if (action == A_PICK) goto is_none;
			Trap_Range(arg);
		}
		if (action == A_PICK) {
pick_it:
			if (IS_BINARY(value)) {
				SET_INTEGER(DS_RETURN, *VAL_BIN_SKIP(value, index));
			}
			else
				str_to_char(DS_RETURN, value, index);
			return R_RET;
		}
		else {
			REBUNI c = 0;
			arg = D_ARG(3);
			if (IS_CHAR(arg))
				c = VAL_CHAR(arg);
			else if (IS_INTEGER(arg) && VAL_UNT64(arg) <= MAX_CHAR)
				c = VAL_INT32(arg);
			else Trap_Arg(arg);

			ser = VAL_SERIES(value);
			if (IS_BINARY(value)) {
				if (c > 0xff) Trap_Range(arg);
				BIN_HEAD(ser)[index] = (REBYTE)c;
			}
			else {
				if (BYTE_SIZE(ser) && c > 0xff) Widen_String(ser);
				SET_ANY_CHAR(ser, index, c);
			}
			value = arg;
		}
		break;

	case A_TAKE:
		if (D_REF(2)) {
			len = Partial(value, 0, D_ARG(3), 0);
			if (len == 0) {
zero_str:
				Set_Series(VAL_TYPE(value), D_RET, Make_Binary(0));
				return R_RET;
			}
		} else 
			len = 1;

		index = VAL_INDEX(value); // /part can change index

		// take/last:
		if (tail <= index) goto is_none;
		if (D_REF(5)) index = tail - len;
		if (index < 0 || index >= tail) {
			if (!D_REF(2)) goto is_none;
			goto zero_str;
		}

		ser = VAL_SERIES(value);
		// if no /part, just return value, else return string:
		if (!D_REF(2)) {
			if (IS_BINARY(value)) {
				SET_INTEGER(value, *VAL_BIN_SKIP(value, index));
			} else
				str_to_char(value, value, index);
		}
		else Set_Series(VAL_TYPE(value), value, Copy_String(ser, index, len));
		Remove_Series(ser, index, len);
		break;

	case A_CLEAR:
		if (index < tail) {
			if (index == 0) Reset_Series(VAL_SERIES(value));
			else {
				VAL_TAIL(value) = (REBCNT)index;
				TERM_SERIES(VAL_SERIES(value));
			}
		}
		break;

	//-- Creation:

	case A_COPY:
		len = Partial(value, 0, D_ARG(3), 0); // Can modify value index.
		ser = Copy_String(VAL_SERIES(value), VAL_INDEX(value), len);
		goto ser_exit;

	case A_MAKE:
	case A_TO:
		// Determine the datatype to create:
		type = VAL_TYPE(value);
		if (type == REB_DATATYPE) type = VAL_DATATYPE(value);

		if (IS_NONE(arg)) Trap_Make(type, arg);

		ser = (type != REB_BINARY) 
			? make_string(arg, (REBOOL)(action == A_MAKE))
			: make_binary(arg, (REBOOL)(action == A_MAKE));

		if (ser) goto str_exit;
		Trap_Arg(arg);

	//-- Bitwise:

	case A_AND:
	case A_OR:
	case A_XOR:
		if (!IS_BINARY(arg)) Trap_Arg(arg);
		VAL_LIMIT_SERIES(value);
		VAL_LIMIT_SERIES(arg);
		ser = Xandor_Binary(action, value, arg);
		goto ser_exit;

	case A_COMPLEMENT:
		if (!IS_BINARY(arg)) Trap_Arg(arg);
		ser = Complement_Binary(value);
		goto ser_exit;

	//-- Special actions:

	case A_TRIM:
		// Check for valid arg combinations:
		args = Find_Refines(ds, ALL_TRIM_REFS);
		if (
			((args & (AM_TRIM_ALL | AM_TRIM_WITH)) &&
			(args & (AM_TRIM_HEAD | AM_TRIM_TAIL | AM_TRIM_LINES | AM_TRIM_AUTO))) ||
			((args & AM_TRIM_AUTO) &&
			(args & (AM_TRIM_HEAD | AM_TRIM_TAIL | AM_TRIM_LINES | AM_TRIM_ALL | AM_TRIM_WITH)))
		)
			Trap0(RE_BAD_REFINES);
		if (IS_BINARY(value))
			Trim_Binary(VAL_SERIES(value), VAL_INDEX(value), VAL_LEN(value), args, D_ARG(ARG_TRIM_STR));
		else
			Trim_String(VAL_SERIES(value), VAL_INDEX(value), VAL_LEN(value), args, D_ARG(ARG_TRIM_STR));
		break;

	case A_SWAP:
		if (VAL_TYPE(value) != VAL_TYPE(arg)) Trap0(RE_NOT_SAME_TYPE);
		if (IS_PROTECT_SERIES(VAL_SERIES(arg))) Trap0(RE_PROTECTED);
		if (index < tail && VAL_INDEX(arg) < VAL_TAIL(arg))
			swap_chars(value, arg);
		// Trap_Range(arg);  // ignore range error
		break;

	case A_REVERSE:
		len = Partial(value, 0, D_ARG(3), 0);
		if (len > 0) reverse_string(value, len);
		break;

	case A_SORT:
		Sort_String(
			value,
			D_REF(2),	// case sensitive
			D_ARG(4),	// skip size
			D_ARG(6),	// comparator
			D_ARG(8),	// part-length
			D_REF(9),	// all fields
			D_REF(10)	// reverse
		);
		break;

	case A_RANDOM:
		if(IS_PROTECT_SERIES(VAL_SERIES(value))) Trap0(RE_PROTECTED);
		if (D_REF(2)) { // seed
			Set_Random(Compute_CRC24(VAL_BIN_DATA(value), VAL_LEN(value)));
			return R_UNSET;
		}
		if (D_REF(4)) { // /only
			if (index >= tail) goto is_none;
			index += (REBCNT)Random_Int(D_REF(3)) % (tail - index);  // /secure
			goto pick_it;
		}
		Shuffle_String(value, D_REF(3));  // /secure
		break;

	default:
		Trap_Action(VAL_TYPE(value), action);
	}

	DS_RET_VALUE(value);
	return R_RET;

ser_exit:
	type = VAL_TYPE(value);
str_exit:
	Set_Series(type, D_RET, ser);
	return R_RET;

is_none:
	return R_NONE;
}


#ifdef oldcode
/***********************************************************************
**
x*/	void Modify_StringX(REBCNT action, REBVAL *string, REBVAL *arg)
/*
**		Actions: INSERT, APPEND, CHANGE
**
**		string [string!] {Series at point to insert}
**		value [any-type!] {The value to insert}
**		/part {Limits to a given length or position.}
**		length [number! series! pair!]
**		/only {Inserts a series as a series.}
**		/dup {Duplicates the insert a specified number of times.}
**		count [number! pair!]
**
***********************************************************************/
{
	REBSER *series = VAL_SERIES(string);
	REBCNT index = VAL_INDEX(string);
	REBCNT tail  = VAL_TAIL(string);
	REBINT rlen;  // length to be removed
	REBINT ilen  = 1;  // length to be inserted
	REBINT cnt   = 1;  // DUP count
	REBINT size;
	REBVAL *val;
	REBSER *arg_ser = 0; // argument series

	// Length of target (may modify index): (arg can be anything)
	rlen = Partial1((action == A_CHANGE) ? string : arg, DS_ARG(AN_LENGTH));

	index = VAL_INDEX(string);
	if (action == A_APPEND || index > tail) index = tail;

	// If the arg is not a string, then we need to create a string:
	if (IS_BINARY(string)) {
		if (IS_INTEGER(arg)) {
			if (VAL_INT64(arg) > 255 || VAL_INT64(arg) < 0)
				Trap_Range(arg);
			arg_ser = Make_Binary(1);
			Append_Byte(arg_ser, VAL_CHAR(arg)); // check for size!!!
		}
		else if (!ANY_BINSTR(arg)) Trap_Arg(arg);
	}
	else if (IS_BLOCK(arg)) {
		// MOVE!
		REB_MOLD mo = {0};
		arg_ser = mo.series = Make_Unicode(VAL_BLK_LEN(arg) * 10); // GC!?
		for (val = VAL_BLK_DATA(arg); NOT_END(val); val++)
			Mold_Value(&mo, val, 0);
	}
	else if (IS_CHAR(arg)) {
		// Optimize this case !!!
		arg_ser = Make_Unicode(1);
		Append_Byte(arg_ser, VAL_CHAR(arg));
	}
	else if (!ANY_STR(arg) || IS_TAG(arg)) {
		arg_ser = Copy_Form_Value(arg, 0);
	}
	if (arg_ser) Set_String(arg, arg_ser);
	else arg_ser = VAL_SERIES(arg);

	// Length of insertion:
	ilen = (action != A_CHANGE && DS_REF(AN_PART)) ? rlen : VAL_LEN(arg);

	// If Source == Destination we need to prevent possible conflicts.
	// Clone the argument just to be safe.
	// (Note: It may be possible to optimize special cases like append !!)
	if (series == VAL_SERIES(arg)) {
		arg_ser = Copy_Series_Part(arg_ser, VAL_INDEX(arg), ilen);  // GC!?
	}

	// Get /DUP count:
	if (DS_REF(AN_DUP)) {
		cnt = Int32(DS_ARG(AN_COUNT));
		if (cnt <= 0) return; // no changes
	}

	// Total to insert:
	size = cnt * ilen;

	if (action != A_CHANGE) {
		// Always expand series for INSERT and APPEND actions:
		Expand_Series(series, index, size);
	} else {
		if (size > rlen) 
			Expand_Series(series, index, size-rlen);
		else if (size < rlen && DS_REF(AN_PART))
			Remove_Series(series, index, rlen-size);
		else if (size + index > tail) {
			EXPAND_SERIES_TAIL(series, size - (tail - index));
		}
	}

	// For dup count:
	for (; cnt > 0; cnt--) {
		Insert_String(series, index, arg_ser, VAL_INDEX(arg), ilen, TRUE);
		index += ilen;
	}

	TERM_SERIES(series);

	VAL_INDEX(string) = (action == A_APPEND) ? 0 : index;
}
#endif
