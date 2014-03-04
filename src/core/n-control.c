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
**  Module:  n-control.c
**  Summary: native functions for control flow
**  Section: natives
**  Author:  Carl Sassenrath
**  Notes:
**    Warning: Do not cache pointer to stack ARGS (stack may expand).
**
***********************************************************************/

#include "sys-core.h"


// Local flags used for Protect functions below:
enum {
	PROT_SET,
	PROT_DEEP,
	PROT_HIDE,
	PROT_WORD,
};


/***********************************************************************
**
*/	void Protected(REBVAL *word)
/*
**		Throw an error if word is protected.
**
***********************************************************************/
{
	REBSER *frm;
	REBINT index = VAL_WORD_INDEX(word);

	if (index > 0) {
		frm = VAL_WORD_FRAME(word);
		if (VAL_PROTECTED(FRM_WORDS(frm)+index))
			Trap1(RE_LOCKED_WORD, word);
	}
	else if (index == 0) Trap0(RE_SELF_PROTECTED);
}


/***********************************************************************
**
*/	static void Protect_Word(REBVAL *value, REBCNT flags)
/*
***********************************************************************/
{
	if (GET_FLAG(flags, PROT_WORD)) {
		if (GET_FLAG(flags, PROT_SET)) VAL_SET_OPT(value, OPTS_LOCK);
		else VAL_CLR_OPT(value, OPTS_LOCK);
	}

	if (GET_FLAG(flags, PROT_HIDE)) {
		if GET_FLAG(flags, PROT_SET) VAL_SET_OPT(value, OPTS_HIDE);
		else VAL_CLR_OPT(value, OPTS_HIDE);
	}
}


/***********************************************************************
**
*/	static void Protect_Value(REBVAL *value, REBCNT flags)
/*
**		Anything that calls this must call Unmark() when done.
**
***********************************************************************/
{
	if (ANY_SERIES(value) || IS_MAP(value))
		Protect_Series(value, flags);
	else if (IS_OBJECT(value) || IS_MODULE(value))
		Protect_Object(value, flags);
}


/***********************************************************************
**
*/	void Protect_Series(REBVAL *val, REBCNT flags)
/*
**		Anything that calls this must call Unmark() when done.
**
***********************************************************************/
{
	REBSER *series = VAL_SERIES(val);

	if (IS_MARK_SERIES(series)) return; // avoid loop

	if (GET_FLAG(flags, PROT_SET))
		PROTECT_SERIES(series);
	else
		UNPROTECT_SERIES(series);

	if (!ANY_BLOCK(val) || !GET_FLAG(flags, PROT_DEEP)) return;

	MARK_SERIES(series); // recursion protection

	for (val = VAL_BLK_DATA(val); NOT_END(val); val++) {
		Protect_Value(val, flags);
	}
}


/***********************************************************************
**
*/	void Protect_Object(REBVAL *value, REBCNT flags)
/*
**		Anything that calls this must call Unmark() when done.
**
***********************************************************************/
{
	REBSER *series = VAL_OBJ_FRAME(value);

	if (IS_MARK_SERIES(series)) return; // avoid loop

	if (GET_FLAG(flags, PROT_SET)) PROTECT_SERIES(series);
	else UNPROTECT_SERIES(series);

	for (value = FRM_WORDS(series)+1; NOT_END(value); value++) {
		Protect_Word(value, flags);
	}

	if (!GET_FLAG(flags, PROT_DEEP)) return;

	MARK_SERIES(series); // recursion protection

	for (value = FRM_VALUES(series)+1; NOT_END(value); value++) {
		Protect_Value(value, flags);
	}
}


/***********************************************************************
**
*/	static void Protect_Word_Value(REBVAL *word, REBCNT flags)
/*
***********************************************************************/
{
	REBVAL *wrd;
	REBVAL *val;

	if (ANY_WORD(word) && HAS_FRAME(word) && VAL_WORD_INDEX(word) > 0) {
		wrd = FRM_WORDS(VAL_WORD_FRAME(word))+VAL_WORD_INDEX(word);
		Protect_Word(wrd, flags);
		if (GET_FLAG(flags, PROT_DEEP)) {
			val = Get_Var(word);
			Protect_Value(val, flags);
			Unmark(val);
		}
	}
	else if (ANY_PATH(word)) {
		REBCNT index;
		REBSER *obj;
		if (NZ(obj = Resolve_Path(word, &index))) {
			wrd = FRM_WORD(obj, index);
			Protect_Word(wrd, flags);
			if (GET_FLAG(flags, PROT_DEEP)) {
				Protect_Value(val = FRM_VALUE(obj, index), flags);
				Unmark(val);
			}
		}
	}
}


/***********************************************************************
**
*/	static int Protect(REBVAL *ds, REBCNT flags)
/*
**		1: value
**		2: /deep  - recursive
**		3: /words  - list of words
**		4: /values - list of values
**		5: /hide  - hide variables
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);

	// flags has PROT_SET bit (set or not)

	Check_Security(SYM_PROTECT, POL_WRITE, val);

	if (D_REF(2)) SET_FLAG(flags, PROT_DEEP);
	//if (D_REF(3)) SET_FLAG(flags, PROT_WORD);

	if (D_REF(5)) SET_FLAG(flags, PROT_HIDE);
	else SET_FLAG(flags, PROT_WORD); // there is no unhide

	if (IS_WORD(val) || IS_PATH(val)) {
		Protect_Word_Value(val, flags); // will unmark if deep
		return R_ARG1;
	}

	if (IS_BLOCK(val)) {
		if (D_REF(3)) { // /words
			for (val = VAL_BLK_DATA(val); NOT_END(val); val++)
				Protect_Word_Value(val, flags);  // will unmark if deep
			return R_ARG1;
		}
		if (D_REF(4)) { // /values
			REBVAL *val2;
			for (val = VAL_BLK_DATA(val); NOT_END(val); val++) {
				Protect_Value(val2 = Get_Any_Var(val), flags);
				if (GET_FLAG(flags, PROT_DEEP)) Unmark(val2);
			}
			return R_ARG1;
		}
	}
	
	if (GET_FLAG(flags, PROT_HIDE)) Trap0(RE_BAD_REFINES);

	Protect_Value(val, flags);

	if (GET_FLAG(flags, PROT_DEEP)) Unmark(val);

	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(also)
/*
***********************************************************************/
{
	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(all)
/*
***********************************************************************/
{
	REBSER *block = VAL_SERIES(D_ARG(1));
	REBCNT index = VAL_INDEX(D_ARG(1));

	ds = 0;
	while (index < SERIES_TAIL(block)) {
		index = Do_Next(block, index, 0); // stack volatile
		ds = DS_POP;  // volatile stack reference
		if (IS_FALSE(ds)) return R_NONE;
		if (THROWN(ds)) break;
	}
	if (ds == 0) return R_TRUE;
	return R_TOS1;
}


/***********************************************************************
**
*/	REBNATIVE(any)
/*
***********************************************************************/
{
	REBSER *block = VAL_SERIES(D_ARG(1));
	REBCNT index = VAL_INDEX(D_ARG(1));

	while (index < SERIES_TAIL(block)) {
		index = Do_Next(block, index, 0); // stack volatile
		ds = DS_POP;  // volatile stack reference
		if (!IS_FALSE(ds) && !IS_UNSET(ds)) return R_TOS1;
	}
	return R_NONE;
}


/***********************************************************************
**
*/	REBNATIVE(apply)
/*
***********************************************************************/
{
	Apply_Block(D_ARG(1), D_ARG(2), !D_REF(3)); // stack volatile
	return R_TOS;
}


/***********************************************************************
**
*/	REBNATIVE(attempt)
/*
***********************************************************************/
{
	Try_Block(VAL_SERIES(D_ARG(1)), VAL_INDEX(D_ARG(1)));
	if (IS_ERROR(DS_NEXT) && !IS_THROW(DS_NEXT)) return R_NONE;
	return R_TOS1;
}


/***********************************************************************
**
*/	REBNATIVE(break)
/*
***********************************************************************/
{
	REBVAL *value = 0;

	if (D_REF(1)) value = D_ARG(2);  // /return
	SET_THROW(ds, RE_BREAK, value);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(case)
/*
***********************************************************************/
{
	REBSER *block = VAL_SERIES(D_ARG(1));
	REBCNT index = VAL_INDEX(D_ARG(1));
	REBFLG all_flag = D_REF(2);

	while (index < SERIES_TAIL(block)) {
		//DSP = top; // reset stack  -- not needed?
		index = Do_Next(block, index, 0);
		ds = DS_POP;  // volatile stack reference
		if (IS_FALSE(ds)) index++;
		else {
			if (IS_UNSET(ds)) Trap0(RE_NO_RETURN);
			if (THROWN(ds)) return R_TOS1;
			if (index >= SERIES_TAIL(block)) return R_TRUE;
			index = Do_Next(block, index, 0);
			ds = DS_POP;  // volatile stack reference
			if (IS_BLOCK(ds)) {
				ds = DO_BLK(ds);
				if (IS_UNSET(ds) && !all_flag) return R_TRUE;
			}
			if (THROWN(ds) || !all_flag || index >= SERIES_TAIL(block))
				return R_TOS1;
		}
	}
	return R_NONE;
}


/***********************************************************************
**
*/	REBNATIVE(catch)
/*
***********************************************************************/
{
	REBVAL *val;
	REBVAL *ret;
	REBCNT sym;

	if (D_REF(4)) {	//QUIT
		if (Try_Block_Halt(VAL_SERIES(D_ARG(1)), VAL_INDEX(D_ARG(1)))) {
			// We are here because of a QUIT/HALT condition.
			ret = DS_NEXT;
			if (VAL_ERR_NUM(ret) == RE_QUIT)
				ret = VAL_ERR_VALUE(ret);
			else if (VAL_ERR_NUM(ret) == RE_HALT)
				Halt_Code(RE_HALT, 0);
			else
				Crash(RP_NO_CATCH);
			*DS_RETURN = *ret;
			return R_RET;
		}
		return R_TOS1;
	}

	// Evaluate the block:
	ret = DO_BLK(D_ARG(1));

	// If it is a throw, process it:
	if (IS_ERROR(ret) && VAL_ERR_NUM(ret) == RE_THROW) {

		// If a named throw, then check it:
		if (D_REF(2)) { // /name

			sym = VAL_ERR_SYM(ret);
			val = D_ARG(3); // name symbol

			// If name is the same word:
			if (IS_WORD(val) && sym == VAL_WORD_CANON(val)) goto got_err;

			// If it is a block of words:
			else if (IS_BLOCK(val)) {
				for (val = VAL_BLK_DATA(val); NOT_END(val); val++) {
					if (IS_WORD(val) && sym == VAL_WORD_CANON(val)) goto got_err;
				}
			}
		} else {
got_err:
			*ds = *(VAL_ERR_VALUE(ret));
			return R_RET;
		}
	}

	return R_TOS1;
}


/***********************************************************************
**
*/	REBNATIVE(throw)
/*
***********************************************************************/
{
	SET_THROW(ds, RE_THROW, D_ARG(1));
	if (D_REF(2)) // /name
		VAL_ERR_SYM(ds) = VAL_WORD_SYM(D_ARG(3));
	return R_RET;
}


#ifdef not_used
/***********************************************************************
**
*/	REBNATIVE(cause)
/*
***********************************************************************/
{
	Throw_Error(VAL_ERR_OBJECT(D_ARG(1)));
	DEAD_END;
}
#endif


/***********************************************************************
**
*/	REBNATIVE(comment)
/*
***********************************************************************/
{
	return R_UNSET;
}


/***********************************************************************
**
*/	REBNATIVE(compose)
/*
**		{Evaluates a block of expressions, only evaluating parens, and returns a block.}
**		value "Block to compose"
**		/deep "Compose nested blocks"
**		/only "Inserts a block value as a block"
**
***********************************************************************/
{
	REBVAL *value = D_ARG(1);

	if (!IS_BLOCK(value)) return R_ARG1;
	Compose_Block(value, D_REF(2), D_REF(3), D_REF(4) ? D_ARG(5) : 0);
	return R_TOS;
}


/***********************************************************************
**
*/	REBNATIVE(continue)
/*
***********************************************************************/
{
	SET_THROW(ds, RE_CONTINUE, NONE_VALUE);
	return R_RET;
}

#ifdef removed
/***********************************************************************
**
*/	REBNATIVE(disarm)
/*
***********************************************************************/
{
	return R_ARG1;
/*
	REBVAL *value = D_ARG(1);

	if (IS_ERROR(value)) VAL_SET(value, REB_OBJECT);
	*ds = *value;
	return R_RET;
*/
}
#endif

/***********************************************************************
**
*/	REBNATIVE(do)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);

	switch (VAL_TYPE(value)) {

	case REB_BLOCK:
	case REB_PAREN:
		if (D_REF(4)) { // next
			VAL_INDEX(value) = Do_Next(VAL_SERIES(value), VAL_INDEX(value), 0);
			if (VAL_INDEX(value) == END_FLAG) {
				VAL_INDEX(value) = VAL_TAIL(value);
				Set_Var(D_ARG(5), value);
				return R_UNSET;
			}
			Set_Var(D_ARG(5), value); // "continuation" of block
			return R_TOS;
		}
		else DO_BLK(value);
		return R_TOS1;

    case REB_NATIVE:
	case REB_ACTION:
    case REB_COMMAND:
    case REB_REBCODE:
    case REB_OP:
    case REB_CLOSURE:
	case REB_FUNCTION:
		VAL_SET_OPT(value, OPTS_REVAL);
		return R_ARG1;

//	case REB_PATH:  ? is it used?

	case REB_WORD:
	case REB_GET_WORD:
		*D_RET = *Get_Var(value);
		return R_RET;

	case REB_LIT_WORD:
		*D_RET = *value;
		SET_TYPE(D_RET, REB_WORD);
		return R_RET;

	case REB_ERROR:
		if (IS_THROW(value)) return R_ARG1;
		Throw_Error(VAL_ERR_OBJECT(value));

	case REB_BINARY:
	case REB_STRING:
	case REB_URL:
	case REB_FILE:
		// DO native and sys/do* must use same arg list:
		Do_Sys_Func(SYS_CTX_DO_P, value, D_ARG(2), D_ARG(3), D_ARG(4), D_ARG(5), 0);
		return R_TOS1;

	case REB_TASK:
		Do_Task(value);
		return R_ARG1;

	case REB_SET_WORD:
		Trap_Arg(value);

	default:
		return R_ARG1;
	}
}


#ifdef removed_b1505
/***********************************************************************
**
*/	REBNATIVE(eval)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);

	if (IS_BLOCK(value)) {DO_BLK(value); return R_TOS1;}
	if (IS_TASK(value)) Do_Task(value);
	return R_ARG1;
}
#endif

/***********************************************************************
**
*/	REBNATIVE(either)
/*
***********************************************************************/
{
	REBCNT argnum = IS_FALSE(D_ARG(1)) ? 3 : 2;

	if (IS_BLOCK(D_ARG(argnum)) && !D_REF(4) /* not using /ONLY */) {
		DO_BLK(D_ARG(argnum));
		return R_TOS1;
	} else {
		return argnum == 2 ? R_ARG2 : R_ARG3;
	}
}


/***********************************************************************
**
*/	REBNATIVE(exit)
/*
***********************************************************************/
{
	SET_THROW(ds, RE_RETURN, 0);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(if)
/*
***********************************************************************/
{
	if (IS_FALSE(D_ARG(1))) return R_NONE;
	if (IS_BLOCK(D_ARG(2)) && !D_REF(3) /* not using /ONLY */) {
		DO_BLK(D_ARG(2));
		return R_TOS1;
	} else {
		return R_ARG2;
	}
}


/***********************************************************************
**
*/	REBNATIVE(protect)
/*
***********************************************************************/
{
	return Protect(ds, 1); // PROT_SET
}


/***********************************************************************
**
*/	REBNATIVE(unprotect)
/*
***********************************************************************/
{
	SET_NONE(D_ARG(5)); // necessary, bogus, but no harm to stack
	return Protect(ds, 0);
}


/***********************************************************************
**
*/	REBNATIVE(reduce)
/*
***********************************************************************/
{
	if (IS_BLOCK(D_ARG(1))) {
		REBSER *ser = VAL_SERIES(D_ARG(1));
		REBCNT index = VAL_INDEX(D_ARG(1));
		REBVAL *val = D_REF(5) ? D_ARG(6) : 0;

		if (D_REF(2))
			Reduce_Block_No_Set(ser, index, val);
		else if (D_REF(3))
			Reduce_Only(ser, index, D_ARG(4), val);
		else
			Reduce_Block(ser, index, val);
		return R_TOS;
	}

	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(return)
/*
**		Returns a value from the current function. The error value
**		is built in the RETURN slot, with the arg being kept in
**		the ARG1 slot on the stack.  As long as DSP is greater, both
**		values are safe from GC.
**
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);

	SET_THROW(ds, RE_RETURN, arg);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(switch)
/*
**		value
**		cases [block!]
**		/default
**		case
**      /all {Check all cases}
**
***********************************************************************/
{
	REBVAL *blk = VAL_BLK_DATA(D_ARG(2));
	REBVAL *result;
	REBOOL all = D_REF(5);
	REBOOL found = FALSE;

	// Find value in case block...
	for (; NOT_END(blk); blk++) {
		if (!IS_BLOCK(blk) && 0 == Cmp_Value(DS_ARG(1), blk, FALSE)) { // avoid stack move
			// Skip forward to block...
			for (; !IS_BLOCK(blk) && NOT_END(blk); blk++);
			if (IS_END(blk)) break;
			found = TRUE;
			// Evaluate the case block
			result = DO_BLK(blk);
			if (!all) return R_TOS1;
			if (THROWN(result) && Check_Error(result) >= 0) break;
		}
	}

	if (!found && IS_BLOCK(result = D_ARG(4))) {
		DO_BLK(result);
		return R_TOS1;
	}

	return R_NONE;
}


/***********************************************************************
**
*/	REBNATIVE(try)
/*
***********************************************************************/
{
	REBVAL value = *D_ARG(3); // TRY exception will trim the stack
	REBFLG except = D_REF(2);

	if (Try_Block(VAL_SERIES(D_ARG(1)), VAL_INDEX(D_ARG(1)))) {
		if (except) {
			if (IS_BLOCK(&value)) {
				DO_BLK(&value);
			}
			else { // do func[error] arg
				REBVAL arg = *DS_NEXT; // will get overwritten
				Apply_Func(0, &value, &arg, 0);
			}
		}
	}

	return R_TOS1;
}


/***********************************************************************
**
*/	REBNATIVE(unless)
/*
***********************************************************************/
{
	if (IS_FALSE(D_ARG(1))) {
		if (IS_BLOCK(D_ARG(2)) && !D_REF(3) /* not using /ONLY */) {
			DO_BLK(D_ARG(2));
			return R_TOS1;
		} else {
			return R_ARG2;
		}
	}
	return R_NONE;
}
