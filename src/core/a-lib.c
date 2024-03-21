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
**  Module:  a-lib.c
**  Summary: exported REBOL library functions
**  Section: environment
**  Author:  Carl Sassenrath, Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "reb-dialect.h"
#include "reb-ext.h"
#include "reb-evtypes.h"

// Linkage back to HOST functions. Needed when we compile as a DLL
// in order to use the OS_* macro functions.
#ifdef REB_API  // Included by C command line
REBOL_HOST_LIB *Host_Lib;
#endif

#include "reb-lib.h"

//#define DUMP_INIT_SCRIPT
#ifdef DUMP_INIT_SCRIPT
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#endif

extern const REBYTE Reb_To_RXT[REB_MAX];
extern RXIARG Value_To_RXI(REBVAL *val); // f-extension.c
extern void RXI_To_Value(REBVAL *val, RXIARG arg, REBCNT type); // f-extension.c
extern void RXI_To_Block(RXIFRM *frm, REBVAL *out); // f-extension.c
extern int Do_Callback(REBSER *obj, u32 name, RXIARG *args, RXIARG *result);


/***********************************************************************
**
*/	RL_API void RL_Version(REBYTE vers[])
/*
**	Obtain current REBOL interpreter version information.
**
**	Returns:
**		A byte array containing version, revision, update, and more.
**	Arguments:
**		vers - a byte array to hold the version info. First byte is length,
**			followed by version, revision, update, system, variation.
**	Notes:
**		This function can be called before any other initialization
**		to determine version compatiblity with the caller.
**
***********************************************************************/
{
	// [0] is length
	vers[1] = REBOL_VER;
	vers[2] = REBOL_REV;
	vers[3] = REBOL_UPD;
	vers[4] = REBOL_SYS;
	vers[5] = REBOL_VAR;
}


/***********************************************************************
**
*/	RL_API int RL_Init(REBARGS *rargs, void *lib)
/*
**	Initialize the REBOL interpreter.
**
**	Returns:
**		Zero on success, otherwise an error indicating that the
**		host library is not compatible with this release.
**	Arguments:
**		rargs - REBOL command line args and options structure.
**			See the host-args.c module for details.
**		lib - the host lib (OS_ functions) to be used by REBOL.
**			See host-lib.c for details.
**	Notes:
**		This function will allocate and initialize all memory
**		structures used by the REBOL interpreter. This is an
**		extensive process that takes time.
**
***********************************************************************/
{
	int marker;
	REBUPT bounds;

	Host_Lib = lib;

	if (Host_Lib->size < HOST_LIB_SIZE) return 1;
	if (((HOST_LIB_VER << 16) + HOST_LIB_SUM) != Host_Lib->ver_sum) return 2;

	bounds = (REBUPT)OS_CONFIG(1, 0);
	if (bounds == 0) bounds = (REBUPT)STACK_BOUNDS;

#ifdef OS_STACK_GROWS_UP
	Stack_Limit = (REBUPT)(&marker) + bounds;
#else
	if (bounds > (REBUPT) &marker) Stack_Limit = 100;
	else Stack_Limit = (REBUPT)&marker - bounds;
#endif

	Init_Core(rargs);
	GC_Active = TRUE; // Turn on GC
	if (rargs->options & RO_TRACE) {
		Trace_Level = 9999;
		Trace_Flags = 1;
	}

	return 0;
}


/***********************************************************************
**
*/	RL_API void RL_Dispose(void)
/*
**	Disposes the REBOL interpreter.
**
**	Returns:
**		nothing
**	Arguments:
**		none
**	Notes:
**		This function will deallocate and release all memory
**		structures used by the REBOL interpreter.
**
***********************************************************************/
{
	Dispose_Core();
}


/***********************************************************************
**
*/	RL_API int RL_Start(REBYTE *bin, REBINT len, REBCNT flags)
/*
**	Evaluate the default boot function.
**
**	Returns:
**		Zero on success, otherwise indicates an error occurred.
**	Arguments:
**		bin - optional startup code (compressed), can be null
**		len - length of above bin
**		flags - special flags
**	Notes:
**		This function completes the startup sequence by calling
**		the sys/start function.
**
***********************************************************************/
{
	REBVAL *val;
	REBSER spec = {0};
	REBSER *ser;

	if (bin) {
		spec.data = bin;
		spec.tail = len;
		ser = DecompressZlib(&spec, 0, -1, 0, 0);
		if (!ser) return 1;

		val = BLK_SKIP(Sys_Context, SYS_CTX_BOOT_HOST);
		Set_Binary(val, ser);
	}

	return Init_Mezz(0);
}


/***********************************************************************
**
*/	RL_API void RL_Reset(void)
/*
**	Reset REBOL (not implemented)
**
**	Returns:
**		nothing
**	Arguments:
**		none
**	Notes:
**		Intended to reset the REBOL interpreter.
**
***********************************************************************/
{
	DS_RESET;
}


/***********************************************************************
**
*/	RL_API void *RL_Extend(REBYTE *source, RXICAL call)
/*
**	Appends embedded extension to system/catalog/boot-exts.
**
**	Returns:
**		A pointer to the REBOL library (see reb-lib.h).
**	Arguments:
**		source - A pointer to a UTF-8 (or ASCII) string that provides
**			extension module header, function definitions, and other
**			related functions and data.
**		call - A pointer to the extension's command dispatcher.
**	Notes:
**		This function simply adds the embedded extension to the
**		boot-exts list. All other processing and initialization
**		happens later during startup. Each embedded extension is
**		queried and init using LOAD-EXTENSION system native.
**		See c:extensions-embedded
**
***********************************************************************/
{
	REBVAL *value;
	REBSER *ser;

	value = BLK_SKIP(Sys_Context, SYS_CTX_BOOT_EXTS);
	if (IS_BLOCK(value)) ser = VAL_SERIES(value);
	else {
		ser = Make_Block(2);
		Set_Block(value, ser);
	}
	value = Append_Value(ser);
	Set_Binary(value, Copy_Bytes(source, -1)); // UTF-8
	value = Append_Value(ser);
	SET_HANDLE(value, call, SYM_EXTENSION, HANDLE_FUNCTION);

	return Extension_Lib();
}


/***********************************************************************
**
*/	RL_API void RL_Escape(REBINT reserved)
/*
**	Signal that code evaluation needs to be interrupted.
**
**	Returns:
**		nothing
**	Arguments:
**		reserved - must be set to zero.
**	Notes:
**		This function set's a signal that is checked during evaluation
**		and will cause the interpreter to begin processing an escape
**		trap. Note that control must be passed back to REBOL for the
**		signal to be recognized and handled.
**
***********************************************************************/
{
	SET_SIGNAL(SIG_ESCAPE);
}


/***********************************************************************
**
*/	RL_API int RL_Do_String(REBYTE *text, REBCNT flags, RXIARG *result)
/*
**	Load a string and evaluate the resulting block.
**
**	Returns:
**		The datatype of the result.
**	Arguments:
**		text - A null terminated UTF-8 (or ASCII) string to transcode
**			into a block and evaluate.
**		flags - set to zero for now
**		result - value returned from evaluation.
**
***********************************************************************/
{
	REBVAL *val;
	
	val = Do_String(text, 0);

	if (result) {
		*result = Value_To_RXI(val);
		return Reb_To_RXT[VAL_TYPE(val)];
	}
	return 0;
}


/***********************************************************************
**
*/	RL_API int RL_Do_Binary(REBYTE *bin, REBINT length, REBCNT flags, REBCNT key, RXIARG *result)
/*
**	Evaluate an encoded binary script such as compressed text.
**
**	Returns:
**		The datatype of the result or zero if error in the encoding.
**	Arguments:
**		bin - by default, a REBOL compressed UTF-8 (or ASCII) script.
**		length - the length of the data.
**		flags - special flags (set to zero at this time).
**		key - encoding, encryption, or signature key.
**		result - value returned from evaluation.
**	Notes:
**		As of A104, only compressed scripts are supported, however,
**		rebin, cloaked, signed, and encrypted formats will be supported.
**
***********************************************************************/
{
	REBSER spec = {0};
	REBSER *text;
	REBVAL *val;
#ifdef DUMP_INIT_SCRIPT
	int f;
#endif

	//Cloak(TRUE, code, NAT_SPEC_SIZE, &key[0], 20, TRUE);
	spec.data = bin;
	spec.tail = length;
	text = DecompressZlib(&spec, 0, -1, 0, 0);
	if (!text) return FALSE;
	Append_Byte(text, 0);

#ifdef DUMP_INIT_SCRIPT
	f = _open("host-boot.reb", _O_CREAT | _O_RDWR, _S_IREAD | _S_IWRITE );
	_write(f, STR_HEAD(text), LEN_BYTES(STR_HEAD(text)));
	_close(f);
#endif

	SAVE_SERIES(text);
	val = Do_String(text->data, flags);
	UNSAVE_SERIES(text);
	if (IS_ERROR(val)) // && (VAL_ERR_NUM(val) != RE_QUIT)) {
		Print_Value(val, 1000, FALSE, TRUE);

	if (result) {
		*result = Value_To_RXI(val);
		return Reb_To_RXT[VAL_TYPE(val)];
	}
	return 0;
}


/***********************************************************************
**
*/	RL_API int RL_Do_Block(REBSER *blk, REBCNT flags, RXIARG *result)
/*
**	Evaluate a block. (not implemented)
**
**	Returns:
**		The datatype of the result or zero if error in the encoding.
**	Arguments:
**		blk - A pointer to the block series
**		flags - set to zero for now
**		result - value returned from evaluation
**	Notes:
**		Not implemented. Contact Carl on R3 Chat if you think you
**		could use it for something.
**
***********************************************************************/
{
	return 0;
}


/***********************************************************************
**
*/	RL_API void RL_Do_Commands(REBSER *blk, REBCNT flags, REBCEC *context)
/*
**	Evaluate a block of extension commands at high speed.
**
**	Returns:
**		Nothing
**	Arguments:
**		blk - a pointer to the block series
**		flags - set to zero for now
**		context - command evaluation context struct or zero if not used.
**	Notes:
**		For command blocks only, not for other blocks.
**		The context allows passing to each command a struct that is
**		used for back-referencing your environment data or for tracking
**		the evaluation block and its index.
**
***********************************************************************/
{
	Do_Commands(blk, context);
}


/***********************************************************************
**
*/	RL_API void RL_Print(REBYTE *fmt, ...)
/*
**	Low level print of formatted data to the console.
**
**	Returns:
**		nothing
**	Arguments:
**		fmt - A format string similar but not identical to printf.
**			Special options are available.
**		... - Values to be formatted.
**	Notes:
**		This function is low level and handles only a few C datatypes
**		at this time.
**
***********************************************************************/
{
	va_list args;
	va_start(args, fmt);
	Debug_Buf(NO_LIMIT, fmt, args); // Limits line size
	va_end(args);
}


/***********************************************************************
**
*/	RL_API void RL_Print_TOS(REBCNT flags, REBYTE *marker)
/*
**	Print top REBOL stack value to the console. (pending changes)
**
**	Returns:
**		Nothing
**	Arguments:
**		flags - special flags (set to zero at this time).
**		marker - placed at beginning of line to indicate output.
**	Notes:
**		This function is used for the main console evaluation
**		input loop to print the results of evaluation from stack.
**		The REBOL data stack is an abstract structure that can
**		change between releases. This function allows the host
**		to print the result of processed functions.
**		Note that what is printed is actually TOS+1.
**		Marker is usually "==" to show output.
**		The system/options/result-types determine which values
**		are automatically printed.
**
***********************************************************************/
{
	REBINT dsp = DSP;
	REBVAL *top = DS_VALUE(dsp+1);
	REBOL_STATE state;
	REBVAL *types;
	REBVAL *last;

	if (dsp != 0) Debug_Fmt(Str_Stack_Misaligned, dsp);

	PUSH_STATE(state, Saved_State);
	if (SET_JUMP(state)) {
		POP_STATE(state, Saved_State);
		Catch_Error(DS_NEXT); // Stores error value here
		Out_Value(DS_NEXT, 0, FALSE, 0, TRUE); // error
		DSP = 0;
		return;
	}
	SET_STATE(state, Saved_State);

	if (!IS_UNSET(top)) {
		if (!IS_ERROR(top)) {
			types = Get_System(SYS_OPTIONS, OPTIONS_RESULT_TYPES);
			if (IS_TYPESET(types) && TYPE_CHECK(types, VAL_TYPE(top))) {
				if (marker) {
					DS_SKIP; // protect `top` from modification
					Out_Str(marker, 0, FALSE);
					DS_DROP; 
				}
				Out_Value(top, 500, TRUE, 1, FALSE); // limit, molded
			}
//			else {
//				Out_Str(Get_Type_Name(top), 1);
//			}
		} else {
			if (VAL_ERR_NUM(top) != RE_HALT) {
#ifdef COLOR_CONSOLE 
				Out_Str(cb_cast("\x1B[1;35m"), 0, TRUE);
				Out_Value(top, 640, FALSE, 0, TRUE); // error FORMed
				Out_Str(cb_cast("\x1B[0m"), 0, TRUE);
#else
				Out_Value(top, 640, FALSE, 0); // error FORMed
#endif
//				if (VAL_ERR_NUM(top) > RE_THROW_MAX) {
//					Out_Str("** Note: use WHY? for more about this error", 1, TRUE);
//				}
			}
		}
	}
	// store last result in system/state/last-result
	last = Get_System(SYS_STATE, STATE_LAST_RESULT);
	*last = *top;
	POP_STATE(state, Saved_State);
	DSP = 0;
}


/***********************************************************************
**
*/	RL_API int RL_Event(REBEVT *evt)
/*
**	Appends an application event (e.g. GUI) to the event port.
**
**	Returns:
**		Returns TRUE if queued, or FALSE if event queue is full.
**	Arguments:
**		evt - A properly initialized event structure. The
**			contents of this structure are copied as part of
**			the function, allowing use of locals.
**	Notes:
**		Sets a signal to get REBOL attention for WAIT and awake.
**		To avoid environment problems, this function only appends
**		to the event queue (no auto-expand). So if the queue is full
**
***********************************************************************/
{
	REBVAL *event = Append_Event();		// sets signal

	if (event) {						// null if no room left in series
		VAL_SET(event, REB_EVENT);		// (has more space, if we need it)
		event->data.event = *evt;
		return 1;
	}

	return 0;
}

/***********************************************************************
**
*/	RL_API int RL_Update_Event(REBEVT *evt)
/*
**	Updates an application event (e.g. GUI) to the event port.
**
**	Returns:
**		Returns 1 if updated, or 0 if event appended, and -1 if full.
**	Arguments:
**		evt - A properly initialized event structure. The
**			 model and type of the event are used to address 
**           the unhandled event in the queue, when it is found,
**           it will be replaced with this one
**
***********************************************************************/

{
	REBVAL *event = Find_Event(evt->model, evt->type, evt->ser);

	if (event) {
		event->data.event = *evt;
		return 1;
	}
	
	return RL_Event(evt) - 1;
}

RL_API void *RL_Make_Block(u32 size)
/*
**	Allocate a new block.
**
**	Returns:
**		A pointer to a block series.
**	Arguments:
**		size - the length of the block. The system will add one extra
**			for the end-of-block marker.
**	Notes:
**		Blocks are allocated with REBOL's internal memory manager.
**		Internal structures may change, so NO assumptions should be made!
**		Blocks are automatically garbage collected if there are
**		no references to them from REBOL code (C code does nothing.)
**		However, you can lock blocks to prevent deallocation. (?? default)
*/
{
	return Make_Block(size);
}

RL_API void RL_Expand_Series(REBSER *series, REBCNT index, REBCNT delta)
/*
**	Expand a series at a particular index point by the number
**	number of units specified by delta.
**
**	Returns:
**		
**	Arguments:
**		series - series to expand
**		index - position where to expand
**		delta - number of UNITS to expand from TAIL (keeping terminator)
*/
{
	Expand_Series(series, index, delta);
}


RL_API void *RL_Make_String(u32 size, int unicode)
/*
**	Allocate a new string or binary series.
**
**	Returns:
**		A pointer to a string or binary series.
**	Arguments:
**		size - the length of the string. The system will add one extra
**			for a null terminator (not strictly required, but good for C.)
**		unicode - set FALSE for ASCII/Latin1 strings, set TRUE for Unicode.
**	Notes:
**		Strings can be REBYTE or REBCHR sized (depends on R3 config.)
**		Strings are allocated with REBOL's internal memory manager.
**		Internal structures may change, so NO assumptions should be made!
**		Strings are automatically garbage collected if there are
**		no references to them from REBOL code (C code does nothing.)
**		However, you can lock strings to prevent deallocation. (?? default)
*/
{
	return unicode ? Make_Unicode(size) : Make_Binary(size);
}

RL_API void *RL_Make_Image(u32 width, u32 height)
/*
**	Allocate a new image of the given size.
**
**	Returns:
**		A pointer to an image series, or zero if size is too large.
**	Arguments:
**		width - the width of the image in pixels
**		height - the height of the image in lines
**	Notes:
**		Images are allocated with REBOL's internal memory manager.
**		Image are automatically garbage collected if there are
**		no references to them from REBOL code (C code does nothing.)
*/
{
	return Make_Image(width, height, FALSE);
}

RL_API void *RL_Make_Vector(REBINT type, REBINT sign, REBINT dims, REBINT bits, REBINT size)
/*
**	Allocate a new vector of the given attributes.
**
**	Returns:
**		A pointer to a vector series or zero.
**	Arguments:
**		type: the datatype
**		sign: signed or unsigned
**		dims: number of dimensions
**		bits: number of bits per unit (8, 16, 32, 64)
**		size: number of values
**	Notes:
**		Allocated with REBOL's internal memory manager.
**		Vectors are automatically garbage collected if there are
**		no references to them from REBOL code (C code does nothing.)
*/
{
	// check if bits is valid 
	if(!(bits == 8 || bits == 16 || bits == 32 || bits == 64)) return 0;
	return Make_Vector(type, sign, dims, bits, size);
}

RL_API void RL_Protect_GC(REBSER *series, u32 flags)
/*
**	Protect memory from garbage collection.
**
**	Returns:
**		nothing
**	Arguments:
**		series - a series to protect (block, string, image, ...)
**		flags - set to 1 to protect, 0 to unprotect
**	Notes:
**		You should only use this function when absolutely necessary,
**		because it bypasses garbage collection for the specified series.
**		Meaning: if you protect a series, it will never be freed.
**		Also, you only need this function if you allocate several series
**		such as strings, blocks, images, etc. within the same command
**		and you don't store those references somewhere where the GC can
**		find them, such as in an existing block or object (variable).
*/
{
	(flags == 1) ? SERIES_SET_FLAG(series, SER_KEEP) : SERIES_CLR_FLAG(series, SER_KEEP);
}

RL_API int RL_Get_String(REBSER *series, u32 index, void **str, REBOOL needs_wide)
/*
**	Obtain a pointer into a string (bytes or unicode).
**
**	Returns:
**		The length and type of string. When len > 0, string is unicode.
**		When len < 0, string is bytes.
**  Arguments:
**		series - string series pointer
**		index - index from beginning (zero-based)
**		str   - pointer to first character
**		needs_wide - unicode string is required, converts if needed
**	Notes:
**		If the len is less than zero, then the string is optimized to
**		codepoints (chars) 255 or less for ASCII and LATIN-1 charsets.
**		Strings are allowed to move in memory. Therefore, you will want
**		to make a copy of the string if needed.
*/
{	// ret: len or -len
	int len = (index >= series->tail) ? 0 : series->tail - index;

	if (BYTE_SIZE(series)) {
		if (needs_wide) {
			Widen_String(series);
		} else {
			*str = BIN_SKIP(series, index);
			return -len;
		}
	}
	*str = UNI_SKIP(series, index);
	return len;
}

RL_API int RL_Get_UTF8_String(REBSER *series, u32 index, void **str)
/*
**	Obtain a pointer into an UTF8 encoded string.
**
**	Returns:
**		The length of string is bytes.
**  Arguments:
**		series - string series pointer
**		index - index from beginning (zero-based)
**		str   - pointer to first character
**	Notes:
**		Strings are allowed to move in memory. Therefore, you will want
**		to make a copy of the string if needed.
*/
{
	int len = (index >= series->tail) ? 0 : series->tail - index;

	if (BYTE_SIZE(series)) {
		*str = BIN_SKIP(series, index);
	}
	else {
		REBSER *ser = Encode_UTF8_String((void*)UNI_SKIP(series, index), len, TRUE, 0);
		*str = BIN_HEAD(ser);
		len = BIN_LEN(ser);
	}

	return len;
}

RL_API u32 RL_Map_Word(REBYTE *string)
/*
**	Given a word as a string, return its global word identifier.
**
**	Returns:
**		The word identifier that matches the string.
**	Arguments:
**		string - a valid word as a UTF-8 encoded string.
**	Notes:
**		Word identifiers are persistent, and you can use them anytime.
**		If the word is new (not found in master symbol table)
**		it will be added and the new word identifier is returned.
*/
{
	return Make_Word(string, 0);
}

RL_API u32 *RL_Map_Words(REBSER *series)
/*
**	Given a block of word values, return an array of word ids.
**
**	Returns:
**		An array of global word identifiers (integers). The [0] value is the size.
**	Arguments:
**		series - block of words as values (from REBOL blocks, not strings.)
**	Notes:
**		Word identifiers are persistent, and you can use them anytime.
**		The block can include any kind of word, including set-words, lit-words, etc.
**		If the input block contains non-words, they will be skipped.
**		The array is allocated with OS_MAKE and you can OS_FREE it any time.
*/
{
	REBCNT i = 1;
	u32 *words;
	REBVAL *val = BLK_HEAD(series);

	words = OS_MAKE((series->tail+2) * sizeof(u32));

	for (; NOT_END(val); val++) {
		if (ANY_WORD(val)) words[i++] = VAL_WORD_CANON(val);
	}

	words[0] = i;
	words[i] = 0;

	return words;
}

RL_API REBYTE *RL_Word_String(u32 word)
/*
**	Return a string related to a given global word identifier.
**
**	Returns:
**		A copy of the word string, null terminated.
**	Arguments:
**		word - a global word identifier
**	Notes:
**		The result is a null terminated copy of the name for your own use.
**		The string is always UTF-8 encoded (chars > 127 are encoded.)
**		In this API, word identifiers are always canonical. Therefore,
**		the returned string may have different spelling/casing than expected.
**		The string is allocated with OS_MAKE and you can OS_FREE it any time.
*/
{
	REBYTE *s1, *s2;
	s1 = Get_Sym_Name(word);
	s2 = OS_MAKE(strlen(cs_cast(s1)) + 1);
	strcpy(s_cast(s2), cs_cast(s1));
	return s2;
}

RL_API u32 RL_Find_Word(u32 *words, u32 word)
/*
**	Given an array of word ids, return the index of the given word.
**
**	Returns:
**		The index of the given word or zero.
**	Arguments:
**		words - a word array like that returned from MAP_WORDS (first element is size)
**		word - a word id
**	Notes:
**		The first element of the word array is the length of the array.
*/
{
	REBCNT n = 0;

	if (words == 0) return 0;

	for (n = 1; n < words[0]; n++) {
		if (words[n] == word) return n;
	}
	return 0;
}

RL_API REBUPT RL_Series(REBSER *series, REBCNT what)
/*
**	Get series information.
**
**	Returns:
**		Returns information related to a series.
**	Arguments:
**		series - any series pointer (string or block)
**		what - indicates what information to return (see RXI_SER enum)
**	Notes:
**		Invalid what arg nums will return zero.
*/
{
	switch (what) {
	case RXI_SER_DATA: return (REBUPT)SERIES_DATA(series);
	case RXI_SER_TAIL: return SERIES_TAIL(series);
	case RXI_SER_LEFT: return SERIES_AVAIL(series);
	case RXI_SER_SIZE: return SERIES_REST(series);
	case RXI_SER_WIDE: return SERIES_WIDE(series);
	}
	return 0;
}

RL_API int RL_Get_Char(REBSER *series, u32 index)
/*
**	Get a character from byte or unicode string.
**
**	Returns:
**		A Unicode character point from string. If index is
**		at or past the tail, a -1 is returned.
**	Arguments:
**		series - string series pointer
**		index - zero based index of character
**	Notes:
**		This function works for byte and unicoded strings.
**		The maximum size of a Unicode char is determined by
**		R3 build options. The default is 16 bits.
*/
{
	if (index >= series->tail) return -1;
	return GET_ANY_CHAR(series, index);
}

RL_API u32 RL_Set_Char(REBSER *series, u32 index, u32 chr)
/*
**	Set a character into a byte or unicode string.
**
**	Returns:
**		The index passed as an argument.
**	Arguments:
**		series - string series pointer
**		index - where to store the character. If past the tail,
**			the string will be auto-expanded by one and the char
**			will be appended.
*/
{
	if (index >= series->tail) {
		index = series->tail;
		EXPAND_SERIES_TAIL(series, 1);
	}
	SET_ANY_CHAR(series, index, chr);
	return index;
}

RL_API int RL_Get_Value_Resolved(REBSER *series, u32 index, RXIARG *result)
/*
**	Get a value from a block. If value is WORD or PATH, than its value is resolved.
**
**	Returns:
**		Datatype of value or zero if index is past tail.
**	Arguments:
**		series - block series pointer
**		index - index of the value in the block (zero based)
**		result - set to the value of the field
*/
{
	REBVAL *value;
	if (index >= series->tail) return 0;
	value = BLK_SKIP(series, index);
	switch (VAL_TYPE(value)) {
	case REB_WORD:
	case REB_GET_WORD:
		
		value = Get_Var(value);
		//printf("resolved type: %u\n", VAL_TYPE(value));
		break;
	case REB_PATH:
	case REB_GET_PATH:
		Do_Path(&value, NULL);
		value = DS_TOP; // only safe for short time!
		break;
	}
	*result = Value_To_RXI(value);
	return Reb_To_RXT[VAL_TYPE(value)];
}

RL_API int RL_Get_Value(REBSER *series, u32 index, RXIARG *result)
/*
**	Get a value from a block.
**
**	Returns:
**		Datatype of value or zero if index is past tail.
**	Arguments:
**		series - block series pointer
**		index - index of the value in the block (zero based)
**		result - set to the value of the field
*/
{
	REBVAL *value;
	if (index >= series->tail) return 0;
	value = BLK_SKIP(series, index);
	*result = Value_To_RXI(value);
	return Reb_To_RXT[VAL_TYPE(value)];
}

RL_API int RL_Set_Value(REBSER *series, u32 index, RXIARG val, int type)
/*
**	Set a value in a block.
**
**	Returns:
**		TRUE if index past end and value was appended to tail of block.
**	Arguments:
**		series - block series pointer
**		index - index of the value in the block (zero based)
**		val  - new value for field
**		type - datatype of value
*/
{
	REBVAL value = {0};
	RXI_To_Value(&value, val, type);
	if (index >= series->tail) {
		Append_Val(series, &value);
		return TRUE;
	}
	*BLK_SKIP(series, index) = value;
	return FALSE;
}

RL_API u32 *RL_Words_Of_Object(REBSER *obj)
/*
**	Returns information about the object.
**
**	Returns:
**		Returns an array of words used as fields of the object.
**	Arguments:
**		obj  - object pointer (e.g. from RXA_OBJECT)
**	Notes:
**		Returns a word array similar to MAP_WORDS().
**		The array is allocated with OS_MAKE. You can OS_FREE it any time.
*/
{
	REBCNT index;
	u32 *words;
	REBVAL *syms;

	syms = FRM_WORD(obj, 1);
	words = OS_MAKE(obj->tail * sizeof(u32)); // One less, because SELF not included.
	for (index = 0; index < (obj->tail-1); syms++, index++) {
		words[index] = VAL_BIND_CANON(syms);
	}
	words[index] = 0;
	return words;
}

RL_API int RL_Get_Field(REBSER *obj, u32 word, RXIARG *result)
/*
**	Get a field value (context variable) of an object.
**
**	Returns:
**		Datatype of value or zero if word is not found in the object.
**	Arguments:
**		obj  - object pointer (e.g. from RXA_OBJECT)
**		word - global word identifier (integer)
**		result - gets set to the value of the field
*/
{
	REBVAL *value;
	if (!(word = Find_Word_Index(obj, word, FALSE))) return 0;
	value = BLK_SKIP(obj, word);
	*result = Value_To_RXI(value);
	return Reb_To_RXT[VAL_TYPE(value)];
}

RL_API int RL_Set_Field(REBSER *obj, u32 word, RXIARG val, int type)
/*
**	Set a field (context variable) of an object.
**
**	Returns:
**		The type arg, or zero if word not found in object or if field is protected.
**	Arguments:
**		obj  - object pointer (e.g. from RXA_OBJECT)
**		word - global word identifier (integer)
**		val  - new value for field
**		type - datatype of value
*/
{
	//REBVAL value = {0};
	if (!(word = Find_Word_Index(obj, word, FALSE))) return 0;
	if (VAL_PROTECTED(FRM_WORDS(obj)+word)) return 0; //	Trap1(RE_LOCKED_WORD, word);
	RXI_To_Value(FRM_VALUES(obj)+word, val, type);
	return type;
}

RL_API int RL_Callback(RXICBI *cbi)
/*
**	Evaluate a REBOL callback function, either synchronous or asynchronous.
**
**	Returns:
**		Sync callback: type of the result; async callback: true if queued
**	Arguments:
**		cbi - callback information including special option flags,
**			object pointer (where function is located), function name
**			as global word identifier (within above object), argument list
**			passed to callback (see notes below), and result value.
**	Notes:
**		The flag value will determine the type of callback. It can be either
**		synchronous, where the code will re-enter the interpreter environment
**		and call the specified function, or asynchronous where an EVT_CALLBACK
**		event is queued, and the callback will be evaluated later when events
**		are processed within the interpreter's environment.
**		For asynchronous callbacks, the cbi and the args array must be managed
**		because the data isn't processed until the callback event is
**		handled. Therefore, these cannot be allocated locally on
**		the C stack; they should be dynamic (or global if so desired.)
**		See c:extensions-callbacks
*/
{
	REBEVT evt;

	// Synchronous callback?
	if (!GET_FLAG(cbi->flags, RXC_ASYNC)) {
		return Do_Callback(cbi->obj, cbi->word, cbi->args, &(cbi->result));
	}

	CLEARS(&evt);
	evt.type = EVT_CALLBACK;
	evt.model = EVM_CALLBACK;
	evt.ser = (void*)cbi;
	SET_FLAG(cbi->flags, RXC_QUEUED);

	return RL_Event(&evt);	// (returns 0 if queue is full, ignored)
}

RL_API REBCNT RL_Encode_UTF8(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG opts)
/*
**	Encode the unicode into UTF8 byte string.
**		
**	Returns:
**		Number of source chars used.
**		Updates len for dst bytes used.
**		Does not add a terminator.
**	Arguments:
**		max  - The maximum size of the result (UTF8).
**		uni  - Source string can be byte or unichar sized (uni = TRUE);
**		opts - Convert LF/CRLF
*/
{
	return Encode_UTF8(dst, max, src, len, uni, opts);
}

RL_API REBSER* RL_Encode_UTF8_String(void *src, REBCNT len, REBFLG uni, REBFLG opts)
/*
**	Encode the unicode into UTF8 byte string.
**
**	Returns:
**		Rebol series value with an UTF8 encoded data.
**	Arguments:
**		src  - series as a REBYTE or REBUNI.
**		len  - number of source bytes to convert.
**		uni  - Source string can be byte or unichar sized (uni = TRUE);
**		opts - Convert LF/CRLF
*/
{
	return Encode_UTF8_String(src, len, uni, opts);
}

RL_API REBSER* RL_Decode_UTF_String(REBYTE *src, REBCNT len, REBINT utf, REBFLG ccr, REBFLG uni)
/*
**	Decode the UTF8 encoded data into Rebol series.
**
**	Returns:
**		Rebol series with char size 1 or 2
**	Arguments:
**		src  - UTF8 encoded data
**		len  - number of source bytes to convert.
**		utf  - is 0, 8, +/-16, +/-32.
**		ccr  - Convert LF/CRLF
**		uni  - keep uni version even for plain ascii
*/
{
	return Decode_UTF_String(src, len, utf, ccr, uni);
}

/***********************************************************************
**
*/	RL_API REBCNT RL_Register_Handle(REBYTE *name, REBCNT size, void* free_func)
/*
**	Stores handle's specification (required data size and optional free callback.
**
**	Returns:
**		symbol id of the word (whether found or new)
**		or NOT_FOUND if handle with give ID is already registered.
**	Arguments:
**		name      - handle's name as a c-string (length is being detected)
**		size      - size of needed memory to handle
**		free_func - custom function to be called when handle is released
**
***********************************************************************/
{
	REBCNT sym;
	REBCNT len;
	REBCNT idx;

	// Convert C-string to Rebol word
	len = LEN_BYTES(name);
	sym = Scan_Word(name, len);
	if (!sym) return NOT_FOUND; //TODO: use different value if word is invalid?
	idx = Register_Handle(sym, size, (REB_HANDLE_FREE_FUNC)free_func);
	return (idx == NOT_FOUND) ? NOT_FOUND : sym;
}

RL_API REBHOB* RL_Make_Handle_Context(REBCNT sym)
/*
**	Allocates memory large enough to hold given handle's id
**
**	Returns:
**		A pointer to a Rebol's handle value.
**	Arguments:
**		sym - handle's word id
**
***********************************************************************/
{
	return Make_Handle_Context(sym);
}

RL_API void RL_Free_Handle_Context(REBHOB *hob)
/*
**	Frees memory of given handle's context
**
**	Returns:
**		nothing
**	Arguments:
**		hob - handle's context
**
***********************************************************************/
{
	Free_Hob(hob);
}


RL_API REBCNT RL_Decode_UTF8_Char(const REBYTE *str, REBCNT *len)
/*
**	Converts a single UTF8 code-point (to 32 bit).
**
**	Returns:
**		32 bit character code
**	Arguments:
**		src  - UTF8 encoded data
**		len  - number of source bytes consumed.
*/
{
	return  Decode_UTF8_Char(&str, len);
}

/***********************************************************************
**
*/	RL_API REBCNT RL_Register_Handle_Spec(REBYTE *name, REBHSP *spec)
/*
**	Stores handle's specification (required data size and optional callbacks).
**  It's an extended version of old RL_Register_Handle function.
**
**	Returns:
**		symbol id of the word (whether found or new)
**		or NOT_FOUND if handle with give ID is already registered.
**	Arguments:
**		name      - handle's name as a c-string (length is being detected)
**		spec      - Handle's specification:
**                  * size of needed memory to handle,
**                  * reserved flags
**                  * release function
**                  * get path accessor
**                  * set path accessor
**
***********************************************************************/
{
	REBCNT sym;
	REBCNT len;
	REBCNT idx;

	// Convert C-string to Rebol word
	len = LEN_BYTES(name);
	sym = Scan_Word(name, len);
	if (!sym) return NOT_FOUND; //TODO: use different value if word is invalid?
	idx = Register_Handle_Spec(sym, spec);
	return (idx == NOT_FOUND) ? NOT_FOUND : sym;
}


/***********************************************************************
**
*/	RL_API REBSER* RL_To_Local_Path(RXIARG *file, REBFLG full, REBFLG utf8)
/*
**	Convert REBOL filename to a local filename.
**
**	Returns:
**		A new series with the converted path or 0 on error.
**	Arguments:
**		file - Rebol file as an extension argument (series + index)
**		full - prepend current directory
**		utf8 - convert to UTF-8 if needed
**
***********************************************************************/
{
	REBSER *ser   = file->series;
	REBLEN  index = file->index;

	if (!ser || index > SERIES_TAIL(ser)) return 0;

	ser = To_Local_Path(SERIES_SKIP(ser, index), SERIES_TAIL(ser)-index, !BYTE_SIZE(ser), full);
	// To_Local_Path always returns REBUNI series
	if (ser && utf8) {
		ser = Encode_UTF8_String(SERIES_DATA(ser), SERIES_TAIL(ser), 1, 0);
	}
	return ser;
}

/***********************************************************************
**
*/	RL_API REBSER* RL_To_Rebol_Path(void *src, REBCNT len, REBINT uni)
/*
**	Convert local filename to a REBOL filename.
**
**	Returns:
**		A new series with the converted path or 0 on error.
**	Arguments:
**		ser - series as a REBYTE or REBUNI.
**		len - number of source bytes consumed.
**		uni - if series is REBYTE (0) or REBUNI (1)
**
***********************************************************************/
{
	return To_REBOL_Path(src, len, uni, 0);
}


#include "reb-lib-lib.h"

/***********************************************************************
**
*/	void *Extension_Lib(void)
/*
***********************************************************************/
{
	return &Ext_Lib;
}
