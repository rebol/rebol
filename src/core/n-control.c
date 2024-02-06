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
	PROT_WORDS,
	PROT_LOCK
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
	if (ANY_SERIES(value) || IS_MAP(value) || IS_BITSET(value))
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

	if (GET_FLAG(flags, PROT_SET)) {
		PROTECT_SERIES(series);
		if (GET_FLAG(flags, PROT_LOCK)) LOCK_SERIES(series);
	} 
	else
		//unprotect series only when not locked (using protect/permanently)
		if (!IS_LOCK_SERIES(series))
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
	for (value = FRM_WORDS(series)+1; NOT_END(value); value++) {
		Protect_Word(value, flags);
	}
	if (GET_FLAG(flags, PROT_SET)) {
		// protecting...
		if (!GET_FLAG(flags, PROT_WORDS)) {
			PROTECT_SERIES(series);
			if (GET_FLAG(flags, PROT_LOCK))
				LOCK_SERIES(series);
		}
	} else {
		// unprotecting...
		if(!GET_FLAG(flags, PROT_WORDS)) {
			//unprotect series only when not locked (using protect/permanently)
			if (!IS_LOCK_SERIES(series))
				UNPROTECT_SERIES(series);
		}
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
			if(!IS_SCALAR(val)) {
				Protect_Value(val, flags);
				Unmark(val);
			}
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
**		6: /permanent - protects permanently (unprotect would fail)
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);

	// flags has PROT_SET bit (set or not)

	Check_Security(SYM_PROTECT, POL_WRITE, val);

	if (D_REF(2)) SET_FLAG(flags, PROT_DEEP);
	if (D_REF(3)) SET_FLAG(flags, PROT_WORDS);

	if (D_REF(5)) SET_FLAG(flags, PROT_HIDE);
	else SET_FLAG(flags, PROT_WORD); // there is no unhide

	if (D_REF(6)) SET_FLAG(flags, PROT_LOCK);

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
*/	REBNATIVE(protectedq)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);
	if (IS_WORD(value) || IS_PATH(value)) {
		REBVAL *wrd = NULL;
		if (ANY_WORD(value) && HAS_FRAME(value) && VAL_WORD_INDEX(value) > 0) {
			wrd = FRM_WORDS(VAL_WORD_FRAME(value))+VAL_WORD_INDEX(value);
		}
		else if (ANY_PATH(value)) {
			REBCNT index;
			REBSER *obj;
			if (NZ(obj = Resolve_Path(value, &index))) {
				wrd = FRM_WORD(obj, index);
			}
		}
		if(wrd && VAL_GET_OPT(wrd, OPTS_LOCK)) return R_TRUE;
	}
	else if (ANY_SERIES(value) || IS_MAP(value)) {
		if(IS_PROTECT_SERIES(VAL_SERIES(value))) return R_TRUE;
	}
	else if (IS_OBJECT(value) || IS_MODULE(value)) {
		if(IS_PROTECT_SERIES(VAL_OBJ_FRAME(value))) return R_TRUE;
	}
	
	return R_FALSE;
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
		// ignore unset result
		if (IS_UNSET(DS_TOP)) {
			DS_DROP;
			continue;
		}
		ds = DS_POP;  // volatile stack reference
		if (IS_FALSE(ds)) return R_NONE;
		// store the value as a potencial result
		DS_RET_VALUE(ds);
		// stop precessing if the value was thrown
		if (THROWN(ds)) break;
	}
	return R_RET;
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
		if (IS_UNSET(ds)) continue; // ignore unset result
		if (!IS_FALSE(ds)) return R_TOS1;
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
	if (IS_ERROR(DS_NEXT) && (D_REF(2) || !IS_THROW(DS_NEXT))) return R_NONE;
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
				if (IS_UNSET(ds) && !all_flag) return R_UNSET;
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
	REBCNT sym = 0;
	REBVAL callback = *D_ARG(ARG_CATCH_CALLBACK);
	REBVAL *last_result = Get_System(SYS_STATE, STATE_LAST_RESULT);
	REBOOL quit;

	if (D_REF(ARG_CATCH_QUIT)) {
		quit = Try_Block_Halt(VAL_SERIES(D_ARG(ARG_CATCH_BLOCK)), VAL_INDEX(D_ARG(ARG_CATCH_BLOCK)));
		ret = DS_NEXT;
		if (quit) {
			// We are here because of a QUIT or HALT condition.
			if (VAL_ERR_NUM(ret) == RE_QUIT)
				*DS_RETURN = *(VAL_ERR_VALUE(ret));
			else if (VAL_ERR_NUM(ret) == RE_HALT)
				VAL_SET(DS_RETURN, REB_UNSET);
				//Halt_Code(RE_HALT, 0); // Don't use this if we want to be able catch all!
			else
				Crash(RP_NO_CATCH);

			goto callback;
		}
		if (!D_REF(ARG_CATCH_NAME)) return R_TOS1;
	} else {
		// Evaluate the block:
		ret = DO_BLK(D_ARG(ARG_CATCH_BLOCK));
	}	

	// If it is a throw, process it:
	if (IS_ERROR(ret) && VAL_ERR_NUM(ret) == RE_THROW) {
		// Get optional thrown name
		sym = VAL_ERR_SYM(ret);

		if (D_REF(ARG_CATCH_ALL)) goto caught;

		// If a named catch, then check it:
		if (D_REF(ARG_CATCH_NAME)) {
			val = D_ARG(ARG_CATCH_WORD); // catch/name value

			// If name is the same word:
			if (IS_WORD(val) && sym == VAL_WORD_CANON(val)) goto caught;

			// If it is a block of words:
			else if (IS_BLOCK(val)) {
				for (val = VAL_BLK_DATA(val); NOT_END(val); val++) {
					if (IS_WORD(val) && sym == VAL_WORD_CANON(val)) goto caught;
				}
			}
			//else if (IS_LOGIC(val) && VAL_LOGIC(val)) goto caught; // used CATCH/name [] true
		} else {
			// Used catch without name. If there was thrown a name, then let it pass thru.
			if (sym != 0) {
				*DS_RETURN = *ret;
				return R_RET;
			}
caught:     // Thrown is being caught.
			// Store the thrown value as the return value...
			*DS_RETURN = *(VAL_ERR_VALUE(ret));
callback:	// ...and the last result.
			*last_result = *DS_RETURN;
			// If there is a callback code, then evaluate it.
			if (IS_FUNCTION(&callback)) {
				// catch [throw 1] func[value name][value]
				// Return result of the callback function
				REBVAL name = *DS_NEXT;
				if(sym) {
					Set_Word(&name, sym, 0, 0);
					VAL_SET(&name, REB_WORD);
				} else {
					SET_NONE(&name);
				}
				Apply_Func(0, &callback, last_result, &name, 0);
			}
			else if (IS_BLOCK(&callback)) {
				// (catch/with [throw 1][2]) == 2
				// Return result of the callback block evaluation.
				*last_result = *DO_BLK(&callback);
			}
			else {
				// (catch [throw 1]) == 1
				// Return the thrown value.
				return R_RET; 
			}
			// Return the result of the callback code evaluation.
			return R_TOS1;
		}
	}
	// No throw (return just the result of the block evaluation),
	// or an unhandled throw (return the thrown error value, so it may be catched later)
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
	REBVAL *into = D_REF(4) ? D_ARG(5) : 0;

	if (IS_BLOCK(value)) {
		Compose_Block(value, D_REF(2), D_REF(3), into);
		return R_TOS;
	}
	else if (into != 0) {
		REBINT start = DSP + 1;
		if (IS_WORD(value)) {
			value = Get_Var(value);
			DS_PUSH(value);
		}
		else if (IS_PATH(value)) {
			Do_Path(&value, 0); // pushes val on stack
		}
		else DS_PUSH(value);
		Copy_Stack_Values(start, into);
		return R_TOS;
	}
	
	return R_ARG1;
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
	REBINT ret = R_RET;

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
		ret = R_ARG1;
		break;

	case REB_PATH:
		Do_Path(&value, 0);
		value = DS_POP; // volatile stack reference
		if (ANY_FUNC(value)) VAL_SET_OPT(value, OPTS_REVAL);
		*D_RET = *value;
		break;

	case REB_WORD:
	case REB_GET_WORD:
		*D_RET = *Get_Var(value);
		if (ANY_FUNC(D_RET)) VAL_SET_OPT(D_RET, OPTS_REVAL);
		break;

	case REB_LIT_WORD:
		*D_RET = *value;
		SET_TYPE(D_RET, REB_WORD);
		break;

	case REB_LIT_PATH:
		*D_RET = *value;
		SET_TYPE(D_RET, REB_PATH);
		break;

	case REB_ERROR:
		if (IS_THROW(value)) return R_ARG1;
		Throw_Error(VAL_ERR_OBJECT(value));

	case REB_BINARY:
	case REB_STRING:
	case REB_URL:
	case REB_FILE:
		// DO native and sys/do* must use same arg list:
		Do_Sys_Func(SYS_CTX_DO_P, value, D_ARG(2), D_ARG(3), D_ARG(4), D_ARG(5), NULL);
		return R_TOS1;

	case REB_TASK:
		Do_Task(value);
		ret = R_ARG1;
		break;

	case REB_SET_WORD:
	case REB_SET_PATH:
		Trap_Arg(value);

	default:
		ret = R_ARG1;
	}

	if (D_REF(4)) Set_Var(D_ARG(5), NONE_VALUE); // fallback /next behavior

	return ret;
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
	REBVAL *val = D_ARG(1);
	REBVAL *into = D_REF(5) ? D_ARG(6) : 0;
	REBCNT type = VAL_TYPE(val);

	if (type == REB_BLOCK || type == REB_PAREN) {
		REBSER *ser = VAL_SERIES(val);
		REBCNT index = VAL_INDEX(val);

		if (D_REF(2))
			Reduce_Block_No_Set(ser, index, into);
		else if (D_REF(3))
			Reduce_Only(ser, index, D_ARG(4), into);
		else
			Reduce_Block(ser, index, into);

		if (type == REB_PAREN)
			// there must be also test, if the result is a block,
			// because it can be also THROWN! see issue-1760
			if (!into && IS_BLOCK(DS_TOP))
				SET_TYPE(DS_TOP, REB_PAREN);
		
		return R_TOS;
	}
	else if (into != 0) {
		REBINT start = DSP + 1;
		if (IS_WORD(val)) {
			val = Get_Var(val);
			DS_PUSH(val);
		}
		else if (IS_PATH(val)) {
			Do_Path(&val, 0); // pushes val on stack
		}
		else DS_PUSH(val);
		Copy_Stack_Values(start, into);
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
	REBOOL is_case = D_REF(6);
	REBOOL found = FALSE;

	// Find value in case block...
	for (; NOT_END(blk); blk++) {
		if (!IS_BLOCK(blk) && 0 == Cmp_Value(DS_ARG(1), blk, is_case)) { // avoid stack move
			// Skip forward to block...
			for (; !IS_BLOCK(blk) && NOT_END(blk); blk++);
			if (IS_END(blk)) break;
			found = TRUE;
			// Evaluate the case block
			result = DO_BLK(blk);
			if (!all) return R_TOS1;
			if (THROWN(result)) return R_TOS1;
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
	REBFLG   with = D_REF(ARG_TRY_WITH);
	REBVAL   handler;
	REBVAL  *error = Get_System(SYS_STATE, STATE_LAST_ERROR);
	SET_NONE(error); // reset the last error


	// If not used the new /with refine, try to use the deprecated /except
	if (with) {
		handler = *D_ARG(ARG_TRY_HANDLER);
	} else {
		with    =  D_REF(ARG_TRY_EXCEPT);
		handler = *D_ARG(ARG_TRY_CODE);
	}
	// TRY exception will trim the stack
	if (Try_Block(VAL_SERIES(D_ARG(ARG_TRY_BLOCK)), VAL_INDEX(D_ARG(ARG_TRY_BLOCK)))) {
		// save the error as a system/state/last-error value
	on_error:
		*error = *DS_NEXT;

		if (with) {
			if (IS_BLOCK(&handler)) {
				DO_BLK(&handler);
			}
			else { // do func[err] error
				REBVAL *args = BLK_SKIP(VAL_FUNC_ARGS(&handler), 1);
				if (NOT_END(args) && !TYPE_CHECK(args, VAL_TYPE(error))) {
					// TODO: This results in an error message such as "action!
					// does not allow error! for its value1 argument". A better
					// message would be more like "except handler does not
					// allow error! for its value1 argument."
					Trap3(RE_EXPECT_ARG, Of_Type(&handler), args, Of_Type(error));
				}
				Apply_Func(0, &handler, error, 0);
			}
		}
	}
	else if (D_REF(ARG_TRY_ALL) && THROWN(DS_NEXT)) {
		Disarm_Throw_Error(DS_NEXT);
		goto on_error;
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
