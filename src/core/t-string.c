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
	SET_CHAR(out, GET_UTF8_CHAR(VAL_SERIES(val), idx));
}

static void swap_chars(REBVAL *val1, REBVAL *val2)
{
	REBSER *s1 = VAL_SERIES(val1);
	REBSER *s2 = VAL_SERIES(val2);
	REBLEN  i1 = VAL_INDEX(val1);
	REBLEN  i2 = VAL_INDEX(val2);

	if (IS_UTF8_SERIES(s1) || IS_UTF8_SERIES(s2)) {
		REBU32 c1 = UTF8_Get_Codepoint(BIN_SKIP(s1, i1));
		REBU32 c2 = UTF8_Get_Codepoint(BIN_SKIP(s2, i2));
		// Replacing a character with different width may invalidate index!
        // Replace higher index first to keep lower index valid
        // when both operate on the same series buffer
		if (i2 > i1) {
			SET_ANY_CHAR(s2, i2, c1);
			SET_ANY_CHAR(s1, i1, c2);
		}
		else {
			SET_ANY_CHAR(s1, i1, c2);
			SET_ANY_CHAR(s2, i2, c1);
		}
	}
	else {
		// Fast byte swap
		REBYTE tmp = BIN_HEAD(s1)[i1];
		BIN_HEAD(s1)[i1] = BIN_HEAD(s2)[i2];
		BIN_HEAD(s2)[i2] = tmp;
	}
}

static void reverse_string(REBVAL *value, REBCNT len)
{
	REBCNT n;
	REBCNT m;
	REBUNI c;

	if (IS_UTF8_STRING(value)) {
		REBYTE *out = Reset_Buffer(BUF_SCAN, len);
		const REBYTE *bp = VAL_BIN(value);
		REBUNI index = VAL_TAIL(value);
		REBCNT bytes;
		while (index > VAL_INDEX(value)) {
			bytes = UTF8_Prev_Char_Size(VAL_BIN(value), index);
			index -= bytes;
			bp = VAL_BIN_SKIP(value, index);
			REBU32 chr = UTF8_Decode_Codepoint(&bp, &bytes);
			out += Encode_UTF8_Char(out, chr);
		}
		COPY_MEM(VAL_BIN_DATA(value), BIN_HEAD(BUF_SCAN), len);

	}
	else {
		REBYTE *bp = VAL_BIN_DATA(value);

		for (n = 0, m = len-1; n < len / 2; n++, m--) {
			c = bp[n];
			bp[n] = bp[m];
			bp[m] = (REBYTE)c;
		}
	}
}

static REBCNT find_string(REBVAL *value, REBCNT index, REBCNT end, REBVAL *target, REBCNT len, REBCNT flags, REBINT skip, REBVAL *wild)
{
	REBSER *series = VAL_SERIES(value);
	REBCNT start = index;

	if (flags & (AM_FIND_REVERSE | AM_FIND_LAST)) {
		skip = -1;
		if (flags & AM_FIND_LAST) {
			start = index;
			index = end - len;
		}
		else {
			start = 0;
			index--;
		}
	}

	if (flags & AM_FIND_SAME) flags |= AM_FIND_CASE; // /SAME has same functionality as /CASE for any-string!

	//O: not using ANY_BINSTR as TAG is now handled separately
	if (VAL_TYPE(target) >= REB_BINARY && VAL_TYPE(target) < REB_TAG) {
		// Do the optimal search or the general search?
		if ((IS_BINARY(value) || (!IS_UTF8_SERIES(series) && !IS_UTF8_STRING(target))) && !(flags & ~(AM_FIND_CASE|AM_FIND_MATCH|AM_FIND_TAIL))) {
			index = Find_Byte_Str(series, start, VAL_BIN_DATA(target), len, !GET_FLAG(flags, ARG_FIND_CASE-1), GET_FLAG(flags, ARG_FIND_MATCH-1));
			if (flags & AM_FIND_TAIL && index != NOT_FOUND) index += len;
			return index;
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
		return Find_Str_Char(series, start, index, end, skip, VAL_INT32(target), flags);
	}
	else if (IS_BITSET(target)) {
		return Find_Str_Bitset(series, start, index, end, skip, VAL_SERIES(target), flags);
	}

	return NOT_FOUND;
}

static REBSER *make_string(REBVAL *arg, REBOOL make, REBCNT type)
{
	REBSER *ser = 0;

	// MAKE <type> 123
	if (make && (IS_INTEGER(arg) || IS_DECIMAL(arg))) {
		ser = Make_Binary(Int32s(arg, 0));
	}
	// MAKE/TO <type> <binary!>
	// MAKE/TO <type> <any-string>
	else if (IS_BINARY(arg)) {
		REBCNT err = NOT_FOUND;
		ser = Decode_UTF_String(VAL_BIN_AT(arg), VAL_LEN(arg), -1, FALSE, &err);
		if (!ser) {
			VAL_INDEX(arg) = err;
			Trap1(RE_INVALID_UTF, arg);
		}
	}
	else if (ANY_STR(arg)) {
		ser = Copy_String(VAL_SERIES(arg), VAL_INDEX(arg), VAL_LEN(arg));
	}
	// MAKE/TO <type> <any-word>
	else if (ANY_WORD(arg) || ANY_PATH(arg)) {
		ser = Form_Value(arg, TRUE, TRUE);
		//ser = Append_UTF8(0, Get_Word_Name(arg), -1);
	}
	// MAKE/TO <type> #"A"
	else if (IS_CHAR(arg)) {
		ser = Append_Byte(ser, VAL_CHAR(arg));
		if (ser->tail > 1) UTF8_SERIES(ser);
	}
	// MAKE/TO <type> <any-value>
//	else if (IS_NONE(arg)) {
//		ser = Make_Binary(0);
//	}
	else if (IS_BLOCK(arg) && (type == REB_EMAIL || type == REB_URL)) {
		if (VAL_LEN(arg) == 0) Trap_Make(REB_EMAIL, arg);
		REBVAL* val = VAL_BLK_DATA(arg);
		if (type == REB_EMAIL) {
			REBCNT sep = '@';
			ser = Form_Value(val, FLAGIT(MOPT_TIGHT), TRUE);
			while (!IS_END(++val)) {
				Append_Byte(ser, sep);
				sep = '.';
				Modify_String(A_APPEND, ser, SERIES_TAIL(ser), val, 0, 0, 1);
			}
		}
		else {// url
			ser = Form_Value(val, FLAGIT(MOPT_TIGHT), TRUE);
			Append_Bytes(ser, "://");
			REBFLG first = TRUE;
			while (!IS_END(++val)) {
				if (first) first = FALSE;
				else Append_Byte(ser, '/');
				Modify_String(A_APPEND, ser, SERIES_TAIL(ser), val, 0, 0, 1);
			}
		}	
	}
	else
		ser = Form_Value(arg, 1<<MOPT_TIGHT, TRUE);

	return ser;
}

REBSER *Make_Binary_BE64(REBVAL *arg)
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
	REBSER *ser = NULL;

	// MAKE BINARY! 123
	switch (VAL_TYPE(arg)) {
	case REB_INTEGER:
	case REB_DECIMAL:
		if (make) ser = Make_Binary(Int32s(arg, 0));
		else ser = Make_Binary_BE64(arg);
		break;

	// MAKE/TO BINARY! BINARY!
	// MAKE/TO BINARY! <any-string>
	case REB_BINARY:
	case REB_STRING:
	case REB_FILE:
	case REB_EMAIL:
	case REB_URL:
	case REB_TAG:
	case REB_REF:
	//case REB_ISSUE:
		ser = Copy_Bytes(VAL_BIN_DATA(arg), VAL_LEN(arg));
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
		ser = Append_Byte(ser, VAL_CHAR(arg));
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
		ser = Copy_Series_Part(VAL_STRUCT_DATA(arg), VAL_STRUCT_OFFSET(arg), VAL_STRUCT_SIZE(arg));
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


static int Compare_Chr_Cased(const void *v1, const void *v2) {
	return ((int)*(REBYTE*)v1) - ((int)*(REBYTE*)v2);
}
static int Compare_Chr_Cased_Rev(const void *v1, const void *v2) {
	return ((int)*(REBYTE*)v2) - ((int)*(REBYTE*)v1);
}
static int Compare_Chr_Uncased(const void *v1, const void *v2) {
	return ((int)LO_CASE(*(REBYTE*)v1)) - ((int)LO_CASE(*(REBYTE*)v2));
}
static int Compare_Chr_Uncased_Rev(const void *v1, const void *v2) {
	return ((int)LO_CASE(*(REBYTE*)v2)) - ((int)LO_CASE(*(REBYTE*)v1));
}

static int Compare_U32_Cased(const void *v1, const void *v2) {
	return ((int)*(REBU32 *)v1) - ((int)*(REBU32 *)v2);
}
static int Compare_U32_Cased_Rev(const void *v1, const void *v2) {
	return ((int)*(REBU32 *)v2) - ((int)*(REBU32 *)v1);
}
static int Compare_U32_Uncased(const void *v1, const void *v2) {
	REBU32 a = *(REBU32 *)v1;
	REBU32 b = *(REBU32 *)v2;
	if (a < UNICODE_CASES) a = LO_CASE(a);
	if (b < UNICODE_CASES) b = LO_CASE(b);
	return ((int)a - (int)b);
}
static int Compare_U32_Uncased_Rev(const void *v1, const void *v2) {
	REBU32 a = *(REBU32 *)v1;
	REBU32 b = *(REBU32 *)v2;
	if (a < UNICODE_CASES) a = LO_CASE(a);
	if (b < UNICODE_CASES) b = LO_CASE(b);
	return ((int)b - (int)a);
}

static int Compare_All_Chr_Cased(const void *v1, const void *v2) {
	REBCNT size = VAL_UNT32(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;
	while (size-- > 0 && result == 0) {
		result = ((int)*((REBYTE *)v1+offset)) - ((int)*((REBYTE *)v2 + offset));
		offset++;
	}
	return result;
}
static int Compare_All_Chr_Cased_Rev(const void *v1, const void *v2) {
	REBCNT size = VAL_UNT32(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;
	while (size-- > 0 && result == 0) {
		result = ((int)*((REBYTE *)v2 + offset)) - ((int)*((REBYTE *)v1 + offset));
		offset++;
	}
	return result;
}
static int Compare_All_Chr_Uncased(const void *v1, const void *v2) {
	REBCNT size = VAL_UNT32(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;
	while (size-- > 0 && result == 0) {
		result = ((int)LO_CASE(*((REBYTE *)v1 + offset))) - ((int)LO_CASE(*((REBYTE *)v2 + offset)));
		offset++;
	}
	return result;
}
static int Compare_All_Chr_Uncased_Rev(const void *v1, const void *v2) {
	REBCNT size = VAL_UNT32(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;
	while (size-- > 0 && result == 0) {
		result = ((int)LO_CASE(*((REBYTE *)v2 + offset))) - ((int)LO_CASE(*((REBYTE *)v1 + offset)));
		offset++;
	}
	return result;
}

static int Compare_All_U32_Cased(const void *v1, const void *v2) {
	REBCNT size = VAL_UNT32(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;
	while (size-- > 0 && result == 0) {
		result = ((int)*((REBU32 *)v1 + offset)) - ((int)*((REBU32 *)v2 + offset));
		offset++;
	}
	return result;
}
static int Compare_All_U32_Cased_Rev(const void *v1, const void *v2) {
	REBCNT size = VAL_UNT32(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;
	while (size-- > 0 && result == 0) {
		result = ((int)*((REBU32 *)v2 + offset)) - ((int)*((REBU32 *)v1 + offset));
		offset++;
	}
	return result;
}
static int Compare_All_U32_Uncased(const void *v1, const void *v2) {
	REBCNT size = VAL_UNT32(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;
	REBU32 a, b;
	while (size-- > 0 && result == 0) {
		a = *((REBU32 *)v1 + offset);
		b = *((REBU32 *)v2 + offset);
		if (a < UNICODE_CASES) a = LO_CASE(a);
		if (b < UNICODE_CASES) b = LO_CASE(b);
		result = (int)a - (int)b;
		offset++;
	}
	return result;
}
static int Compare_All_U32_Uncased_Rev(const void *v1, const void *v2) {
	REBCNT size = VAL_UNT32(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;
	REBU32 a, b;
	while (size-- > 0 && result == 0) {
		a = *((REBU32 *)v1 + offset);
		b = *((REBU32 *)v2 + offset);
		if (a < UNICODE_CASES) a = LO_CASE(a);
		if (b < UNICODE_CASES) b = LO_CASE(b);
		result = (int)b - (int)1;
		offset++;
	}
	return result;
}

static int Compare_Comp(const void *v1, const void *v2) {
	REBINT offset = VAL_INT64(DS_GET(DSP - 1));
	REBU64 flags  = VAL_UNT64(DS_TOP);
	REBU32 a, b;
	if (GET_FLAG(flags, SORT_FLAG_WIDE)) {
		a = *((REBU32 *)v1 + offset);
		b = *((REBU32 *)v2 + offset);
	}
	else {
		a = *((REBYTE *)v1 + offset);
		b = *((REBYTE *)v2 + offset);
	}
	if (!GET_FLAG(flags, SORT_FLAG_CASE)) {
		if (a < UNICODE_CASES) a = LO_CASE(a);
		if (b < UNICODE_CASES) b = LO_CASE(b);
	}
	return (GET_FLAG(flags, SORT_FLAG_REVERSE))
		? (int)b - (int)a
		: (int)a - (int)b;
}

static int Compare_Call(const void *p1, const void *p2) {
	REBVAL *v1;
	REBVAL *v2;
	REBVAL *val = NULL;
	REBVAL *func;
	REBU64 flags;
	REBCNT count;
	REBINT result = -1;

	count = VAL_UNT64(DS_GET(DSP - 2)); // > 1 when /all is used
	func  = DS_GET(DSP - 1);
	flags = VAL_UNT64(DS_TOP);

	if (!count) return 0;


	DS_SKIP; v1 = DS_TOP;
	DS_SKIP; v2 = DS_TOP;

	if (count == 1) {
		// We apply the custom compare function to 2 chars.
		if (GET_FLAG(flags, SORT_FLAG_WIDE)) {
			SET_CHAR(v1, (int)(*(REBU32 *)p2));
			SET_CHAR(v2, (int)(*(REBU32 *)p1));
		}
		else {
			SET_CHAR(v1, (int)(*(REBYTE *)p2));
			SET_CHAR(v2, (int)(*(REBYTE *)p1));
		}
	}
	else {
		if (GET_FLAG(flags, SORT_FLAG_WIDE)) {
			Set_String(v1, UTF32_To_UTF8(NULL, (REBYTE *)p2, count * 4, OS_LITTLE_ENDIAN));
			Set_String(v2, UTF32_To_UTF8(NULL, (REBYTE *)p1, count * 4, OS_LITTLE_ENDIAN));
		}
		else {
			Set_String(v1, Copy_Bytes((REBYTE *)p2, count));
			Set_String(v2, Copy_Bytes((REBYTE *)p1, count));
			if (GET_FLAG(flags, SORT_FLAG_BINARY)) {
				VAL_TYPE(v1) = REB_BINARY;
				VAL_TYPE(v2) = REB_BINARY;
			}
		}
	}

	val = Apply_Func(0, func, v1, v2, 0);

	// v1 and v2 no more needed...
	DS_DROP;
	DS_DROP;

	if (IS_LOGIC(val)) {
		if (IS_TRUE(val)) result = 1;
	}
	else if (IS_INTEGER(val)) {
		if (VAL_INT64(val) < 0) result = 1;
		if (VAL_INT64(val) == 0) result = 0;
	}
	else if (IS_DECIMAL(val)) {
		if (VAL_DECIMAL(val) < 0) result = 1;
		if (VAL_DECIMAL(val) == 0) result = 0;
	}
	if (GET_FLAG(flags, SORT_FLAG_REVERSE)) result = -result;
	return result;
}

typedef int (*cmp_func)(const void *, const void *);
// [all][case][width][rev]
static const cmp_func sfunc_table[2][2][2][2] = {
	// all == 0: not-All
	{
		// ccase == 0: uncased
		{
			{ Compare_Chr_Uncased,   Compare_Chr_Uncased_Rev },
			{ Compare_U32_Uncased,   Compare_U32_Uncased_Rev }
		},
		// ccase == 1: cased
		{
			{ Compare_Chr_Cased,     Compare_Chr_Cased_Rev },
			{ Compare_U32_Cased,     Compare_U32_Cased_Rev }
		}
	},
	// all == 1: All
	{
		// ccase == 0: uncased
		{
			{ Compare_All_Chr_Uncased,   Compare_All_Chr_Uncased_Rev },
			{ Compare_All_U32_Uncased,   Compare_All_U32_Uncased_Rev }
		},
		// ccase == 1: cased
		{
			{ Compare_All_Chr_Cased,     Compare_All_Chr_Cased_Rev },
			{ Compare_All_U32_Cased,     Compare_All_U32_Cased_Rev }
		}
	  }
};

/***********************************************************************
**
*/	static void Sort_String(REBVAL *string, REBFLG ccase, REBVAL *skipv, REBVAL *compv, REBVAL *part, REBFLG all, REBFLG rev, REBFLG uns)
/*
***********************************************************************/
{
	REBCNT len;
	REBCNT skip = 1;
	REBCNT size = 1;
	REBSER *args;
	REBYTE *str_bin;
	REBCNT wide = 1;
	REBU64 flags = 0;
	int (*sfunc)(const void *v1, const void *v2);

	ASSERT1(BYTE_SIZE(VAL_SERIES(string)), RP_BAD_SIZE);

	// Determine length of sort:
	len = Partial(string, 0, part, 0);
	if (len <= 1) return;

	if (IS_UTF8_STRING(string)) {
		UTF8_To_UTF32(BUF_SCAN, VAL_DATA(string), len, OS_LITTLE_ENDIAN);
		str_bin = BIN_HEAD(BUF_SCAN);
		wide = 4;
		len = SERIES_TAIL(BUF_SCAN) / 4;
	}
	else
		str_bin = VAL_DATA(string);

	// Skip factor:
	if (!IS_NONE(skipv)) {
		skip = Get_Num_Arg(skipv);
		if (skip <= 0 || len % skip != 0 || skip > len)
			Trap_Arg(skipv);
	}

	// Use fast quicksort library function:
	if (skip > 1) len /= skip, size *= skip;

	if (!IS_NONE(compv)) {
		if (rev) SET_FLAG(flags, SORT_FLAG_REVERSE);
		if (all) SET_FLAG(flags, SORT_FLAG_ALL);
		if (IS_UTF8_STRING(string)) SET_FLAG(flags, SORT_FLAG_WIDE);
	}
	if (ANY_FUNC(compv)) {
		// Check argument types of the comparator function.
		args = VAL_FUNC_ARGS(compv);
		REBCNT type = all ? REB_STRING : REB_CHAR;
		if (IS_BINARY(string)) {
			SET_FLAG(flags, SORT_FLAG_BINARY);
			type = REB_BINARY;
		}
		if (BLK_LEN(args) > 1 && !TYPE_CHECK(BLK_SKIP(args, 1), type))
			Trap3(RE_EXPECT_ARG, Of_Type(compv), BLK_SKIP(args, 1), Get_Type_Word(type));
		if (BLK_LEN(args) > 2 && !TYPE_CHECK(BLK_SKIP(args, 2), type))
			Trap3(RE_EXPECT_ARG, Of_Type(compv), BLK_SKIP(args, 2), Get_Type_Word(type));

		// Store the skip (used to implement /all)
		DS_PUSH_INTEGER(all ? skip : 1);

		// Store the comparator function and flags on the stack
		DS_PUSH(compv);
		DS_PUSH_INTEGER(flags);
		sfunc = Compare_Call;

	}
	else if (IS_INTEGER(compv)) {
		// Using the offset comparator
		if (all) Trap0(RE_BAD_REFINES); // not compatible
		REBI64 ofs = VAL_INT64(compv);
		if (ofs < 1 || ofs > skip || IS_NONE(skipv)) Trap_Arg(compv);
		if (ccase) SET_FLAG(flags, SORT_FLAG_CASE);
		DS_PUSH_INTEGER(ofs-1);
		DS_PUSH_INTEGER(flags);
		sfunc = Compare_Comp;
	}
	else if (IS_BLOCK(compv)) {
		// Not implemented for string series.
		Trap0(RE_FEATURE_NA);
	}
	else {
		// Store the skip (used to implement /all)
		DS_PUSH_INTEGER(all ? skip : 1);

		if (all && !IS_NONE(compv)) Trap0(RE_BAD_REFINES);
		sfunc = sfunc_table[all][ccase][wide != 1][rev];
	}
	if (uns) {
		unstable_sort((void*)str_bin, len, size * wide, sfunc);
	}
	else {
		stable_sort((void*)str_bin, len, size * wide, sfunc);
	}
	if (wide == 4) {
		UTF32_To_UTF8(VAL_SERIES(string), str_bin, len*4*skip, OS_LITTLE_ENDIAN);
	}

	DS_DROP; // Stored skip or offset
	if (ANY_FUNC(compv)) {
		// Stored comparator and flags are not needed anymore
		DS_DROP;
		DS_DROP;
	}
	else if (IS_INTEGER(compv)) {
		DS_DROP; // Stored flags
	}
}

FORCE_INLINE
/***********************************************************************
**
*/	REBLEN Skip_UTF8_String(REBVAL *str, REBINT chars)
/*
***********************************************************************/
{
	REBLEN pos = VAL_INDEX(str);
	REBLEN tail;
	const REBYTE *bin = VAL_BIN_HEAD(str);

	if (chars > 0) {
		tail = VAL_TAIL(str);
		while (pos < tail && chars-- > 0) {
			pos += UTF8_Next_Char_Size(bin, pos);
		}
		if (chars > 0) return NOT_FOUND;
	}
	else if (chars < 0) {
		while (pos > 0 && chars++ < 0) {
			pos -= UTF8_Prev_Char_Size(bin, pos);
		}
		if (chars < 0) return NOT_FOUND;
	}
	return pos;

}

/***********************************************************************
**
*/	REBINT PD_String(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *data = pvs->value;
	REBVAL *val = pvs->setval;
	REBLEN n = 0;
	REBINT i;
	REBINT c;
	REBSER *ser = VAL_SERIES(data);

	if (IS_INTEGER(pvs->select) || IS_DECIMAL(pvs->select)) {
		i = Int32(pvs->select);
		if (i == 0)
			return (pvs->setval) ? PE_BAD_RANGE : PE_NONE;
		if (i < 0) i++;
		if (IS_UTF8_SERIES(ser)) {
			n = i - 1;
			n = Skip_UTF8_String(data, n);
			if (n == NOT_FOUND || n >= VAL_TAIL(data)) return PE_NONE;
		}
		else {
			n = i + VAL_INDEX(data) - 1;
		}
	}
	else if (IS_WORD(pvs->select)) {
		REBU32  len;
		REBSER* ser  = VAL_SERIES(pvs->value);
		REBCNT  idx  = VAL_INDEX(pvs->value);
		REBCNT  tail = VAL_TAIL(pvs->value);
		REBYTE* data = VAL_BIN_DATA(pvs->value);
		REBCNT  word = VAL_WORD_CANON(pvs->select);
		if (idx > tail) idx = tail;

		if (pvs->setval) {
			switch (word) {
			case SYM_USER:
			case SYM_HOST:
				if (!IS_EMAIL(pvs->value)) return PE_BAD_SET;
				else {
					REBLEN idx = Find_Str_Char(ser, 0, 0, tail, 1, '@', 0);
					if (word == SYM_HOST) {
						if (idx == NOT_FOUND) {
							Append_Byte(ser, '@');
							Modify_String(A_APPEND, ser, tail + 1, pvs->setval, 0, 0, 1);
						}
						else {
							Modify_String(A_CHANGE, ser, idx + 1, pvs->setval, FLAGIT(AN_PART), tail - idx - 1, 1);
						}
					}
					else {// user
						Modify_String(A_CHANGE, ser, 0, pvs->setval, FLAGIT(AN_PART), idx == NOT_FOUND ? tail : idx, 1);
					}
					return PE_OK;
				}
				break;
			default:
				return PE_BAD_SET;
			}

		}
		switch (word) {
		case SYM_LENGTH:
			len = IS_UTF8_SERIES(ser)
				? Length_As_UTF8_Code_Points(data)
				: tail - idx;
			break;
		case SYM_WIDTH:
			len = Length_As_Terminal_Width(data, VAL_BIN_TAIL(pvs->value));
			break;
		case SYM_SIZE:
			len = tail - idx;
			break;
		case SYM_USER:
		case SYM_HOST:
			if (!IS_EMAIL(pvs->value)) return PE_BAD_SELECT;
			else {
				REBLEN idx = Find_Str_Char(ser, 0, 0, tail, 1, '@', 0);
				if (word == SYM_HOST) {
					if (idx == NOT_FOUND) return PE_NONE;
					ser = Copy_String(ser, 1 + idx, tail - idx - 1);
				}
				else {// user
					if (idx == NOT_FOUND) idx = tail;
					ser = Copy_String(ser, 0, idx);
				}
				Set_String(pvs->store, ser);
				return PE_USE;
			}
			break;
		default:
			return PE_BAD_SELECT;
		}

		SET_INTEGER(pvs->store, len);
		return PE_USE;
	}
	else return PE_BAD_SELECT;

	if (val == 0) {
		if (n < 0 || (REBCNT)n >= SERIES_TAIL(ser)) return PE_NONE;
		if (IS_BINARY(data)) {
			SET_INTEGER(pvs->store, *BIN_SKIP(ser, n));
		} else {
			SET_CHAR(pvs->store, GET_UTF8_CHAR(ser, n));
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
		c = GET_UTF8_CHAR(VAL_SERIES(val), VAL_INDEX(val));
	}
	else
		return PE_BAD_SELECT;

	TRAP_PROTECT(ser);

//	if (c > 0x7F || IS_UTF8_SERIES(ser)) {
//		UTF8_SERIES(ser); // in case we are adding unicode char to ascii series
//		UTF8_Replace_Codepoint(ser, n, c);
//	}
//	else {
//		BIN_HEAD(ser)[n] = (REBYTE)c;
//	}
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

	if (ANY_STR(pvs->select)) {
		arg = VAL_SERIES(pvs->select);
		n = VAL_INDEX(pvs->select);
	}
	else {
		Reset_Mold(&mo);
		Mold_Value(&mo, pvs->select, 0);
		arg = mo.series;
		n = 0;
	}
	c = GET_UTF8_CHAR(arg, n);
	n += (c == '/' || c == '\\') ? 1 : 0;
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
	REBLEN	index = 0;
	REBLEN	tail = 0;
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
			if (!ANY_BINSTR(arg) && !IS_INTEGER(arg) && !IS_BITSET(arg) && !IS_CHAR(arg)) Trap0(RE_NOT_SAME_TYPE);
			if (IS_INTEGER(arg)) {
				if (VAL_INT64(arg) < 0 || VAL_INT64(arg) > 255) Trap_Range(arg);
				len = 1;
			}
			if (IS_CHAR(arg) && VAL_CHAR(arg) > 0x7F) {
				if (VAL_CHAR(arg) <= 0xFF) {
					// Search for the byte...
					BIN_HEAD(BUF_SCAN)[0] = VAL_CHAR(arg);
					SERIES_TAIL(BUF_SCAN) = 1;
				}
				else {
					// Search for UTF-8 encoded character...
					SERIES_TAIL(BUF_SCAN) = Encode_UTF8_Char(BIN_HEAD(BUF_SCAN), VAL_CHAR(arg));
				}
				Set_String(arg, BUF_SCAN);
			}
		}
		else {
			if (IS_CHAR(arg) || IS_BITSET(arg)) len = 1;
			else if (!ANY_STR(arg)) {
				Set_String(arg, Form_Value(arg, 0, FALSE));
			}
		}

		if (ANY_BINSTR(arg)) len = VAL_LEN(arg);

		if (args & AM_FIND_PART) tail = index + Partial(value, 0, D_ARG(ARG_FIND_RANGE), 0);
		ret = 1; // skip size
		if (args & AM_FIND_SKIP) {
			ret = Int32(D_ARG(ARG_FIND_SIZE));
			if(!ret) return R_NONE;
		}

		if (action == A_SELECT) args |= AM_FIND_TAIL;

		ret = find_string(value, index, tail, arg, len, args, ret, D_ARG(ARG_FIND_WILD));

		if (ret > (REBCNT)tail) return R_NONE;
		if (args & AM_FIND_ONLY) len = 1;

		if (action == A_FIND) {
			VAL_INDEX(value) = ret;
		}
		else {
			if (ret >= (REBCNT)tail) return R_NONE;
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
		if (IS_UTF8_STRING(value)) {
			if (len == 0) Trap_Range(arg);
			if (len > 0) len--;
			index = Skip_UTF8_String(value, len);
			if (index == NOT_FOUND || index >= VAL_TAIL(value)) return PE_NONE;
		}
		else {
			if (len < 0) REB_I32_ADD_OF(index, 1, &index);
			if (len == 0
				|| REB_I32_SUB_OF(len, 1, &len)
				|| REB_I32_ADD_OF(index, len, &index)
				|| index < 0 || index >= tail) {
				if (action == A_PICK) return R_NONE;
				Trap_Range(arg);
			}
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
			REBU32 c = 0;
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
				SET_ANY_CHAR(ser, index, c);
			}
			value = arg;
		}
		break;

	case A_TAKE:
		ser = VAL_SERIES(value);
		if (D_REF(ARG_TAKE_ALL)) {
			if (tail <= index) goto empty_str;
			len = tail - index;
			SET_TRUE(D_ARG(ARG_TAKE_PART));
		}
		else if (D_REF(ARG_TAKE_PART)) {
			REBVAL* part = D_ARG(ARG_TAKE_RANGE);
			if (D_REF(ARG_TAKE_LAST) && IS_UTF8_SERIES(ser)) {
				// special case for Unicode when taking from the tail
				REBINT range;
				if (IS_INTEGER(part) || IS_DECIMAL(part)) {
					range = MIN(UTF8_Index_To_Position(BIN_SKIP(ser, VAL_INDEX(value)), tail), Int32(part));
				}
				else {
					// part provided as a series position
					if (VAL_SERIES(part) != VAL_SERIES(value))
						Trap1(RE_INVALID_PART, part);
					range = UTF8_Index_To_Position(BIN_SKIP(ser, VAL_INDEX(value)), tail)
						  - UTF8_Index_To_Position(BIN_SKIP(ser, VAL_INDEX(value)), VAL_INDEX(part));
				}
				index = UTF8_Skip(ser, tail, -range);
				if (index == UNKNOWN) index = 0; // e.g.: take/part/last "abc" 100
				len = tail - index; // in bytes

				if (len == 0) goto empty_str;
				Set_Series(VAL_TYPE(value), value, Copy_String(ser, index, len));
				goto finish_take;
			}
			len = Partial(value, 0, part, 0);
			if (len == 0) goto empty_str;
		} else 
			len = 1;

		index = VAL_INDEX(value); // /part can change index
		if (tail <= index) return R_NONE;

		if (D_REF(ARG_TAKE_LAST)) {
			index = IS_UTF8_SERIES(ser) ? UTF8_Skip(ser, tail, -len) : tail - len;
		}
		if (index >= tail) {
			if (!D_REF(ARG_TAKE_PART)) return R_NONE;;
			goto empty_str;
		}

		
		// if no /part, just return value, else return string:
		if (!D_REF(ARG_TAKE_PART)) {
			if (IS_BINARY(value)) {
				SET_INTEGER(value, *VAL_BIN_SKIP(value, index));
			}
			else {
				REBU32 chr = UTF8_Get_Codepoint(VAL_BIN_SKIP(value, index));
				SET_CHAR(value, chr);
				len = UTF8_Codepoint_Size(chr);
			}
		}
		else Set_Series(VAL_TYPE(value), value, Copy_String(ser, index, len));
	finish_take:
		Remove_Series(ser, index, len);
		break;
	empty_str:
		Set_Series(VAL_TYPE(value), D_RET, Make_Binary(0));
		return R_RET;

	case A_CLEAR:
		if (index < tail) {
			if (index == 0) Reset_Series(VAL_SERIES(value));
			else {
				VAL_TAIL(value) = (REBCNT)index;
				TERM_SERIES(VAL_SERIES(value));
			}
			if (IS_UTF8_STRING(value) && Is_ASCII(VAL_BIN(value), VAL_TAIL(value))) {
				SERIES_CLR_FLAG(VAL_SERIES(value), SER_UTF8);
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
			? make_string(arg, (REBOOL)(action == A_MAKE), type)
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
			(args & (AM_TRIM_HEAD | AM_TRIM_LINES | AM_TRIM_ALL | AM_TRIM_WITH)))
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
			D_REF(10),	// reverse
			(D_REF(11) || IS_BINARY(value)) // unstable
		);
		break;

	case A_RANDOM:
		if(IS_PROTECT_SERIES(VAL_SERIES(value))) Trap0(RE_PROTECTED);
		if (D_REF(2)) { // seed
			Set_Random(Compute_CRC24(VAL_BIN_DATA(value), VAL_LEN(value)));
			return R_UNSET;
		}
		if (D_REF(4)) { // /only
			if (index >= tail) return R_NONE;
			index += (REBCNT)Random_Int(D_REF(3)) % (tail - index);  // /secure
			if ((VAL_BIN_HEAD(value)[index] & 0xC0) == 0x80) {
				index = UTF8_Prev_Char_Position(VAL_BIN_HEAD(value), index);
			}
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
