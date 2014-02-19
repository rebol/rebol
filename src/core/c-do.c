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
**  Module:  c-do.c
**  Summary: the core interpreter - the heart of REBOL
**  Section: core
**  Author:  Carl Sassenrath
**  Notes:
**    WARNING WARNING WARNING
**    This is highly tuned code that should only be modified by experts
**    who fully understand its design. It is very easy to create odd
**    side effects so please be careful and extensively test all changes!
**
***********************************************************************/

#include "sys-core.h"
#include <stdio.h>
#include "sys-state.h"

enum Eval_Types {
	ET_INVALID,		// not valid to evaluate
	ET_WORD,
	ET_SELF,		// returns itself
	ET_FUNCTION,
	ET_OPERATOR,
	ET_PAREN,
	ET_SET_WORD,
	ET_LIT_WORD,
	ET_GET_WORD,
	ET_PATH,
	ET_LIT_PATH,
	ET_END			// end of block
};

static jmp_buf *Halt_State = 0;  //!!!!!!!!!! global?

/*
void T_Error(REBCNT n) {;}

// Deferred:
void T_Series(REBCNT n) {;}		// image
void T_List(REBCNT n) {;}		// list
*/

void Do_Rebcode(REBVAL *v) {;}

#include "tmp-evaltypes.h"

#define EVAL_TYPE(val) (Eval_Type_Map[VAL_TYPE(val)])

#define PUSH_ERROR(v, a)
#define PUSH_FUNC(v, w, s)
#define PUSH_BLOCK(b)

static REBVAL *Func_Word(REBINT dsf)
{
	static REBVAL val;  // Safe: Lifetime is limited to passage to error object.
	Init_Word(&val, VAL_WORD_SYM(DSF_WORD(dsf)));
	return &val;
}


/***********************************************************************
**
*/	void Do_Op(REBVAL *func)
/*
**		A trampoline.
**
***********************************************************************/
{
	Func_Dispatch[VAL_GET_EXT(func) - REB_NATIVE](func);
}


/***********************************************************************
**
*/	void Expand_Stack(REBCNT amount)
/*
**		Expand the datastack. Invalidates any references to stack
**		values, so code should generally use stack index integers,
**		not pointers into the stack.
**
***********************************************************************/
{
	if (SERIES_REST(DS_Series) >= STACK_LIMIT) Trap0(RE_STACK_OVERFLOW);
	DS_Series->tail = DSP+1;
	Extend_Series(DS_Series, amount);
	DS_Base = BLK_HEAD(DS_Series);
	Debug_Fmt(BOOT_STR(RS_STACK, 0), DSP, SERIES_REST(DS_Series));
}


/***********************************************************************
**
*/	void DS_Ret_Int(REBINT n)
/*
**		Memsaver: set integer as return result on data stack.
**
***********************************************************************/
{
	DS_RET_INT(n);
}


/***********************************************************************
**
*/	void DS_Ret_Val(REBVAL *value)
/*
**		Memsaver: set any value as return result on data stack.
**
***********************************************************************/
{
	*DS_RETURN = *value;
}


/***********************************************************************
**
*/  REBINT Eval_Depth()
/*
***********************************************************************/
{
	REBINT depth = 0;
	REBINT dsf;

	for (dsf = DSF; dsf > 0; dsf = PRIOR_DSF(dsf), depth++);
	return depth;
}


/***********************************************************************
**
*/	REBVAL *Stack_Frame(REBCNT n)
/*
***********************************************************************/
{
	REBCNT dsf = DSF;

	for (dsf = DSF; dsf > 0; dsf = PRIOR_DSF(dsf)) {
		if (n-- <= 0) return DS_VALUE(dsf);
	}

	return 0;
}


/***********************************************************************
**
*/  REBNATIVE(trace)
/*
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);

	Check_Security(SYM_DEBUG, POL_READ, 0);

	// The /back option: ON and OFF, or INTEGER! for # of lines:
	if (D_REF(2)) { // /back
		if (IS_LOGIC(arg)) {
			Enable_Backtrace(IS_TRUE(arg));
		}
		else if (IS_INTEGER(arg)) {
			Trace_Flags = 0;
			Display_Backtrace(Int32(arg));
			return R_UNSET;
		}
	}
	else Enable_Backtrace(FALSE);

	// Set the trace level:
	if (IS_LOGIC(arg)) {
		Trace_Level = IS_TRUE(arg) ? 100000 : 0;
	}
	else Trace_Level = Int32(arg);

	if (Trace_Level) {
		Trace_Flags = 1;
		if (D_REF(3)) SET_FLAG(Trace_Flags, 1); // function
		Trace_Depth = Eval_Depth() - 1; // subtract current TRACE frame
	}
	else Trace_Flags = 0;

	return R_UNSET;
}

static REBINT Init_Depth(void)
{
	// Check the trace depth is ok:
	int depth = Eval_Depth() - Trace_Depth;
	if (depth < 0 || depth >= Trace_Level) return -1;
	if (depth > 10) depth = 10;
	Debug_Space(4 * depth);
	return depth;
}

#define CHECK_DEPTH(d) if ((d = Init_Depth()) < 0) return;\

void Trace_Line(REBSER *block, REBINT index, REBVAL *value)
{
	int depth;

	if (GET_FLAG(Trace_Flags, 1)) return; // function
	if (ANY_FUNC(value)) return;

	CHECK_DEPTH(depth);

	Debug_Fmt_(BOOT_STR(RS_TRACE,1), index+1, value);
	if (IS_WORD(value) || IS_GET_WORD(value)) {
		value = Get_Var(value);
		if (VAL_TYPE(value) < REB_NATIVE)
			Debug_Fmt_(BOOT_STR(RS_TRACE,2), value);
		else if (VAL_TYPE(value) >= REB_NATIVE && VAL_TYPE(value) <= REB_FUNCTION)
			Debug_Fmt_(BOOT_STR(RS_TRACE,3), Get_Type_Name(value), List_Func_Words(value));
		else
			Debug_Fmt_(BOOT_STR(RS_TRACE,4), Get_Type_Name(value));
	}
	/*if (ANY_WORD(value)) {
		word = value;
		if (IS_WORD(value)) value = Get_Var(word);
		Debug_Fmt_(BOOT_STR(RS_TRACE,2), VAL_WORD_FRAME(word), VAL_WORD_INDEX(word), Get_Type_Name(value));
	}
	if (Trace_Stack) Debug_Fmt(BOOT_STR(RS_TRACE,3), DSP, DSF);
	else
	*/
	Debug_Line();
}

void Trace_Func(REBVAL *word, REBVAL *value)
{
	int depth;
	CHECK_DEPTH(depth);
	Debug_Fmt_(BOOT_STR(RS_TRACE,5), Get_Word_Name(word), Get_Type_Name(value));
	if (GET_FLAG(Trace_Flags, 1)) Debug_Values(DS_GET(DS_ARG_BASE+1), DS_ARGC, 20);
	else Debug_Line();
}

void Trace_Return(REBVAL *word, REBVAL *value)
{
	int depth;
	CHECK_DEPTH(depth);
	Debug_Fmt_(BOOT_STR(RS_TRACE,6), Get_Word_Name(word));
	Debug_Values(value, 1, 50);
}

void Trace_Arg(REBINT num, REBVAL *arg, REBVAL *path)
{
	int depth;
	if (IS_REFINEMENT(arg) && (!path || IS_END(path))) return;
	CHECK_DEPTH(depth);
	Debug_Fmt(BOOT_STR(RS_TRACE,6), num+1, arg);
}


/***********************************************************************
**
*/	void Trace_Value(REBINT n, REBVAL *value)
/*
***********************************************************************/
{
	int depth;
	CHECK_DEPTH(depth);
	Debug_Fmt(BOOT_STR(RS_TRACE,n), value);
}

/***********************************************************************
**
*/	void Trace_String(REBINT n, REBYTE *str, REBINT limit)
/*
***********************************************************************/
{
	static char tracebuf[64];
	int depth;
	CHECK_DEPTH(depth);
	memcpy(tracebuf, str, MIN(60, limit));
	Debug_Fmt(BOOT_STR(RS_TRACE,n), tracebuf);
}


/***********************************************************************
**
*/	void Trace_Error(REBVAL *value)
/*
***********************************************************************/
{
	int depth;
	CHECK_DEPTH(depth);
	Debug_Fmt(BOOT_STR(RS_TRACE, 10), &VAL_ERR_VALUES(value)->type, &VAL_ERR_VALUES(value)->id);
}


/***********************************************************************
**
*/	REBCNT Push_Func(REBFLG keep, REBSER *block, REBCNT index, REBCNT word, REBVAL *func)
/*
**		Push on stack a function call frame as defined in stack.h.
**		Optimized to reduce usage of thread globals (TLS).
**		Block value must not be NULL (otherwise will cause GC fault).
**
**		keep: use current top of stack as the return value; do not push
**			  a new value for the return.
**
**		returns: the stack index for the return value.
**
***********************************************************************/
{
	REBCNT dsp = DSP;
	REBVAL *tos = DS_VALUE(dsp);
	REBVAL *ret;

	// Set RETURN slot to its default value:
	if (keep) ret = 0, dsp--;
	else ret = ++tos; // don't unset it until bottom of this func

	// Save BLOCK current evaluation position and prior DSF;
	tos++;
	VAL_SET(tos, REB_BLOCK);
	VAL_SERIES(tos) = block;
	VAL_INDEX(tos)  = index;
	VAL_BACK(tos)   = DSF;

	// Save WORD for function and fake frame for relative arg lookup:
	tos++;
	VAL_SET(tos, REB_HANDLE); // Was REB_WORD, but GC does not like bad fields.
	VAL_WORD_SYM(tos) = word ? word : SYM__APPLY_;
	VAL_WORD_INDEX(tos) = -1; // avoid GC access to invalid FRAME above
	if (func) {
		VAL_WORD_FRAME(tos) = VAL_FUNC_ARGS(func);
		// Save FUNC value for safety (spec, args, code):
		tos++;
		*tos = *func;	// the DSF_FUNC
	} else {
		VAL_WORD_FRAME(tos) = 0;
		tos++;
		SET_NONE(tos);	// the DSF_FUNC
	}

	if (ret) SET_UNSET(ret);

	DSP = dsp + DSF_BIAS;
	return dsp + 1;
}


/***********************************************************************
**
*/	void Next_Path(REBPVS *pvs)
/*
**		Evaluate next part of a path.
**
***********************************************************************/
{
	REBVAL *path;
	REBPEF func;

	// Path must have dispatcher, else return:
	func = Path_Dispatch[VAL_TYPE(pvs->value)];
	if (!func) return; // unwind, then check for errors

	pvs->path++;

	//Debug_Fmt("Next_Path: %r/%r", pvs->path-1, pvs->path);

	// object/:field case:
	if (IS_GET_WORD(path = pvs->path)) {
		pvs->select = Get_Var(path);
		if (IS_UNSET(pvs->select)) Trap1(RE_NO_VALUE, path);
	}
	// object/(expr) case:
	else if (IS_PAREN(path)) {
		// ?? GC protect stuff !!!!!! stack could expand!
		pvs->select = Do_Blk(VAL_SERIES(path), 0);
	}
	else // object/word and object/value case:
		pvs->select = path;

	// Uses selector on the value.
	// .path - must be advanced as path is used (modified by func)
	// .value - holds currently evaluated path value (modified by func)
	// .select - selector on value
    // .store - storage (usually TOS) for constructed values
	// .setval - non-zero for SET-PATH (set to zero after SET is done)
	// .orig - original path for error messages
	switch (func(pvs)) {
	case PE_OK:
		break;
	case PE_SET: // only sets if end of path
		if (pvs->setval && IS_END(pvs->path+1)) {
			*pvs->value = *pvs->setval;
			pvs->setval = 0;
		}
		break;
	case PE_NONE:
		SET_NONE(pvs->store);
	case PE_USE:
		pvs->value = pvs->store;
		break;
	case PE_BAD_SELECT:
		Trap2(RE_INVALID_PATH, pvs->orig, pvs->path);
	case PE_BAD_SET:
		Trap2(RE_BAD_PATH_SET, pvs->orig, pvs->path);
	case PE_BAD_RANGE:
		Trap_Range(pvs->path);
	case PE_BAD_SET_TYPE:
		Trap2(RE_BAD_FIELD_SET, pvs->path, Of_Type(pvs->setval));
	}

	if (NOT_END(pvs->path+1)) Next_Path(pvs);
}


/***********************************************************************
**
*/	REBVAL *Do_Path(REBVAL **path_val, REBVAL *val)
/*
**		Evaluate a path value. Path_val is updated so
**		result can be used for function refinements.
**		If val is not zero, then this is a SET-PATH.
**		Returns value only if result is a function,
**		otherwise the result is on TOS.
**
***********************************************************************/
{
	REBPVS pvs;

	if (val && THROWN(val)) {
		// If unwind/throw value is not coming from TOS, push it.
		if (val != DS_TOP) DS_PUSH(val);
		return 0;
	}

	pvs.setval = val;		// Set to this new value
	DS_PUSH_NONE;
	pvs.store = DS_TOP;		// Temp space for constructed results

	// Get first block value:
	pvs.path = VAL_BLK_DATA(pvs.orig = *path_val);

	// Lookup the value of the variable:
	if (IS_WORD(pvs.path)) {
		pvs.value = Get_Var(pvs.path);
		if (IS_UNSET(pvs.value)) Trap1(RE_NO_VALUE, pvs.path);
	} else pvs.value = pvs.path; //Trap2(RE_INVALID_PATH, pvs.orig, pvs.path);

	// Start evaluation of path:
	if (Path_Dispatch[VAL_TYPE(pvs.value)]) {
		Next_Path(&pvs);
		// Check for errors:
		if (NOT_END(pvs.path+1) && !ANY_FUNC(pvs.value)) {
			// Only function refinements should get by this line:
			Trap2(RE_INVALID_PATH, pvs.orig, pvs.path);
		}
	}
	else if (NOT_END(pvs.path+1) && !ANY_FUNC(pvs.value))
		Trap2(RE_BAD_PATH_TYPE, pvs.orig, Of_Type(pvs.value));

	// If SET then we can drop result storage created above.
	if (val) {
		DS_DROP; // on SET, we do not care about returned value
		return 0;
	} else {
		//if (ANY_FUNC(pvs.value) && IS_GET_PATH(pvs.orig)) Debug_Fmt("FUNC %r %r", pvs.orig, pvs.path);
		// If TOS was not used, then copy final value back to it:
		if (pvs.value != pvs.store) *pvs.store = *pvs.value;
		// Return 0 if not function or is :path/word...
		if (!ANY_FUNC(pvs.value) || IS_GET_PATH(pvs.orig)) return 0;
		*path_val = pvs.path; // return new path (for func refinements)
		return pvs.value; // only used for functions
	}
}


/***********************************************************************
**
*/	void Pick_Path(REBVAL *value, REBVAL *selector, REBVAL *val)
/*
**		Lightweight version of Do_Path used for A_PICK actions.
**		Result on TOS.
**
***********************************************************************/
{
	REBPVS pvs;
	REBPEF func;

	pvs.value = value;
	pvs.path = 0;
	pvs.select = selector;
	pvs.setval = val;
	DS_PUSH_NONE;
	pvs.store = DS_TOP;		// Temp space for constructed results

	// Path must have dispatcher, else return:
	func = Path_Dispatch[VAL_TYPE(value)];
	if (!func) return; // unwind, then check for errors

	switch (func(&pvs)) {
	case PE_OK:
		break;
	case PE_SET: // only sets if end of path
		if (pvs.setval) *pvs.value = *pvs.setval;
		break;
	case PE_NONE:
		SET_NONE(pvs.store);
	case PE_USE:
		pvs.value = pvs.store;
		break;
	case PE_BAD_SELECT:
		Trap2(RE_INVALID_PATH, pvs.value, pvs.select);
	case PE_BAD_SET:
		Trap2(RE_BAD_PATH_SET, pvs.value, pvs.select);
		break;
	}
}


#ifdef removed_func
/***********************************************************************
**
x*/	static REBINT Do_Args_Light(REBVAL *func, REBVAL *path, REBSER *block, REBCNT index)
/*
**		Evaluate code block according to the function arg spec.
**		Args are pushed onto the data stack in the same order
**		as the function frame. Args not evaluated.
**
***********************************************************************/
{
	REBVAL *args;
	REBSER *words;
	REBINT ds = 0;			// stack argument position
	REBINT dsp = DSP + 1;	// stack base
	REBINT dsf = dsp - DSF_BIAS;
	REBVAL *tos;
	REBVAL *val;

	if ((dsp + 100) > (REBINT)SERIES_REST(DS_Series))
		Trap0(RE_STACK_OVERFLOW); //Expand_Stack();

	// If closure, get args from object context:
	words = VAL_FUNC_WORDS(func);
	//if (IS_CLOSURE(func)) words = FRM_WORD_SERIES(words);
	args = BLK_SKIP(words, 1);

	// Fill stack variables with default values:
	ds = SERIES_TAIL(words)-1;	// length of stack fill below
	tos = DS_NEXT;
	DSP += ds;
	for (; ds > 0; ds--) SET_NONE(tos++);

	// Go thru the word list args:
	ds = dsp;
	for (; NOT_END(args); args++, ds++) {

		if (index >= BLK_LEN(block)) {
			if (!IS_REFINEMENT(args))
				Trap2(RE_NO_ARG, Func_Word(dsf), args);
			break;
		}

		// Process each argument according to the argument block:
		switch (VAL_TYPE(args)) {
		case REB_WORD:
		case REB_LIT_WORD:
		case REB_GET_WORD:
			DS_Base[ds] = *BLK_SKIP(block, index);
			index++;
			break;
		case REB_REFINEMENT:
			val = BLK_SKIP(block, index);
			index++;
			if (IS_NONE(val) || IS_FALSE(val)) SET_NONE(&DS_Base[ds]);
			else if (IS_LOGIC(val) && VAL_LOGIC(val)) SET_TRUE(&DS_Base[ds]);
			else Trap1(RE_BAD_REFINE, args);
			break;
		}

		// If word words is typed, verify correct argument datatype:
		if (!TYPE_CHECK(args, VAL_TYPE(DS_VALUE(ds))))
			Trap3(RE_EXPECT_ARG, Func_Word(dsf), args, Of_Type(DS_VALUE(ds)));
	}

	if (path && NOT_END(path))
		Trap2(RE_NO_REFINE, Func_Word(dsf), path);

	return index;
}
#endif


/***********************************************************************
**
*/	static REBINT Do_Args(REBVAL *func, REBVAL *path, REBSER *block, REBCNT index)
/*
**		Evaluate code block according to the function arg spec.
**		Args are pushed onto the data stack in the same order
**		as the function frame.
**
**			func:  function or path value
**			path:  refinements or object/function path
**			block: current evaluation block
**			index: current evaluation index
**
***********************************************************************/
{
	REBVAL *value;
	REBVAL *args;
	REBSER *words;
	REBINT ds = 0;			// stack argument position
	REBINT dsp = DSP + 1;	// stack base
	REBINT dsf = dsp - DSF_BIAS;
	REBVAL *tos;

	if (IS_OP(func)) dsf--; // adjust for extra arg

	if ((dsp + 100) > (REBINT)SERIES_REST(DS_Series)) 
		Trap0(RE_STACK_OVERFLOW); //Expand_Stack();

	// Get list of words:
	words = VAL_FUNC_WORDS(func);
	args = BLK_SKIP(words, 1);
	ds = SERIES_TAIL(words)-1;	// length of stack fill below
	//Debug_Fmt("Args: %z", VAL_FUNC_ARGS(func));

	// If func is operator, first arg is already on stack:
	if (IS_OP(func)) {
		//if (!TYPE_CHECK(args, VAL_TYPE(DS_VALUE(DSP))))
		//	Trap3(RE_EXPECT_ARG, Func_Word(dsf), args, Of_Type(DS_VALUE(ds)));
		args++;	 	// skip evaluation, but continue with type check
		ds--;		// shorten stack fill below
	}

	// Fill stack variables with default values:
	tos = DS_NEXT;
	DSP += ds;
	for (; ds > 0; ds--) SET_NONE(tos++);

	// Go thru the word list args:
	ds = dsp;
	for (; NOT_END(args); args++, ds++) {

		//if (Trace_Flags) Trace_Arg(ds - dsp, args, path);

		// Process each formal argument:
		switch (VAL_TYPE(args)) {

		case REB_WORD:		// WORD - Evaluate next value
			index = Do_Next(block, index, IS_OP(func));
			// THROWN is handled after the switch.
			if (index == END_FLAG) Trap2(RE_NO_ARG, Func_Word(dsf), args);
			DS_Base[ds] = *DS_POP;
			break;

		case REB_LIT_WORD:	// 'WORD - Just get next value
			if (index < BLK_LEN(block)) {
				value = BLK_SKIP(block, index);
				if (IS_PAREN(value) || IS_GET_WORD(value) || IS_GET_PATH(value)) {
					index = Do_Next(block, index, IS_OP(func));
					// THROWN is handled after the switch.
					DS_Base[ds] = *DS_POP;
				}
				else {
					index++;
					DS_Base[ds] = *value;
				}
			} else
				SET_UNSET(&DS_Base[ds]); // allowed to be none
			break;

		case REB_GET_WORD:	// :WORD - Get value
			if (index < BLK_LEN(block)) {
				DS_Base[ds] = *BLK_SKIP(block, index);
				index++;
			} else
				SET_UNSET(&DS_Base[ds]); // allowed to be none
			break;
/*
				value = BLK_SKIP(block, index);
				index++;
				if (IS_WORD(value) && VAL_WORD_FRAME(value)) value = Get_Var(value);
				DS_Base[ds] = *value;
*/
		case REB_REFINEMENT: // /WORD - Function refinement
			if (!path || IS_END(path)) return index;
			if (IS_WORD(path)) {
				// Optimize, if the refinement is the next arg:
				if (SAME_SYM(path, args)) {
					SET_TRUE(DS_VALUE(ds)); // set refinement stack value true
					path++;				// remove processed refinement
					continue;
				}
				// Refinement out of sequence, resequence arg order:
more_path:
				ds = dsp;
				args = BLK_SKIP(words, 1);
				for (; NOT_END(args); args++, ds++) {
					if (IS_REFINEMENT(args) && VAL_WORD_CANON(args) == VAL_WORD_CANON(path)) {
						SET_TRUE(DS_VALUE(ds)); // set refinement stack value true
						path++;				// remove processed refinement
						break;
					}
				}
				// Was refinement found? If not, error:
				if (IS_END(args)) Trap2(RE_NO_REFINE, Func_Word(dsf), path);
				continue;
			}
			else Trap1(RE_BAD_REFINE, path);
			break;

		case REB_SET_WORD:	// WORD: - reserved for special features
		default:
			Trap_Arg(args);
		}

		if (THROWN(DS_VALUE(ds))) {
			// Store THROWN value in TOS, so that Do_Next can handle it.
			*DS_TOP = *DS_VALUE(ds);
			return index;
		}

		// If word is typed, verify correct argument datatype:
		if (!TYPE_CHECK(args, VAL_TYPE(DS_VALUE(ds))))
			Trap3(RE_EXPECT_ARG, Func_Word(dsf), args, Of_Type(DS_VALUE(ds)));
	}

	// Hack to process remaining path:
	if (path && NOT_END(path)) goto more_path;
	//	Trap2(RE_NO_REFINE, Func_Word(dsf), path);

	return index;
}


/***********************************************************************
**
*/	void Do_Signals(void)
/*
**		Special events to process during evaluation.
**		Search for SET_SIGNAL to find them.
**
***********************************************************************/
{
	REBCNT sigs;
	REBCNT mask;

	// Accumulate evaluation counter and reset countdown:
	if (Eval_Count <= 0) {
		//Debug_Num("Poll:", (REBINT) Eval_Cycles);
		Eval_Cycles += Eval_Dose - Eval_Count;
		Eval_Count = Eval_Dose;
		if (Eval_Limit != 0 && Eval_Cycles > Eval_Limit)
			Check_Security(SYM_EVAL, POL_EXEC, 0);
	}

	if (!(Eval_Signals & Eval_Sigmask)) return;

	// Be careful of signal loops! EG: do not PRINT from here.
	sigs = Eval_Signals & (mask = Eval_Sigmask);
	Eval_Sigmask = 0;	// avoid infinite loop
	//Debug_Num("Signals:", Eval_Signals);

	// Check for recycle signal:
	if (GET_FLAG(sigs, SIG_RECYCLE)) {
		CLR_SIGNAL(SIG_RECYCLE);
		Recycle();
	}

#ifdef NOT_USED_INVESTIGATE
	if (GET_FLAG(sigs, SIG_EVENT_PORT)) {  // !!! Why not used?
		CLR_SIGNAL(SIG_EVENT_PORT);
		Awake_Event_Port();
	}
#endif

	// Escape only allowed after MEZZ boot (no handlers):
	if (GET_FLAG(sigs, SIG_ESCAPE) && PG_Boot_Phase >= BOOT_MEZZ) {
		CLR_SIGNAL(SIG_ESCAPE);
		Eval_Sigmask = mask;
		Halt_Code(RE_HALT, 0); // Throws!
	}

	Eval_Sigmask = mask;
}


/***********************************************************************
**
*/	REBCNT Do_Next(REBSER *block, REBCNT index, REBFLG op)
/*
**		Evaluate the code block until we have:
**			1. An irreducible value (return next index)
**			2. Reached the end of the block (return END_FLAG)
**			3. Encountered an error
**
**		Index is a zero-based index into the block.
**		Op indicates infix operator is being evaluated (precedence);
**		The value (or error) is placed on top of the data stack.
**
***********************************************************************/
{
	REBVAL *value;
	REBVAL *word = 0;
	REBINT ftype;
	REBCNT dsf;

	//CHECK_MEMORY(1);
	CHECK_STACK(&value);
	if ((DSP + 20) > (REBINT)SERIES_REST(DS_Series)) Expand_Stack(STACK_MIN); //Trap0(RE_STACK_OVERFLOW);
	if (--Eval_Count <= 0 || Eval_Signals) Do_Signals();

	value = BLK_SKIP(block, index);
	//if (Trace_Flags) Trace_Eval(block, index);

reval:
	if (Trace_Flags) Trace_Line(block, index, value);

	//getchar();
	switch (EVAL_TYPE(value)) {

	case ET_WORD:
		value = Get_Var(word = value);
		if (IS_UNSET(value)) Trap1(RE_NO_VALUE, word);
		if (VAL_TYPE(value) >= REB_NATIVE && VAL_TYPE(value) <= REB_FUNCTION) goto reval; // || IS_LIT_PATH(value)
		DS_PUSH(value);
		if (IS_LIT_WORD(value)) VAL_SET(DS_TOP, REB_WORD);
		if (IS_FRAME(value)) Init_Obj_Value(DS_TOP, VAL_WORD_FRAME(word));
		index++;
		break;

	case ET_SELF:
		DS_PUSH(value);
		index++;
		break;

	case ET_SET_WORD:
		word = value;
		//if (!VAL_WORD_FRAME(word)) Trap1(RE_NOT_DEFINED, word); (checked in set_var)
		index = Do_Next(block, index+1, 0);
		// THROWN is handled in Set_Var.
		if (index == END_FLAG || VAL_TYPE(DS_TOP) <= REB_UNSET) Trap1(RE_NEED_VALUE, word);
		Set_Var(word, DS_TOP);
		//Set_Word(word, DS_TOP); // (value stays on stack)
		//Dump_Frame(Main_Frame);
		break;

	case ET_FUNCTION:
eval_func0:
		ftype = VAL_TYPE(value) - REB_NATIVE; // function type
		if (!word) word = ROOT_NONAME;
		dsf = Push_Func(FALSE, block, index, VAL_WORD_SYM(word), value);
eval_func:
		value = DSF_FUNC(dsf); // a safe copy of function
		if (VAL_TYPE(value) < REB_NATIVE) {
			Debug_Value(word, 4, 0);
			Dump_Values(value, 4);
		}
		index = Do_Args(value, 0, block, index+1); // uses old DSF, updates DSP
eval_func2:
		// Evaluate the function:
		DSF = dsf;	// Set new DSF
		if (!THROWN(DS_TOP)) {
			if (Trace_Flags) Trace_Func(word, value);
			Func_Dispatch[ftype](value);
		}
		else {
			*DS_RETURN = *DS_TOP;
		}

		// Reset the stack to prior function frame, but keep the
		// return value (function result) on the top of the stack.
		DSP = dsf;
		DSF = PRIOR_DSF(dsf);
		if (Trace_Flags) Trace_Return(word, DS_TOP);

		// The return value is a FUNC that needs to be re-evaluated.
		if (VAL_GET_OPT(DS_TOP, OPTS_REVAL) && ANY_FUNC(DS_TOP)) {
			value = DS_POP; // WARNING: value is volatile on TOS1 !
			word = Get_Type_Word(VAL_TYPE(value));
			index--;		// Backup block index to re-evaluate.
			if (IS_OP(value)) Trap_Type(value); // not allowed
			goto eval_func0;
		}
		break;

	case ET_OPERATOR:
		// An operator can be native or function, so its true evaluation
		// datatype is stored in the extended flags part of the value.
		if (!word) word = ROOT_NONAME;
		if (DSP <= 0 || index == 0) Trap1(RE_NO_OP_ARG, word);
		ftype = VAL_GET_EXT(value) - REB_NATIVE;
		dsf = Push_Func(TRUE, block, index, VAL_WORD_SYM(word), value); // TOS has first arg
		DS_PUSH(DS_VALUE(dsf)); // Copy prior to first argument
		goto eval_func;

	case ET_PATH:  // PATH, SET_PATH
		ftype = VAL_TYPE(value);
		word = value; // a path
		//index++; // now done below with +1

		//Debug_Fmt("t: %r", value);
		if (ftype == REB_SET_PATH) {
			index = Do_Next(block, index+1, 0);
			// THROWN is handled in Do_Path.
			if (index == END_FLAG || VAL_TYPE(DS_TOP) <= REB_UNSET) Trap1(RE_NEED_VALUE, word);
			Do_Path(&word, DS_TOP);
		} else {
			// Can be a path or get-path:
			value = Do_Path(&word, 0); // returns in word the path item, DS_TOP has value
			//Debug_Fmt("v: %r", value);
			// Value returned only for functions that need evaluation (but not GET_PATH):
			if (value && ANY_FUNC(value)) {
				if (IS_OP(value)) Trap_Type(value); // (because prior value is wiped out above)
				// Can be object/func or func/refinements or object/func/refinement:
				dsf = Push_Func(TRUE, block, index, VAL_WORD_SYM(word), value); // Do not unset TOS1 (it is the value)
				value = DS_TOP;
				index = Do_Args(value, word+1, block, index+1);
				ftype = VAL_TYPE(value)-REB_NATIVE;
				goto eval_func2;
			} else
				index++;
		}
		break;

	case ET_PAREN:
		DO_BLK(value);
		DSP++; // keep it on top
		index++;
		break;

	case ET_LIT_WORD:
		DS_PUSH(value);
		VAL_SET(DS_TOP, REB_WORD);
		index++;
		break;

	case ET_GET_WORD:
		DS_PUSH(Get_Var(value));
		index++;
		break;

	case ET_LIT_PATH:
		DS_PUSH(value);
		VAL_SET(DS_TOP, REB_PATH);
		index++;
		break;

	case ET_END:
		 return END_FLAG;

	default:
		//Debug_Fmt("Bad eval: %d %s", VAL_TYPE(value), Get_Type_Name(value));
		Crash(RP_BAD_EVALTYPE, VAL_TYPE(value));
		//return -index;
	}

	// If normal eval (not higher precedence of infix op), check for op:
	if (!op) {
		value = BLK_SKIP(block, index);
		if (IS_WORD(value) && VAL_WORD_FRAME(value) && IS_OP(Get_Var(value)))
			goto reval;
	}

	return index;
}


/***********************************************************************
**
*/	REBVAL *Do_Blk(REBSER *block, REBCNT index)
/*
**		Evaluate a block from the index position specified.
**		Return the result (a pointer to TOS+1).
**
***********************************************************************/
{
	REBVAL *tos = 0;
#if (ALEVEL>1)
	REBINT start = DSP;
//	REBCNT gcd = GC_Disabled;
#endif

	CHECK_MEMORY(4); // Be sure we don't go far with a problem.

	ASSERT1(block->info, RP_GC_OF_BLOCK);

	while (index < BLK_LEN(block)) {
		index = Do_Next(block, index, 0);
		tos = DS_POP;
		if (THROWN(tos)) break;
	}
	// If block was empty:
	if (!tos) {tos = DS_NEXT; SET_UNSET(tos);}

	if (start != DSP || tos != &DS_Base[start+1]) Trap0(RE_MISSING_ARG);

//	ASSERT2(gcd == GC_Disabled, RP_GC_STUCK);

	// Restore data stack and return value:
//	ASSERT2((tos == 0 || (start == DSP && tos == &DS_Base[start+1])), RP_TOS_DRIFT);
//	if (!tos) {tos = DS_NEXT; SET_UNSET(tos);}
	return tos;
}


/***********************************************************************
**
*/	REBVAL *Do_Block_Value_Throw(REBVAL *block)
/*
**		A common form of Do_Blk(). Takes block value. Handles throw.
**
***********************************************************************/
{
	REBSER *series = VAL_SERIES(block);
	REBCNT index = VAL_INDEX(block);
	REBVAL *tos = 0;
	REBINT start = DSP;

	while (index < BLK_LEN(series)) {
		index = Do_Next(series, index, 0);
		tos = DS_POP;
		if (THROWN(tos)) Throw_Break(tos);
	}
	// If series was empty:
	if (!tos) {tos = DS_NEXT; SET_UNSET(tos);}

	if (start != DSP || tos != &DS_Base[start+1]) Trap0(RE_MISSING_ARG);

	return tos;
}


/***********************************************************************
**
*/	REBFLG Try_Block(REBSER *block, REBCNT index)
/*
**		Evaluate a block from the index position specified in the value.
**		TOS+1 holds the result.
**
***********************************************************************/
{
	REBOL_STATE state;
	REBVAL *tos;

	PUSH_STATE(state, Saved_State);
	if (SET_JUMP(state)) {
		POP_STATE(state, Saved_State);
		Catch_Error(DS_NEXT); // Stores error value here
		return TRUE;
	}
	SET_STATE(state, Saved_State);

	tos = 0;
	while (index < BLK_LEN(block)) {
		index = Do_Next(block, index, 0);
		tos = DS_POP;
		if (THROWN(tos)) break;
	}
	if (!tos) {tos = DS_NEXT; SET_UNSET(tos);}

	// Restore data stack and return value at TOS+1:
	DS_Base[state.dsp+1] = *tos;
	POP_STATE(state, Saved_State);

	return FALSE;
}


/***********************************************************************
**
*/	void Reduce_Block(REBSER *block, REBCNT index, REBVAL *into)
/*
**		Reduce block from the index position specified in the value.
**		Collect all values from stack and make them a block.
**
***********************************************************************/
{
	REBINT start = DSP + 1;

	while (index < BLK_LEN(block)) {
		index = Do_Next(block, index, 0);
		if (THROWN(DS_TOP)) return;
	}

	Copy_Stack_Values(start, into);
}


/***********************************************************************
**
*/	void Reduce_Only(REBSER *block, REBCNT index, REBVAL *words, REBVAL *into)
/*
**		Reduce only words and paths not found in word list.
**
***********************************************************************/
{
	REBINT start = DSP + 1;
	REBVAL *val;
	REBVAL *v;
	REBSER *ser = 0;
	REBCNT idx = 0;

	if (IS_BLOCK(words)) {
		ser = VAL_SERIES(words);
		idx = VAL_INDEX(words);
	}

	for (val = BLK_SKIP(block, index); NOT_END(val); val++) {
		if (IS_WORD(val)) {
			// Check for keyword:
			if (ser && NOT_FOUND != Find_Word(ser, idx, VAL_WORD_CANON(val))) {
				DS_PUSH(val);
				continue;
			}
			v = Get_Var(val);
			DS_PUSH(v);
		}
		else if (IS_PATH(val)) {
			if (ser) {
				// Check for keyword/path:
				v = VAL_BLK_DATA(val);
				if (IS_WORD(v)) {
					if (NOT_FOUND != Find_Word(ser, idx, VAL_WORD_CANON(v))) {
						DS_PUSH(val);
						continue;
					}
				}
			}
			v = val;
			Do_Path(&v, 0); // pushes val on stack
		}
		else DS_PUSH(val);
		// No need to check for unwinds (THROWN) here, because unwinds should
		// never be accessible via words or paths.
	}

	Copy_Stack_Values(start, into);
}


/***********************************************************************
**
*/	void Reduce_Block_No_Set(REBSER *block, REBCNT index, REBVAL *into)
/*
***********************************************************************/
{
	REBINT start = DSP + 1;
	REBVAL *val;

	while (index < BLK_LEN(block)) {
		if (IS_SET_WORD(val = BLK_SKIP(block, index))) {
			DS_PUSH(val);
			index++;
		} else
			index = Do_Next(block, index, 0);
		if (THROWN(DS_TOP)) return;
	}

	Copy_Stack_Values(start, into);
}


/***********************************************************************
**
*/	void Reduce_Type_Stack(REBSER *block, REBCNT index, REBCNT type)
/*
**		Reduce a block of words/paths that are of the specified type.
**		Return them on the stack. The change in TOS is the length.
**
***********************************************************************/
{
	//REBINT start = DSP + 1;
	REBVAL *val;
	REBVAL *v;

	// Lookup words and paths and push values on stack:
	for (val = BLK_SKIP(block, index); NOT_END(val); val++) {
		if (IS_WORD(val)) {
			v = Get_Var(val);
			if (VAL_TYPE(v) == type) DS_PUSH(v);
		}
		else if (IS_PATH(val)) {
			v = val;
			if (!Do_Path(&v, 0)) { // pushes val on stack
				if (VAL_TYPE(DS_TOP) != type) DS_DROP;
			}
		}
		else if (VAL_TYPE(val) == type) DS_PUSH(val);
		// !!! check stack size
	}
	SET_END(&DS_Base[++DSP]); // in case caller needs it

	//block = Copy_Values(DS_Base + start, DSP - start + 1);
	//DSP = start;
	//return block;
}


/***********************************************************************
**
*/	void Reduce_In_Frame(REBSER *frame, REBVAL *values)
/*
**		Reduce a block with simple lookup in the context.
**		Only words in that context are valid (e.g. error object).
**		All values are left on the stack. No copy is made.
**
***********************************************************************/
{
	REBVAL *val;

	for (; NOT_END(values); values++) {
		switch (VAL_TYPE(values)) {
		case REB_WORD:
		case REB_SET_WORD:
		case REB_GET_WORD:
			if (NZ(val = Find_Word_Value(frame, VAL_WORD_SYM(values)))) {
				DS_PUSH(val);
				break;
			} // Unknown in context, fall below, use word as value.
		case REB_LIT_WORD:
			DS_PUSH(values);
			VAL_SET(DS_TOP, REB_WORD);
			break;
		default:
			DS_PUSH(values);
		}
	}
}


/***********************************************************************
**
*/	void Compose_Block(REBVAL *block, REBFLG deep, REBFLG only, REBVAL *into)
/*
**		Compose a block from a block of un-evaluated values and
**		paren blocks that are evaluated. Stack holds temp values,
**		which also protects them from GC along the way.
**
**			deep - recurse into sub-blocks
**			only - parens that return blocks are kept as blocks
**
**		Returns result as a block on top of stack.
**
***********************************************************************/
{
	REBVAL *value;
	REBINT start = DSP + 1;

	for (value = VAL_BLK_DATA(block); NOT_END(value); value++) {
		if (IS_PAREN(value)) {
			// Eval the paren, and leave result on the stack:
			DO_BLK(value);
			DSP++; // !!!DSP temp
			if (THROWN(DS_TOP)) return;

			// If result is a block, and not /only, insert its contents:
			if (IS_BLOCK(DS_TOP) && !only) {
				// Append series to the stack:
				SERIES_TAIL(DS_Series) = DSP; // overwrites TOP value
				Append_Series(DS_Series, (REBYTE *)VAL_BLK_DATA(DS_TOP), VAL_BLK_LEN(DS_TOP));
				DSP = SERIES_TAIL(DS_Series) - 1;
				// Note: stack may have moved
			}
			else if (IS_UNSET(DS_TOP)) DS_DROP; // remove unset values
		}
		else if (deep) {
			if (IS_BLOCK(value)) Compose_Block(value, TRUE, only, 0);
			else {
				DS_PUSH(value);
				if (ANY_BLOCK(value)) // Include PATHS
					VAL_SERIES(DS_TOP) = Copy_Block(VAL_SERIES(value), 0);
			}
		}
		else DS_PUSH(value);
	}

	Copy_Stack_Values(start, into);
}


/***********************************************************************
**
*/	void Apply_Block(REBVAL *func, REBVAL *args, REBFLG reduce)
/*
**		Result is on top of stack.
**
***********************************************************************/
{
	REBINT ftype = VAL_TYPE(func) - REB_NATIVE; // function type
	REBSER *block = VAL_SERIES(args);
	REBCNT index = VAL_INDEX(args);
	REBCNT dsf;

	REBSER *words;
	REBINT len;
	REBINT n;
	REBINT start;
	REBVAL *val;

	if (index > SERIES_TAIL(block)) index = SERIES_TAIL(block);

	// Push function frame:
	dsf = Push_Func(0, block, index, 0, func);
	func = DSF_FUNC(dsf); // for safety

	// Determine total number of args:
	words = VAL_FUNC_WORDS(func);
	len = words ? SERIES_TAIL(words)-1 : 0;
	start = DSP+1;

	// Gather arguments:
	if (reduce) {
		// Reduce block contents to stack:
		n = 0;
		while (index < BLK_LEN(block)) {
			index = Do_Next(block, index, 0);
			if (THROWN(DS_TOP)) return;
			n++;
		}
		if (n > len) DSP = start + len;
	}
	else {
		// Copy block contents to stack:
		n = VAL_BLK_LEN(args);
		if (len < n) n = len;
		memcpy(&DS_Base[start], BLK_SKIP(block, index), n * sizeof(REBVAL));
		DSP = start + n - 1;
	}

	// Pad out missing args:
	for (; n < len; n++) DS_PUSH_NONE;

	// Validate arguments:
	if (words) {
		val = DS_Base + start;
		for (args = BLK_SKIP(words, 1); NOT_END(args);) {
			// If arg is refinement, determine its state:
			if (IS_REFINEMENT(args)) {
				if (IS_FALSE(val)) {
					SET_NONE(val);  // ++ ok for none
					while (TRUE) {
						val++;
						args++;
						if (IS_END(args) || IS_REFINEMENT(args)) break;
						SET_NONE(val);
					}
					continue;
				}
				SET_TRUE(val);
			}
			// If arg is typed, verify correct argument datatype:
			if (!TYPE_CHECK(args, VAL_TYPE(val)))
				Trap3(RE_EXPECT_ARG, Func_Word(dsf), args, Of_Type(val));
			args++;
			val++;
		}
	}

	// Evaluate the function:
	DSF = dsf;
	Func_Dispatch[ftype](func);
	DSP = dsf;
	DSF = PRIOR_DSF(dsf);
}


/***********************************************************************
**
*/	REBVAL *Apply_Function(REBSER *wblk, REBCNT widx, REBVAL *func, va_list args)
/*
**		Applies function from args provided by C call. Zero terminated.
**		Result is EXTREMELY VOLATILE - a stack value above the DSP.
**
**		wblk - where block (where we were called)
**		widx - where index (position in above block)
**		func - function to call
**		args - list of function args (null terminated)
**
***********************************************************************/
{
	REBCNT dsf;
	REBSER *words;
	REBCNT ds;
	REBVAL *arg;

	dsf = Push_Func(0, wblk, widx, 0, func);
	func = DSF_FUNC(dsf); // for safety
	words = VAL_FUNC_WORDS(func);
	ds = SERIES_TAIL(words)-1;	// length of stack fill below

	// Gather arguments from C stack:
	for (; ds > 0; ds--) {
		arg = va_arg(args, REBVAL*); // get value
		if (arg) DS_PUSH(arg);  // push it; no type check
		else break;
	}
	for (; ds > 0; ds--) DS_PUSH_NONE; // unused slots

	// Evaluate the function:
	DSF = dsf;
	Func_Dispatch[VAL_TYPE(func) - REB_NATIVE](func);
	DSF = PRIOR_DSF(dsf);
	DSP = dsf-1;

	// Return resulting value from TOS1. But note:
	// EXTREMELY VOLATILE - use or copy quickly
	// before next evaluation, GC, or anything else!
	return DS_VALUE(dsf);
}


/***********************************************************************
**
*/	REBVAL *Apply_Func(REBSER *where, REBVAL *func, ...)
/*
**		Applies function from args provided by C call. Zero terminated.
**		Result is EXTREMELY VOLATILE - a stack value above the DSP.
**
***********************************************************************/
{
	REBVAL *value;
	va_list args;

	if (!ANY_FUNC(func)) Trap_Arg(func);
	if (!where) where = VAL_FUNC_BODY(func); // something/anything ?!!

	va_start(args, func);
	value = Apply_Function(where, 0, func, args);
	va_end(args);

	return value;
}


/***********************************************************************
**
*/	REBVAL *Do_Sys_Func(REBCNT inum, ...)
/*
**		Evaluates a SYS function and TOS1 contains
**		the result (VOLATILE). Uses current stack frame location
**		as the next location (e.g. for error output).
**
***********************************************************************/
{
	REBVAL *value;
	va_list args;
	REBSER *blk = 0;
	REBCNT idx = 0;

	if (DSF) {
		value = DSF_BACK(DSF);
		blk = VAL_SERIES(value);
		idx = VAL_INDEX(value);
	}

	value = FRM_VALUE(Sys_Context, inum);
	if (!ANY_FUNC(value)) Trap1(RE_BAD_SYS_FUNC, value);
	if (!DSF) blk = VAL_FUNC_BODY(value);

	va_start(args, inum);
	value = Apply_Function(blk, idx, value, args);
	va_end(args);

	return value;
}


/***********************************************************************
**
*/	void Do_Construct(REBVAL *value)
/*
**		Do a block with minimal evaluation and no evaluation of
**		functions. Used for things like script headers where security
**		is important.
**
**		Handles cascading set words:  word1: word2: value
**
***********************************************************************/
{
	REBVAL *temp;
	REBINT ssp;  // starting stack pointer

	DS_PUSH_NONE;
	temp = DS_TOP;
	ssp = DSP;

	for (; NOT_END(value); value++) {
		if (IS_SET_WORD(value)) {
			// Next line not needed, because SET words are ALWAYS in frame.
			//if (VAL_WORD_INDEX(value) > 0 && VAL_WORD_FRAME(value) == frame)
				DS_PUSH(value);
		} else {
			// Get value:
			if (IS_WORD(value)) {
				switch (VAL_WORD_CANON(value)) {
				case SYM_NONE:
					SET_NONE(temp);
					break;
				case SYM_TRUE:
				case SYM_ON:
				case SYM_YES:
					SET_TRUE(temp);
					break;
				case SYM_FALSE:
				case SYM_OFF:
				case SYM_NO:
					SET_FALSE(temp);
					break;
				default:
					*temp = *value;
					VAL_SET(temp, REB_WORD);
				}
			}
			else if (IS_LIT_WORD(value)) {
				*temp = *value;
				VAL_SET(temp, REB_WORD);
			}
			else if (IS_LIT_PATH(value)) {
				*temp = *value;
				VAL_SET(temp, REB_PATH);
			}
			else if (VAL_TYPE(value) >= REB_NONE) { // all valid values
				*temp = *value;
			}
			else
				SET_NONE(temp);

			// Set prior set-words:
			while (DSP > ssp) {
				Set_Var(DS_TOP, temp);
				DS_DROP;
			}
		}
	}
	DS_DROP; // temp
}


/***********************************************************************
**
*/	void Do_Min_Construct(REBVAL *value)
/*
**		Do no evaluation of the set values.
**
***********************************************************************/
{
	REBVAL *temp;
	REBINT ssp;  // starting stack pointer

	DS_PUSH_NONE;
	temp = DS_TOP;
	ssp = DSP;

	for (; NOT_END(value); value++) {
		if (IS_SET_WORD(value)) {
			// Next line not needed, because SET words are ALWAYS in frame.
			//if (VAL_WORD_INDEX(value) > 0 && VAL_WORD_FRAME(value) == frame)
				DS_PUSH(value);
		} else {
			// Get value:
			*temp = *value;
			// Set prior set-words:
			while (DSP > ssp) {
				Set_Var(DS_TOP, temp);
				DS_DROP;
			}
		}
	}
	DS_DROP; // temp
}


/***********************************************************************
**
*/	REBVAL *Do_Bind_Block(REBSER *frame, REBVAL *block)
/*
**		Bind deep and evaluate a block value in a given context.
**		Result is left on top of data stack (may be an error).
**
***********************************************************************/
{
	Bind_Block(frame, VAL_BLK_DATA(block), BIND_DEEP);
	return DO_BLK(block);
}


/***********************************************************************
**
*/	void Reduce_Bind_Block(REBSER *frame, REBVAL *block, REBCNT binding)
/*
**		Bind deep and reduce a block value in a given context.
**		Result is left on top of data stack (may be an error).
**
***********************************************************************/
{
	Bind_Block(frame, VAL_BLK_DATA(block), binding);
	Reduce_Block(VAL_SERIES(block), VAL_INDEX(block), 0);
}


/***********************************************************************
**
*/	REBOOL Try_Block_Halt(REBSER *block, REBCNT index)
/*
**		Evaluate a block from the index position specified in the value,
**		with a handler for quit conditions (QUIT, HALT) set up.
**
***********************************************************************/
{
	REBOL_STATE state;
	REBVAL *val;
//	static D = 0;
//	int depth = D++;

//	Debug_Fmt("Set Halt %d", depth);

	PUSH_STATE(state, Halt_State);
	if (SET_JUMP(state)) {
//		Debug_Fmt("Throw Halt %d", depth);
		POP_STATE(state, Halt_State);
		Catch_Error(DS_NEXT); // Stores error value here
		return TRUE;
	}
	SET_STATE(state, Halt_State);

	SAVE_SERIES(block);
	val = Do_Blk(block, index);
	UNSAVE_SERIES(block);

	DS_Base[state.dsp+1] = *val;
	POP_STATE(state, Halt_State);

//	Debug_Fmt("Ret Halt %d", depth);

	return FALSE;
}


/***********************************************************************
**
*/	REBVAL *Do_String(REBYTE *text, REBCNT flags)
/*
**		Do a string. Convert it to code, then evaluate it with
**		the ability to catch errors and also alow HALT if needed.
**
***********************************************************************/
{
	REBOL_STATE state;
	REBSER *code;
	REBVAL *val;
	REBSER *rc;
	REBCNT len;
	REBVAL vali;

	PUSH_STATE(state, Halt_State);
	if (SET_JUMP(state)) {
		POP_STATE(state, Halt_State);
		Saved_State = Halt_State;
		Catch_Error(DS_NEXT); // Stores error value here
		val = Get_System(SYS_STATE, STATE_LAST_ERROR); // Save it for EXPLAIN
		*val = *DS_NEXT;
		if (VAL_ERR_NUM(val) == RE_QUIT) {
			OS_EXIT(VAL_INT32(VAL_ERR_VALUE(DS_NEXT))); // console quit
		}
		return val;
	}
	SET_STATE(state, Halt_State);
	// Use this handler for both, halt conditions (QUIT, HALT) and error
	// conditions. As this is a top-level handler, simply overwriting
	// Saved_State is safe.
	Saved_State = Halt_State;

	code = Scan_Source(text, LEN_BYTES(text));
	SAVE_SERIES(code);

	// Bind into lib or user spaces?
	if (flags) {
		// Top words will be added to lib:
		Bind_Block(Lib_Context, BLK_HEAD(code), BIND_SET);
		Bind_Block(Lib_Context, BLK_HEAD(code), BIND_DEEP);
	}
	else {
		rc = VAL_OBJ_FRAME(Get_System(SYS_CONTEXTS, CTX_USER));
		len = rc->tail;
		Bind_Block(rc, BLK_HEAD(code), BIND_ALL | BIND_DEEP);
		SET_INTEGER(&vali, len);
		Resolve_Context(rc, Lib_Context, &vali, FALSE, 0);
	}

	Do_Blk(code, 0);
	UNSAVE_SERIES(code);

	POP_STATE(state, Halt_State);
	Saved_State = Halt_State;

	return DS_NEXT; // result is volatile
}


/***********************************************************************
**
*/	void Halt_Code(REBINT kind, REBVAL *arg)
/*
**		Halts execution by throwing back to the above Do_String.
**		Kind is RE_HALT or RE_QUIT
**		Arg is the optional return value.
**
**		Future versions may not reset the stack, but leave it as is
**		to allow for examination and a RESUME operation.
**
***********************************************************************/
{
	REBVAL *err = TASK_THIS_ERROR;

	if (!Halt_State) return;

	if (arg) {
		if (IS_NONE(arg)) {
			SET_INTEGER(TASK_THIS_VALUE, 0);
		} else 
			*TASK_THIS_VALUE = *arg;	// save the value
	} else {
		SET_NONE(TASK_THIS_VALUE);
	}

	VAL_SET(err, REB_ERROR);
	VAL_ERR_NUM(err) = kind;
	VAL_ERR_VALUE(err) = TASK_THIS_VALUE;
	VAL_ERR_SYM(err) = 0;

	longjmp(*Halt_State, 1);
}


/***********************************************************************
**
*/	void Call_Func(REBVAL *func_val)
/*
**		Calls a REBOL function from C code.
**
**	Setup:
**		Before calling this, the caller must setup the stack and
**		provide the function arguments on the stack. Any missing
**		args will be set to NONE.
**
**	Return:
**		On return, the stack remains as-is. The caller must reset
**		the DSP and DSF values.
**
***********************************************************************/
{
	REBINT n;

	// Caller must: Prep_Func + Args above
	VAL_WORD_FRAME(DSF_WORD(DSF)) = VAL_FUNC_ARGS(func_val);
	n = DS_ARGC - (SERIES_TAIL(VAL_FUNC_WORDS(func_val)) - 1);
	for (; n > 0; n--) DS_PUSH_NONE;
	Func_Dispatch[VAL_TYPE(func_val)-REB_NATIVE](func_val);
	// Caller must: pop stack back
}


/***********************************************************************
**
*/	void Redo_Func(REBVAL *func_val)
/*
**		Trampoline a function, restacking arguments as needed.
**
**	Setup:
**		The source for arguments is the existing stack frame,
**		or a prior stack frame. (Prep_Func + Args)
**
**	Return:
**		On return, the stack remains as-is. The caller must reset
**		the DSP and DSF values.
**
***********************************************************************/
{
	REBSER *wsrc;		// words of source func
	REBSER *wnew;		// words of target func
	REBCNT isrc;		// index position in source frame
	REBCNT inew;		// index position in target frame
	REBVAL *word;
	REBVAL *word2;

	//!!! NEEDS to check stack for overflow
	//!!! Should check datatypes for new arg passing!

	wsrc = VAL_FUNC_WORDS(DSF_FUNC(DSF));
	wnew = VAL_FUNC_WORDS(func_val);

	// Foreach arg of the target, copy to source until refinement.
	for (isrc = inew = 1; inew < BLK_LEN(wnew); inew++, isrc++) {
		word = BLK_SKIP(wnew, inew);
		if (isrc > BLK_LEN(wsrc)) isrc = BLK_LEN(wsrc);

		switch (VAL_TYPE(word)) {
			case REB_WORD:
			case REB_LIT_WORD:
			case REB_GET_WORD:
				if (VAL_TYPE(word) == VAL_TYPE(BLK_SKIP(wsrc, isrc))) break;
				DS_PUSH_NONE;
				continue;
				//Trap_Arg(word);

			// At refinement, search for it in source, then continue with words.
			case REB_REFINEMENT:
				// Are we aligned on the refinement already? (a common case)
				word2 = BLK_SKIP(wsrc, isrc);
				if (!(IS_REFINEMENT(word2) && VAL_BIND_CANON(word2) == VAL_BIND_CANON(word))) {
					// No, we need to search for it:
					for (isrc = 1; isrc < BLK_LEN(wsrc); isrc++) {
						word2 = BLK_SKIP(wsrc, isrc);
						if (IS_REFINEMENT(word2) && VAL_BIND_CANON(word2) == VAL_BIND_CANON(word)) goto push_arg;
					}
					DS_PUSH_NONE;
					continue;
					//if (isrc >= BLK_LEN(wsrc)) Trap_Arg(word);
				}
				break;

			default:
				ASSERT1(FALSE, RP_ASSERTS);
		}
push_arg:
		DS_PUSH(DSF_ARGS(DSF, isrc));
		//Debug_Fmt("Arg %d -> %d", isrc, inew);
	}

	// Copy values to prior location:
	inew--;
	// memory areas may overlap, so use memmove and not memcpy!
	memmove(DS_ARG(1), DS_TOP-(inew-1), inew * sizeof(REBVAL));
	DSP = DS_ARG_BASE + inew; // new TOS
	//Dump_Block(DS_ARG(1), inew);
	VAL_WORD_FRAME(DSF_WORD(DSF)) = VAL_FUNC_ARGS(func_val);
	*DSF_FUNC(DSF) = *func_val;
	Func_Dispatch[VAL_TYPE(func_val)-REB_NATIVE](func_val);
}


/***********************************************************************
**
*/	REBVAL *Get_Simple_Value(REBVAL *val)
/*
**		Does easy lookup, else just returns the value as is.
**		Note for paths value is left on stack.
**
***********************************************************************/
{
	if (IS_WORD(val) || IS_GET_WORD(val))
		val = Get_Var(val);
	else if (IS_PATH(val) || IS_GET_PATH(val)) { //val = Get_Path_Var(val);
		REBVAL *v = val;
		DS_PUSH_NONE;
		Do_Path(&v, 0);
		val = DS_TOP;
	}

	return val;
}


/***********************************************************************
**
*/	REBSER *Resolve_Path(REBVAL *path, REBCNT *index)
/*
**		Given a path, return a context and index for its terminal.
**
***********************************************************************/
{
	REBVAL *sel; // selector
	REBVAL *val;
	REBSER *blk;
	REBCNT i;

	if (VAL_TAIL(path) < 2) return 0;
	blk = VAL_SERIES(path);
	sel = BLK_HEAD(blk);
	if (!ANY_WORD(sel)) return 0;
	val = Get_Var(sel);

	sel = BLK_SKIP(blk, 1);
	while (TRUE) {
		if (!ANY_OBJECT(val) || !IS_WORD(sel)) return 0;
		i = Find_Word_Index(VAL_OBJ_FRAME(val), VAL_WORD_SYM(sel), FALSE);
		sel++;
		if (IS_END(sel)) {
			*index = i;
			return VAL_OBJ_FRAME(val);
		}
	}

	return 0; // never happens
}


#ifdef obsolete
/***********************************************************************
**
xx*/	REBVAL *Call_Action(REBVAL *word, REBVAL *data, REBCNT act, REBSER *blk, REBCNT i)
/*
**		Calls datatype action with a value and argument.
**		Value is saved on top of stack.
**
***********************************************************************/
{
	REBVAL *ds;

	DSF = Push_Func(TRUE, blk, i, VAL_WORD_SYM(word), 0);
	DS_RELOAD(ds);
	*D_ARG(1) = *data;
	*D_ARG(2) = *BLK_SKIP(blk, i);
	Do_Act(D_RET, VAL_TYPE(data), act); // (properly handles returns)
	DSP = DSF;
	DSF = VAL_BACK(DS_NEXT);
	return DS_TOP;
}


/***********************************************************************
**
xx*/	REBVAL *Get_Path_Var(REBVAL *path)
/*
**		Leaves result on top of stack.
**
***********************************************************************/
{
	REBVAL *word;
	REBVAL *val;
	REBSER *blk;
	REBCNT i;

	if (VAL_TAIL(path) < 2) Trap1(RE_INVALID_PATH, path);	// empty path

	blk = VAL_SERIES(path);
	word = BLK_HEAD(blk);
	if (!ANY_WORD(word)) Trap1(RE_INVALID_PATH, path);

	val = Get_Var(word);

	for (i = 1; i < blk->tail; i++) {
		val = Call_Action(word, val, A_PATH, blk, i); // result is on TOS
	}

	return val;
}


/***********************************************************************
**
x*/	REBVAL *Do_Path(REBVAL **ppath, REBSER *block, REBCNT *index)
/*
**		Evaluate a path.
**
***********************************************************************/
{
	REBVAL *path = *ppath;
	REBVAL *orig = *ppath;
	REBVAL *value;
	REBVAL *selector;
	REBINT dsp;
	REBINT act;
	REBVAL *ds;

	if (VAL_TAIL(path) == 0) Trap1(RE_INVALID_PATH, path);	// empty path
	path = *ppath = VAL_BLK_DATA(path);
	if (!ANY_WORD(path)) Trap1(RE_INVALID_PATH, orig);
	value = Get_Var(path);
	if (IS_UNSET(value)) Trap1(RE_NO_VALUE, path);
	if (ANY_FUNC(value)) {
		DS_PUSH(value);
		value = DS_TOP;
		return value;
	}

	// Call the PATH action for the datatype.
	if (!index) act = 0, index = &act; // Fudge a reference
	DSF = Push_Func(FALSE, block ? block : VAL_SERIES(*ppath), *index, VAL_WORD_SYM(path), 0);
	DS_PUSH_NONE;				// Path dispatch value (from earlier evaluation)
	DS_PUSH_NONE;				// Argument to A_PATH action
	dsp = DSP;

	// Note: above, the backtrace word should not be bound, or it could
	// cause Get_Var() to use the wrong frame for relative values. So,
	// to avoid that, we remove the frame from that word.

	for (path++; NOT_END(path); path++) {

		DSP = dsp;	// Be sure stack does not grow
		act = A_PATH;

		// word/:field case
		if (IS_GET_WORD(path)) {
			// !!! need to add word/:field: case
			selector = Get_Var(path); // The object/:word case
		}
		// word/field: case
		else if (IS_SET_PATH(orig) && IS_END(path+1)) {
			if (!block) break; // evaluating singular path (e.g. in PARSE)
			//if (!IS_END(path+1)) Trap1(RE_INVALID_PATH, orig);
			selector = path;
			act = A_PATH_SET;
			*index = Do_Next(block, *index, 0);
			if (IS_UNSET(DS_TOP)) Trap1(RE_NEED_VALUE, orig);
			if (THROWN(DS_TOP)) {
				value = DS_TOP; // stop, return !!! protected?
				break;
			}
			// Leave value on top of stack for PATH SET.
		}
		// word/(expr) case
		else if (IS_PAREN(path)) {
			if (!block) Trap1(RE_NEED_VALUE, orig);  // !!! not correct error
			selector = DO_BLK(path);
		}
		// The object/word and object/value default case:
		else selector = path;

		// Special case for BLOCK access:
		DS_RELOAD(ds);
		if (ANY_BLOCK(value)) {
			value = Pick_Block(value, selector);
			if (!value) {
				if (act == A_PATH_SET) Trap_Range(selector);
				SET_NONE(D_RET);
				value = D_RET;
			}
			if (act == A_PATH_SET) {
				*D_RET = *value = *D_ARG(3);  // !!!! must check PROTECT flag!!!!!!!!!!
				value = D_RET;
			}
		}
		else {
			// Call the A_PATH action for the given value:
			*D_ARG(1) = *value;
			*D_ARG(2) = *selector;
			// D_ARG(3) too when PATH SET.
			Do_Act(ds, VAL_TYPE(value), act); // (properly handles returns)
			DS_RELOAD(ds);
			// All scalars must be written back to their storage areas:
			if (IS_SET_PATH(orig) && (IS_SCALAR(value) || IS_EVENT(value))) { //act == A_PATH_SET
				*value = *D_RET;  // !!!! must check PROTECT flag!!!!!!!!!!
				*D_RET = *D_ARG(3); // return the value of the set
			}
			value = D_RET;	// not GC protected after stack restore
		}
		if (ANY_FUNC(value)) break;	// evaluate
	}

	//ASSERT(DSF == dsf);
	DSP = DSF;
	DSF = VAL_BACK(DS_NEXT);

	*ppath = path;
	*DS_TOP = *value; // should not be needed!
	return value;
}

/***********************************************************************
**
xx*/	REBVAL *Do_Path(REBVAL **path_val, REBVAL *val)
/*
**		Evaluate a path value.
**		If val is not zero, set path to that new value.
**
**		Args to PD_* func:
**			Data: refernce value - can be modified
**			Sel: any value
**			Val: 0 or temp value on stack
**
**		Stack:
**			TOS-1: val (or not used)
**			TOS: none
**
**		Call the PD_function
**
**		Result code:
**			tos (tos has value)
**			none
**			error - invalid path
**
***********************************************************************/
{
	REBVAL *orig = *path_val;
	REBVAL *path;
	REBVAL *value;
	REBVAL *selector;
	REBPEF func;

	// Get first block value:
	*path_val = path = VAL_BLK_DATA(orig);
	if (!ANY_WORD(path)) Trap1(RE_INVALID_PATH, orig, path);

	// Lookup the value of the variable:
	value = Get_Var(path);
	if (IS_UNSET(value)) Trap1(RE_NO_VALUE, path);
	*DS_TOP = *value;

	// Foreach value in path:
	while (TRUE) {

		// It is a function, return now:
		if (ANY_FUNC(value)) {
			if (IS_GET_PATH(orig)) return 0;
			*path_val = path; // return the function name
			return DS_TOP;
		}

		if (IS_END(++path)) return 0;

		// object/:field case:
		if (IS_GET_WORD(path)) {
			selector = Get_Var(path);
			if (IS_UNSET(selector)) Trap1(RE_NO_VALUE, path);
		}
		// object/(expr) case:
		else if (IS_PAREN(path)) {
			selector = Do_Blk(VAL_SERIES(path), 0);
		}
		else // object/word and object/value case:
			selector = path;

		func = Path_Dispatch[VAL_TYPE(value)];
		if (!func) Trap1(RE_BAD_PATH, orig);

		if (NOT_END(path+1)) {
			// Call datatype value with the given selector.
			// It may return an object to continue.
			value = func(value, selector, 0); // can modify DS_TOP
			if (!value) Trap1(RE_INVALID_PATH, orig, path);
		} else {
			// This is the last field of the path. If val is
			// given then the func sets the value of the field.
			value = func(value, selector, val); // val can be zero
			if (!value) break; // done (field was set)
			*DS_TOP = *value; 
			if (!ANY_FUNC(value)) break; // done
			// Now, use code at top of loop for FUNC
		}
	}

	return 0;
}

#endif

/***********************************************************************
**
*/	REBINT Init_Mezz(REBINT reserved)
/*
***********************************************************************/
{
	REBINT result = 0;
	//REBVAL *val;
	REBOL_STATE state;
	REBVAL *val;
	int MERGE_WITH_Do_String;
//	static D = 0;
//	int depth = D++;

	//Debug_Fmt("Set Halt");

	if (PG_Boot_Level >= BOOT_LEVEL_MODS) {

		PUSH_STATE(state, Halt_State);
		if (SET_JUMP(state)) {
			//Debug_Fmt("Throw Halt");
			POP_STATE(state, Halt_State);
			Saved_State = Halt_State;
			Catch_Error(val = DS_NEXT); // Stores error value here
			if (IS_ERROR(val)) { // (what else could it be?)
				val = Get_System(SYS_STATE, STATE_LAST_ERROR); // Save it for EXPLAIN
				*val = *DS_NEXT;
				if (VAL_ERR_NUM(val) == RE_QUIT) {
					//Debug_Fmt("Quit(init)");
					OS_EXIT(VAL_INT32(VAL_ERR_VALUE(val))); // console quit
				}
				if (VAL_ERR_NUM(val) >= RE_THROW_MAX)
					Print_Value(val, 1000, FALSE);
			}
			return -1;
		}
		SET_STATE(state, Halt_State);
		// Use this handler for both, halt conditions (QUIT, HALT) and error
		// conditions. As this is a top-level handler, simply overwriting
		// Saved_State is safe.
		Saved_State = Halt_State;

		val = Do_Sys_Func(SYS_CTX_START, 0); // what if script contains a HALT?

		if (IS_INTEGER(val)) result = VAL_INT32(val);
		//if (Try_Block_Halt(VAL_SERIES(ROOT_SCRIPT), 0)) {

		//DS_Base[state.dsp+1] = *val;
		POP_STATE(state, Halt_State);
		Saved_State = Halt_State;
	}

	// Cleanup stack and memory:
	DS_RESET;
	Recycle();
	return 0; //result;
}
