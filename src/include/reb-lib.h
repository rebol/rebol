/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**  This is a code-generated file.
**
************************************************************************
**
**  Title: REBOL Host and Extension API
**  Build: A0
**  Date:  14-Jan-2021
**  File:  reb-lib.reb
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-reb-lib.reb)
**
***********************************************************************/


// These constants are created by the release system and can be used to check
// for compatiblity with the reb-lib DLL (using RL_Version.)
#define RL_VER 3
#define RL_REV 4
#define RL_UPD 0

// Compatiblity with the lib requires that structs are aligned using the same
// method. This is concrete, not abstract. The macro below uses struct
// sizes to inform the developer that something is wrong.
#if defined(__LP64__) || defined(__LLP64__)
#define CHECK_STRUCT_ALIGN (sizeof(REBREQ) == 100 && sizeof(REBEVT) == 16)
#else
#define CHECK_STRUCT_ALIGN (sizeof(REBREQ) == 80 && sizeof(REBEVT) == 12)
#endif

// Function entry points for reb-lib (used for MACROS below):
typedef struct rebol_ext_api {
	void (*version)(REBYTE vers[]);
	int (*init)(REBARGS *rargs, void *lib);
	void (*dispose)(void);
	int (*start)(REBYTE *bin, REBINT len, REBCNT flags);
	void (*reset)(void);
	void *(*extend)(REBYTE *source, RXICAL call);
	void (*escape)(REBINT reserved);
	int (*do_string)(REBYTE *text, REBCNT flags, RXIARG *result);
	int (*do_binary)(REBYTE *bin, REBINT length, REBCNT flags, REBCNT key, RXIARG *result);
	int (*do_block)(REBSER *blk, REBCNT flags, RXIARG *result);
	void (*do_commands)(REBSER *blk, REBCNT flags, REBCEC *context);
	void (*print)(REBYTE *fmt, ...);
	void (*print_tos)(REBCNT flags, REBYTE *marker);
	int (*event)(REBEVT *evt);
	int (*update_event)(REBEVT *evt);
	void *(*make_block)(u32 size);
	void (*expand_series)(REBSER *series, REBCNT index, REBCNT delta);
	void *(*make_string)(u32 size, int unicode);
	void *(*make_image)(u32 width, u32 height);
	void *(*make_vector)(REBINT type, REBINT sign, REBINT dims, REBINT bits, REBINT size);
	void (*protect_gc)(REBSER *series, u32 flags);
	int (*get_string)(REBSER *series, u32 index, void **str, REBOOL needs_wide);
	int (*get_utf8_string)(REBSER *series, u32 index, void **str);
	u32 (*map_word)(REBYTE *string);
	u32 *(*map_words)(REBSER *series);
	REBYTE *(*word_string)(u32 word);
	u32 (*find_word)(u32 *words, u32 word);
	REBUPT (*series)(REBSER *series, REBCNT what);
	int (*get_char)(REBSER *series, u32 index);
	u32 (*set_char)(REBSER *series, u32 index, u32 chr);
	int (*get_value_resolved)(REBSER *series, u32 index, RXIARG *result);
	int (*get_value)(REBSER *series, u32 index, RXIARG *result);
	int (*set_value)(REBSER *series, u32 index, RXIARG val, int type);
	u32 *(*words_of_object)(REBSER *obj);
	int (*get_field)(REBSER *obj, u32 word, RXIARG *result);
	int (*set_field)(REBSER *obj, u32 word, RXIARG val, int type);
	int (*callback)(RXICBI *cbi);
	REBCNT (*encode_utf8)(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG opts);
	REBSER* (*encode_utf8_string)(void *src, REBCNT len, REBFLG uni, REBFLG opts);
	REBSER* (*decode_utf_string)(REBYTE *src, REBCNT len, REBINT utf, REBFLG ccr, REBFLG uni);
} RL_LIB;

// Extension entry point functions:
#ifdef TO_WINDOWS
#ifdef __cplusplus
#define RXIEXT extern "C" __declspec(dllexport)
#else
#define RXIEXT __declspec(dllexport)
#endif
#else
#define RXIEXT extern
#endif

RXIEXT const char *RX_Init(int opts, RL_LIB *lib);
RXIEXT int RX_Quit(int opts);
RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *data);

// The macros below will require this base pointer:
extern RL_LIB *RL;  // is passed to the RX_Init() function

// Macros to access reb-lib functions (from non-linked extensions):

#define RL_VERSION(a)               RL->version(a)
/*
**	void RL_Version(REBYTE vers[])
**
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
*/

#define RL_INIT(a,b)                RL->init(a,b)
/*
**	int RL_Init(REBARGS *rargs, void *lib)
**
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
*/

#define RL_DISPOSE(void)            RL->dispose(void)
/*
**	void RL_Dispose(void)
**
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
*/

#define RL_START(a,b,c)             RL->start(a,b,c)
/*
**	int RL_Start(REBYTE *bin, REBINT len, REBCNT flags)
**
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
*/

#define RL_RESET(void)              RL->reset(void)
/*
**	void RL_Reset(void)
**
**	Reset REBOL (not implemented)
**
**	Returns:
**		nothing
**	Arguments:
**		none
**	Notes:
**		Intended to reset the REBOL interpreter.
**
*/

#define RL_EXTEND(a,b)              RL->extend(a,b)
/*
**	void *RL_Extend(REBYTE *source, RXICAL call)
**
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
*/

#define RL_ESCAPE(a)                RL->escape(a)
/*
**	void RL_Escape(REBINT reserved)
**
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
*/

#define RL_DO_STRING(a,b,c)         RL->do_string(a,b,c)
/*
**	int RL_Do_String(REBYTE *text, REBCNT flags, RXIARG *result)
**
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
*/

#define RL_DO_BINARY(a,b,c,d,e)     RL->do_binary(a,b,c,d,e)
/*
**	int RL_Do_Binary(REBYTE *bin, REBINT length, REBCNT flags, REBCNT key, RXIARG *result)
**
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
*/

#define RL_DO_BLOCK(a,b,c)          RL->do_block(a,b,c)
/*
**	int RL_Do_Block(REBSER *blk, REBCNT flags, RXIARG *result)
**
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
*/

#define RL_DO_COMMANDS(a,b,c)       RL->do_commands(a,b,c)
/*
**	void RL_Do_Commands(REBSER *blk, REBCNT flags, REBCEC *context)
**
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
*/

#define RL_PRINT(a,b)               RL->print(a,b)
/*
**	void RL_Print(REBYTE *fmt, ...)
**
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
*/

#define RL_PRINT_TOS(a,b)           RL->print_tos(a,b)
/*
**	void RL_Print_TOS(REBCNT flags, REBYTE *marker)
**
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
*/

#define RL_EVENT(a)                 RL->event(a)
/*
**	int RL_Event(REBEVT *evt)
**
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
*/

#define RL_UPDATE_EVENT(a)          RL->update_event(a)
/*
**	int RL_Update_Event(REBEVT *evt)
**
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
*/

#define RL_MAKE_BLOCK(a)            RL->make_block(a)
/*
**	void *RL_Make_Block(u32 size)
**
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

#define RL_EXPAND_SERIES(a,b,c)     RL->expand_series(a,b,c)
/*
**	void RL_Expand_Series(REBSER *series, REBCNT index, REBCNT delta)
**
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

#define RL_MAKE_STRING(a,b)         RL->make_string(a,b)
/*
**	void *RL_Make_String(u32 size, int unicode)
**
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

#define RL_MAKE_IMAGE(a,b)          RL->make_image(a,b)
/*
**	void *RL_Make_Image(u32 width, u32 height)
**
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

#define RL_MAKE_VECTOR(a,b,c,d,e)   RL->make_vector(a,b,c,d,e)
/*
**	void *RL_Make_Vector(REBINT type, REBINT sign, REBINT dims, REBINT bits, REBINT size)
**
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

#define RL_PROTECT_GC(a,b)          RL->protect_gc(a,b)
/*
**	void RL_Protect_GC(REBSER *series, u32 flags)
**
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

#define RL_GET_STRING(a,b,c,d)      RL->get_string(a,b,c,d)
/*
**	int RL_Get_String(REBSER *series, u32 index, void **str, REBOOL needs_wide)
**
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

#define RL_GET_UTF8_STRING(a,b,c)   RL->get_utf8_string(a,b,c)
/*
**	int RL_Get_UTF8_String(REBSER *series, u32 index, void **str)
**
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

#define RL_MAP_WORD(a)              RL->map_word(a)
/*
**	u32 RL_Map_Word(REBYTE *string)
**
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

#define RL_MAP_WORDS(a)             RL->map_words(a)
/*
**	u32 *RL_Map_Words(REBSER *series)
**
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

#define RL_WORD_STRING(a)           RL->word_string(a)
/*
**	REBYTE *RL_Word_String(u32 word)
**
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

#define RL_FIND_WORD(a,b)           RL->find_word(a,b)
/*
**	u32 RL_Find_Word(u32 *words, u32 word)
**
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

#define RL_SERIES(a,b)              RL->series(a,b)
/*
**	REBUPT RL_Series(REBSER *series, REBCNT what)
**
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

#define RL_GET_CHAR(a,b)            RL->get_char(a,b)
/*
**	int RL_Get_Char(REBSER *series, u32 index)
**
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

#define RL_SET_CHAR(a,b,c)          RL->set_char(a,b,c)
/*
**	u32 RL_Set_Char(REBSER *series, u32 index, u32 chr)
**
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

#define RL_GET_VALUE_RESOLVED(a,b,c) RL->get_value_resolved(a,b,c)
/*
**	int RL_Get_Value_Resolved(REBSER *series, u32 index, RXIARG *result)
**
**	Get a value from a block. If value is WORD or PATH, than its value is resolved.
**
**	Returns:
**		Datatype of value or zero if index is past tail.
**	Arguments:
**		series - block series pointer
**		index - index of the value in the block (zero based)
**		result - set to the value of the field
*/

#define RL_GET_VALUE(a,b,c)         RL->get_value(a,b,c)
/*
**	int RL_Get_Value(REBSER *series, u32 index, RXIARG *result)
**
**	Get a value from a block.
**
**	Returns:
**		Datatype of value or zero if index is past tail.
**	Arguments:
**		series - block series pointer
**		index - index of the value in the block (zero based)
**		result - set to the value of the field
*/

#define RL_SET_VALUE(a,b,c,d)       RL->set_value(a,b,c,d)
/*
**	int RL_Set_Value(REBSER *series, u32 index, RXIARG val, int type)
**
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

#define RL_WORDS_OF_OBJECT(a)       RL->words_of_object(a)
/*
**	u32 *RL_Words_Of_Object(REBSER *obj)
**
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

#define RL_GET_FIELD(a,b,c)         RL->get_field(a,b,c)
/*
**	int RL_Get_Field(REBSER *obj, u32 word, RXIARG *result)
**
**	Get a field value (context variable) of an object.
**
**	Returns:
**		Datatype of value or zero if word is not found in the object.
**	Arguments:
**		obj  - object pointer (e.g. from RXA_OBJECT)
**		word - global word identifier (integer)
**		result - gets set to the value of the field
*/

#define RL_SET_FIELD(a,b,c,d)       RL->set_field(a,b,c,d)
/*
**	int RL_Set_Field(REBSER *obj, u32 word, RXIARG val, int type)
**
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

#define RL_CALLBACK(a)              RL->callback(a)
/*
**	int RL_Callback(RXICBI *cbi)
**
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

#define RL_ENCODE_UTF8(a,b,c,d,e,f) RL->encode_utf8(a,b,c,d,e,f)
/*
**	REBCNT RL_Encode_UTF8(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG opts)
**
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

#define RL_ENCODE_UTF8_STRING(a,b,c,d) RL->encode_utf8_string(a,b,c,d)
/*
**	REBSER* RL_Encode_UTF8_String(void *src, REBCNT len, REBFLG uni, REBFLG opts)
**
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

#define RL_DECODE_UTF_STRING(a,b,c,d,e) RL->decode_utf_string(a,b,c,d,e)
/*
**	REBSER* RL_Decode_UTF_String(REBYTE *src, REBCNT len, REBINT utf, REBFLG ccr, REBFLG uni)
**
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



#define RL_MAKE_BINARY(s) RL_MAKE_STRING(s, FALSE)

#ifndef REB_EXT // not extension lib, use direct calls to r3lib

RL_API void RL_Version(REBYTE vers[]);
RL_API int RL_Init(REBARGS *rargs, void *lib);
RL_API void RL_Dispose(void);
RL_API int RL_Start(REBYTE *bin, REBINT len, REBCNT flags);
RL_API void RL_Reset(void);
RL_API void *RL_Extend(REBYTE *source, RXICAL call);
RL_API void RL_Escape(REBINT reserved);
RL_API int RL_Do_String(REBYTE *text, REBCNT flags, RXIARG *result);
RL_API int RL_Do_Binary(REBYTE *bin, REBINT length, REBCNT flags, REBCNT key, RXIARG *result);
RL_API int RL_Do_Block(REBSER *blk, REBCNT flags, RXIARG *result);
RL_API void RL_Do_Commands(REBSER *blk, REBCNT flags, REBCEC *context);
RL_API void RL_Print(REBYTE *fmt, ...);
RL_API void RL_Print_TOS(REBCNT flags, REBYTE *marker);
RL_API int RL_Event(REBEVT *evt);
RL_API int RL_Update_Event(REBEVT *evt);
RL_API void *RL_Make_Block(u32 size);
RL_API void RL_Expand_Series(REBSER *series, REBCNT index, REBCNT delta);
RL_API void *RL_Make_String(u32 size, int unicode);
RL_API void *RL_Make_Image(u32 width, u32 height);
RL_API void *RL_Make_Vector(REBINT type, REBINT sign, REBINT dims, REBINT bits, REBINT size);
RL_API void RL_Protect_GC(REBSER *series, u32 flags);
RL_API int RL_Get_String(REBSER *series, u32 index, void **str, REBOOL needs_wide);
RL_API int RL_Get_UTF8_String(REBSER *series, u32 index, void **str);
RL_API u32 RL_Map_Word(REBYTE *string);
RL_API u32 *RL_Map_Words(REBSER *series);
RL_API REBYTE *RL_Word_String(u32 word);
RL_API u32 RL_Find_Word(u32 *words, u32 word);
RL_API REBUPT RL_Series(REBSER *series, REBCNT what);
RL_API int RL_Get_Char(REBSER *series, u32 index);
RL_API u32 RL_Set_Char(REBSER *series, u32 index, u32 chr);
RL_API int RL_Get_Value_Resolved(REBSER *series, u32 index, RXIARG *result);
RL_API int RL_Get_Value(REBSER *series, u32 index, RXIARG *result);
RL_API int RL_Set_Value(REBSER *series, u32 index, RXIARG val, int type);
RL_API u32 *RL_Words_Of_Object(REBSER *obj);
RL_API int RL_Get_Field(REBSER *obj, u32 word, RXIARG *result);
RL_API int RL_Set_Field(REBSER *obj, u32 word, RXIARG val, int type);
RL_API int RL_Callback(RXICBI *cbi);
RL_API REBCNT RL_Encode_UTF8(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG opts);
RL_API REBSER* RL_Encode_UTF8_String(void *src, REBCNT len, REBFLG uni, REBFLG opts);
RL_API REBSER* RL_Decode_UTF_String(REBYTE *src, REBCNT len, REBINT utf, REBFLG ccr, REBFLG uni);

#endif
