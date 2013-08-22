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
**  Module:  c-function.c
**  Summary: support for functions, actions, and closures
**  Section: core
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
/*
	Structure of functions:

		spec - interface spec block
		body - body code
		args - args list (see below)

	Args list is a block of word+values:

		word - word, 'word, :word, /word
		value - typeset! or none (valid datatypes)

	Args list provides:

		1. specifies arg order, arg kind (e.g. 'word)
		2. specifies valid datatypes (typesets)
		3. used for word and type in error output
		4. used for debugging tools (stack dumps)
		5. not used for MOLD (spec is used)
		6. used as a (pseudo) frame of function variables

*/

#include "sys-core.h"

/***********************************************************************
**
*/	REBSER *List_Func_Words(REBVAL *func)
/*
**		Return a block of function words, unbound.
**		Note: skips 0th entry.
**
***********************************************************************/
{
	REBSER *block;
	REBSER *words = VAL_FUNC_WORDS(func);
	REBCNT n;
	REBVAL *value;
	REBVAL *word;

	block = Make_Block(SERIES_TAIL(words));
	word = BLK_SKIP(words, 1);

	for (n = 1; n < SERIES_TAIL(words); word++, n++) {
		value = Append_Value(block);
		VAL_SET(value, VAL_TYPE(word));
		VAL_WORD_SYM(value) = VAL_BIND_SYM(word);
		UNBIND(value);
	}

	return block;
}


/***********************************************************************
**
*/	REBSER *List_Func_Types(REBVAL *func)
/*
**		Return a block of function arg types.
**		Note: skips 0th entry.
**
***********************************************************************/
{
	REBSER *block;
	REBSER *words = VAL_FUNC_WORDS(func);
	REBCNT n;
	REBVAL *value;
	REBVAL *word;

	block = Make_Block(SERIES_TAIL(words));
	word = BLK_SKIP(words, 1);

	for (n = 1; n < SERIES_TAIL(words); word++, n++) {
		value = Append_Value(block);
		VAL_SET(value, VAL_TYPE(word));
		VAL_WORD_SYM(value) = VAL_BIND_SYM(word);
		UNBIND(value);
	}

	return block;
}


/***********************************************************************
**
*/	REBSER *Check_Func_Spec(REBSER *block)
/*
**		Check function spec of the form:
**
**		["description" arg "notes" [type! type2! ...] /ref ...]
**
**		Throw an error for invalid values.
**
***********************************************************************/
{
	REBVAL *blk;
	REBSER *words;
	REBINT n = 0;
	REBVAL *value;

	blk = BLK_HEAD(block);
	words = Collect_Frame(BIND_ALL | BIND_NO_DUP | BIND_NO_SELF, 0, blk);

	// !!! needs more checks
	for (; NOT_END(blk); blk++) {
		switch (VAL_TYPE(blk)) {
		case REB_BLOCK:
			// Skip the SPEC block as an arg. Use other blocks as datatypes:
			if (n > 0) Make_Typeset(VAL_BLK(blk), BLK_SKIP(words, n), 0);
			break;
		case REB_STRING:
		case REB_INTEGER:	// special case used by datatype test actions
			break;
		case REB_WORD:
		case REB_GET_WORD:
		case REB_LIT_WORD:
			n++;
			break;
		case REB_REFINEMENT:
			// Refinement only allows logic! and none! for its datatype:
			n++;
			value = BLK_SKIP(words, n);
			VAL_TYPESET(value) = (TYPESET(REB_LOGIC) | TYPESET(REB_NONE));
			break;
		case REB_SET_WORD:
		default:
			Trap1(RE_BAD_FUNC_DEF, blk);
		}
	}

	return words; //Create_Frame(words, 0);
}


/***********************************************************************
**
*/	void Make_Native(REBVAL *value, REBSER *spec, REBFUN func, REBINT type)
/*
***********************************************************************/
{
	//Print("Make_Native: %s spec %d", Get_Sym_Name(type+1), SERIES_TAIL(spec));
	VAL_FUNC_SPEC(value) = spec;
	VAL_FUNC_ARGS(value) = Check_Func_Spec(spec);
	VAL_FUNC_CODE(value) = func;
	VAL_SET(value, type);
}


/***********************************************************************
**
*/	REBFLG Make_Function(REBCNT type, REBVAL *value, REBVAL *def)
/*
***********************************************************************/
{
	REBVAL *spec;
	REBVAL *body;
	REBCNT len;

	if (
		!IS_BLOCK(def)
		|| (len = VAL_LEN(def)) < 2
		|| !IS_BLOCK(spec = VAL_BLK(def))
	) return FALSE;

	body = VAL_BLK_SKIP(def, 1);

	VAL_FUNC_SPEC(value) = VAL_SERIES(spec);
	VAL_FUNC_ARGS(value) = Check_Func_Spec(VAL_SERIES(spec));

	if (type != REB_COMMAND) {
		if (len != 2 || !IS_BLOCK(body)) return FALSE;
		VAL_FUNC_BODY(value) = VAL_SERIES(body);
	}
	else
		Make_Command(value, def);

	VAL_SET(value, type);

	if (type == REB_FUNCTION || type == REB_CLOSURE)
		Bind_Relative(VAL_FUNC_ARGS(value), VAL_FUNC_ARGS(value), VAL_FUNC_BODY(value));

	return TRUE;
}


/***********************************************************************
**
*/	REBFLG Copy_Function(REBVAL *value, REBVAL *args)
/*
***********************************************************************/
{
	REBVAL *spec;
	REBVAL *body;

	if (!args || ((spec = VAL_BLK(args)) && IS_END(spec))) {
		body = 0;
		if (IS_FUNCTION(value) || IS_CLOSURE(value))
			VAL_FUNC_ARGS(value) = Copy_Block(VAL_FUNC_ARGS(value), 0);
	} else {
		body = VAL_BLK_SKIP(args, 1);
		// Spec given, must be block or *
		if (IS_BLOCK(spec)) {
			VAL_FUNC_SPEC(value) = VAL_SERIES(spec);
			VAL_FUNC_ARGS(value) = Check_Func_Spec(VAL_SERIES(spec));
		} else {
			if (!IS_STAR(spec)) return FALSE;
			VAL_FUNC_ARGS(value) = Copy_Block(VAL_FUNC_ARGS(value), 0);
		}
	}

	if (body && !IS_END(body)) {
		if (!IS_FUNCTION(value) && !IS_CLOSURE(value)) return FALSE;
		// Body must be block:
		if (!IS_BLOCK(body)) return FALSE;
		VAL_FUNC_BODY(value) = VAL_SERIES(body);
	}
	// No body, use prototype:
	else if (IS_FUNCTION(value) || IS_CLOSURE(value))
		VAL_FUNC_BODY(value) = Clone_Block(VAL_FUNC_BODY(value));

	// Rebind function words:
	if (IS_FUNCTION(value) || IS_CLOSURE(value))
		Bind_Relative(VAL_FUNC_ARGS(value), VAL_FUNC_ARGS(value), VAL_FUNC_BODY(value));

	return TRUE;
}


/***********************************************************************
**
*/	void Clone_Function(REBVAL *value, REBVAL *func)
/*
***********************************************************************/
{
	REBSER *src_frame = VAL_FUNC_ARGS(func);

	VAL_FUNC_SPEC(value) = VAL_FUNC_SPEC(func);
	VAL_FUNC_BODY(value) = Clone_Block(VAL_FUNC_BODY(func));
	VAL_FUNC_ARGS(value) = Copy_Block(src_frame, 0);
	// VAL_FUNC_BODY(value) = Clone_Block(VAL_FUNC_BODY(func));
	VAL_FUNC_BODY(value) = Copy_Block_Values(VAL_FUNC_BODY(func), 0, SERIES_TAIL(VAL_FUNC_BODY(func)), TS_CLONE);
	Rebind_Block(src_frame, VAL_FUNC_ARGS(value), BLK_HEAD(VAL_FUNC_BODY(value)), 0);
}


/***********************************************************************
**
*/	void Do_Native(REBVAL *func)
/*
***********************************************************************/
{
	REBVAL *ds;
	REBINT n;
#ifdef DEBUGGING
	REBYTE *fname = Get_Word_Name(DSF_WORD(DSF));	// for DEBUG
	Debug_Str(fname);
#endif

	Eval_Natives++;

	if (NZ(n = VAL_FUNC_CODE(func)(DS_RETURN))) {
		ds = DS_RETURN;
		switch (n) {
		case R_RET: // for compiler opt
			break;
		case R_TOS:
			*ds = *DS_TOP;
			break;
		case R_TOS1:
			*ds = *DS_NEXT;
			break;
		case R_NONE:
			SET_NONE(ds);
			break;
		case R_UNSET:
			SET_UNSET(ds);
			break;
		case R_TRUE:
			SET_TRUE(ds);
			break;
		case R_FALSE:
			SET_FALSE(ds);
			break;
		case R_ARG1:
			*ds = *D_ARG(1);
			break;
		case R_ARG2:
			*ds = *D_ARG(2);
			break;
		case R_ARG3:
			*ds = *D_ARG(3);
			break;
		}
	}
}


/***********************************************************************
**
*/	void Do_Act(REBVAL *ds, REBCNT type, REBCNT act)
/*
***********************************************************************/
{
	REBACT action;
	REBINT ret;

	action = Value_Dispatch[type];
	//ASSERT2(action != 0, RP_NO_ACTION);
	if (!action) Trap_Action(type, act);
	ret = action(ds, act);
	if (ret > 0) {
		ds = DS_RETURN;
		switch (ret) {
		case R_RET: // for compiler opt
			break;
		case R_TOS:
			*ds = *DS_TOP;
			break;
		case R_TOS1:
			*ds = *DS_NEXT;
			break;
		case R_NONE:
			SET_NONE(ds);
			break;
		case R_UNSET:
			SET_UNSET(ds);
			break;
		case R_TRUE:
			SET_TRUE(ds);
			break;
		case R_FALSE:
			SET_FALSE(ds);
			break;
		case R_ARG1:
			*ds = *D_ARG(1);
			break;
		case R_ARG2:
			*ds = *D_ARG(2);
			break;
		case R_ARG3:
			*ds = *D_ARG(3);
			break;
		}
	}
}


/***********************************************************************
**
*/	void Do_Action(REBVAL *func)
/*
***********************************************************************/
{
	REBVAL *ds = DS_RETURN;
	REBCNT type = VAL_TYPE(D_ARG(1));

	Eval_Natives++;

	ASSERT1(type < REB_MAX, RP_BAD_TYPE_ACTION);

	// Handle special datatype test cases (eg. integer?)
	if (VAL_FUNC_ACT(func) == 0) {
		VAL_SET(D_RET, REB_LOGIC);
		VAL_LOGIC(D_RET) = (type == VAL_INT64(BLK_LAST(VAL_FUNC_SPEC(func))));
		return;
	}

	Do_Act(D_RET, type, VAL_FUNC_ACT(func));
}


/***********************************************************************
**
*/	void Do_Function(REBVAL *func)
/*
***********************************************************************/
{
	REBVAL *result;
	REBVAL *ds;
#ifdef DEBUGGING
	REBYTE *name = Get_Word_Name(DSF_WORD(DSF));
#endif

	Eval_Functions++;

	//Dump_Block(VAL_FUNC_BODY(func));
	result = Do_Blk(VAL_FUNC_BODY(func), 0);
	ds = DS_RETURN;

	if (IS_ERROR(result) && IS_RETURN(result)) {
		// Value below is kept safe from GC because no-allocation is
		// done between point of SET_THROW and here.
		if (VAL_ERR_VALUE(result))
			*ds = *VAL_ERR_VALUE(result);
		else
			SET_UNSET(ds);
	}
	else *ds = *result; // Set return value (atomic)
}


/***********************************************************************
**
*/	void Do_Closure(REBVAL *func)
/*
**		Do a closure by cloning its body and rebinding it to
**		a new frame of words/values.
**
***********************************************************************/
{
	REBSER *body;
	REBSER *frame;
	REBVAL *result;
	REBVAL *ds;

	Eval_Functions++;
	//DISABLE_GC;

	// Clone the body of the function to allow rebinding to it:
	body = Clone_Block(VAL_FUNC_BODY(func));

	// Copy stack frame args as the closure object (one extra at head)
	frame = Copy_Values(BLK_SKIP(DS_Series, DS_ARG_BASE), SERIES_TAIL(VAL_FUNC_ARGS(func)));
	SET_FRAME(BLK_HEAD(frame), 0, VAL_FUNC_ARGS(func));

	// Rebind the body to the new context (deeply):
	Rebind_Block(VAL_FUNC_ARGS(func), frame, BLK_HEAD(body), REBIND_TYPE);

	ds = DS_RETURN;
	SET_OBJECT(ds, body); // keep it GC safe
	result = Do_Blk(body, 0); // GC-OK - also, result returned on DS stack
	ds = DS_RETURN;

	if (IS_ERROR(result) && IS_RETURN(result)) {
		// Value below is kept safe from GC because no-allocation is
		// done between point of SET_THROW and here.
		if (VAL_ERR_VALUE(result))
			*ds = *VAL_ERR_VALUE(result);
		else
			SET_UNSET(ds);
	}
	else *ds = *result; // Set return value (atomic)
}
