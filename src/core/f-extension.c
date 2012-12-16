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
**  Module:  f-extension.c
**  Summary: support for extensions
**  Section: functional
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#include "reb-ext.h" // includes copy of ext-types.h
#include "reb-evtypes.h"

#include "reb-lib.h"

// Extension evaluation categories:
enum {
    RXE_NULL,	// unset
	RXE_PTR,	// any pointer
    RXE_32,		// logic
    RXE_64,		// integer, decimal, etc.
    RXE_SYM,	// word
    RXE_SER,	// string
	RXE_IMAGE,	// image
	RXE_DATE,	// from upper section
	RXE_MAX
};

//(*call)(int cmd, RXIFRM *args);

typedef struct reb_ext {
	RXICAL call;				// Call(function) entry point
	void *dll;					// DLL library "handle"
	int  index;					// Index in extension table
	int  object;				// extension object reference
} REBEXT;

#include "tmp-exttypes.h"

extern const REBDOF Func_Dispatch[];

// !!!! The list below should not be hardcoded, but until someone
// needs a lot of extensions, it will do fine.
REBEXT Ext_List[64];
REBCNT Ext_Next = 0;


/***********************************************************************
**
**	Local functions
**
***********************************************************************/

/***********************************************************************
**
x*/	RXIARG Value_To_RXI(REBVAL *val)
/*
***********************************************************************/
{
	RXIARG arg;

	switch (RXT_Eval_Class[Reb_To_RXT[VAL_TYPE(val)]]) {
	case RXE_64:
		arg.int64 = VAL_INT64(val);
		break;
	case RXE_SER:
		arg.series = VAL_SERIES(val);
		arg.index = VAL_INDEX(val);
		break;
	case RXE_PTR:
		arg.addr = VAL_HANDLE(val);
		break;
	case RXE_32:
		arg.int32a = VAL_I32(val);
		arg.int32b = 0;
		break;
	case RXE_DATE:
		arg.int32a = VAL_ALL_BITS(val)[2];
		arg.int32b = 0;
		break;
	case RXE_SYM:
		arg.int32a = VAL_WORD_CANON(val);
		arg.int32b = 0;
		break;
	case RXE_IMAGE:
		arg.series = VAL_SERIES(val);
		arg.width = VAL_IMAGE_WIDE(val);
		arg.height = VAL_IMAGE_HIGH(val);
		break;
	case RXE_NULL:
	default:
		arg.int64 = 0;
		break;
	}
	return arg;
}

/***********************************************************************
**
x*/	void RXI_To_Value(REBVAL *val, RXIARG arg, REBCNT type)
/*
***********************************************************************/
{
	VAL_SET(val, RXT_To_Reb[type]);
	switch (RXT_Eval_Class[type]) {
	case RXE_64:
		VAL_INT64(val) = arg.int64;
		break;
	case RXE_SER:
		VAL_SERIES(val) = arg.series;
		VAL_INDEX(val) = arg.index;
		break;
	case RXE_PTR:
		VAL_HANDLE(val) = arg.addr;
		break;
	case RXE_32:
		VAL_I32(val) = arg.int32a;
		break;
	case RXE_DATE:
		VAL_TIME(val) = NO_TIME;
		VAL_ALL_BITS(val)[2] = arg.int32a;
		break;
	case RXE_SYM:
		VAL_WORD_SYM(val) = arg.int32a;
		VAL_WORD_FRAME(val) = 0;
		VAL_WORD_INDEX(val) = 0;
		break;
	case RXE_IMAGE:
		VAL_SERIES(val) = arg.series;
		VAL_IMAGE_WIDE(val) = arg.width;
		VAL_IMAGE_HIGH(val) = arg.height;
		break;
	case RXE_NULL:
		VAL_INT64(val) = 0;
		break;
	default:
		SET_NONE(val);
	}
}

/***********************************************************************
**
x*/	void RXI_To_Block(RXIFRM *frm, REBVAL *out) {
/*
***********************************************************************/
	REBCNT n;
	REBSER *blk;
	REBVAL *val;
	REBCNT len;

	blk = Make_Block(len = RXA_COUNT(frm));
	for (n = 1; n <= len; n++) {
		val = Append_Value(blk);
		RXI_To_Value(val, frm->args[n], RXA_TYPE(frm, n));
	}
	Set_Block(out, blk);
}


/***********************************************************************
**
x*/	int Do_Callback(REBSER *obj, u32 name, RXIARG *args, RXIARG *result)
/*
**		Given an object and a word id, call a REBOL function.
**		The arguments are converted from extension format directly
**		to the data stack. The result is passed back in ext format,
**		with the datatype returned or zero if there was a problem.
**
***********************************************************************/
{
	REBVAL *val;
	REBCNT dsf;
	REBCNT len;
	REBCNT n;
	REBCNT dsp = DSP; // to restore stack on errors

	// Find word in object, verify it is a function.
	if (!(val = Find_Word_Value(obj, name))) {
		SET_EXT_ERROR(result, RXE_NO_WORD);
		return 0;
	}
	if (!ANY_FUNC(val)) {
		SET_EXT_ERROR(result, RXE_NOT_FUNC);
		return 0;
	}

	// Get block and index from prior function stack frame:
	dsf = PRIOR_DSF(DSF);

	// Create stack frame (use prior stack frame for location info):
	dsf = Push_Func(0, VAL_SERIES(DSF_BACK(dsf)), VAL_INDEX(DSF_BACK(dsf)), name, val);
	val = DSF_FUNC(dsf);        // for safety from GC
	obj = VAL_FUNC_WORDS(val);  // func words
	len = SERIES_TAIL(obj)-1;	// number of args (may include locals)

	// Push args. Too short or too long arg frames are handled W/O error.
	// Note that refinements args can be set to anything.
	for (n = 1; n <= len && n <= RXI_COUNT(args); n++) {
		DS_SKIP;
		RXI_To_Value(DS_TOP, args[n], RXI_TYPE(args, n));
		// Check type for word at the given offset:
		if (!TYPE_CHECK(BLK_SKIP(obj, n), VAL_TYPE(DS_TOP))) {
			result->int32b = n;
			SET_EXT_ERROR(result, RXE_BAD_ARGS);
			DSP = dsp;
			return 0;
		}
	}
	// Fill with NONE if necessary:
	for (; n <= len; n++) {
		DS_SKIP;
		SET_NONE(DS_TOP);
		if (!TYPE_CHECK(BLK_SKIP(obj, n), VAL_TYPE(DS_TOP))) {
			result->int32b = n;
			SET_EXT_ERROR(result, RXE_BAD_ARGS);
			DSP = dsp;
			return 0;
		}
	}

	// Evaluate the function:
	DSF = dsf;
	Func_Dispatch[VAL_TYPE(val) - REB_NATIVE](val);
	DSF = PRIOR_DSF(dsf);
	DSP = dsf-1;

	// Return resulting value from TOS1 (volatile location):
	*result = Value_To_RXI(DS_VALUE(dsf));
	return Reb_To_RXT[VAL_TYPE(DS_VALUE(dsf))];
}


/***********************************************************************
**
*/	REBNATIVE(do_callback)
/*
**		object word arg1 arg2
**
***********************************************************************/
{
	RXICBI *cbi;
	REBVAL *event = D_ARG(1);
	REBCNT n;

	// Sanity check:
	if (VAL_EVENT_TYPE(event) != EVT_CALLBACK || !(cbi = VAL_EVENT_SER(event)))
		return R_NONE;

	n = Do_Callback(cbi->obj, cbi->word, cbi->args, &(cbi->result));

	SET_FLAG(cbi->flags, RXC_DONE);

	if (!n) Trap_Num(RE_INVALID_ARG, GET_EXT_ERROR(&cbi->result));

	RXI_To_Value(ds, cbi->result, n);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(load_extension)
/*
**		arg 1: filename | body binary string (UTF-8)
**		arg 2: dispatch
**		arg 3: function handle
**
**	Low level extension loader:
**
**		1. Opens the DLL for the extension
**		2. Calls its Info() command to get its definition header (REBOL)
**		3. Inits an extension structure (dll, Call() function)
**		4. Creates a extension object and returns it
**		5. REBOL code then uses that object to define the extension module
**		   including commands, functions, data, exports, etc.
**
**	Each extension is defined as DLL with:
**
**		init() - init anything needed
**		quit() - cleanup anything needed
**		call() - dispatch a native
**
***********************************************************************/
{
	REBCHR *name;
	void *dll;
	REBCNT error;
	REBYTE *code;
	REBYTE *(*info)(REBINT opts, void *lib);
	REBSER *obj;
	REBVAL *val = D_ARG(1);
	REBEXT *ext;
	RXICAL call;
	REBSER *src;
	int Remove_after_first_run;
	//Check_Security(SYM_EXTENSION, POL_EXEC, val);

	if (!D_REF(2)) { // No /dispatch, use the DLL file:

		if (!IS_FILE(val)) Trap_Arg(val);

		name = Val_Str_To_OS(val);

		// Try to load the DLL file:
		if (!(dll = OS_OPEN_LIBRARY(name, &error))) {
			Trap1(RE_NO_EXTENSION, val);
		}

		// Call its info() function for header and code body:
		if (!(info = OS_FIND_FUNCTION(dll, BOOT_STR(RS_EXTENSION, 0)))){
			OS_CLOSE_LIBRARY(dll);
			Trap1(RE_BAD_EXTENSION, val);
		}

		// Obtain info string as UTF8:
		if (!(code = info(0, Extension_Lib()))) {
			OS_CLOSE_LIBRARY(dll);
			Trap1(RE_EXTENSION_INIT, val);
		}

		// Import the string into REBOL-land:
		src = Copy_Bytes(code, -1); // Nursery protected
		call = OS_FIND_FUNCTION(dll, BOOT_STR(RS_EXTENSION, 2)); // zero is allowed
	}
	else {
		// Hosted extension:
		src = VAL_SERIES(val);
		call = (RXICAL)VAL_HANDLE(D_ARG(3));
		dll = 0;
	}

	ext = &Ext_List[Ext_Next];
	CLEARS(ext);
	ext->call = call;
	ext->dll = dll;
	ext->index = Ext_Next++;

	// Extension return: dll, info, filename
	obj = VAL_OBJ_FRAME(Get_System(SYS_STANDARD, STD_EXTENSION));
	obj = CLONE_OBJECT(obj);
	Set_Object(D_RET, obj);

	// Set extension fields needed:
	val = FRM_VALUE(obj, STD_EXTENSION_LIB_BASE);
	VAL_SET(val, REB_HANDLE);
	VAL_I32(val) = ext->index;
	if (!D_REF(2)) *FRM_VALUE(obj, STD_EXTENSION_LIB_FILE) = *D_ARG(1);
	Set_Binary(FRM_VALUE(obj, STD_EXTENSION_LIB_BOOT), src);

	return R_RET;
}


/***********************************************************************
**
*/	void Make_Command(REBVAL *value, REBVAL *def)
/*
**		Assumes prior function has already stored the spec and args
**		series. This function validates the body.
**
***********************************************************************/
{
	REBVAL *args = BLK_HEAD(VAL_FUNC_ARGS(value));
	REBCNT n;
	REBVAL *val = VAL_BLK_SKIP(def, 1);
	REBEXT *ext;

	if (
		VAL_LEN(def) != 3
		|| !(IS_MODULE(val) || IS_OBJECT(val))
		|| !IS_HANDLE(VAL_OBJ_VALUE(val, 1))
		|| !IS_INTEGER(val+1)
		|| VAL_INT64(val+1) > 0xffff
	) Trap1(RE_BAD_FUNC_DEF, def);

	val = VAL_OBJ_VALUE(val, 1);
	if (
		!(ext = &Ext_List[VAL_I32(val)])
		|| !(ext->call)
	) Trap1(RE_BAD_EXTENSION, def);

	// make command! [[arg-spec] handle cmd-index]
	VAL_FUNC_BODY(value) = Copy_Block_Len(VAL_SERIES(def), 1, 2);

	// Check for valid command arg datatypes:
	args++; // skip self
	n = 1;
	for (; NOT_END(args); args++, n++) {
		// If the typeset contains args that are not valid:
		// (3 is the default when no args given, for not END and UNSET)
		if (3 != ~VAL_TYPESET(args) && (VAL_TYPESET(args) & ~RXT_ALLOWED_TYPES))
			Trap1(RE_BAD_FUNC_ARG, args);
	}

	VAL_SET(value, REB_COMMAND);
}


/***********************************************************************
**
*/	void Do_Command(REBVAL *value)
/*
**	Evaluates the arguments for a command function and creates
**	a resulting stack frame (struct or object) for command processing.
**
**	A command value consists of:
**		args - same as other funcs
**		spec - same as other funcs
**		body - [ext-obj func-index]
**
***********************************************************************/
{
	REBVAL *val = BLK_HEAD(VAL_FUNC_BODY(value));
	REBEXT *ext;
	REBCNT cmd;
	REBCNT argc;
	REBCNT n;
	RXIFRM frm;	// args stored here

	// All of these were checked above on definition:
	val = BLK_HEAD(VAL_FUNC_BODY(value));
	cmd = (int)VAL_INT64(val+1);
	ext = &Ext_List[VAL_I32(VAL_OBJ_VALUE(val, 1))]; // Handler

	// Copy args to command frame (array of args):
	RXA_COUNT(&frm) = argc = SERIES_TAIL(VAL_FUNC_ARGS(value))-1; // not self
	if (argc > 7) Trap0(RE_BAD_COMMAND);
	val = DS_ARG(1);
	for (n = 1; n <= argc; n++, val++) {
		RXA_TYPE(&frm, n) = Reb_To_RXT[VAL_TYPE(val)];
		frm.args[n] = Value_To_RXI(val);
	}

	// Call the command:
	n = ext->call(cmd, &frm, 0);
	val = DS_RETURN;
	switch (n) {
	case RXR_VALUE:
		RXI_To_Value(val, frm.args[1], RXA_TYPE(&frm, 1));
		break;
	case RXR_BLOCK:
		RXI_To_Block(&frm, val);
		break;
	case RXR_UNSET:
		SET_UNSET(val);
		break;
	case RXR_NONE:
		SET_NONE(val);
		break;
	case RXR_TRUE:
		SET_TRUE(val);
		break;
	case RXR_FALSE:
		SET_FALSE(val);
		break;
	case RXR_ERROR:
	default:
		SET_UNSET(val);
	}
}


/***********************************************************************
**
*/	void Do_Commands(REBSER *cmds, void *context)
/*
**		Evaluate a block of commands as efficiently as possible.
**		The arguments to each command must already be reduced or
**		use only variable lookup.
**
**		Returns the last evaluated value, if provided.
**
***********************************************************************/
{
	REBVAL *blk;
	REBCNT index = 0;
	REBVAL *set_word = 0;
	REBVAL *cmd_word;
	REBSER *words;
	REBVAL *args;
	REBVAL *val;
	REBVAL *func;
	RXIFRM frm;	// args stored here
	REBCNT n;
	REBEXT *ext;
	REBCEC *ctx;

	if ((ctx = context)) ctx->block = cmds;
	blk = BLK_HEAD(cmds);

	while (NOT_END(blk)) {

		// var: command result
		if IS_SET_WORD(blk) {
			set_word = blk++;
			index++;
		};

		// get command function
		if (IS_WORD(cmd_word = blk)) {
			// Optimized var fetch:
			n = VAL_WORD_INDEX(blk);
			if (n > 0) func = FRM_VALUES(VAL_WORD_FRAME(blk)) + n;
			else func = Get_Var(blk); // fallback
		} else func = blk;

		if (!IS_COMMAND(func)) Trap2(RE_EXPECT_VAL, Get_Type_Word(REB_COMMAND), blk);

		// Advance to next value
		blk++;
		if (ctx) ctx->index = index; // position of function
		index++;

		// get command arguments and body
		words = VAL_FUNC_WORDS(func);
		RXA_COUNT(&frm) = SERIES_TAIL(VAL_FUNC_ARGS(func))-1; // not self

		// collect each argument (arg list already validated on MAKE)
		n = 0;
		for (args = BLK_SKIP(words, 1); NOT_END(args); args++) {

			//Debug_Type(args);
			val = blk++;
			index++;
			if (IS_END(val)) Trap2(RE_NO_ARG, cmd_word, args);
			//Debug_Type(val);

			// actual arg is a word, lookup?
			if (VAL_TYPE(val) >= REB_WORD) {
				if (IS_WORD(val)) {
					if (IS_WORD(args)) val = Get_Var(val);
				}
				else if (IS_PATH(val)) {
					if (IS_WORD(args)) val = Get_Any_Var(val); // volatile value!
				}
				else if (IS_PAREN(val)) {
					val = Do_Blk(VAL_SERIES(val), 0); // volatile value!
				}
				// all others fall through
			}

			// check datatype
			if (!TYPE_CHECK(args, VAL_TYPE(val)))
				Trap3(RE_EXPECT_ARG, cmd_word, args, Of_Type(val));

			// put arg into command frame
			n++;
			RXA_TYPE(&frm, n) = Reb_To_RXT[VAL_TYPE(val)];
			frm.args[n] = Value_To_RXI(val);
		}

		// Call the command (also supports different extension modules):
		func  = BLK_HEAD(VAL_FUNC_BODY(func));
		n = (REBCNT)VAL_INT64(func + 1);
		ext = &Ext_List[VAL_I32(VAL_OBJ_VALUE(func, 1))]; // Handler
		n = ext->call(n, &frm, context);
		val = DS_RETURN;
		switch (n) {
		case RXR_VALUE:
			RXI_To_Value(val, frm.args[1], RXA_TYPE(&frm, 1));
			break;
		case RXR_BLOCK:
			RXI_To_Block(&frm, val);
			break;
		case RXR_UNSET:
			SET_UNSET(val);
			break;
		case RXR_NONE:
			SET_NONE(val);
			break;
		case RXR_TRUE:
			SET_TRUE(val);
			break;
		case RXR_FALSE:
			SET_FALSE(val);
			break;
		case RXR_ERROR:
		default:
			SET_UNSET(val);
		}

		if (set_word) {
			Set_Var(set_word, val);
			set_word = 0;
		}
	}
}


/***********************************************************************
**
*/	REBNATIVE(do_commands)
/*
***********************************************************************/
{
	REBCEC ctx;

	ctx.envr = 0;
	ctx.block = VAL_SERIES(D_ARG(1));
	ctx.index = 0;
	Do_Commands(ctx.block, &ctx);

	return R_RET;
}



#ifdef notused
/***********************************************************************
**
xx*/	REBVAL *Prior_Func_Frame(void)
/*
***********************************************************************/
{
	REBCNT dsf = DSF;
	REBVAL *val;

	for (dsf = DSF; dsf > 0; dsf = PRIOR_DSF(dsf)) {
		val = DSF_BACK(dsf);
		if (IS_BLOCK(val) && VAL_SERIES(val))
			return val;
	}

	return 0;
}
#endif

