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
**  Module:  n-loop.c
**  Summary: native functions for loops
**  Section: natives
**  Author:  Carl Sassenrath
**  Notes:
**    Warning: Do not cache pointer to stack ARGS (stack may expand).
**
***********************************************************************/

#include "sys-core.h"
#include "sys-int-funcs.h" //REB_I64_ADD_OF

enum loop_each_mode {
	LM_FOR = 0,
	LM_REMOVE,
	LM_REMOVE_COUNT,
	LM_MAP
};

/***********************************************************************
**
*/	static REBSER *Init_Loop(REBVAL *spec, REBVAL *body_blk, REBSER **fram)
/*
**		Initialize standard for loops (copy block, make frame, bind).
**		Spec: WORD or [WORD ...]
**
***********************************************************************/
{
	REBSER *frame;
	REBINT len;
	REBVAL *word;
	REBVAL *vals;
	REBSER *body;

	// For :WORD format, get the var's value:
	if (IS_GET_WORD(spec)) spec = Get_Var(spec);

	// Hand-make a FRAME (done for for speed):
	len = IS_BLOCK(spec) ? VAL_LEN(spec) : 1;
	if (len == 0) Trap_Arg(spec);
	frame = Make_Frame(len);
	SET_SELFLESS(frame);
	SERIES_TAIL(frame) = len+1;
	SERIES_TAIL(FRM_WORD_SERIES(frame)) = len+1;
	
	// Mark the frame as internal series so it is not accessible!
	// See: https://github.com/Oldes/Rebol-issues/issues/2531
	INT_SERIES(frame);

	// Setup for loop:
	word = FRM_WORD(frame, 1); // skip SELF
	vals = BLK_SKIP(frame, 1);
	if (IS_BLOCK(spec)) spec = VAL_BLK_DATA(spec);

	// Optimally create the FOREACH frame:
	while (len-- > 0) {
		if (!IS_WORD(spec) && !IS_SET_WORD(spec)) {
			// Prevent inconsistent GC state:
			Free_Series(FRM_WORD_SERIES(frame));
			Free_Series(frame);
			Trap_Arg(spec);
		}
		VAL_SET(word, VAL_TYPE(spec));
		VAL_BIND_SYM(word) = VAL_WORD_SYM(spec);
		VAL_BIND_TYPESET(word) = ALL_64;
		word++;
		SET_NONE(vals);
		vals++;
		spec++;
	}
	SET_END(word);
	SET_END(vals);

	body = Clone_Block_Value(body_blk);
	Bind_Block(frame, BLK_HEAD(body), BIND_DEEP);

	*fram = frame;

	return body;
}


/***********************************************************************
**
*/	static void Loop_Series(REBVAL *var, REBSER* body, REBVAL *start, REBINT ei, REBINT ii)
/*
***********************************************************************/
{
	REBVAL *result;
	REBINT si = VAL_INDEX(start);
	REBCNT type = VAL_TYPE(start);

	*var = *start;
	
	if (ei >= (REBINT)VAL_TAIL(start)) ei = (REBINT)VAL_TAIL(start);
	if (ei < 0) ei = 0;

	for (; (ii > 0) ? si <= ei : si >= ei; si += ii) {
		VAL_INDEX(var) = si;
		result = Do_Blk(body, 0);
		if (THROWN(result) && Check_Error(result) >= 0) break;
		if (VAL_TYPE(var) != type) Trap1(RE_INVALID_TYPE, var);
		si = VAL_INDEX(var);
	}
}


/***********************************************************************
**
*/	static void Loop_Integer(REBVAL *var, REBSER* body, REBI64 start, REBI64 end, REBI64 incr)
/*
***********************************************************************/
{
	REBVAL *result;

	VAL_SET(var, REB_INTEGER);
	
	while ((incr > 0) ? start <= end : start >= end) {
		VAL_INT64(var) = start;
		result = Do_Blk(body, 0);
		if (THROWN(result) && Check_Error(result) >= 0) break;
		if (!IS_INTEGER(var)) Trap_Type(var);
		start = VAL_INT64(var);
		
		if (REB_I64_ADD_OF(start, incr, &start)) {
			Trap0(RE_OVERFLOW);
		}
	}
}


/***********************************************************************
**
*/	static void Loop_Pair(REBVAL *var, REBSER* body, REBD32 xstart, REBD32 ystart, REBD32 xend, REBD32 yend, REBD32 xincr, REBD32 yincr)
/*
***********************************************************************/
{
	REBD32 xi;
	REBD32 yi;
	REBVAL *result;

	VAL_SET(var, REB_PAIR);
	for (yi = ystart; (yincr > 0.) ? yi <= yend : yi >= yend; yi += yincr) {
		for (xi = xstart; (xincr > 0.) ? xi <= xend : xi >= xend; xi += xincr) {
			VAL_PAIR_X(var) = xi;
			VAL_PAIR_Y(var) = yi;
			result = Do_Blk(body, 0);
			if (THROWN(result) && Check_Error(result) >= 0) return;
			if (!IS_PAIR(var)) Trap_Type(var);
			xi = VAL_PAIR_X(var);
			yi = VAL_PAIR_Y(var);
		}
	}
}


/***********************************************************************
**
*/	static void Loop_Number(REBVAL *var, REBSER* body, REBVAL *start, REBVAL *end, REBVAL *incr)
/*
***********************************************************************/
{
	REBVAL *result;
	REBDEC s=0;
	REBDEC e=0;
	REBDEC i=0;

	if (IS_INTEGER(start)) s = (REBDEC)VAL_INT64(start);
	else if (IS_DECIMAL(start) || IS_PERCENT(start)) s = VAL_DECIMAL(start);
	else Trap_Arg(start);

	if (IS_INTEGER(end)) e = (REBDEC)VAL_INT64(end);
	else if (IS_DECIMAL(end) || IS_PERCENT(end)) e = VAL_DECIMAL(end);
	else Trap_Arg(end);

	if (IS_INTEGER(incr)) i = (REBDEC)VAL_INT64(incr);
	else if (IS_DECIMAL(incr) || IS_PERCENT(incr)) i = VAL_DECIMAL(incr);
	else Trap_Arg(incr);

	VAL_SET(var, REB_DECIMAL);

	for (; (i > 0.0) ? s <= e : s >= e; s += i) {
		VAL_DECIMAL(var) = s;
		result = Do_Blk(body, 0);
		if (THROWN(result) && Check_Error(result) >= 0) break;
		if (!IS_DECIMAL(var)) Trap_Type(var);
		s = VAL_DECIMAL(var);
	}
}


/***********************************************************************
**
*/	static int Loop_All(REBVAL *ds, REBINT mode)
/*
**		0: forall
**		1: forskip
**
***********************************************************************/
{
	REBVAL *var;
	REBSER *body;
	REBCNT bodi;
	REBSER *dat;
	REBINT idx;
	REBINT inc = 1;
	REBCNT type;

	var = Get_Var(D_ARG(1));
	if (IS_NONE(var)) return R_NONE;

	// Save the starting var value:
	*D_ARG(1) = *var;

	SET_NONE(D_RET);

	if (mode == 1) inc = Int32(D_ARG(2));

	type = VAL_TYPE(var);
	body = VAL_SERIES(D_ARG(mode+2));
	bodi = VAL_INDEX(D_ARG(mode+2));

	// Starting location when past end with negative skip:
	if (inc < 0 && VAL_INDEX(var) >= VAL_TAIL(var)) {
		VAL_INDEX(var) = (REBINT)VAL_TAIL(var) + inc;
	}

	// NOTE: This math only works for index in positive ranges!

	if (ANY_SERIES(var)) {
		while (TRUE) {
			dat = VAL_SERIES(var);
			idx = (REBINT)VAL_INDEX(var);
			if (idx < 0) break;
			if (idx >= (REBINT)SERIES_TAIL(dat)) {
				if (inc >= 0) break;
				idx = (REBINT)SERIES_TAIL(dat) + inc; // negative
				if (idx < 0) break;
				VAL_INDEX(var) = idx;
			}

			ds = Do_Blk(body, bodi); // (may move stack)

			if (THROWN(ds)) {	// Break, throw, continue, error.
				if (Check_Error(ds) >= 0) {
					*DS_RETURN = *DS_NEXT; // use thrown result as a return
					return R_RET; // does not resets series position
				}
			}
			*DS_RETURN = *ds;

			if (VAL_TYPE(var) != type) Trap_Arg(var);

			VAL_INDEX(var) += inc;
		}
	}
	else Trap_Arg(var);

	*var = *DS_ARG(1); // restores starting value position
	return R_RET;
}

/***********************************************************************
**
*/	static int Loop_Each(REBVAL *ds, REBINT mode)
/*
**		Supports these natives (modes):
**			0: foreach
**			1: remove-each
**			2: remove-each/count
**			3: map-each
**
***********************************************************************/
{
	REBSER *body;
	REBVAL *vars;
	REBVAL *words;
	REBSER *frame;
	REBVAL *value;
	REBSER *series;
	REBSER *out = NULL;	// output block (for LM_MAP, mode = 2)

	REBINT index;	// !!!! should these be REBCNT?
	REBINT tail;
	REBINT windex;	// write
	REBINT rindex;	// read
	REBINT err = 0;
	REBCNT i;
	REBCNT j;
	REBOOL return_count = FALSE;

	ASSERT2(mode >= 0 && mode < 4, RP_MISC);

	value = D_ARG(2); // series
	if (IS_NONE(value)) return R_NONE;

	body = Init_Loop(D_ARG(1), D_ARG(3), &frame); // vars, body
	SET_OBJECT(D_ARG(1), frame); // keep GC safe
	Set_Block(D_ARG(3), body);	 // keep GC safe

	SET_NONE(D_RET);
	SET_NONE(DS_NEXT);

	// If it's `map-each`, create result block:
	if (mode == LM_MAP) {
		out = Make_Block(VAL_LEN(value));
		Set_Block(D_RET, out);
	}
	else if (mode == LM_REMOVE_COUNT) {
		mode = LM_REMOVE;
		return_count = TRUE;
	}

	// Get series info:
	if (ANY_OBJECT(value)) {
		series = VAL_OBJ_FRAME(value);
		out = FRM_WORD_SERIES(series); // words (the out local reused)
		index = 1;
		//if (frame->tail > 3) Trap_Arg(FRM_WORD(frame, 3));
	}
	else if (IS_MAP(value)) {
		series = VAL_SERIES(value);
		index = 0;
		//if (frame->tail > 3) Trap_Arg(FRM_WORD(frame, 3));
	}
	else {
		series = VAL_SERIES(value);
		index  = VAL_INDEX(value);
		if (index >= (REBINT)SERIES_TAIL(series)) {
			if (mode == LM_REMOVE) {
				if(return_count)
					SET_INTEGER(D_RET, 0);
				else return R_ARG2;
			}
			return R_RET;
		}
	}

	if (mode==LM_REMOVE && IS_PROTECT_SERIES(series)) 
		Trap0(RE_PROTECTED);
	
	windex = index;

	// Iterate over each value in the series block:
	while (index < (tail = SERIES_TAIL(series))) {

		rindex = index;  // remember starting spot
		j = 0;

		// Set the FOREACH loop variables from the series:
		for (i = 1; i < frame->tail; i++) {

			vars = FRM_VALUE(frame, i);
			words = FRM_WORD(frame, i);

			// var spec is WORD
			if (IS_WORD(words)) {

				if (index < tail) {

					if (ANY_BLOCK(value)) {
						*vars = *BLK_SKIP(series, index);
					}

					else if (ANY_OBJECT(value)) {
						if (!VAL_GET_OPT(BLK_SKIP(out, index), OPTS_HIDE)) {
							// Alternate between word and value parts of object:
							if (j == 0) {
								Set_Word(vars, VAL_WORD_SYM(BLK_SKIP(out, index)), series, index);
								if (NOT_END(vars+1)) index--; // reset index for the value part
							}
							else if (j == 1)
								*vars = *BLK_SKIP(series, index);
							else
								Trap_Arg(words);
							j++;
						}
						else {
							// Do not evaluate this iteration
							index++;
							goto skip_hidden;
						}
					}

					else if (IS_VECTOR(value)) {
						Set_Vector_Value(vars, series, index);
					}

					else if (IS_MAP(value)) {
						if (!VAL_GET_OPT(BLK_SKIP(series, index), OPTS_HIDE)) {
							if (j == 0) {
								*vars = *BLK_SKIP(series, index & ~1);
								if (IS_END(vars+1)) index++; // only words
							}
							else if (j == 1)
								*vars = *BLK_SKIP(series, index);
							else
								Trap_Arg(words);
							j++;
						}
						else {
							index += 2;
							goto skip_hidden;
						}
					}

					else { // A string or binary
						if (IS_BINARY(value)) {
							SET_INTEGER(vars, (REBI64)(BIN_HEAD(series)[index]));
						}
						else if (IS_IMAGE(value)) {
							Set_Tuple_Pixel(BIN_SKIP(series, index<<2), vars);
						}
						else {
							VAL_SET(vars, REB_CHAR);
							VAL_CHAR(vars) = GET_ANY_CHAR(series, index);
						}
					}
					index++;
				}
				else SET_NONE(vars);
			}

			// var spec is SET_WORD:
			else if (IS_SET_WORD(words)) {
				if (ANY_OBJECT(value) || IS_MAP(value)) {
					*vars = *value;
				} else {
					VAL_SET(vars, VAL_TYPE(value));
					VAL_SERIES(vars) = series;
					VAL_INDEX(vars) = index;
				}
				//if (index < tail) index++; // do not increment block.
			}
			else Trap_Arg(words);
		}
		if (index == rindex) index++; //the word block has only set-words: foreach [a:] [1 2 3][]

		ds = Do_Blk(body, 0);

		if (THROWN(ds)) {
			if ((err = Check_Error(ds)) >= 0) {
				index = rindex;
				break;
			}
			// else CONTINUE:
			if (mode == LM_REMOVE) SET_FALSE(ds); // keep the value (for mode == LM_REMOVE)
		} else {
			err = 0; // prevent later test against uninitialized value
		}

		if (mode > LM_FOR) {
			//if (ANY_OBJECT(value)) Trap_Types(words, REB_BLOCK, VAL_TYPE(value)); //check not needed

			// If FALSE return, copy values to the write location:
			if (mode == LM_REMOVE) {  // remove-each
				if (IS_FALSE(ds)) {
					REBCNT wide = SERIES_WIDE(series);
					// memory areas may overlap, so use memmove and not memcpy!
					memmove(series->data + (windex * wide), series->data + (rindex * wide), (index - rindex) * wide);
					windex += index - rindex;
					// old: while (rindex < index) *BLK_SKIP(series, windex++) = *BLK_SKIP(series, rindex++);
				}
			}
			else
				if (!IS_UNSET(ds)) Append_Val(out, ds); // (mode == LM_MAP)
		}
skip_hidden: ;
	}

	// Finish up:
	if (mode == LM_REMOVE) {
		// Remove hole (updates tail):
		if (windex < index) Remove_Series(series, windex, index - windex);
		if (err == 2) return R_TOS1;  // If BREAK/RETURN
		if (return_count) {
			index -= windex;
			SET_INTEGER(DS_RETURN, IS_MAP(value) ? index / 2 : index);
			return R_RET;
		}
		return R_ARG2;
	}

	// If map-each and not BREAK/RETURN:
	if (mode == LM_MAP && err != 2) return R_RET;

	return R_TOS1; // foreach
}


/***********************************************************************
**
*/	REBNATIVE(for)
/*
**		FOR var start end bump [ body ]
**
***********************************************************************/
{
	REBSER *body;
	REBSER *frame;
	REBVAL *var;
	REBVAL *start = D_ARG(2);
	REBVAL *end   = D_ARG(3);
	REBVAL *incr  = D_ARG(4);

	// Copy body block, make a frame, bind loop var to it:
	body = Init_Loop(D_ARG(1), D_ARG(5), &frame);
	var = FRM_VALUE(frame, 1); // safe: not on stack
	SET_OBJECT(D_ARG(1), frame); // keep GC safe
	Set_Block(D_ARG(5), body);	 // keep GC safe

	SET_NONE(DS_NEXT); // in case nothing below happens

	// NOTE: during loop, stack may expand, so references to stack
	// values must not be absolute.

	if (IS_INTEGER(start) && IS_INTEGER(end) && IS_INTEGER(incr)) {
		Loop_Integer(var, body, VAL_INT64(start), 
			IS_DECIMAL(end) ? (REBI64)VAL_DECIMAL(end) : VAL_INT64(end), VAL_INT64(incr));
	}
	else if (ANY_SERIES(start)) {
		// Check that start and end are same type and series:
		//if (ANY_SERIES(end) && VAL_SERIES(start) != VAL_SERIES(end)) Trap_Arg(end);
		Loop_Series(var, body, start, ANY_SERIES(end) ? VAL_INDEX(end) : (Int32s(end, 1) - 1), Int32(incr));
	}
	else if (IS_PAIR(start) && IS_PAIR(end) && IS_PAIR(incr)) {
		Loop_Pair(var, body,
			VAL_PAIR_X(start), VAL_PAIR_Y(start),
			VAL_PAIR_X(end), VAL_PAIR_Y(end),
			VAL_PAIR_X(incr), VAL_PAIR_Y(incr));
	}
	else
		Loop_Number(var, body, start, end, incr);

	return R_TOS1;
}


/***********************************************************************
**
*/	REBNATIVE(forall)
/*
***********************************************************************/
{
	return Loop_All(ds, 0);
}


/***********************************************************************
**
*/	REBNATIVE(forskip)
/*
***********************************************************************/
{
	return Loop_All(ds, 1);
}


/***********************************************************************
**
*/	REBNATIVE(forever)
/*
***********************************************************************/
{
	REBVAL *result;

	SET_NONE(DS_NEXT);

	while (1) {
		result = DO_BLK(DS_ARG(1));
		if (THROWN(result) && Check_Error(result) >= 0) break;
	}

	return R_TOS1;
}


/***********************************************************************
**
*/	REBNATIVE(foreach)
/*
**		{Evaluates a block for each value(s) in a series.}
**		'word [get-word! word! block!] {Word or block of words}
**		data [series!] {The series to traverse}
**		body [block!] {Block to evaluate each time}
**
***********************************************************************/
{
	return Loop_Each(ds, LM_FOR);
}


/***********************************************************************
**
*/	REBNATIVE(remove_each)
/*
**		'word [get-word! word! block!] {Word or block of words}
**		data [series!] {The series to traverse}
**		body [block!] {Block to evaluate each time}
**		/count
**
***********************************************************************/
{
	return Loop_Each(ds, D_REF(4) ? LM_REMOVE_COUNT : LM_REMOVE);
}


/***********************************************************************
**
*/	REBNATIVE(map_each)
/*
**		'word [get-word! word! block!] {Word or block of words}
**		data [series!] {The series to traverse}
**		body [block!] {Block to evaluate each time}
**
***********************************************************************/
{
	return Loop_Each(ds, LM_MAP);
}


/***********************************************************************
**
*/	REBNATIVE(loop)
/*
***********************************************************************/
{
	REBI64 count = Int64(D_ARG(1));
	REBSER *block = VAL_SERIES(D_ARG(2));
	REBCNT index  = VAL_INDEX(D_ARG(2));

	ds = 0;
	for (; count > 0; count--) {
		ds = Do_Blk(block, index);
		if (THROWN(ds)) {
			if (Check_Error(ds) >= 0) break;
		}
	}
	if (ds) return R_TOS1;
	return R_NONE;
}


/***********************************************************************
**
*/	REBNATIVE(repeat)
/*
**		REPEAT var 123 [ body ]
**
***********************************************************************/
{
	REBSER *body;
	REBSER *frame;
	REBVAL *var;
	REBVAL *count = D_ARG(2);

	if (IS_NONE(count)) return R_NONE;

	if (IS_DECIMAL(count) || IS_PERCENT(count)) {
		VAL_INT64(count) = Int64(count);
		VAL_SET(count, REB_INTEGER);
	}

	body = Init_Loop(D_ARG(1), D_ARG(3), &frame);
	var = FRM_VALUE(frame, 1); // safe: not on stack
	SET_OBJECT(D_ARG(1), frame); // keep GC safe
	Set_Block(D_ARG(3), body);	 // keep GC safe

	SET_NONE(DS_NEXT); // in case nothing below happens

	if (ANY_SERIES(count)) {
		Loop_Series(var, body, count, VAL_TAIL(count)-1, 1);
	}
	else if (IS_INTEGER(count)) {
		Loop_Integer(var, body, 1, VAL_INT64(count), 1);
	}
	else if (IS_PAIR(count)) {
		Loop_Pair(var, body, 1., 1., VAL_PAIR_X(count), VAL_PAIR_Y(count), 1., 1.);
	}

	return R_TOS1;
}


/***********************************************************************
**
*/	REBNATIVE(until)
/*
***********************************************************************/
{
	REBSER *b1 = VAL_SERIES(D_ARG(1));
	REBCNT i1  = VAL_INDEX(D_ARG(1));

	do {
utop:
		ds = Do_Blk(b1, i1);
		if (IS_UNSET(ds)) Trap0(RE_NO_RETURN);
		if (THROWN(ds)) {
			if (Check_Error(ds) >= 0) break;
			goto utop;
		}
	} while (IS_FALSE(ds));  // Break, return errors fall out.
	return R_TOS1;
}


/***********************************************************************
**
*/	REBNATIVE(while)
/*
***********************************************************************/
{
	REBSER *b1 = VAL_SERIES(D_ARG(1));
	REBCNT i1  = VAL_INDEX(D_ARG(1));
	REBSER *b2 = VAL_SERIES(D_ARG(2));
	REBCNT i2  = VAL_INDEX(D_ARG(2));

	SET_NONE(D_RET);

	do {
		ds = Do_Blk(b1, i1);
		if (IS_UNSET(ds) || IS_ERROR(ds)) {	// Unset, break, throw, error.
			if (Check_Error(ds) >= 0) return R_TOS1;
		}
		if (!IS_TRUE(ds)) return R_RET;
		ds = Do_Blk(b2, i2);
		*DS_RETURN = *ds;	// save here (to avoid GC during error handling)
		if (THROWN(ds)) {	// Break, throw, continue, error.
			if (Check_Error(ds) >= 0) return R_TOS1;
			*DS_RETURN = *ds; // Check_Error modified it
		}
	} while (TRUE);
}
