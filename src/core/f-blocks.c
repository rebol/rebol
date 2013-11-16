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
**  Module:  f-blocks.c
**  Summary: primary block series support functions
**  Section: functional
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	REBSER *Make_Block(REBCNT length)
/*
**		Make a block series. Add 1 extra for the terminator.
**		Set TAIL to zero and set terminator.
**
***********************************************************************/
{
	REBSER *series = Make_Series(length + 1, sizeof(REBVAL), FALSE);
	SET_END(BLK_HEAD(series));
	PG_Reb_Stats->Blocks++;
	return series;
}


/***********************************************************************
**
*/	REBSER *Copy_Block(REBSER *block, REBCNT index)
/*
**		Shallow copy a block from the given index thru the tail.
**
***********************************************************************/
{
	REBCNT len = SERIES_TAIL(block);
	REBSER *series;

	if (index > len) return Make_Block(0);

	len -= index;
	series = Make_Series(len + 1, sizeof(REBVAL), FALSE);
	COPY_BLK_PART(series, BLK_SKIP(block, index), len);

	PG_Reb_Stats->Blocks++;

	return series;
}


/***********************************************************************
**
*/	REBSER *Copy_Block_Len(REBSER *block, REBCNT index, REBCNT len)
/*
**		Shallow copy a block from the given index for given length.
**
***********************************************************************/
{
	REBSER *series;

	if (index > SERIES_TAIL(block)) return Make_Block(0);
	if (index + len > SERIES_TAIL(block)) len = SERIES_TAIL(block) - index;

	series = Make_Series(len + 1, sizeof(REBVAL), FALSE);
	COPY_BLK_PART(series, BLK_SKIP(block, index), len);

	PG_Reb_Stats->Blocks++;

	return series;
}


/***********************************************************************
**
*/	REBSER *Copy_Values(REBVAL *blk, REBCNT length)
/*
**		Shallow copy a block from current value for length values.
**
***********************************************************************/
{
	REBSER *series;
	
	series = Make_Series(length + 1, sizeof(REBVAL), FALSE);
	COPY_BLK_PART(series, blk, length);

	PG_Reb_Stats->Blocks++;

	return series;
}


/***********************************************************************
**
*/	void Copy_Deep_Values(REBSER *block, REBCNT index, REBCNT tail, REBU64 types)
/*
**		Copy the contents of values specified by types. If the
**		DEEP flag is set, recurse into sub-blocks and objects.
**
***********************************************************************/
{
	REBVAL *val;

	for (; index < tail; index++) {

		val = BLK_SKIP(block, index);

		if ((types & TYPESET(VAL_TYPE(val)) & TS_SERIES_OBJ) != 0) {
			// Replace just the series field of the value
			// Note that this should work for objects too (the frame).
			VAL_SERIES(val) = Copy_Series(VAL_SERIES(val));
			if ((types & TYPESET(VAL_TYPE(val)) & TS_BLOCKS_OBJ) != 0) {
				PG_Reb_Stats->Blocks++;
				// If we need to copy recursively (deep):
				if ((types & CP_DEEP) != 0)
					Copy_Deep_Values(VAL_SERIES(val), 0, VAL_TAIL(val), types);
			}
		} else if (types & TYPESET(VAL_TYPE(val)) & TS_FUNCLOS)
			Clone_Function(val, val);
	}
}


/***********************************************************************
**
*/	REBSER *Copy_Block_Values(REBSER *block, REBCNT index, REBCNT tail, REBU64 types)
/*
**		Copy a block, copy specified values, deeply if indicated.
**
***********************************************************************/
{
	REBSER *series;

	if (index > tail) index = tail;
	if (index > SERIES_TAIL(block)) return Make_Block(0);

	series = Copy_Values(BLK_SKIP(block, index), tail - index);

	if (types != 0) Copy_Deep_Values(series, 0, SERIES_TAIL(series), types);
	
	return series;
}


/***********************************************************************
**
*/	REBSER *Clone_Block(REBSER *block)
/*
**		Deep copy block, including all series (strings and blocks),
**		but not images, bitsets, maps, etc.
**
***********************************************************************/
{
	return Copy_Block_Values(block, 0, SERIES_TAIL(block), TS_CODE);
}


/***********************************************************************
**
*/	REBSER *Clone_Block_Value(REBVAL *code)
/*
**		Same as above, but uses a value.
**
***********************************************************************/
{
	// Note: TAIL will be clipped to correct size if INDEX is not zero.
	return Copy_Block_Values(VAL_SERIES(code), VAL_INDEX(code), VAL_TAIL(code), TS_CODE);
}


#ifdef obsolete
/***********************************************************************
**
x*/	REBSER *Copy_Block_Deep(REBSER *block, REBCNT index, REBINT len, REBCNT mode)
/*
**		A useful function for copying a block and its contents.
**
**		index - used to indicate the start of the copy.
**		length - can be zero, which means use the series length - index,
**			or it can be any length, which if its less than the length
**			of the series will clip it, or if it's longer will allocate
**			extra space for it.
**		mode - indicates what to copy, how deep to copy.
**
***********************************************************************/
{
	REBSER *series;
	REBVAL *val;
	REBINT maxlen = (REBINT)SERIES_TAIL(block) - index;

	CHECK_STACK(&series);

	if (mode & COPY_OBJECT) mode |= COPY_STRINGS;

	//DISABLE_GC; // Copy deep may trigger recycle

	if (maxlen < 0) maxlen = 0;
	if (len == 0 || len > maxlen) len = maxlen; // (clip size)

	series = (mode & COPY_SAME) ? block : Copy_Values(BLK_SKIP(block, index), len);

	val = BLK_HEAD(series);
	if (mode & COPY_SAME) {
		val += index;
		mode &= ~COPY_SAME;
	}

	for (; len > 0; len--, val++) {
		if (
			((mode & COPY_DEEP) && (ANY_BLOCK(val) || IS_OBJECT(val) || IS_PORT(val)))
			||
			((mode & COPY_OBJECT) && ANY_BLOCK(val))
		) {
			VAL_SERIES(val) = Copy_Block_Deep(VAL_SERIES(val), 0, 0, mode);
		}
		if ((mode & COPY_STRINGS) && ANY_BINSTR(val)) {
			VAL_SERIES(val) = Copy_Series(VAL_SERIES(val));
		}
	}
	//ENABLE_GC;

	return series;
}
#endif


/***********************************************************************
**
*/	REBSER *Copy_Expand_Block(REBSER *block, REBCNT extra)
/*
**		Create an expanded copy of the block, but with same tail.
**
***********************************************************************/
{
	REBCNT len = SERIES_TAIL(block);
	REBSER *series = Make_Series(len + extra + 1, sizeof(REBVAL), FALSE);
	COPY_BLK_PART(series, BLK_HEAD(block), len);
	PG_Reb_Stats->Blocks++;
	return series;
}


/***********************************************************************
**
*/	void Copy_Stack_Values(REBINT start, REBVAL *into)
/*
**		Copy computed values from the stack into the series
**		specified by "into", or if into is NULL then store it as a
**		block on top of the stack.  (Also checks to see if into
**		is protected, and will trigger a trap if that is the case.)
**
***********************************************************************/
{
	REBSER *series;
	REBVAL *blk = DS_Base + start;
	REBCNT len = DSP - start + 1;
	REBCNT type;
	
	if (into) {
		type = VAL_TYPE(into);
		series = VAL_SERIES(into);
		if (IS_PROTECT_SERIES(series)) Trap0(RE_PROTECTED);
		len = Insert_Series(series, VAL_INDEX(into), (REBYTE*)blk, len);
	} else {
		series = Make_Series(len + 1, sizeof(REBVAL), FALSE);
		COPY_BLK_PART(series, blk, len);
		len = 0;
		type = REB_BLOCK;
		PG_Reb_Stats->Blocks++;
	}

	DSP = start;
	blk = DS_TOP;
	VAL_SET(blk, type);
	VAL_SERIES(blk) = series;
	VAL_INDEX(blk) = len;
	VAL_SERIES_SIDE(blk) = 0;
}


/***********************************************************************
**
*/	REBVAL *Append_Value(REBSER *block)
/*
**		Append a value to a block series at its tail.
**		Expand it if necessary. Update the termination and tail.
**		Returns the new value for you to initialize.
**
***********************************************************************/
{
	REBVAL *value;

	EXPAND_SERIES_TAIL(block, 1);
	value = BLK_TAIL(block);
	SET_END(value);
	value--;
	SET_NONE(value);  // Expand_Series leaves a hole here to be filled
	return value;
}

#ifdef ndef
/***********************************************************************
**
*/	void Append_Block(REBSER *block, REBSER *added)
/*
**		Append a block to the tail of a block.
**		Expand it if necessary. Update the termination and tail.
**
***********************************************************************/
{
	Insert_Series(block, block->tail, (void*)BLK_HEAD(added), added->tail);
}
#endif

/***********************************************************************
**
*/	void Append_Val(REBSER *block, REBVAL *val)
/*
**		Append a value to a block series at its tail.
**		Expand it if necessary. Update the termination and tail.
**
***********************************************************************/
{
	REBVAL *value;

	EXPAND_SERIES_TAIL(block, 1);
	value = BLK_TAIL(block);
	SET_END(value);
	value--;
	*value = *val;
}


/***********************************************************************
**
*/	REBINT Find_Same_Block(REBSER *blk, REBVAL *val)
/*
**		Scan a block for any values that reference blocks related
**		to the value provided.
**
**		Defect: only checks certain kinds of values.
**
***********************************************************************/
{
	REBVAL *bp;
	REBINT index = 0;

	for (bp = BLK_HEAD(blk); NOT_END(bp); bp++, index++) {

		if (VAL_TYPE(bp) >= REB_BLOCK &&
			VAL_TYPE(bp) <= REB_MAP && 
			VAL_BLK(bp) == VAL_BLK(val)
		) return index+1;

		if (
			VAL_TYPE(bp) >= REB_OBJECT &&
			VAL_TYPE(bp) <= REB_PORT &&
			VAL_OBJ_FRAME(bp) == VAL_OBJ_FRAME(val)
		) return index+1;
	}
	return -1;
}

#ifdef ndef
/***********************************************************************
**
*/	REBSER *Copy_Side_Series(REBSER *ser)
/*
**		Copy a hash or list side series
**
***********************************************************************/
{
	REBSER *ret;

	ret = Make_Series(ser->tail, SERIES_WIDE(ser), FALSE);
	ret->tail = ser->tail;
	memcpy(ret->data, ser->data, ret->tail * SERIES_WIDE(ret));
	return ret;
}
#endif

/***********************************************************************
**
*/	void Clear_Value_Opts(REBSER *ser)
/*
**		Clear all options for values of a block series.
**
***********************************************************************/
{
	REBVAL *val = BLK_HEAD(ser);

	for (; NOT_END(val); val++) {
		VAL_OPTS(val) = 0;
	}
}


/***********************************************************************
**
*/	void Unmark(REBVAL *val)
/*
**		Clear the recusion markers for series and object trees.
**
**		Note: these markers are also used for GC. Functions that
**		call this must not be able to trigger GC!
**
***********************************************************************/
{
	// The next line works because VAL_OBJ_FRAME(val) == VAL_SERIES(val)
	REBSER *series = VAL_SERIES(val);

	if (!IS_MARK_SERIES(series)) return; // avoid loop

	UNMARK_SERIES(series);

	for (val = VAL_BLK(val); NOT_END(val); val++) {
		if (ANY_SERIES(val) || IS_OBJECT(val) || IS_MODULE(val)
			|| IS_ERROR(val) || IS_PORT(val))
			Unmark(val);
	}
}
