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
**  Module:  t-block.c
**  Summary: block related datatypes
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	REBINT CT_Block(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT num;

	if (mode == 3)
		return VAL_SERIES(a) == VAL_SERIES(b) && VAL_INDEX(a) == VAL_INDEX(b);

	num = Cmp_Block(a, b, mode > 1);
	if (mode >= 0) return (num == 0);
	if (mode == -1) return (num >= 0);
	return (num > 0);
}

static void No_Nones(REBVAL *arg) {
	arg = VAL_BLK_DATA(arg);
	for (; NOT_END(arg); arg++) {
		if (IS_NONE(arg)) Trap_Arg(arg);
	}
}

/***********************************************************************
**
*/	REBFLG MT_Block(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBCNT i;

	if (!ANY_BLOCK(data)) return FALSE;
	if (type >= REB_PATH && type <= REB_LIT_PATH)
		if (!ANY_WORD(VAL_BLK(data))) return FALSE;

	*out = *data++;
	VAL_SET(out, type);
	i = IS_INTEGER(data) ? Int32(data) - 1 : 0;
	if (i > VAL_TAIL(out)) i = VAL_TAIL(out); // clip it
	VAL_INDEX(out) = i;
	return TRUE;
}


/***********************************************************************
**
*/	REBCNT Find_Block(REBSER *series, REBCNT index, REBCNT end, REBVAL *target, REBCNT len, REBCNT flags, REBINT skip)
/*
**		Flags are set according to: ALL_FIND_REFS
**
**	Main Parameters:
**		start - index to start search
**		end   - ending position
**		len   - length of target
**		skip  - skip factor
**		dir   - direction
**
**	Comparison Parameters:
**		case  - case sensitivity
**		wild  - wild cards/keys
**
**	Final Parmameters:
**		tail  - tail position
**		match - sequence
**		SELECT - (value that follows)
**
***********************************************************************/
{
	REBVAL *value;
	REBVAL *val;
	REBCNT cnt;
	REBCNT start = index;

	if (flags & (AM_FIND_REVERSE | AM_FIND_LAST)) {
		skip = -1;
		start = 0;
		if (flags & AM_FIND_LAST) index = end - len;
		else index--;
	}

	// Optimized find word in block:
	if (ANY_WORD(target)) {
		for (; index >= start && index < end; index += skip) {
			value = BLK_SKIP(series, index);
			if (ANY_WORD(value)) {
				cnt = (VAL_WORD_SYM(value) == VAL_WORD_SYM(target));
				if (flags & AM_FIND_CASE) {
					// Must be same type and spelling:
					if (cnt && VAL_TYPE(value) == VAL_TYPE(target)) return index;
				}
				else {
					// Can be different type or alias:
					if (cnt || VAL_WORD_CANON(value) == VAL_WORD_CANON(target)) return index;
				}
			}
			if (flags & AM_FIND_MATCH) break;
		}
		return NOT_FOUND;
	}
	// Match a block against a block:
	else if (ANY_BLOCK(target) && !(flags & AM_FIND_ONLY)) {
		for (; index >= start && index < end; index += skip) {
			cnt = 0;
			value = BLK_SKIP(series, index);
			for (val = VAL_BLK_DATA(target); NOT_END(val); val++, value++) {
				if (0 != Cmp_Value(value, val, (REBOOL)(flags & AM_FIND_CASE))) break;
				if (++cnt >= len) {
					return index;
				}
			}
			if (flags & AM_FIND_MATCH) break;
		}
		return NOT_FOUND;
	}
	// Find a datatype in block:
	else if (IS_DATATYPE(target) || IS_TYPESET(target)) {
		for (; index >= start && index < end; index += skip) {
			value = BLK_SKIP(series, index);
			// Used if's so we can trace it...
			if (IS_DATATYPE(target)) {
				if ((REBINT)VAL_TYPE(value) == VAL_DATATYPE(target)) return index;
				if (IS_DATATYPE(value) && VAL_DATATYPE(value) == VAL_DATATYPE(target)) return index;
			}
			if (IS_TYPESET(target)) {
				if (TYPE_CHECK(target, VAL_TYPE(value))) return index;
				if (IS_DATATYPE(value) && TYPE_CHECK(target, VAL_DATATYPE(value))) return index;
				if (IS_TYPESET(value) && EQUAL_TYPESET(value, target)) return index;
			}
			if (flags & AM_FIND_MATCH) break;
		}
		return NOT_FOUND;
	}
	// All other cases:
	else {
		for (; index >= start && index < end; index += skip) {
			value = BLK_SKIP(series, index);
			if (0 == Cmp_Value(value, target, (REBOOL)(flags & AM_FIND_CASE))) return index;
			if (flags & AM_FIND_MATCH) break;
		}
		return NOT_FOUND;
	}
}


/***********************************************************************
**
*/	void Modify_Blockx(REBCNT action, REBVAL *block, REBVAL *arg)
/*
**		Actions: INSERT, APPEND, CHANGE
**
**		block [block!] {Series at point to insert}
**		value [any-type!] {The value to insert}
**		/part {Limits to a given length or position.}
**		length [number! series! pair!]
**		/only {Inserts a series as a series.}
**		/dup {Duplicates the insert a specified number of times.}
**		count [number! pair!]
**
**	Add:
**		Handle insert [] () case
**		What does insert () [] do?
**		/deep option for cloning subcontents?
**
***********************************************************************/
{
	REBSER *series = VAL_SERIES(block);
	REBCNT index = VAL_INDEX(block);
	REBCNT tail  = VAL_TAIL(block);
	REBFLG only  = DS_REF(AN_ONLY);
	REBINT rlen;  // length to be removed
	REBINT ilen  = 1;  // length to be inserted
	REBINT cnt   = 1;  // DUP count
	REBINT size;
	REBFLG is_blk = FALSE; // arg is a block not a value

	// Length of target (may modify index): (arg can be anything)
	rlen = Partial1((action == A_CHANGE) ? block : arg, DS_ARG(AN_LENGTH));

	index = VAL_INDEX(block);
	if (action == A_APPEND || index > tail) index = tail;

	// Check /PART, compute LEN:
	if (!only && ANY_BLOCK(arg)) {
		is_blk = TRUE; // arg is a block
		// Are we modifying ourselves? If so, copy arg block first:
		if (series == VAL_SERIES(arg))  {
			VAL_SERIES(arg) = Copy_Block(VAL_SERIES(arg), VAL_INDEX(arg));
			VAL_INDEX(arg) = 0;
		}
		// Length of insertion:
		ilen = (action != A_CHANGE && DS_REF(AN_PART)) ? rlen : VAL_LEN(arg);
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

	if (is_blk) arg = VAL_BLK_DATA(arg);

	// For dup count:
	VAL_INDEX(block) = (action == A_APPEND) ? 0 : size + index;

	index *= SERIES_WIDE(series); // loop invariant
	ilen *= SERIES_WIDE(series);   // loop invariant
	for (; cnt > 0; cnt--) {
		memcpy(series->data + index, (REBYTE *)arg, ilen);
		index += ilen;
	}
	BLK_TERM(series);
}


/***********************************************************************
**
*/	void Make_Block_Type(REBFLG make, REBVAL *value, REBVAL *arg)
/*
**		Value can be:
**			1. a datatype (e.g. BLOCK!)
**			2. a value (e.g. [...])
**
**		Arg can be:
**			1. integer (length of block)
**			2. block (copy it)
**			3. value (convert to a block)
**
***********************************************************************/
{
	REBCNT type;
	REBCNT len;
	REBSER *ser;

	// make block! ...
	if (IS_DATATYPE(value))
		type = VAL_DATATYPE(value);
	else  // make [...] ....
		type = VAL_TYPE(value);

	// make block! [1 2 3]
	if (ANY_BLOCK(arg)) {
		len = VAL_BLK_LEN(arg);
		if (len > 0 && type >= REB_PATH && type <= REB_LIT_PATH)
			No_Nones(arg);
		ser = Copy_Values(VAL_BLK_DATA(arg), len);
		goto done;
	}

	if (IS_STRING(arg)) {
		REBCNT index, len = 0;
		VAL_SERIES(arg) = Prep_Bin_Str(arg, &index, &len); // (keeps safe)
		ser = Scan_Source(VAL_BIN(arg), VAL_LEN(arg));
		goto done;
	}

	if (IS_BINARY(arg)) {
		ser = Scan_Source(VAL_BIN_DATA(arg), VAL_LEN(arg));
		goto done;
	}

	if (IS_MAP(arg)) {
		ser = Map_To_Block(VAL_SERIES(arg), 0);
		goto done;
	}

	if (ANY_OBJECT(arg)) {
		ser = Make_Object_Block(VAL_OBJ_FRAME(arg), 3);
		goto done;
	}

	if (IS_VECTOR(arg)) {
		ser = Make_Vector_Block(arg);
		goto done;
	}

//	if (make && IS_NONE(arg)) {
//		ser = Make_Block(0);
//		goto done;
//	}

	// to block! typset
	if (!make && IS_TYPESET(arg) && type == REB_BLOCK) {
		Set_Block(value, Typeset_To_Block(arg));
		return;
	}

	if (make) {
		// make block! 10
		if (IS_INTEGER(arg) || IS_DECIMAL(arg)) {
			len = Int32s(arg, 0);
			Set_Series(type, value, Make_Block(len));
			return;
		}
		Trap_Arg(arg);
	}

	ser = Copy_Values(arg, 1);

done:
	Set_Series(type, value, ser);
	return;
}

// WARNING! Not re-entrant. !!!  Must find a way to push it on stack?
static struct {
	REBFLG cased;
	REBFLG reverse;
	REBCNT offset;
	REBVAL *compare;
} sort_flags = {0};

/***********************************************************************
**
*/	static int Compare_Val(const void *v1, const void *v2)
/*
***********************************************************************/
{
	// !!!! BE SURE that 64 bit large difference comparisons work

	if (sort_flags.reverse)
		return Cmp_Value((REBVAL*)v2+sort_flags.offset, (REBVAL*)v1+sort_flags.offset, sort_flags.cased);
	else
		return Cmp_Value((REBVAL*)v1+sort_flags.offset, (REBVAL*)v2+sort_flags.offset, sort_flags.cased);

/*
	REBI64 n = VAL_INT64((REBVAL*)v1) - VAL_INT64((REBVAL*)v2);
	if (n > 0) return 1;
	if (n < 0) return -1;
	return 0;
*/
}


/***********************************************************************
**
*/	static int Compare_Call(const void *v1, const void *v2)
/*
***********************************************************************/
{
	REBVAL *val;
	
	if (sort_flags.reverse)
		val = Apply_Func(0, sort_flags.compare, v1, v2, 0);	
	else
		val = Apply_Func(0, sort_flags.compare, v2, v1, 0);	

	if (IS_LOGIC(val)) {
		if (IS_TRUE(val)) return 1;
		return -1;
	}
	if (IS_INTEGER(val)) {
		if (VAL_INT64(val) > 0) return 1;
		if (VAL_INT64(val) == 0) return 0;
		return -1;
	}
	if (IS_DECIMAL(val)) {
		if (VAL_DECIMAL(val) > 0) return 1;
		if (VAL_DECIMAL(val) == 0) return 0;
		return -1;
	}
	if (IS_TRUE(val)) return 1;
	return -1;
}


/***********************************************************************
**
*/	static void Sort_Block(REBVAL *block, REBFLG ccase, REBVAL *skipv, REBVAL *compv, REBVAL *part, REBFLG all, REBFLG rev)
/*
**		series [series!]
**		/case {Case sensitive sort}
**		/skip {Treat the series as records of fixed size}
**		size [integer!] {Size of each record}
**		/compare  {Comparator offset, block or function}
**		comparator [integer! block! function!]
**		/part {Sort only part of a series}
**		length [number! series!] {Length of series to sort}
**		/all {Compare all fields}
**		/reverse {Reverse sort order}
**
***********************************************************************/
{
	REBCNT len;
	REBCNT skip = 1;
	REBCNT size = sizeof(REBVAL);
//	int (*sfunc)(const void *v1, const void *v2);

	sort_flags.cased = ccase;
	sort_flags.reverse = rev;
	sort_flags.compare = 0;
	sort_flags.offset = 0;

	if (IS_INTEGER(compv)) sort_flags.offset = Int32(compv)-1; 
	if (ANY_FUNC(compv)) sort_flags.compare = compv; 

	// Determine length of sort:
	len = Partial1(block, part);
	if (len <= 1) return;

	// Skip factor:
	if (!IS_NONE(skipv)) {
		skip = Get_Num_Arg(skipv);
		if (skip <= 0 || len % skip != 0 || skip > len)
			Trap_Range(skipv);
	}

	// Use fast quicksort library function:
	if (skip > 1) len /= skip, size *= skip;

	if (sort_flags.compare)
		qsort((void *)VAL_BLK_DATA(block), len, size, Compare_Call);
	else
		qsort((void *)VAL_BLK_DATA(block), len, size, Compare_Val);

}


/***********************************************************************
**
*/	static void Trim_Block(REBSER *ser, REBCNT index, REBCNT flags)
/*
**		See Trim_String().
**
***********************************************************************/
{
	REBVAL *blk = BLK_HEAD(ser);
	REBCNT out = index;
	REBCNT end = ser->tail;

	if (flags & AM_TRIM_TAIL) {
		for (; end >= (index+1); end--) {
			if (VAL_TYPE(blk+end-1) > REB_NONE) break;
		}
		Remove_Series(ser, end, ser->tail - end);
		if (!(flags & AM_TRIM_HEAD) || index >= end) return;
	}

	if (flags & AM_TRIM_HEAD) {
		for (; index < end; index++) {
			if (VAL_TYPE(blk+index) > REB_NONE) break;
		}
		Remove_Series(ser, out, index - out);
	}

	if (flags == 0) {
		for (; index < end; index++) {
			if (VAL_TYPE(blk+index) > REB_NONE) {
				*BLK_SKIP(ser, out) = blk[index];
				out++;
			}
		}
		Remove_Series(ser, out, end - out);
	}
}


/***********************************************************************
**
*/	void Shuffle_Block(REBVAL *value, REBFLG secure)
/*
***********************************************************************/
{
	REBCNT n;
	REBCNT k;
	REBCNT idx = VAL_INDEX(value);
	REBVAL *data = VAL_BLK(value);
	REBVAL swap;

	for (n = VAL_LEN(value); n > 1;) {
		k = idx + (REBCNT)Random_Int(secure) % n;
		n--;
		swap = data[k];
		data[k] = data[n + idx];
		data[n + idx] = swap;
	}
}


/***********************************************************************
**
*/	REBINT PD_Block(REBPVS *pvs)
/*
***********************************************************************/
{
	REBINT n = 0;

	/* Issues!!!
		a/1.3
		a/not-found: 10 error or append?
		a/not-followed: 10 error or append?
	*/

	if (IS_INTEGER(pvs->select)) {
		n = Int32(pvs->select) + VAL_INDEX(pvs->value) - 1;
	}
	else if (IS_WORD(pvs->select)) {
		n = Find_Word(VAL_SERIES(pvs->value), VAL_INDEX(pvs->value), VAL_WORD_CANON(pvs->select));
		if (n != NOT_FOUND) n++;
	}
	else {
		// other values:
		n = Find_Block_Simple(VAL_SERIES(pvs->value), VAL_INDEX(pvs->value), pvs->select) + 1;
	}

	if (n < 0 || (REBCNT)n >= VAL_TAIL(pvs->value)) {
		if (pvs->setval) return PE_BAD_SELECT;
		return PE_NONE;
	}

	if (pvs->setval) TRAP_PROTECT(VAL_SERIES(pvs->value));
	pvs->value = VAL_BLK_SKIP(pvs->value, n);
	// if valset - check PROTECT on block
	//if (NOT_END(pvs->path+1)) Next_Path(pvs); return PE_OK;
	return PE_SET;
}


/***********************************************************************
**
*/	REBVAL *Pick_Block(REBVAL *block, REBVAL *selector)
/*
***********************************************************************/
{
	REBINT n = 0;

	n = Get_Num_Arg(selector);
	n += VAL_INDEX(block) - 1;
	if (n < 0 || (REBCNT)n >= VAL_TAIL(block)) return 0;
	return VAL_BLK_SKIP(block, n);
}


/***********************************************************************
**
*/	REBTYPE(Block)
/*
***********************************************************************/
{
	REBVAL	*value = D_ARG(1);
	REBVAL  *arg = D_ARG(2);
	REBSER  *ser;
	REBINT	index;
	REBINT	tail;
	REBINT	len;
	REBVAL  val;
	REBCNT	args;
	REBCNT  ret;

	// Support for port: OPEN [scheme: ...], READ [ ], etc.
	if (action >= PORT_ACTIONS && IS_BLOCK(value))
		return T_Port(ds, action);

	// Most common series actions:  !!! speed this up!
	len = Do_Series_Action(action, value, arg);
	if (len >= 0) return len; // return code

	// Special case (to avoid fetch of index and tail below):
	if (action == A_MAKE || action == A_TO) {
		Make_Block_Type(action == A_MAKE, value, arg); // returned in value
		if (ANY_PATH(value)) Clear_Value_Opts(VAL_SERIES(value));
		*D_RET = *value;
		return R_RET;
	}

	index = (REBINT)VAL_INDEX(value);
	tail  = (REBINT)VAL_TAIL(value);
	ser   = VAL_SERIES(value);

	// Check must be in this order (to avoid checking a non-series value);
	if (action >= A_TAKE && action <= A_SORT && IS_PROTECT_SERIES(ser))
		Trap0(RE_PROTECTED);

	switch (action) {

	//-- Picking:

#ifdef REMOVE_THIS

//CHANGE SELECT TO USE PD_BLOCK?

	case A_PATH:
		if (IS_INTEGER(arg)) {
			action = A_PICK;
			goto repick;
		}
		// block/select case:
		ret = Find_Block_Simple(ser, index, arg);
		goto select_val;

	case A_PATH_SET:
		action = A_POKE;
		// no SELECT case allowed !!!!
#endif

	case A_POKE:
	case A_PICK:
repick:
		value = Pick_Block(value, arg);
		if (action == A_PICK) {
			if (!value) goto is_none;
			*D_RET = *value;
		} else {
			if (!value) Trap_Range(arg);
			arg = D_ARG(3);
			*value = *arg;
			*D_RET = *arg;
		}
		return R_RET;

/*
		len = Get_Num_Arg(arg); // Position
		index += len;
		if (len > 0) index--;
		if (len == 0 || index < 0 || index >= tail) {
			if (action == A_PICK) goto is_none;
			Trap_Range(arg);
		}
		if (action == A_PICK) {
pick_it:
			*D_RET = BLK_HEAD(ser)[index];
			return R_RET;
		}
		arg = D_ARG(3);
		*D_RET = *arg;
		BLK_HEAD(ser)[index] = *arg;
		return R_RET;
*/

	case A_TAKE:
		// take/part:
		if (D_REF(2)) {
			len = Partial1(value, D_ARG(3));
			if (len == 0) {
zero_blk:
				Set_Block(D_RET, Make_Block(0));
				return R_RET;
			}
		} else 
			len = 1;

		index = VAL_INDEX(value); // /part can change index
		// take/last:
		if (D_REF(5)) index = tail - len;
		if (index < 0 || index >= tail) {
			if (!D_REF(2)) goto is_none;
			goto zero_blk;
		}

		// if no /part, just return value, else return block:
		if (!D_REF(2)) *D_RET = BLK_HEAD(ser)[index];
		else Set_Block(D_RET, Copy_Block_Len(ser, index, len)); // no more /DEEP
//		else Set_Block(D_RET, Copy_Block_Deep(ser, index, len, D_REF(4) ? COPY_DEEP: 0));
		Remove_Series(ser, index, len);
		return R_RET;

	//-- Search:

	case A_FIND:
	case A_SELECT:
		args = Find_Refines(ds, ALL_FIND_REFS);
//		if (ANY_BLOCK(arg) || args) {
			len = ANY_BLOCK(arg) ? VAL_BLK_LEN(arg) : 1;
			if (args & AM_FIND_PART) tail = Partial1(value, D_ARG(ARG_FIND_LENGTH));
			ret = 1;
			if (args & AM_FIND_SKIP) ret = Int32s(D_ARG(ARG_FIND_SIZE), 1);
			ret = Find_Block(ser, index, tail, arg, len, args, ret);
//		}
/*		else {
			len = 1;
			ret = Find_Block_Simple(ser, index, arg);
		}
*/
		if (ret >= (REBCNT)tail) goto is_none;
		if (args & AM_FIND_ONLY) len = 1;
		if (action == A_FIND) {
			if (args & (AM_FIND_TAIL | AM_FIND_MATCH)) ret += len;
			VAL_INDEX(value) = ret;
		}
		else {
			ret += len;
			if (ret >= (REBCNT)tail) goto is_none;
			value = BLK_SKIP(ser, ret);
		}
		break;

	//-- Modification:
	case A_APPEND:
	case A_INSERT:
	case A_CHANGE:
		// Length of target (may modify index): (arg can be anything)
		len = Partial1((action == A_CHANGE) ? value : arg, DS_ARG(AN_LENGTH));
		index = VAL_INDEX(value);
		args = 0;
		if (DS_REF(AN_ONLY)) SET_FLAG(args, AN_ONLY);
		if (DS_REF(AN_PART)) SET_FLAG(args, AN_PART);
		index = Modify_Block(action, ser, index, arg, args, len, DS_REF(AN_DUP) ? Int32(DS_ARG(AN_COUNT)) : 1);
		VAL_INDEX(value) = index;
		break;

	case A_CLEAR:
		if (index < tail) {
			if (index == 0) Reset_Series(ser);
			else {
				SET_END(BLK_SKIP(ser, index));
				VAL_TAIL(value) = (REBCNT)index;
			}
		}
		break;

	//-- Creation:

	case A_COPY: // /PART len /DEEP /TYPES kinds
#if 0
		args = D_REF(ARG_COPY_DEEP) ? COPY_ALL : 0;
		len = Partial1(value, D_ARG(ARG_COPY_LENGTH));
		index = (REBINT)VAL_INDEX(value);
//		VAL_SERIES(value) = (len > 0) ? Copy_Block_Deep(ser, index, len, args) : Make_Block(0);
		VAL_INDEX(value) = 0;
#else
	{
		REBU64 types = 0;
		if (D_REF(ARG_COPY_DEEP)) {
			types |= CP_DEEP | (D_REF(ARG_COPY_TYPES) ? 0 : TS_STD_SERIES);
		}
		if D_REF(ARG_COPY_TYPES) {
			arg = D_ARG(ARG_COPY_KINDS);
			if (IS_DATATYPE(arg)) types |= TYPESET(VAL_DATATYPE(arg));
			else types |= VAL_TYPESET(arg);
		}
		len = Partial1(value, D_ARG(ARG_COPY_LENGTH));
		VAL_SERIES(value) = Copy_Block_Values(ser, VAL_INDEX(value), VAL_INDEX(value)+len, types);
		VAL_INDEX(value) = 0;
	}
#endif
		break;

	//-- Special actions:

	case A_TRIM:
		args = Find_Refines(ds, ALL_TRIM_REFS);
		if (args & ~(AM_TRIM_HEAD|AM_TRIM_TAIL)) Trap0(RE_BAD_REFINES);
		Trim_Block(ser, index, args);
		break;

	case A_SWAP:
		if (SERIES_WIDE(ser) != SERIES_WIDE(VAL_SERIES(arg)))
			Trap_Arg(arg);
		if (IS_PROTECT_SERIES(VAL_SERIES(arg))) Trap0(RE_PROTECTED);
		if (index < tail && VAL_INDEX(arg) < VAL_TAIL(arg)) {
			val = *VAL_BLK_DATA(value);
			*VAL_BLK_DATA(value) = *VAL_BLK_DATA(arg);
			*VAL_BLK_DATA(arg) = val;
		}
		value = 0;
		break;

	case A_REVERSE:
		len = Partial1(value, D_ARG(3));
		if (len == 0) break;
		value = VAL_BLK_DATA(value);
		arg = value + len - 1;
		for (len /= 2; len > 0; len--) {
			val = *value;
			*value++ = *arg;
			*arg-- = val;
		}
		value = 0;
		break;

	case A_SORT:
		Sort_Block(
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
		if (!IS_BLOCK(value)) Trap_Action(VAL_TYPE(value), action);
		if (D_REF(2)) Trap0(RE_BAD_REFINES); // seed
		if (D_REF(4)) { // /only
			if (index >= tail) goto is_none;
			len = (REBCNT)Random_Int(D_REF(3)) % (tail - index);  // /secure
			arg = D_ARG(2); // pass to pick
			SET_INTEGER(arg, len+1);
			action = A_PICK;
			goto repick;
		}
		Shuffle_Block(value, D_REF(3));
		break;

	default:
		Trap_Action(VAL_TYPE(value), action);
	}

	if (!value) value = D_ARG(1);
	DS_RET_VALUE(value);
	return R_RET;

is_none:
	return R_NONE;
}
