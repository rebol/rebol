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
**  Module:  t-block.c
**  Summary: block related datatypes
**  Section: datatypes
**  Author:  Carl Sassenrath, Oldes
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

#ifdef not_used
static void No_Nones_Or_Logic(REBVAL *arg) {
	arg = VAL_BLK_DATA(arg);
	for (; NOT_END(arg); arg++) {
		if (IS_NONE(arg) || IS_LOGIC(arg)) Trap_Arg(arg);
	}
}
#endif

/***********************************************************************
**
*/	REBFLG MT_Block(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBCNT i;

	if (!ANY_BLOCK(data)) return FALSE;
	// set the output first...
	*out = *data;
	VAL_SET(out, type);
	// ... to allow construction of empty path types too
	if (IS_END(VAL_BLK(data)) && type >= REB_PATH && type <= REB_LIT_PATH)
		return TRUE;
	// and update the index, if needed...
	data++;
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
		if (flags & AM_FIND_LAST) {
			start = index;
			index = end - len;
		}
		else {
			start = 0;
			index--;
		}
	}

	// Optimized find word in block:
	if (ANY_WORD(target)) {
		for (; index >= start && index < end; index += skip) {
			value = BLK_SKIP(series, index);
			if (ANY_WORD(value)) {
				cnt = (VAL_WORD_SYM(value) == VAL_WORD_SYM(target));
				if (flags & (AM_FIND_CASE | AM_FIND_SAME)) {
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
				if ((flags & AM_FIND_SAME)) {
					if (0 == Compare_Values(value, val, 3))
						break;
				}
				else if (0 != Cmp_Value(value, val, (REBOOL)(flags & AM_FIND_CASE))) break;
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
		if (flags & AM_FIND_ONLY) {
			// not checking value types, only if value and target are really same
			for (; index >= start && index < end; index += skip) {
				value = BLK_SKIP(series, index);
				if (VAL_TYPE(value) == VAL_TYPE(target)) {
					if (IS_DATATYPE(target)) {
						if (VAL_DATATYPE(value) == VAL_DATATYPE(target))
							return index;
					}
					else if (EQUAL_TYPESET(value, target))
						return index;
				}
				if (flags & AM_FIND_MATCH) break;
			}
		} else {
			for (; index >= start && index < end; index += skip) {
				value = BLK_SKIP(series, index);
				// Used if's so we can trace it...
				if (IS_DATATYPE(target)) {
					if ((REBINT)VAL_TYPE(value) == VAL_DATATYPE(target))
						return index;
				}
				else if (TYPE_CHECK(target, VAL_TYPE(value)))
						return index;
				if (flags & AM_FIND_MATCH) break;
			}
		}
		return NOT_FOUND;
	}
	// All other cases:
	else {
		if (flags & AM_FIND_SAME) {
			for (; index >= start && index < end; index += skip) {
				value = BLK_SKIP(series, index);
				if (Compare_Values(target, value, 3)) return index;
				if (flags & AM_FIND_MATCH) break;
			}
		} else {
			for (; index >= start && index < end; index += skip) {
				value = BLK_SKIP(series, index);
				if (0 == Cmp_Value(value, target, (REBOOL)(flags & AM_FIND_CASE))) return index;
				if (flags & AM_FIND_MATCH) break;
			}
		}
		return NOT_FOUND;
	}
}

/***********************************************************************
**
*/	REBCNT Find_Block_Key(REBSER* series, REBVAL* key, REBCNT skip, REBOOL cased)
/*
**		Try to find the key value in the block.
**
**		RETURNS: the index to the KEY or NOT_FOUND if there is none.
**
***********************************************************************/
{
//	REBSER* hser = series->series; // can be null
//	REBCNT* hashes = NULL;
	REBCNT n;
	REBVAL* val;

	val = BLK_HEAD(series);
	if (ANY_WORD(key)) {
		for (n = 0; n < series->tail; n += skip, val += skip) {
			if (
				ANY_WORD(val) && (
					VAL_WORD_SYM(key) == VAL_BIND_SYM(val) ||
					(!cased && VAL_WORD_CANON(key) == VAL_BIND_CANON(val))
				)
			) {
				return n;
			}
		}
	}
	else if (ANY_BINSTR(key)) {
		cased = !(IS_BINARY(key) || cased);
		for (n = 0; n < series->tail; n += skip, val += skip) {
			if (
				VAL_TYPE(val) == VAL_TYPE(key)
				&& 0 == Compare_String_Vals(key, val, cased)
			) {
				return n;
			}
		}
	}
	else {
		for (n = 0; n < series->tail; n += skip, val += skip) {
			if (VAL_TYPE(val) == VAL_TYPE(key) && 0 == Cmp_Value(key, val, cased)) {
				return n;
			}
		}
	}
	return NOT_FOUND;
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
	rlen = (REBINT)Partial1((action == A_CHANGE) ? block : arg, DS_ARG(AN_LENGTH));

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

	// make block! [1 2 3] but also: to block! 'a/b/c
	if (ANY_BLOCK(arg)) {
		len = VAL_BLK_LEN(arg);
#ifdef not_used
		if (len > 0 && type >= REB_PATH && type <= REB_LIT_PATH)
			No_Nones_Or_Logic(arg);
#endif
		ser = Copy_Values(VAL_BLK_DATA(arg), len);
		goto done;
	}

	// map!, object! and vector! have same result for make/to
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

	if (make) {
		// make block! 10
		if (IS_INTEGER(arg) || IS_DECIMAL(arg)) {
			len = Int32s(arg, 0);
			Set_Series(type, value, Make_Block(len));
			return;
		}
		
	} else if (ANY_BLOCK_TYPE(type)) {
		// to block!/paren! typset!
		if (IS_TYPESET(arg) && (type == REB_BLOCK || type == REB_PAREN)) {
			Set_Block(value, Typeset_To_Block(arg));
			return;
		}
		// to block!/paren! ... simplified
		goto to_blk_val;
	}

	// make from string! or binary! with tokenization
	if (IS_STRING(arg) || IS_BINARY(arg)) {
		ser = Scan_Source(VAL_BIN_DATA(arg), VAL_LEN(arg));
		goto done;
	}

	// use `make block! pair!` for block preallocation
	if (IS_PAIR(arg)) {
		len = (REBCNT)MAX(1, VAL_PAIR_X_INT(arg)) * (REBCNT)MAX(1, VAL_PAIR_Y_INT(arg));
		Set_Series(type, value, Make_Block(len));
		return;
	}

	// else not supported make block! ...
	Trap_Arg(arg);


to_blk_val:
	// simplified to block! ..
	// (only copy the value into a new block)
	ser = Copy_Values(arg, 1);

done:
	Set_Series(type, value, ser);
	return;
}

/***********************************************************************
**
*/	static int Compare_Val(const void *v1, const void *v2)
/*
***********************************************************************/
{
	REBVAL *val = DS_GET(DSP - 1);
	REBU64 flags = VAL_UNT64(DS_TOP);
	REBLEN offset = 0;
	REBINT result;
	if (IS_INTEGER(val)) offset = AS_REBLEN(VAL_INT64(val) - 1);

	result = Cmp_Value((REBVAL*)v1+offset, (REBVAL*)v2+offset, GET_FLAG(flags, SORT_FLAG_CASE));
	if (GET_FLAG(flags, SORT_FLAG_REVERSE)) result = -result;
	return result;
}

/***********************************************************************
**
*/	static int Compare_Val_Multi(const void* v1, const void* v2)
/*
**	Used to compare multiple values in the record (sort/compare with block).
**
***********************************************************************/
{
	REBVAL* val = DS_GET(DSP - 1);
	REBU64 flags = VAL_UNT64(DS_TOP);
	REBLEN offset = 0;
	REBINT result = 0;
	ASSERT1(IS_BLOCK(val), RP_BAD_EVALTYPE);
	REBVAL* ofs = VAL_BLK_DATA(val);
	while (result == 0 && IS_INTEGER(ofs)) {
		offset = AS_REBLEN(VAL_INT64(ofs++) - 1);
		result = Cmp_Value((REBVAL*)v1 + offset, (REBVAL*)v2 + offset, GET_FLAG(flags, SORT_FLAG_CASE));
	}
	if (GET_FLAG(flags, SORT_FLAG_REVERSE)) result = -result;
	return result;
}

/***********************************************************************
**
*/	static int Compare_All_Val(const void *v1, const void *v2)
/*
***********************************************************************/
{
	REBCNT size = VAL_UNT32(DS_GET(DSP - 1));
	REBU64 flags = VAL_UNT64(DS_TOP);
	REBINT offset = 0;
	REBINT result = 0;

	while (size-- > 0 && result == 0) {
		result = Cmp_Value((REBVAL *)v1 + offset, (REBVAL *)v2 + offset, GET_FLAG(flags, SORT_FLAG_CASE));
		offset++;
	}
	if (GET_FLAG(flags, SORT_FLAG_REVERSE)) result = -result;
	return result;
}


/***********************************************************************
**
*/	static int Compare_Call(const void *p1, const void *p2)
/*
***********************************************************************/
{
	REBVAL *v1;
	REBVAL *v2;
	REBVAL *val;
	REBSER *args;
	REBVAL *func;
	REBU64 flags;
	int result = -1;

	func = DS_GET(DSP - 1);
	ASSERT1(ANY_FUNC(func), RP_MISC);
	flags = VAL_UNT64(DS_TOP);

	if (GET_FLAG(flags, SORT_FLAG_ALL)) {
		// Retrieve the pre-allocated argument blocks from the stack
		v1 = DS_GET(DSP - 3);
		v2 = DS_GET(DSP - 2);
		ASSERT1(IS_BLOCK(v1) && IS_BLOCK(v2), RP_MISC);
		// Calculate the byte size of one block's worth of values
		REBCNT len = sizeof(REBVAL) * SERIES_TAIL(VAL_SERIES(v1));
		// Copy data into the argument blocks
		COPY_MEM((REBYTE*)VAL_BLK(v1), p2, len);
		COPY_MEM((REBYTE*)VAL_BLK(v2), p1, len);
	}
	else {
		v1 = (REBVAL*)p2;
		v2 = (REBVAL*)p1;
	}

	// Check argument types of comparator function.
	// TODO: The below results in an error message such as "op! does not allow
	// unset! for its value1 argument". A better message would be more like
	// "compare handler does not allow error! for its value1 argument."
	args = VAL_FUNC_ARGS(func);
	if (BLK_LEN(args) > 1 && !TYPE_CHECK(BLK_SKIP(args, 1), VAL_TYPE(v1)))
		Trap3(RE_EXPECT_ARG, Of_Type(func), BLK_SKIP(args, 1), Of_Type(v1));
	if (BLK_LEN(args) > 2 && !TYPE_CHECK(BLK_SKIP(args, 2), VAL_TYPE(v2)))
		Trap3(RE_EXPECT_ARG, Of_Type(func), BLK_SKIP(args, 2), Of_Type(v2));

	val = Apply_Func(0, func, v1, v2, 0);

	if (IS_LOGIC(val)) {
		if (IS_TRUE(val))
			result = 1;
	}
	else if (IS_INTEGER(val)) {
		if (VAL_INT64(val) >= 0)
			result = (VAL_INT64(val) > 0) ? 1 : 0;
	}
	else if (IS_DECIMAL(val)) {
		if (VAL_DECIMAL(val) >= 0)
			result = (VAL_DECIMAL(val) > 0) ? 1 : 0;
	}
	return GET_FLAG(flags, SORT_FLAG_REVERSE) ? -result : result;
}


/***********************************************************************
**
*/	static void Sort_Block(REBVAL *block, REBFLG ccase, REBVAL *skipv, REBVAL *compv, REBVAL *part, REBFLG all, REBFLG rev, REBFLG unst)
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
**		/unstable {Unstable Adaptive Symmetry Partition sort}
**
***********************************************************************/
{
	REBCNT len;
	REBCNT skip = 1;
	REBCNT size = sizeof(REBVAL);
	REBVAL v1, v2;
	REBINT stack = DSP; // current stack pointer
//	int (*sfunc)(const void *v1, const void *v2);

	// Determine length of sort:
	len = Partial1(block, part);
	if (len <= 1) return;

	// Skip factor:
	if (!IS_NONE(skipv)) {
		skip = Get_Num_Arg(skipv);
		if (skip <= 0 || len % skip != 0 || skip > len)
			Trap_Range(skipv);
	}
	if (IS_INTEGER(compv)) {
		if (IS_NONE(skipv) || VAL_INT64(compv)<1 || VAL_INT64(compv)>VAL_INT64(skipv))
			Trap1(RE_INVALID_ARG, compv);
	}

	REBU64 flags = 0;
	if (ccase) SET_FLAG(flags, SORT_FLAG_CASE);
	if (rev)   SET_FLAG(flags, SORT_FLAG_REVERSE);
	
	if (all) {
		if (IS_INTEGER(compv))
			Trap0(RE_BAD_REFINES);
		DS_PUSH_INTEGER(skip);
		if (IS_FUNCTION(compv)) {
			// Create temporary blocks to hold comparator function arguments
			Set_Block(&v1, Make_Block(skip));
			Set_Block(&v2, Make_Block(skip));
			SERIES_TAIL(VAL_SERIES(&v1)) = skip;
			SERIES_TAIL(VAL_SERIES(&v2)) = skip;
			BLK_TERM(VAL_SERIES(&v1));
			BLK_TERM(VAL_SERIES(&v2));
			// Lock them to prevent modification during sorting
			PROTECT_SERIES(VAL_SERIES(&v1));
			PROTECT_SERIES(VAL_SERIES(&v2));
			LOCK_SERIES(VAL_SERIES(&v1));
			LOCK_SERIES(VAL_SERIES(&v2));
			// Push argument blocks and comparator onto the stack
			DS_PUSH(&v1);
			DS_PUSH(&v2);
			DS_PUSH(compv);
			SET_FLAG(flags, SORT_FLAG_ALL);
		}
	}
	else {
		DS_PUSH(compv);
	}
	DS_PUSH_INTEGER(flags);

	// Use fast quicksort library function:
	if (skip > 1) len /= skip, size *= skip;

	int (*cmp)(const void*, const void*);

	if (ANY_FUNC(compv)) {
		cmp = Compare_Call;
	}
	else if (all && skip > 1) {
		cmp = Compare_All_Val;
	}
	else if (IS_BLOCK(compv)) {
		// Validate first...
		REBVAL* tmp = VAL_BLK_DATA(compv);
		while (!IS_END(tmp)) {
			if (!IS_INTEGER(tmp) || VAL_INT64(tmp) < 1 || VAL_INT64(tmp) > skip)
				Trap1(RE_INVALID_ARG, tmp);
			tmp++;
		}
		cmp = Compare_Val_Multi;
	}
	else {
		cmp = Compare_Val;
	}
	if (unst) {
		unstable_sort((void*)VAL_BLK_DATA(block), len, size, cmp);
	}
	else {
		stable_sort((void*)VAL_BLK_DATA(block), len, size, cmp);
	}

	if (all && IS_FUNCTION(compv)) {
		// Release temporary blocks
		ASSERT1(IS_BLOCK(&v1) && IS_BLOCK(&v2), RP_MISC);
		Free_Series(VAL_SERIES(&v1));
		Free_Series(VAL_SERIES(&v2));
	}
	// Stored comparator and flags are not needed anymore
	DSP = stack;
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
		a/not-found: 10 error or append?
		a/not-followed: 10 error or append?
	*/

	if (IS_INTEGER(pvs->select) || IS_DECIMAL(pvs->select)) {
		REBINT i = Int32(pvs->select);
		if (i == 0) {
			// "value out of range" error with: path/0: 10
			// returns NONE for: path/0
			return (pvs->setval) ? PE_BAD_RANGE : PE_NONE;
		}
		if (i < 0) i++;
		n = i + VAL_INDEX(pvs->value) - 1;
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
		return (pvs->setval)
			// in case of setting a path,
			// return "cannot access c in path" error when used path/c: 10
			// and or "value out of range" error with: path/10: 10
			? ((IS_INTEGER(pvs->select) || IS_DECIMAL(pvs->select)) ? PE_BAD_RANGE : PE_BAD_SELECT)
			: PE_NONE;
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
	if (n == 0) return 0;
	if (n < 0) n++;
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
	REBVAL  *arg2;
	REBSER  *ser;
	REBCNT	index;
	REBLEN	tail;
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

	index = VAL_INDEX(value);
	tail  = VAL_TAIL(value);
	ser   = VAL_SERIES(value);
	if (index > tail)
		VAL_INDEX(value) = index = tail;

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
		if (D_REF(ARG_TAKE_ALL)) {
			if (tail <= index) goto zero_blk;
			len = tail - index;
			SET_TRUE(D_ARG(ARG_TAKE_PART));
		}
		else if (D_REF(ARG_TAKE_PART)) {
			len = Partial1(value, D_ARG(ARG_TAKE_RANGE));
			if (len == 0) {
zero_blk:
				Set_Block(D_RET, Make_Block(0));
				return R_RET;
			}
		} else 
			len = 1;

		index = VAL_INDEX(value); // /part can change index
		// take/last:
		if (tail <= index) goto is_none;
		if (D_REF(ARG_TAKE_LAST)) index = tail - len;
		if (index >= tail) {
			if (!D_REF(ARG_TAKE_PART)) goto is_none;
			goto zero_blk;
		}

		// if no /part, just return value, else return block:
		if (!D_REF(ARG_TAKE_PART)) {
			*D_RET = BLK_HEAD(ser)[index];
			if (D_REF(ARG_TAKE_DEEP) && ANY_SERIES(D_RET)) {
				VAL_SERIES(D_RET) = ANY_BLOCK(D_RET)
					? Clone_Block(VAL_SERIES(D_RET))
					: Copy_Series(VAL_SERIES(D_RET));
			}
		}
		else {
			Set_Series(
				VAL_TYPE(value), D_RET,
				D_REF(ARG_TAKE_DEEP)
					? Copy_Block_Values(ser, 0, len, CP_DEEP | TS_DEEP_COPIED)
					: Copy_Block_Len(ser, index, len)
			);
		}
		Remove_Series(ser, index, len);
		return R_RET;

	case A_PUT:
		arg2 = D_ARG(ARG_PUT_VALUE);
		args = D_REF(ARG_PUT_CASE) ? AM_FIND_CASE : 0;
		ret = IS_INTEGER(D_ARG(ARG_PUT_SIZE)) ? Int32s(D_ARG(ARG_PUT_SIZE), 1) : 1;
		ret = Find_Block(ser, index, tail, arg, len, args, ret);
		if(ret != NOT_FOUND) {
			ret++;
			if (ret >= tail) {
				// when key is last value in the block
				Expand_Series(ser, tail, 1);
			}
			*BLK_SKIP(ser, ret) = *arg2;
		}
		else {
			Expand_Series(ser, tail, 2);
			*BLK_SKIP(ser, tail) = *arg;
			*BLK_SKIP(ser, tail+1) = *arg2;
		}
		return R_ARG3;

	//-- Search:

	case A_FIND:
	case A_SELECT:
		args = Find_Refines(ds, ALL_FIND_REFS);
//		if (ANY_BLOCK(arg) || args) {
			len = ANY_BLOCK(arg) ? VAL_BLK_LEN(arg) : 1;
			if (args & AM_FIND_PART) tail = index + Partial1(value, D_ARG(ARG_FIND_RANGE));
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
			if (args & AM_FIND_TAIL) ret += len;
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
		len = Partial1(value, D_ARG(ARG_COPY_RANGE));
		index = (REBINT)VAL_INDEX(value);
//		VAL_SERIES(value) = (len > 0) ? Copy_Block_Deep(ser, index, len, args) : Make_Block(0);
		VAL_INDEX(value) = 0;
#else
	{
		REBU64 types = 0;
		if (D_REF(ARG_COPY_DEEP)) {
			types |= CP_DEEP | (D_REF(ARG_COPY_TYPES) ? 0 : TS_DEEP_COPIED);
		}
		if D_REF(ARG_COPY_TYPES) {
			arg = D_ARG(ARG_COPY_KINDS);
			if (IS_DATATYPE(arg)) types |= TYPESET(VAL_DATATYPE(arg));
			else types |= VAL_TYPESET(arg);
		}
		len = Partial1(value, D_ARG(ARG_COPY_RANGE));
		VAL_SERIES(value) = Copy_Block_Values(ser, VAL_INDEX(value), VAL_INDEX(value)+len, types);
		VAL_INDEX(value) = 0;
	}
#endif
		break;

	//-- Special actions:

	case A_TRIM:
		args = Find_Refines(ds, ALL_TRIM_REFS);
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
			D_REF(10),	// reverse
			D_REF(11)	// unstable
		);
		break;

	case A_RANDOM:
		if (IS_PROTECT_SERIES(VAL_SERIES(value))) Trap0(RE_PROTECTED);
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
