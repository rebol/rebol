/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2021 Rebol Open Source Contributors
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
**  Module:  b-init.c
**  Summary: initialization functions
**  Section: bootstrap
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#define EVAL_DOSE 10000

// Boot Vars used locally:
static	REBCNT	Native_Count;
static	REBCNT	Native_Limit;
static	REBCNT	Action_Count;
static	REBCNT	Action_Marker;
static	REBFUN  *Native_Functions;
static	BOOT_BLK *Boot_Block;

extern const REBYTE Str_Banner[];

#ifdef WATCH_BOOT
#define DOUT(s) puts(s)
#else
#define DOUT(s)
#endif


/***********************************************************************
**
*/	static void Assert_Basics(void)
/*
***********************************************************************/
{
	REBVAL val;

	VAL_SET(&val, 123);
#ifdef WATCH_BOOT
	printf("TYPE(123)=%d val=%u dat=%u gob=%u\n",
		VAL_TYPE(&val), (REBCNT)sizeof(REBVAL), (REBCNT)sizeof(REBDAT), (REBCNT)sizeof(REBGOB));
#endif

#ifdef SHOW_SIZEOFS
	// For debugging ports to some systems:
	printf("%u %s\n", (REBCNT)sizeof(REBVAL), "rebval");
	printf("%u %s\n", (REBCNT)sizeof(REBSER), "rebser");
	printf("%u %s\n", (REBCNT)sizeof(REBWRD), "word");
	printf("%u %s\n", (REBCNT)sizeof(REBSRI), "series");
	printf("%u %s\n", (REBCNT)sizeof(REBCNT), "logic");
	printf("%u %s\n", (REBCNT)sizeof(REBI64), "integer");
	printf("%u %s\n", (REBCNT)sizeof(REBU64), "unteger");
	printf("%u %s\n", (REBCNT)sizeof(REBINT), "int32");
	printf("%u %s\n", (REBCNT)sizeof(REBDEC), "decimal");
	printf("%u %s\n", (REBCNT)sizeof(REBUNI), "uchar");
	printf("%u %s\n", (REBCNT)sizeof(REBERR), "error");
	printf("%u %s\n", (REBCNT)sizeof(REBTYP), "datatype");
	printf("%u %s\n", (REBCNT)sizeof(REBFRM), "frame");
	printf("%u %s\n", (REBCNT)sizeof(REBWRS), "wordspec");
	printf("%u %s\n", (REBCNT)sizeof(REBTYS), "typeset");
	printf("%u %s\n", (REBCNT)sizeof(REBSYM), "symbol");
	printf("%u %s\n", (REBCNT)sizeof(REBTIM), "time");
	printf("%u %s\n", (REBCNT)sizeof(REBTUP), "tuple");
	printf("%u %s\n", (REBCNT)sizeof(REBFCN), "func");
	printf("%u %s\n", (REBCNT)sizeof(REBOBJ), "object");
	printf("%u %s\n", (REBCNT)sizeof(REBXYF), "pair");
	printf("%u %s\n", (REBCNT)sizeof(REBEVT), "event");
	printf("%u %s\n", (REBCNT)sizeof(REBLIB), "library");
	printf("%u %s\n", (REBCNT)sizeof(REBROT), "routine");
	printf("%u %s\n", (REBCNT)sizeof(REBSTU), "structure");
	printf("%u %s\n", (REBCNT)sizeof(REBGOB), "gob");
	printf("%u %s\n", (REBCNT)sizeof(REBGBO), "gbo");
	printf("%u %s\n", (REBCNT)sizeof(REBUDT), "utype");
	printf("%u %s\n", (REBCNT)sizeof(REBDCI), "deci");
	printf("%u %s\n", (REBCNT)sizeof(REBHAN), "handle");
	printf("%u %s\n", (REBCNT)sizeof(REBALL), "all");
#endif

	ASSERT(VAL_TYPE(&val) == 123,  RP_REBVAL_ALIGNMENT);
	if (sizeof(void *) == 8) {
		ASSERT(sizeof(REBVAL) == 32,   RP_REBVAL_ALIGNMENT);
		//ASSERT1(sizeof(REBGOB) == 84,  RP_BAD_SIZE);
	} else {
		ASSERT(sizeof(REBVAL) == 16,   RP_REBVAL_ALIGNMENT);
		//ASSERT1(sizeof(REBGOB) == 64,  RP_BAD_SIZE);
	}
	ASSERT1(sizeof(REBDAT) == 4,   RP_BAD_SIZE);
}


/***********************************************************************
**
*/	static void Print_Banner(REBARGS *rargs)
/*
***********************************************************************/
{
	if (rargs->options & RO_VERS) {
		Out_Str(cb_cast(REBOL_VERSION), 0, FALSE);
		OS_EXIT(0);
	}
}


/***********************************************************************
**
*/	static void Do_Global_Block(REBSER *block, REBINT rebind)
/*
**		Bind and evaluate a global block.
**		Rebind:
**			0: bind set into sys or lib
**		   -1: bind shallow into sys (for NATIVE and ACTION)
**			1: add new words to LIB, bind/deep to LIB
**			2: add new words to SYS, bind/deep to LIB
**		A single result is left on top of data stack (may be an error).
**
***********************************************************************/
{
	Bind_Block(rebind > 1 ? Sys_Context : Lib_Context, BLK_HEAD(block), BIND_SET);
	if (rebind < 0) Bind_Block(Sys_Context, BLK_HEAD(block), 0);
	if (rebind > 0) Bind_Block(Lib_Context, BLK_HEAD(block), BIND_DEEP);
	if (rebind > 1) Bind_Block(Sys_Context, BLK_HEAD(block), BIND_DEEP);
	Do_Blk(block, 0);
}


/***********************************************************************
**
*/	static void Load_Boot(void)
/*
**		Decompress and scan in the boot block structure.  Can
**		only be called at the correct point because it will
**		create new symbols.
**
***********************************************************************/
{
	REBSER *boot;

	// Decompress binary data in Native_Specs to get the textual source
	// of the function specs of the native routines.  (This compressed
	// array lives in b-boot.c which is generated by make-boot.r)
	// Then load that into a Rebol series as `boot`.  Note that the
	// first four bytes of Native_Specs is a little-endian 32-bit
	// length of the uncompressed spec data.
	{
		REBSER spec;
		REBSER *text;
		REBCNT textlen;

		// REVIEW: This is a nasty casting away of a const.  But there's
		// nothing that can be done about it as long as Decompress takes
		// a REBSER, as the data field is not const
		spec.data = ((REBYTE*)Native_Specs) + 4;
		spec.tail = NAT_SPEC_SIZE;

		textlen = Bytes_To_REBCNT(Native_Specs);
		text = DecompressZlib(&spec, 0, -1, textlen, 0);
		if (!text || (STR_LEN(text) != textlen)) Crash(RP_BOOT_DATA);
		boot = Scan_Source(STR_HEAD(text), textlen);
		//Dump_Block_Raw(boot, 0, 2);
		Free_Series(text);
	}

	Set_Root_Series(ROOT_BOOT, boot, cb_cast("boot block"));	// Do not let it get GC'd

	Boot_Block = (BOOT_BLK *)VAL_BLK(BLK_HEAD(boot));

	ASSERT(VAL_TAIL(&Boot_Block->types) == REB_MAX, RP_BAD_BOOT_TYPE_BLOCK);
	ASSERT(VAL_WORD_SYM(VAL_BLK(&Boot_Block->types)) == SYM_END_TYPE, RP_BAD_END_TYPE_WORD);

	// Create low-level string pointers (used by RS_ constants):
	{
		REBYTE *cp;
		REBINT i;

		PG_Boot_Strs = (REBYTE **)Make_Mem(RS_MAX * sizeof(REBYTE *));
		*ROOT_STRINGS = Boot_Block->strings;
		cp = VAL_BIN(ROOT_STRINGS);
		for (i = 0; i < RS_MAX; i++) {
			BOOT_STR(i,0) = cp;
			while (*cp++);
		}
	}

	ASSERT(!CMP_BYTES(cb_cast("end!"), Get_Sym_Name(SYM_END_TYPE)), RP_BAD_END_CANON_WORD);
	ASSERT(!CMP_BYTES(cb_cast("true"), Get_Sym_Name(SYM_TRUE)), RP_BAD_TRUE_CANON_WORD);
	ASSERT(!CMP_BYTES(cb_cast("line"), BOOT_STR(RS_SCAN,1)), RP_BAD_BOOT_STRING);
}


/***********************************************************************
**
*/	static void Init_Datatypes(void)
/*
**		Create the datatypes.
**
***********************************************************************/
{
	REBVAL *word = VAL_BLK(&Boot_Block->types);
	REBSER *specs = VAL_SERIES(&Boot_Block->typespecs);
	REBVAL *value;
	REBINT n;

	for (n = 0; NOT_END(word); word++, n++) {
		value = Append_Frame(Lib_Context, word, 0);
		VAL_SET(value, REB_DATATYPE);
		VAL_DATATYPE(value) = n;
		VAL_TYPE_SPEC(value) = VAL_SERIES(BLK_SKIP(specs, n));
	}
}


/***********************************************************************
**
*/	static void Init_Datatype_Checks(void)
/*
**		Create datatype test functions (e.g. integer?, time?, etc)
**		Must be done after typesets are initialized, so this cannot
**		be merged with the above.
**
***********************************************************************/
{
	REBVAL *word = VAL_BLK(&Boot_Block->types);
	REBVAL *value;
	REBSER *spec;
	REBCNT sym;
	REBINT n = 1;
	REBYTE str[32];

	spec = VAL_SERIES(VAL_BLK(&Boot_Block->booters));

	for (word++; NOT_END(word); word++, n++) {
		COPY_BYTES(str, Get_Word_Name(word), 32);
		str[31] = '\0';
		str[LEN_BYTES(str)-1] = '?';
		sym = Make_Word(str, 0);
		//Print("sym: %s", Get_Sym_Name(sym));
		value = Append_Frame(Lib_Context, 0, sym);
		VAL_INT64(BLK_LAST(spec)) = n;  // special datatype id location
		Make_Native(value, Copy_Block(spec, 0), (REBFUN)A_TYPE, REB_ACTION);
	}

	value = Append_Frame(Lib_Context, 0, SYM_DATATYPES);
	*value = Boot_Block->types;
}


/***********************************************************************
**
*/	static void Init_Constants(void)
/*
**		Init constant words.
**
**		WARNING: Do not create direct pointers into the Lib_Context
**		because it may get expanded and the pointers will be invalid.
**
***********************************************************************/
{
	REBVAL *value;
	extern const double pi1;

	value = Append_Frame(Lib_Context, 0, SYM_NONE);
	SET_NONE(value);

	value = Append_Frame(Lib_Context, 0, SYM_TRUE);
	SET_LOGIC(value, TRUE);

	value = Append_Frame(Lib_Context, 0, SYM_FALSE);
	SET_LOGIC(value, FALSE);

	value = Append_Frame(Lib_Context, 0, SYM_PI);
	SET_DECIMAL(value, pi1);
}


/***********************************************************************
**
*/	void Use_Natives(REBFUN *funcs, REBCNT limit)
/*
**		Setup to use NATIVE function. If limit == 0, then the
**		native function table will be zero terminated (N_native).
**
***********************************************************************/
{
	Native_Count = 0;
	Native_Limit = limit;
	Native_Functions = funcs;
}


/***********************************************************************
**
*/	REBNATIVE(native)
/*
***********************************************************************/
{
	if ((Native_Limit == 0 && *Native_Functions) || (Native_Count < Native_Limit))
		Make_Native(ds, VAL_SERIES(D_ARG(1)), *Native_Functions++, REB_NATIVE);
	else Trap0(RE_MAX_NATIVES);
	Native_Count++;
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(action)
/*
***********************************************************************/
{
	Action_Count++;
	if (Action_Count >= A_MAX_ACTION) Crash(RP_ACTION_OVERFLOW);
	Make_Native(ds, VAL_SERIES(D_ARG(1)), (REBFUN)(REBUPT)Action_Count, REB_ACTION);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(object)
/*
**		The spec block has already been bound to Lib_Context, to
**		allow any embedded values and functions to evaluate.
**
***********************************************************************/
{
	REBVAL *spec = D_ARG(1);

	SET_OBJECT(ds, Make_Object(0, VAL_BLK(spec)));
	Bind_Block(VAL_OBJ_FRAME(ds), VAL_BLK(spec), D_REF(2)?BIND_ONLY:BIND_DEEP); // not deep
	Do_Blk(VAL_SERIES(spec), 0); // result ignored
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(version)
/*
//	version: native [
//		"Return Rebol version string"
//	]
***********************************************************************/
{
	const REBYTE*version = BOOT_STR(RS_VERSION, 0);
	Set_String(ds, Copy_Bytes(version, strlen(cs_cast(version))));
	return R_RET;
}


/***********************************************************************
**
*/	static void Init_Ops(void)
/*
***********************************************************************/
{
	REBVAL *word;
	REBVAL *func;
	REBVAL *val;

	for (word = VAL_BLK(&Boot_Block->ops); NOT_END(word); word+=2) {
		// Append the operator name to the lib frame:
		val = Append_Frame(Lib_Context, word, 0);
		// Find the related function:
		func = Find_Word_Value(Lib_Context, VAL_WORD_SYM(word+1));
		if (!func) Crash(9912);
		*val = *func;
		VAL_SET(val, REB_OP);
		VAL_SET_EXT(val, VAL_TYPE(func));
	}
}


/***********************************************************************
**
*/	static void Init_Natives(void)
/*
**		Create native functions.
**
***********************************************************************/
{
	REBVAL *word;
	REBVAL *val;

	Action_Count = 0;
	Use_Natives((REBFUN *)Native_Funcs, MAX_NATS);

	// Construct the first native, which is the NATIVE function creator itself:
	// native: native [spec [block!]]
	word = VAL_BLK_SKIP(&Boot_Block->booters, 1);
	ASSERT2(IS_SET_WORD(word) && VAL_WORD_SYM(word) == SYM_NATIVE, RE_NATIVE_BOOT);
	//val = BLK_SKIP(Sys_Context, SYS_CTX_NATIVE);
	val = Append_Frame(Lib_Context, word, 0);
	Make_Native(val, VAL_SERIES(word+2), Native_Functions[0], REB_NATIVE);

	word += 3; // action: native []
	//val = BLK_SKIP(Sys_Context, SYS_CTX_ACTION);
	val = Append_Frame(Lib_Context, word, 0);
	Make_Native(val, VAL_SERIES(word+2), Native_Functions[1], REB_NATIVE);
	Native_Count = 2;
	Native_Functions += 2;

	Action_Marker = SERIES_TAIL(Lib_Context)-1; // Save index for action words.
	Do_Global_Block(VAL_SERIES(&Boot_Block->actions), -1);
	Do_Global_Block(VAL_SERIES(&Boot_Block->natives), -1);
}


/***********************************************************************
**
*/	REBVAL *Get_Action_Word(REBCNT action)
/*
**		Return the word symbol for a given Action number.
**
***********************************************************************/
{
	return FRM_WORD(Lib_Context, Action_Marker+action);
}


/***********************************************************************
**
*/	REBVAL *Get_Action_Value(REBCNT action)
/*
**		Return the value (function) for a given Action number.
**
***********************************************************************/
{
	return FRM_VALUE(Lib_Context, Action_Marker+action);
}


/***********************************************************************
**
*/	void Init_UType_Proto(void)
/*
**		Create prototype func object for UTypes.
**
***********************************************************************/
{
	REBSER *frm = Make_Frame(A_MAX_ACTION-1);
	REBVAL *obj;
	REBINT n;

	Insert_Series(FRM_WORD_SERIES(frm), 1, (REBYTE*)FRM_WORD(Lib_Context, Action_Marker+1), A_MAX_ACTION); 

	SERIES_TAIL(frm) = A_MAX_ACTION;
	for (n = 1; n < A_MAX_ACTION; n++)
		SET_NONE(BLK_SKIP(frm, n));
	BLK_TERM(frm);

	obj = Get_System(SYS_STANDARD, STD_UTYPE);
	SET_OBJECT(obj, frm);
}


/***********************************************************************
**
*/	static void Init_Data_Stack(REBCNT size)
/*
***********************************************************************/
{
	DS_Series = Make_Block(size);
	Set_Root_Series(TASK_STACK, DS_Series, cb_cast("data stack")); // uses special GC
	DS_Base = BLK_HEAD(DS_Series);
	DSP = DSF = 0;
	SET_NONE(DS_TOP); // avoids it being set to END (GC problem)
}


/***********************************************************************
**
*/	static void Init_Root_Context(void)
/*
**		Hand-build the root context where special REBOL values are
**		stored. Called early, so it cannot depend on any other
**		system structures or values.
**
**		Note that the Root_Context's word table is unset!
**		None of its values are exported.
**
***********************************************************************/
{
	REBVAL *value;
	REBINT n;
	REBSER *frame;

	frame = Make_Block(ROOT_MAX);  // Only half the context! (No words)
	KEEP_SERIES(frame, "root context");
	LOCK_SERIES(frame);
	Root_Context = (ROOT_CTX*)(frame->data);

	// Get first value (the SELF for the context):
	value = ROOT_SELF;
	SET_FRAME(value, 0, 0); // No words or spec (at first)

	// Set all other values to NONE:
	for (n = 1; n < ROOT_MAX; n++) SET_NONE(value+n);
	SET_END(value+ROOT_MAX);
	SERIES_TAIL(frame) = ROOT_MAX;

	// Initialize a few fields:
	Set_Block(ROOT_ROOT, frame);
	Init_Word(ROOT_NONAME, SYM__UNNAMED_);
}


/***********************************************************************
**
*/	void Set_Root_Series(REBVAL *value, REBSER *ser, const REBYTE *label)
/*
**		Used to set block and string values in the ROOT context.
**
***********************************************************************/
{
	LABEL_SERIES(ser, label);

	if (SERIES_WIDE(ser) == sizeof(REBVAL))
		Set_Block(value, ser); // VAL_SET(value, REB_BLOCK);
	else
		Set_String(value, ser);	//VAL_SET(value, REB_STRING);
}


/***********************************************************************
**
*/	static void Init_Task_Context(void)
/*
**		See above notes (same as root context, except for tasks)
**
***********************************************************************/
{
	REBVAL *value;
	REBINT n;
	REBSER *frame;

	//Print_Str("Task Context");

	Task_Series = frame = Make_Block(TASK_MAX);
	KEEP_SERIES(frame, "task context");
	LOCK_SERIES(frame);
	Task_Context = (TASK_CTX*)(frame->data);

	// Get first value (the SELF for the context):
	value = TASK_SELF;
	SET_FRAME(value, 0, 0); // No words or spec (at first)

	// Set all other values to NONE:
	for (n = 1; n < TASK_MAX; n++) SET_NONE(value+n);
	SET_END(value+TASK_MAX);
	SERIES_TAIL(frame) = TASK_MAX;

	// Initialize a few fields:
	SET_INTEGER(TASK_BALLAST, MEM_BALLAST);
	SET_INTEGER(TASK_MAX_BALLAST, MEM_BALLAST);
}


/***********************************************************************
**
*/	static void Init_System_Object(void)
/*
**		The system object is defined in boot.reb.
**
***********************************************************************/
{
	REBSER *frame;
	REBVAL *value;
	REBCNT n;

	// Evaluate the system object and create the global SYSTEM word.
	// We do not BIND_ALL here to keep the internal system words out
	// of the global context. See also N_context() which creates the
	// subobjects of the system object.

	// Create the system object from the sysobj block:
	value = VAL_BLK(&Boot_Block->sysobj);
	frame = Make_Object(0, value);

	// Bind it so CONTEXT native will work and bind its fields:
	Bind_Block(Lib_Context, value, BIND_DEEP);
	Bind_Block(frame, value, BIND_ONLY);  // No need to go deeper

	// Evaluate the block (will eval FRAMEs within):
	Do_Blk(VAL_SERIES(&Boot_Block->sysobj), 0);

	// Create a global value for it:
	value = Append_Frame(Lib_Context, 0, SYM_SYSTEM);
	SET_OBJECT(value, frame);
	SET_OBJECT(ROOT_SYSTEM, frame);

	// Create system/datatypes block:
//	value = Get_System(SYS_DATATYPES, 0);
	value = Get_System(SYS_CATALOG, CAT_DATATYPES);
	frame = VAL_SERIES(value);
	Extend_Series(frame, REB_MAX-1);
	for (n = 1; n <= REB_MAX; n++) {
		Append_Val(frame, FRM_VALUES(Lib_Context) + n);
	}

	// Create system/catalog/datatypes block:
//	value = Get_System(SYS_CATALOG, CAT_DATATYPES);
//	Set_Block(value, Copy_Blk(VAL_SERIES(&Boot_Block->types)));

	// Create system/catalog/actions block:
	value = Get_System(SYS_CATALOG, CAT_ACTIONS);
	Set_Block(value, Collect_Set_Words(VAL_BLK(&Boot_Block->actions)));

	// Create system/catalog/natives block:
	value = Get_System(SYS_CATALOG, CAT_NATIVES);
	Set_Block(value, Collect_Set_Words(VAL_BLK(&Boot_Block->natives)));

	// Create system/catalog/handles block
	// and register core handle types
	Init_Handles();

	// Create system/codecs object:
	value = Get_System(SYS_CODECS, 0);
	frame = Make_Frame(10);
	SET_OBJECT(value, frame);

	// Set system/words to be the main context:
//	value = Get_System(SYS_WORDS, 0);
//	SET_OBJECT(value, Lib_Context);

	Init_UType_Proto();
}


/***********************************************************************
**
*/	static void Init_Contexts_Object(void)
/*
***********************************************************************/
{
	REBVAL *value;
//	REBSER *frame;

	value = Get_System(SYS_CONTEXTS, CTX_SYS);
	SET_OBJECT(value, Sys_Context);

	value = Get_System(SYS_CONTEXTS, CTX_LIB);
	SET_OBJECT(value, Lib_Context);

	value = Get_System(SYS_CONTEXTS, CTX_USER);  // default for new code evaluation
	SET_OBJECT(value, Lib_Context);

	// Make the boot context - used to store values created
	// during boot, but processed in REBOL code (e.g. codecs)
//	value = Get_System(SYS_CONTEXTS, CTX_BOOT);
//	frame = Make_Frame(4);
//	SET_OBJECT(value, frame);
}

/***********************************************************************
**
*/	REBINT Codec_Text(REBCDI *codi)
/*
***********************************************************************/
{
	codi->error = 0;

	if (codi->action == CODI_IDENTIFY) {
		return CODI_CHECK; // error code is inverted result
	}

	if (codi->action == CODI_DECODE) {
		codi->other = (void*)Decode_UTF_String(codi->data, codi->len, -1, TRUE, FALSE);
		return CODI_STRING;
	}

	if (codi->action == CODI_ENCODE) {
		//O: This does not happen as in n-system.c only image is allowed to be encoded!
		return CODI_BINARY;
	}

	codi->error = CODI_ERR_NA;
	return CODI_ERROR;
}


/***********************************************************************
**
*/	REBINT Codec_Markup(REBCDI *codi)
/*
***********************************************************************/
{
	codi->error = 0;

	if (codi->action == CODI_IDENTIFY) {
		codi->error = 1;   // never identified... would require markup validator
		return CODI_CHECK; // error code is inverted result
	}

	if (codi->action == CODI_DECODE) {
		codi->other = (void*)Load_Markup(codi->data, codi->len);
		return CODI_BLOCK;
	}

	codi->error = CODI_ERR_NA;
	return CODI_ERROR;
}


/***********************************************************************
**
*/	void Register_Codec(const char *name, codo dispatcher)
/*
**		Internal function for adding a codec.
**
***********************************************************************/
{
	REBVAL *value = Get_System(SYS_CODECS, 0);
	REBCNT sym = Make_Word(cb_cast(name), 0);

	value = Append_Frame(VAL_OBJ_FRAME(value), 0, sym);
	SET_HANDLE(value, dispatcher, SYM_CODEC, HANDLE_FUNCTION);
}


/***********************************************************************
**
*/	static void Init_Codecs(void)
/*
***********************************************************************/
{
	Register_Codec("text", Codec_Text);
	Register_Codec("markup", Codec_Markup);
#ifdef INCLUDE_BMP_CODEC
	Init_BMP_Codec();
#endif
#ifdef INCLUDE_GIF_CODEC
	Init_GIF_Codec();
#endif
#ifdef INCLUDE_PNG_CODEC
	Init_PNG_Codec();
#endif
#ifdef INCLUDE_QOI_CODEC
	Init_QOI_Codec();
#endif
#ifdef INCLUDE_JPG_CODEC
	Init_JPEG_Codec();
#endif
#ifdef INCLUDE_WAV_CODEC
	Init_WAV_Codec();
#endif
}


static void Set_Option_String(REBCHR *str, REBCNT field)
{
	REBVAL *val;
	if (str) {
		val = Get_System(SYS_OPTIONS, field);
		Set_String(val, Copy_OS_Str(str, (REBINT)LEN_STR(str)));
	}
}

static REBCNT Set_Option_Word(REBCHR *str, REBCNT field)
{
	REBVAL *val;
	REBYTE *bp;
	REBYTE buf[40]; // option words always short ASCII strings
	REBCNT n = 0;

	if (str) {
		n = (REBCNT)LEN_STR(str); // WC correct
		if (n > 38) return 0;
		bp = &buf[0];
		while ((*bp++ = (REBYTE)*str++)); // clips unicode
		n = Make_Word(buf, n);
		val = Get_System(SYS_OPTIONS, field);
		Init_Word(val, n);
	}
	return n;
}

static void Set_Option_File(REBCNT field, REBYTE* src, REBOOL dir )
{
	REBSER *ser;
	REBVAL *val;
	if (OS_WIDE) {
		ser = To_REBOL_Path(src, 0, OS_WIDE, dir);
	}
	else {
		ser = Decode_UTF_String(src, LEN_BYTES(src), 8, FALSE, FALSE);
		ser = To_REBOL_Path(BIN_DATA(ser), BIN_LEN(ser), (REBOOL)!BYTE_SIZE(ser), dir);
	}
	val = Get_System(SYS_OPTIONS, field);
	Set_Series(REB_FILE, val, ser);
}

/***********************************************************************
**
*/	static void Init_Main_Args(REBARGS *rargs)
/*
**		The system object is defined in boot.reb.
**
***********************************************************************/
{
	REBVAL *val;
	REBCHR *arg;
	REBVAL *new;
	REBSER *ser;
	REBCHR *data;
	REBCNT n;

#ifdef RAW_MAIN_ARGS
	// make system/options/flags block even when not used...
	val = Get_System(SYS_OPTIONS, OPTIONS_FLAGS);
	Set_Block(val, Make_Block(3));
	// clear list of default flags (system/catalog/boot-flags)
	val = Get_System(SYS_CATALOG, CAT_BOOT_FLAGS);
	SET_NONE(val);
	// convert raw argument strings to block of strings...
	ser = Make_Block(3);
	for (n = 1; n < rargs->argc ; n++) {
		arg = rargs->argv[n];
		if (arg == 0) continue; // shell bug
		new = Append_Value(ser);
		Set_String(new, Copy_OS_Str(arg, (REBINT)LEN_STR(arg)));
		//if(arg[0]=='-')	VAL_SET_LINE(new);
	}
	val = Get_System(SYS_OPTIONS, OPTIONS_ARGS);
	Set_Block(val, ser);
#else
	// collect flags...
	ser = Make_Block(3);
	n = 2; // skip first flag (ROF_EXT)
	val = Get_System(SYS_CATALOG, CAT_BOOT_FLAGS);
	for (val = VAL_BLK(val); NOT_END(val); val++) {
		VAL_CLR_LINE(val);
		if (rargs->options & n) Append_Val(ser, val); 
		n <<= 1;
	}
	// last value is always TRUE, so it's possible to use just *path* instead of `find`
	// like: `if system/options/flags/verbose [...]`
	val = Append_Value(ser);
	SET_TRUE(val);
	
	val = Get_System(SYS_OPTIONS, OPTIONS_FLAGS);
	Set_Block(val, ser);

	// For compatibility:
	if (rargs->options & RO_QUIET) {
		val = Get_System(SYS_OPTIONS, OPTIONS_QUIET);
		SET_TRUE(val);
	}

	// Print("script: %s", rargs->script);
	if (rargs->script) {
		Set_Option_File(OPTIONS_SCRIPT, (REBYTE*)rargs->script, FALSE);
	}

	n = Set_Option_Word(rargs->boot, OPTIONS_BOOT_LEVEL);
	if (n >= SYM_BASE && n <= SYM_MODS)
		PG_Boot_Level = n - SYM_BASE; // 0 - 3
	
	// store args in a block
	ser = Make_Block(3);
	if (rargs->args) {
		// if used --args, store this value as a first one
		new = Append_Value(ser);
		Set_String(new, Copy_OS_Str(rargs->args, (REBINT)LEN_STR(rargs->args)));
	}
	// the rest of args, if there are any...
	for (n = 0; n < rargs->argc; n++) {
		arg = rargs->argv[n];
		if (arg == 0) continue; // shell bug
		new = Append_Value(ser);
		Set_String(new, Copy_OS_Str(arg, (REBINT)LEN_STR(arg)));
		//if(arg[0]=='-')	VAL_SET_LINE(new);
	}
	val = Get_System(SYS_OPTIONS, OPTIONS_ARGS);
	Set_Block(val, ser);

	// other option values...
	Set_Option_String(rargs->do_arg, OPTIONS_DO_ARG);
	Set_Option_String(rargs->debug, OPTIONS_DEBUG);
	Set_Option_String(rargs->version, OPTIONS_VERSION);
	Set_Option_String(rargs->import, OPTIONS_IMPORT);

	Set_Option_Word(rargs->secure, OPTIONS_SECURE);
#endif
	if (rargs->exe_path) {
		Set_Option_File(OPTIONS_BOOT, (REBYTE*)rargs->exe_path, FALSE);
	}

	// Print("home: %s", rargs->current_dir);
	if (rargs->current_dir) {
		Set_Option_File(OPTIONS_PATH, (REBYTE*)rargs->current_dir, TRUE);
		OS_FREE(rargs->current_dir);
	}
	if (NZ(data = OS_GET_LOCALE(0))) {
		val = Get_System(SYS_LOCALE, LOCALE_LANGUAGE);
		Set_String(val, Copy_OS_Str(data, (REBINT)LEN_STR(data)));
		OS_FREE(data);
	}

	if (NZ(data = OS_GET_LOCALE(1))) {
		val = Get_System(SYS_LOCALE, LOCALE_LANGUAGE_P);
		Set_String(val, Copy_OS_Str(data, (REBINT)LEN_STR(data)));
		OS_FREE(data);
	}

	if (NZ(data = OS_GET_LOCALE(2))) {
		val = Get_System(SYS_LOCALE, LOCALE_LOCALE);
		Set_String(val, Copy_OS_Str(data, (REBINT)LEN_STR(data)));
		OS_FREE(data);
	}

	if (NZ(data = OS_GET_LOCALE(3))) {
		val = Get_System(SYS_LOCALE, LOCALE_LOCALE_P);
		Set_String(val, Copy_OS_Str(data, (REBINT)LEN_STR(data)));
		OS_FREE(data);
	}
}


/***********************************************************************
**
*/	void Init_Task(void)
/*
***********************************************************************/
{
	// Thread locals:
	Trace_Level = 0;
	Saved_State = 0;

	Eval_Cycles = 0;
	Eval_Dose = EVAL_DOSE;
	Eval_Signals = 0;
	Eval_Sigmask = ALL_BITS;

	// errors? problem with PG_Boot_Phase shared?

	Init_Memory(-4);
	Init_Task_Context();	// Special REBOL values per task

	Init_Raw_Print();
	Init_Words(TRUE);
	Init_Data_Stack(STACK_MIN/4);
	Init_Scanner();
	Init_Mold(MIN_COMMON/4);
	Init_Frame();
	//Inspect_Series(0);
}


/***********************************************************************
**
*/	void Init_Year(void)
/*
***********************************************************************/
{
	REBOL_DAT dat;

	OS_GET_TIME(&dat);
	Current_Year = dat.year;
}


/***********************************************************************
**
*/	void Init_Core(REBARGS *rargs)
/*
**		GC is disabled during all init code, so these functions
**		need not protect themselves.
**
***********************************************************************/
{
	REBSER *ser;
	DOUT("Main init");

	// Globals
	PG_Boot_Phase = BOOT_START;
	PG_Boot_Level = BOOT_LEVEL_FULL;
	PG_Mem_Usage = 0;
	PG_Mem_Limit = 0;
	PG_Reb_Stats = Make_Mem(sizeof(*PG_Reb_Stats));
	Halt_State = 0;
	Reb_Opts = Make_Mem(sizeof(*Reb_Opts));

	// Thread locals:
	Trace_Level = 0;
	Saved_State = 0;
	Eval_Dose = EVAL_DOSE;
	Eval_Limit = 0;
	Eval_Signals = 0;
	Eval_Sigmask = ALL_BITS; /// dups Init_Task

	Init_StdIO();

	Assert_Basics();
	PG_Boot_Time = OS_DELTA_TIME(0, 0);

	DOUT("Level 0");
	Init_Memory(0);			// Memory allocator
	Init_Root_Context();	// Special REBOL values per program
	Init_Task_Context();	// Special REBOL values per task

	Init_Raw_Print();		// Low level output (Print)

	Print_Banner(rargs);    // Can cause early exit (-v)
	PG_Boot_Phase = BOOT_STARTED;
	DOUT("Level 1");
	Init_Char_Cases();
	Init_CRC();				// For word hashing
	Set_Random(0);
	Init_Words(FALSE);		// Symbol table
	Init_Data_Stack(STACK_MIN*4);
	Init_Scanner();
	Init_Mold(MIN_COMMON);	// Output buffer
	Init_Frame();			// Frames

	Lib_Context = Make_Frame(600);	// !! Have MAKE-BOOT compute # of words
	Sys_Context = Make_Frame(50);

	DOUT("Level 2");
	Load_Boot();			// Protected strings now available
	PG_Boot_Phase = BOOT_LOADED;
	//Debug_Str(BOOT_STR(RS_INFO,0)); // Booting...

	// Get the words of the ROOT context (to avoid it being an exception case):
	PG_Root_Words = Collect_Frame(BIND_ALL, 0, VAL_BLK(&Boot_Block->root));
	VAL_FRM_WORDS(ROOT_SELF) = PG_Root_Words;

	// Create main values:
	DOUT("Level 3");
	Init_Datatypes();		// Create REBOL datatypes
	Init_Typesets();		// Create standard typesets
	Init_Datatype_Checks();	// The TYPE? checks
	Init_Constants();		// Constant values

	// Run actual code:
	DOUT("Level 4");
	Init_Natives();			// Built-in native functions
	Init_Ops();				// Built-in operators
	Init_System_Object();
	Init_Contexts_Object();
	Init_Main_Args(rargs);
	Init_Ports();
	Init_Codecs();
	Init_Errors(&Boot_Block->errors); // Needs system/standard/error object
	PG_Boot_Phase = BOOT_ERRORS;

	Init_Year();

	// Special pre-made error:
	ser = Make_Error(RE_STACK_OVERFLOW, 0, 0, 0);
	SET_ERROR(TASK_STACK_ERROR, RE_STACK_OVERFLOW, ser);

	// Initialize mezzanine functions:
	DOUT("Level 5");
	if (PG_Boot_Level >= BOOT_LEVEL_SYS) {
		Do_Global_Block(VAL_SERIES(&Boot_Block->base), 1);
		Do_Global_Block(VAL_SERIES(&Boot_Block->sys), 2);
	}

	*FRM_VALUE(Sys_Context, SYS_CTX_BOOT_MEZZ) = Boot_Block->mezz;
	*FRM_VALUE(Sys_Context, SYS_CTX_BOOT_PROT) = Boot_Block->protocols;

	// No longer needs protecting:
	SET_NONE(ROOT_BOOT);
	Boot_Block = NULL;
	PG_Boot_Phase = BOOT_MEZZ;
	DS_RESET;

	DOUT("Boot done");
}


/***********************************************************************
**
*/	void Dispose_Core(void)
/*
**		Release Core resources (used before application quits)
**
***********************************************************************/
{
	DOUT("Dispose Core");
	if(!Task_Series)
		return; // can happen when close button, shutdown, etc.

	Free_Series(Task_Series);
	if (PG_Boot_Phase > BOOT_START) {
		Free_Series(Bind_Table);
		Free_Series(PG_Word_Table.hashes);
		Free_Series(PG_Word_Table.series);
		Free_Series(PG_Word_Names);
	}
	Free_Series(VAL_SERIES(ROOT_ROOT));
	if (PG_Boot_Phase >= BOOT_MEZZ) {
		Free_Series(VAL_SERIES(ROOT_SYSTEM));
		Free_Series(VAL_SERIES(ROOT_ERROBJ));
		Free_Series(VAL_SERIES(ROOT_STRINGS));
		Free_Series(VAL_SERIES(ROOT_TYPESETS));
		if (!IS_NONE(ROOT_BOOT))
			Free_Series(VAL_SERIES(ROOT_BOOT));
		Free_Series(VAL_SERIES(TASK_STACK));
		Free_Series(VAL_SERIES(TASK_BUF_EMIT));
		Free_Series(VAL_SERIES(TASK_BUF_WORDS));
		Free_Series(VAL_SERIES(TASK_BUF_UTF8));
		Free_Series(VAL_SERIES(TASK_BUF_PRINT));
		Free_Series(VAL_SERIES(TASK_BUF_FORM));
		Free_Series(VAL_SERIES(TASK_BUF_MOLD));
		Free_Series(VAL_SERIES(TASK_MOLD_LOOP));
	}
	Dispose_Memory();
	if (PG_Boot_Phase > BOOT_STARTED) {
		Dispose_Char_Cases();
		Dispose_Ports();
		Dispose_Mold();
		Dispose_CRC();
		Free_Mem(PG_Boot_Strs, 0);
	}
	Dispose_StdIO();
	Free_Mem(PG_Reb_Stats, 0);
	Free_Mem(Reb_Opts, 0);
}
