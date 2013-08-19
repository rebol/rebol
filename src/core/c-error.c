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
**  Module:  c-error.c
**  Summary: error handling
**  Section: core
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
/*
		The Trap() functions are used for errors within the C code.

		TrapN() provides simple trampoline to the var-arg Make_Error()
		that constructs a new error object.

		The Make_Error function uses the error category objects to
		convert from an error code (integer) to error words and strings.
		Other important state information such as location of error
		and function context are also saved at this point.

		Throw_Error is called to throw the error back to a prior catch.
		A catch is defined using a set of C-macros. During the throw
		the error object is stored in a global: This_Error (because we
		cannot be sure that the longjmp value is able to hold a pointer
		on 64bit CPUs.)

		On the catch side, the Catch_Error function takes the error
		object and stores it into the value provided (normally on the
		DStack).

		Catch_Error can be extended to provide a debugging breakpoint
		for examining the call trace and context frames on the stack.
*/
/*

	Error Handling

	Errors occur in two places:

		1. evaluation of natives and actions
		2. evaluation of a block

	When an error occurs, an error object is built and thrown back to
	the nearest prior catch function. The catch is a longjmp that was
	set by a TRY or similar native.  At that point the interpreter stack
	can be either examined (for debugging) or restored to the current
	catch state.

	The error is returned from the catch as a disarmed error object. At
	that point, the error can be passed around and accessed as a normal
	object (although its datatype is ERROR!). The DISARM function
	becomes unnecessary and will simply copy the fields to a normal
	OBJECT! type.

	Using the new CAUSE native with the error object will re-activate
	the error and throw the error back further to the prior catch.

	The error object will include a new TRACE field that provides a back
	trace of the interpreter stack. This is a block of block pointers
	and may be clipped at some reasonable size (perhaps 10).

	When C code hits an error condition, it calls Trap(id, arg1, arg2, ...).
	This function takes a variable number of arguments.

	BREAK and RETURN

	TRY/RECOVER/EXCEPT.

		try [block]
		try/recover [block] [block]

	TRACE f1, :path/f1, or [f1 f2 f3]
	foo: func [[trace] ...]

*/


#include "sys-core.h"

#include "sys-state.h"

// Globals or Threaded???
static REBOL_STATE Top_State; // Boot var: holds error state during boot


/***********************************************************************
**
*/	void Check_Stack(void)
/*
***********************************************************************/
{
	if ((DSP + 100) > (REBINT)SERIES_REST(DS_Series))
		Trap0(RE_STACK_OVERFLOW);
}


/***********************************************************************
**
*/	void Catch_Error(REBVAL *value)
/*
**		Gets the current error and stores it as a value.
**		Normally the value is on the stack and is returned.
**
***********************************************************************/
{
	if (IS_NONE(TASK_THIS_ERROR)) Crash(RP_ERROR_CATCH);
	*value = *TASK_THIS_ERROR;
//	Print("CE: %r", value);
	SET_NONE(TASK_THIS_ERROR);
	//!!! Reset or ENABLE_GC;
}


/***********************************************************************
**
*/	void Throw_Error(REBSER *err)
/*
**		Throw the C stack.
**
***********************************************************************/
{
	if (!Saved_State) Crash(RP_NO_SAVED_STATE);
	SET_ERROR(TASK_THIS_ERROR, ERR_NUM(err), err);
	if (Trace_Level) Trace_Error(TASK_THIS_ERROR);
	longjmp(*Saved_State, 1);
}


/***********************************************************************
**
*/	void Throw_Break(REBVAL *val)
/*
**		Throw a break or return style error (for special cases
**		where we do not want to unwind the stack).
**
***********************************************************************/
{
	if (!Saved_State) Crash(RP_NO_SAVED_STATE);
	*TASK_THIS_ERROR = *val;
	longjmp(*Saved_State, 1);
}


/***********************************************************************
**
*/	void Throw_Return_Series(REBCNT type, REBSER *series)
/*
**		Throws a series value using error temp values.
**
***********************************************************************/
{
	REBVAL *val;
	REBVAL *err;
	REBSER *blk = VAL_SERIES(TASK_ERR_TEMPS);

	RESET_SERIES(blk);
	val = Append_Value(blk);
	Set_Series(type, val, series);
	err = Append_Value(blk);
	SET_THROW(err, RE_RETURN, val);
	VAL_ERR_SYM(err) = SYM_RETURN; // indicates it is "virtual" (parse return)
	Throw_Break(err);
}


/***********************************************************************
**
*/	void Throw_Return_Value(REBVAL *value)
/*
**		Throws a series value using error temp values.
**
***********************************************************************/
{
	REBVAL *val;
	REBVAL *err;
	REBSER *blk = VAL_SERIES(TASK_ERR_TEMPS);

	RESET_SERIES(blk);
	val = Append_Value(blk);
	*val = *value;
	err = Append_Value(blk);
	SET_THROW(err, RE_RETURN, val);
	VAL_ERR_SYM(err) = SYM_RETURN; // indicates it is "virtual" (parse return)
	Throw_Break(err);
}


/***********************************************************************
**
*/	void Trap_Stack()
/*
***********************************************************************/
{
	if (IS_INTEGER(TASK_THIS_ERROR)) return; // composing prior error.

	if (!Saved_State) Crash(RP_NO_SAVED_STATE);

	*TASK_THIS_ERROR = *TASK_STACK_ERROR; // pre-allocated

	longjmp(*Saved_State, 1);
}


/***********************************************************************
**
*/	REBCNT Stack_Depth()
/*
***********************************************************************/
{
	REBCNT dsf = DSF;
	REBCNT count = 0;

	for (dsf = DSF; dsf > 0; dsf = PRIOR_DSF(dsf)) {
		count++;
	}

	return count;
}


/***********************************************************************
**
*/	REBSER *Make_Backtrace(REBINT start)
/*
**		Return a block of backtrace words.
**
***********************************************************************/
{
	REBCNT depth = Stack_Depth();
	REBSER *blk = Make_Block(depth-start);
	REBINT dsf;
	REBVAL *val;

	for (dsf = DSF; dsf > 0; dsf = PRIOR_DSF(dsf)) {
		if (start-- <= 0) {
			val = Append_Value(blk);
			Init_Word(val, VAL_WORD_SYM(DSF_WORD(dsf)));
		}
	}

	return blk;
}


/***********************************************************************
**
*/	void Set_Error_Type(ERROR_OBJ *error)
/*
**		Sets error type and id fields based on code number.
**
***********************************************************************/
{
	REBSER *cats;		// Error catalog object
	REBSER *cat;		// Error category object
	REBCNT n;		// Word symbol number
	REBCNT code;

	code = VAL_INT32(&error->code);

	// Set error category:
	n = code / 100 + 1;
	cats = VAL_OBJ_FRAME(Get_System(SYS_CATALOG, CAT_ERRORS));

	if (code >= 0 && n < SERIES_TAIL(cats) &&
		NZ(cat = VAL_SERIES(BLK_SKIP(cats, n)))
	) {
		Set_Word(&error->type, FRM_WORD_SYM(cats, n), cats, n);

		// Find word related to the error itself:
		
		n = code % 100 + 3;
		if (n < SERIES_TAIL(cat))
			Set_Word(&error->id, FRM_WORD_SYM(cat, n), cat, n);
	}
}


/***********************************************************************
**
*/	REBVAL *Find_Error_Info(ERROR_OBJ *error, REBINT *num)
/*
**		Return the error message needed to print an error.
**		Must scan the error catalog and its error lists.
**		Note that the error type and id words no longer need
**		to be bound to the error catalog context.
**		If the message is not found, return null.
**
***********************************************************************/
{
	REBSER *frame;
	REBVAL *obj1;
	REBVAL *obj2;

	if (!IS_WORD(&error->type) || !IS_WORD(&error->id)) return 0;

	// Find the correct error type object in the catalog:
	frame = VAL_OBJ_FRAME(Get_System(SYS_CATALOG, CAT_ERRORS));
	obj1 = Find_Word_Value(frame, VAL_WORD_SYM(&error->type));
	if (!obj1) return 0;

	// Now find the correct error message for that type:
	frame = VAL_OBJ_FRAME(obj1);
	obj2 = Find_Word_Value(frame, VAL_WORD_SYM(&error->id));
	if (!obj2) return 0;

	if (num) {
		obj1 = Find_Word_Value(frame, SYM_CODE);
		*num = VAL_INT32(obj1)
			+ Find_Word_Index(frame, VAL_WORD_SYM(&error->id), FALSE)
			- Find_Word_Index(frame, SYM_TYPE, FALSE) - 1;
	}

	return obj2;
}


/***********************************************************************
**
*/	void Make_Error_Object(REBVAL *arg, REBVAL *value)
/*
**		Creates an error object from arg and puts it in value.
**		The arg can be a string or an object body block.
**		This function is called by MAKE ERROR!.
**
***********************************************************************/
{
	REBSER *err;		// Error object
	ERROR_OBJ *error;	// Error object values
	REBINT code = 0;

	// Create a new error object from another object, including any non-standard fields:
	if (IS_ERROR(arg) || IS_OBJECT(arg)) {
		err = Merge_Frames(VAL_OBJ_FRAME(ROOT_ERROBJ),
			IS_ERROR(arg) ? VAL_OBJ_FRAME(arg) : VAL_ERR_OBJECT(arg));
		error = ERR_VALUES(err);
//		if (!IS_INTEGER(&error->code)) {
			if (!Find_Error_Info(error, &code)) code = RE_INVALID_ERROR;
			SET_INTEGER(&error->code, code);
//		}
		SET_ERROR(value, VAL_INT32(&error->code), err);
		return;
	}

	// Make a copy of the error object template:
	err = CLONE_OBJECT(VAL_OBJ_FRAME(ROOT_ERROBJ));
	error = ERR_VALUES(err);
	SET_NONE(&error->id);
	SET_ERROR(value, 0, err);

	// If block arg, evaluate object values (checking done later):
	// If user set error code, use it to setup type and id fields.
	if (IS_BLOCK(arg)) {
		DISABLE_GC;
		Do_Bind_Block(err, arg); // GC-OK (disabled)
		ENABLE_GC;
		if (IS_INTEGER(&error->code) && VAL_INT64(&error->code)) {
			Set_Error_Type(error);
		} else {
			if (Find_Error_Info(error, &code)) {
				SET_INTEGER(&error->code, code);
			}
		}
		// The error code is not valid:
		if (IS_NONE(&error->id)) {
			SET_INTEGER(&error->code, RE_INVALID_ERROR);
			Set_Error_Type(error);
		}
		if (VAL_INT64(&error->code) < 100 || VAL_INT64(&error->code) > 1000)
			Trap_Arg(arg);
	}

	// If string arg, setup other fields
	else if (IS_STRING(arg)) {
		SET_INTEGER(&error->code, RE_USER); // user error
		Set_String(&error->arg1, Copy_Series_Value(arg));
		Set_Error_Type(error);
	}

// No longer allowed:
//	else if (IS_INTEGER(arg)) {
//		error->code = *arg;
//		Set_Error_Type(error);
//	}
	else
		Trap_Arg(arg);

	if (!(VAL_ERR_NUM(value) = VAL_INT32(&error->code))) {
		Trap_Arg(arg);
	}
}


/***********************************************************************
**
*/	REBSER *Make_Error(REBINT code, REBVAL *arg1, REBVAL *arg2, REBVAL *arg3)
/*
**		Create and init a new error object.
**
***********************************************************************/
{
	REBSER *err;		// Error object
	ERROR_OBJ *error;	// Error object values

	if (PG_Boot_Phase < BOOT_ERRORS) Crash(RP_EARLY_ERROR, code); // Not far enough!

	// Make a copy of the error object template:
	err = CLONE_OBJECT(VAL_OBJ_FRAME(ROOT_ERROBJ));
	error = ERR_VALUES(err);

	// Set error number:
	SET_INTEGER(&error->code, (REBINT)code);
	Set_Error_Type(error);

	// Set error argument values:
	if (arg1) error->arg1 = *arg1;
	if (arg2) error->arg2 = *arg2;
	if (arg3) error->arg3 = *arg3;

	// Set backtrace and location information:
	if (DSF > 0) {
		// Where (what function) is the error:
		Set_Block(&error->where, Make_Backtrace(0));
		// Nearby location of the error (in block being evaluated):
		error->nearest = *DSF_BACK(DSF);
	}

	return err;
}


/***********************************************************************
**
*/	void Trap0(REBCNT num)
/*
***********************************************************************/
{
	Throw_Error(Make_Error(num, 0, 0, 0));
}


/***********************************************************************
**
*/	void Trap1(REBCNT num, REBVAL *arg1)
/*
***********************************************************************/
{
	Throw_Error(Make_Error(num, arg1, 0, 0));
}


/***********************************************************************
**
*/	void Trap2(REBCNT num, REBVAL *arg1, REBVAL *arg2)
/*
***********************************************************************/
{
	Throw_Error(Make_Error(num, arg1, arg2, 0));
}


/***********************************************************************
**
*/	void Trap3(REBCNT num, REBVAL *arg1, REBVAL *arg2, REBVAL *arg3)
/*
***********************************************************************/
{
	Throw_Error(Make_Error(num, arg1, arg2, arg3));
}


/***********************************************************************
**
*/	void Trap_Arg(REBVAL *arg)
/*
***********************************************************************/
{
	Trap1(RE_INVALID_ARG, arg);
}


/***********************************************************************
**
*/	void Trap_Type(REBVAL *arg)
/*
**		<type> type is not allowed here
**
***********************************************************************/
{
	Trap1(RE_INVALID_TYPE, Of_Type(arg));
}


/***********************************************************************
**
*/	void Trap_Range(REBVAL *arg)
/*
**		value out of range: <value>
**
***********************************************************************/
{
	Trap1(RE_OUT_OF_RANGE, arg);
}


/***********************************************************************
**
*/	void Trap_Word(REBCNT num, REBCNT sym, REBVAL *arg)
/*
***********************************************************************/
{
	Init_Word(DS_TOP, sym);
	if (arg) Trap2(num, DS_TOP, arg);
	else Trap1(num, DS_TOP);
}


/***********************************************************************
**
*/	void Trap_Action(REBCNT type, REBCNT action)
/*
***********************************************************************/
{
	Trap2(RE_CANNOT_USE, Get_Action_Word(action), Get_Type(type));
}


/***********************************************************************
**
*/	void Trap_Math_Args(REBCNT type, REBCNT action)
/*
***********************************************************************/
{
	Trap2(RE_NOT_RELATED, Get_Action_Word(action), Get_Type(type));
}


/***********************************************************************
**
*/	void Trap_Types(REBCNT errnum, REBCNT type1, REBCNT type2)
/*
***********************************************************************/
{
	if (type2 != 0) Trap2(errnum, Get_Type(type1), Get_Type(type2));
	Trap1(errnum, Get_Type(type1));
}


/***********************************************************************
**
*/	void Trap_Expect(REBVAL *object, REBCNT index, REBCNT type)
/*
**		Object field is not of expected type.
**		PORT expected SCHEME of OBJECT type
**
***********************************************************************/
{
	Trap3(RE_EXPECT_TYPE, Of_Type(object), Obj_Word(object, index), Get_Type(type));
}


/***********************************************************************
**
*/	void Trap_Make(REBCNT type, REBVAL *spec)
/*
***********************************************************************/
{
	Trap2(RE_BAD_MAKE_ARG, Get_Type(type), spec);
}


/***********************************************************************
**
*/	void Trap_Num(REBCNT err, REBCNT num)
/*
***********************************************************************/
{
	DS_PUSH_INTEGER(num);
	Trap1(err, DS_TOP);
}


/***********************************************************************
**
*/	void Trap_Reflect(REBCNT type, REBVAL *arg)
/*
***********************************************************************/
{
	Trap_Arg(arg);
}


/***********************************************************************
**
*/	void Trap_Port(REBCNT errnum, REBSER *port, REBINT err_code)
/*
***********************************************************************/
{
	REBVAL *spec = OFV(port, STD_PORT_SPEC);
	REBVAL *val;

	if (!IS_OBJECT(spec)) Trap0(RE_INVALID_PORT);

	val = Get_Object(spec, STD_PORT_SPEC_HEAD_REF); // most informative
	if (IS_NONE(val)) val = Get_Object(spec, STD_PORT_SPEC_HEAD_TITLE);

	DS_PUSH_INTEGER(err_code);
	Trap2(errnum, val, DS_TOP);
}


/***********************************************************************
**
*/	REBINT Check_Error(REBVAL *val)
/*
**		Process a loop exceptions. Pass in the TOS value, returns:
**
**			 2 - if break/return, change val to that set by break
**			 1 - if break
**			-1 - if continue, change val to unset
**			 0 - if not break or continue
**			else: error if not an ERROR value
**
***********************************************************************/
{
	// It's UNSET, not an error:
	if (!IS_ERROR(val))
		Trap0(RE_NO_RETURN); //!!! change to special msg

	// If it's a BREAK, check for /return value:
	if (IS_BREAK(val)) {
		if (VAL_ERR_VALUE(val)) {
			*val = *VAL_ERR_VALUE(val);
			return 2;
		} else {
			SET_UNSET(val);
			return 1;
		}
	}

	if (IS_CONTINUE(val)) {
		SET_UNSET(val);
		return -1;
	}

	return 0;
	// Else: Let all other errors return as values.
}


/***********************************************************************
**
*/	void Init_Errors(REBVAL *errors)
/*
***********************************************************************/
{
	REBSER *errs;
	REBVAL *val;

	// Create error objects and error type objects:
	*ROOT_ERROBJ = *Get_System(SYS_STANDARD, STD_ERROR);
	errs = Construct_Object(0, VAL_BLK(errors), 0);
	Set_Object(Get_System(SYS_CATALOG, CAT_ERRORS), errs);

	Set_Root_Series(TASK_ERR_TEMPS, Make_Block(3), "task errors");

	// Create objects for all error types:
	for (val = BLK_SKIP(errs, 1); NOT_END(val); val++) {
		errs = Construct_Object(0, VAL_BLK(val), 0);
		SET_OBJECT(val, errs);
	}

	// Catch top level errors, to provide decent output:
	PUSH_STATE(Top_State, Saved_State);
	if (SET_JUMP(Top_State)) {
		POP_STATE(Top_State, Saved_State);
		DSP++; // Room for return value
		Catch_Error(DS_TOP); // Stores error value here
		Print_Value(DS_TOP, 0, FALSE);
		Crash(RP_NO_CATCH);
	}
	SET_STATE(Top_State, Saved_State);
}


/***********************************************************************
**
*/	REBYTE *Security_Policy(REBCNT sym, REBVAL *name)
/*
**	Given a security symbol (like FILE) and a value (like the file
**	path) returns the security policy (RWX) allowed for it.
**
**	Args:
**
**		sym:  word that represents the type ['file 'net]
**		name: file or path value
**
**	Returns BTYE array of flags for the policy class:
**
**		flags: [rrrr wwww xxxx ----]
**
**		Where each byte is:
**			0: SEC_ALLOW
**			1: SEC_ASK
**			2: SEC_THROW
**			3: SEC_QUIT
**
**	The secuity is defined by the system/state/policies object, that
**	is of the form:
**
**		[
**			file:  [%file1 tuple-flags %file2 ... default tuple-flags]
**			net:   [...]
**			call:  tuple-flags
**			stack: tuple-flags
**			eval:  integer (limit)
**		]
**
***********************************************************************/
{
	REBVAL *policy = Get_System(SYS_STATE, STATE_POLICIES);
	REBYTE *flags;
	REBCNT len;
	REBCNT errcode = RE_SECURITY_ERROR;

	if (!IS_OBJECT(policy)) goto error;

	// Find the security class in the block: (file net call...)
	policy = Find_Word_Value(VAL_OBJ_FRAME(policy), sym);
	if (!policy) goto error;

	// Obtain the policies for it:
	// Check for a master tuple: [file rrrr.wwww.xxxx]
	if (IS_TUPLE(policy)) return VAL_TUPLE(policy); // non-aligned
	// removed A90: if (IS_INTEGER(policy)) return (REBYTE*)VAL_INT64(policy); // probably not used

	// Only other form is detailed block:
	if (!IS_BLOCK(policy)) goto error;

	// Scan block of policies for the class: [file [allow read quit write]]
	len = 0;	// file or url length
	flags = 0;	// policy flags
	for (policy = VAL_BLK(policy); NOT_END(policy); policy += 2) {

		// Must be a policy tuple:
		if (!IS_TUPLE(policy+1)) goto error;

		// Is it a policy word:
		if (IS_WORD(policy)) { // any word works here
			// If no strings found, use the default:
			if (len == 0) flags = VAL_TUPLE(policy+1); // non-aligned
		}

		// Is it a string (file or URL):
		else if (ANY_BINSTR(policy) && name) {
			//Debug_Fmt("sec: %r %r", policy, name);
			if (Match_Sub_Path(VAL_SERIES(policy), VAL_SERIES(name))) {
				// Is the match adequate?
				if (VAL_TAIL(name) >= len) {
					len = VAL_TAIL(name);
					flags = VAL_TUPLE(policy+1); // non-aligned
				}
			}
		}
		else goto error;
	}

	if (!flags) {
		errcode = RE_SECURITY;
		policy = name ? name : 0;
error:
		if (!policy) {
			Init_Word(DS_TOP, sym);
			policy = DS_TOP;
		}
		Trap1(errcode, policy);
	}

	return flags;
}


/***********************************************************************
**
*/	void Trap_Security(REBCNT flag, REBCNT sym, REBVAL *value)
/*
**		Take action on the policy flags provided. The sym and value
**		are provided for error message purposes only.
**
***********************************************************************/
{
	if (flag == SEC_THROW) {
		if (!value) {
			Init_Word(DS_TOP, sym);
			value = DS_TOP;
		}
		Trap1(RE_SECURITY, value);
	}
	else if (flag == SEC_QUIT) OS_EXIT(101);
}


/***********************************************************************
**
*/	void Check_Security(REBCNT sym, REBCNT policy, REBVAL *value)
/*
**		A helper function that fetches the security flags for
**		a given symbol (FILE) and value (path), and then tests
**		that they are allowed.
**
***********************************************************************/
{
	REBYTE *flags;
	
	flags = Security_Policy(sym, value);
	Trap_Security(flags[policy], sym, value);
}
