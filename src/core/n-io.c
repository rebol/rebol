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
**  Module:  n-io.c
**  Summary: native functions for input and output
**  Section: natives
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

//#define HELPER


/** Helper Functions **************************************************/

#ifdef HELPER
// Used for file loading during very early development.
static REBSER *Read_All_File(char *fname)
{
	REBREQ file;
	REBSER *ser = 0;

	CLEAR(&file, sizeof(file));

	file.clen = sizeof(file);
	file.device = RDI_FILE;
	file.file.path = fname;

	SET_FLAG(file.modes, RFM_READ);

	OS_DO_DEVICE(&file, RDC_OPEN);

	if (file.error) return 0;

	ser = Make_Binary((REBCNT)(file.file.size));

	file.data = BIN_DATA(ser);
	file.length = (REBCNT)(file.file.size);

	OS_DO_DEVICE(&file, RDC_READ);

	if (file.error) {
		ser = 0;
	}
	else {
		ser->tail = file.actual;
		STR_TERM(ser);
	}

	OS_DO_DEVICE(&file, RDC_CLOSE);
	return ser;
}
#endif


/***********************************************************************
**
*/	REBNATIVE(echo)
/*
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBSER *ser = 0;

	Echo_File(0);

	if (IS_FILE(val))
		ser = Value_To_OS_Path(val);
	else if (IS_LOGIC(val) && IS_TRUE(val))
		ser = To_Local_Path("output.txt", 10, FALSE, TRUE);

	if (ser) {
		if (!Echo_File((REBCHR*)(ser->data))) Trap1(RE_CANNOT_OPEN, val);
	}

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(form)
/*
**		Converts a value to a REBOL readable string.
**		value	"The value to mold"
**		/only   "For a block value, give only contents, no outer [ ]"
**		/all	"Mold in serialized format"
**		/flat	"No line indentation"
**
***********************************************************************/
{
	Set_String(D_RET, Copy_Form_Value(D_ARG(1), 0));
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(mold)
/*
**		Converts a value to a REBOL readable string.
**		value	"The value to mold"
**		/only   "For a block value, give only contents, no outer [ ]"
**		/all	"Mold in serialized format"
**		/flat	"No line indentation"
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REB_MOLD mo = {0};

	if (D_REF(3)) SET_FLAG(mo.opts, MOPT_MOLD_ALL);
	if (D_REF(4)) SET_FLAG(mo.opts, MOPT_INDENT);

	Reset_Mold(&mo);

	if (D_REF(2) && IS_BLOCK(val)) SET_FLAG(mo.opts, MOPT_ONLY);

	Mold_Value(&mo, val, TRUE);

	Set_String(D_RET, Copy_String(mo.series, 0, -1));

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(print)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);

	if (IS_BLOCK(value)) Reduce_Block(VAL_SERIES(value), VAL_INDEX(value), 0);
	Print_Value(DS_TOP, 0, 0);
	return R_UNSET; // reloads ds
}


/***********************************************************************
**
*/	REBNATIVE(prin)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);

	if (IS_BLOCK(value)) Reduce_Block(VAL_SERIES(value), VAL_INDEX(value), 0);
	Prin_Value(DS_TOP, 0, 0);
	return R_UNSET; // reloads ds
}


/***********************************************************************
**
*/	REBNATIVE(new_line)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);
	REBVAL *val;
	REBOOL cond = IS_TRUE(D_ARG(2));
	REBCNT n;
	REBINT skip = -1;

	val = VAL_BLK_DATA(value);
	if (D_REF(3)) skip = 1; // all
	if (D_REF(4)) { // skip
		skip = Int32s(D_ARG(5), 1); // size
		if (skip < 1) skip = 1;
	}
	for (n = 0; NOT_END(val); n++, val++) {
		if (cond ^ (n % skip != 0))
			VAL_SET_LINE(val);
		else
			VAL_CLR_LINE(val);
		if (skip < 0) break;
	}

	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(new_lineq)
/*
***********************************************************************/
{
	if VAL_GET_LINE(VAL_BLK_DATA(D_ARG(1))) return R_TRUE;
	return R_FALSE;
}


/***********************************************************************
**
*/	REBNATIVE(now)
/*
**  Return the current date and time with timezone adjustment.
**
**		1  /year {Returns year only.}
**		2  /month {Returns month only.}
**		3  /day {Returns day of the month only.}
**		4  /time {Returns time only.}
**		5  /zone {Returns time zone offset from GMT only.}
**		6  /date {Returns date only.}
**		7  /weekday {Returns day of the week as integer (Monday is day 1).}
**		8  /yearday {Returns day of the year (Julian)}
**		9  /precise {Higher precision}
**		10 /utc {Universal time (no zone)}
**
***********************************************************************/
{
	REBOL_DAT dat;
	REBINT n = -1;
	REBVAL *ret = D_RET;

	OS_GET_TIME(&dat);
	if (!D_REF(9)) dat.nano = 0; // Not /precise
	Set_Date(ret, &dat);
	Current_Year = dat.year;

	if (D_REF(10)) { // UTC
		VAL_ZONE(ret) = 0;
	}
	else {
		if (D_REF(1) || D_REF(2) || D_REF(3) || D_REF(4)
			|| D_REF(6) || D_REF(7) || D_REF(8))
			Adjust_Date_Zone(ret, FALSE); // Add time zone, adjust date and time
	}

	// Check for /date, /time, /zone
	if (D_REF(6)) {			// date
		VAL_TIME(ret) = NO_TIME;
		VAL_ZONE(ret) = 0;
	}
	else if (D_REF(4)) {	// time
		//if (dat.time == ???) SET_NONE(ret);
		VAL_SET(ret, REB_TIME);
	}
	else if (D_REF(5)) {	// zone
		VAL_SET(ret, REB_TIME);
		VAL_TIME(ret) = VAL_ZONE(ret) * ZONE_MINS * MIN_SEC;
	}
	else if (D_REF(7)) n = Week_Day(VAL_DATE(ret));
	else if (D_REF(8)) n = Julian_Date(VAL_DATE(ret));
	else if (D_REF(1)) n = VAL_YEAR(ret);
	else if (D_REF(2)) n = VAL_MONTH(ret);
	else if (D_REF(3)) n = VAL_DAY(ret);

	if (n > 0) SET_INTEGER(ret, n);

	return R_RET;
}


#ifdef HELPER
/***********************************************************************
**
*/	REBNATIVE(read_file)
/*
***********************************************************************/
{
	REBSER *ser;

	ser = VAL_SERIES(D_ARG(1));

	ser = Read_All_File(STR_HEAD(ser));
	if (!ser) Trap1(RE_CANNOT_OPEN, D_ARG(1));

	Set_Binary(D_RET, ser);

	return R_RET;

#ifdef unused
	if (D_REF(2)) // /binary
		Set_Binary(ret, ser);
	else {
		SERIES_TAIL(ser) = Convert_CRLF(STR_HEAD(ser), SERIES_TAIL(ser));
		if (D_REF(3)) // /lines
			Set_Block(ret, Convert_Lines(ser));
		else
			Set_String(ret, ser);
	}
	return R_RET;
#endif
}
#endif


/***********************************************************************
**
*/	REBNATIVE(wait)
/*
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBINT timeout = 0;	// in milliseconds
	REBSER *ports = 0;
	REBINT n = 0;

	SET_NONE(D_RET);

	if (IS_BLOCK(val)) {
		Reduce_Block(VAL_SERIES(val), VAL_INDEX(val), 0); // [stack-move]
		ports = VAL_SERIES(DS_TOP); // volatile after
		DS_RELOAD(ds);
		for (val = BLK_HEAD(ports); NOT_END(val); val++) { // find timeout
			if (Pending_Port(val)) n++;
			if (IS_INTEGER(val) || IS_DECIMAL(val)) break;
		}
		if (IS_END(val)) {
			if (n == 0) return R_NONE; // has no pending ports!
			// SET_NONE(val); // no timeout -- BUG: unterminated block in GC
		}
	}

	switch (VAL_TYPE(val)) {
	case REB_INTEGER:
		timeout = 1000 * Int32(val);
		goto chk_neg;

	case REB_DECIMAL:
		timeout = (REBINT)(1000 * VAL_DECIMAL(val));
		goto chk_neg;

	case REB_TIME:
		timeout = (REBINT) (VAL_TIME(val) / (SEC_SEC / 1000));
chk_neg:
		if (timeout < 0) Trap_Range(val);
		break;

	case REB_PORT:
		if (!Pending_Port(val)) return R_NONE;
		ports = Make_Block(1);
		Append_Val(ports, val);
		// fall thru...
	case REB_NONE:
	case REB_END:
		timeout = ALL_BITS;	// wait for all windows
		break;

	default:
		Trap_Arg(val);
	}

	// Prevent GC on temp port block:
	// Note: Port block is always a copy of the block.
	if (ports) Set_Block(D_RET, ports);

	// Process port events [stack-move]:
	if (!Wait_Ports(ports, timeout)) return R_NONE;
	if (!ports) return R_NONE;
	DS_RELOAD(ds);

	// Determine what port(s) waked us:
	Sieve_Ports(ports);

	if (!D_REF(2)) { // not /all ports
		val = BLK_HEAD(ports);
		if (IS_PORT(val)) *D_RET = *val;
		else SET_NONE(D_RET);
	}

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(wake_up)
/*
**		Calls port update for native actors.
**		Calls port awake function.
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBSER *port = VAL_PORT(val);

	if (SERIES_TAIL(port) < STD_PORT_MAX) Crash(9910);

	val = OFV(port, STD_PORT_ACTOR);
	if (IS_NATIVE(val)) {
		Do_Port_Action(port, A_UPDATE); // uses current stack frame
	}

	val = OFV(port, STD_PORT_AWAKE);
	if (ANY_FUNC(val)) {
		val = Apply_Func(0, val, D_ARG(2), 0);
		if (!(IS_LOGIC(val) && VAL_LOGIC(val))) return R_FALSE;
	}
	return R_TRUE;  // wake it up
}


/***********************************************************************
**
*/	REBNATIVE(to_rebol_file)
/*
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);
	REBSER *ser;

	ser = Value_To_REBOL_Path(arg, 0);
	if (!ser) Trap_Arg(arg);
	Set_Series(REB_FILE, D_RET, ser);

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(to_local_file)
/*
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);
	REBSER *ser;

	ser = Value_To_Local_Path(arg, D_REF(2));
	if (!ser) Trap_Arg(arg);
	Set_Series(REB_STRING, D_RET, ser);

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(what_dir)
/*
***********************************************************************/
{
	REBSER *ser;
	REBCHR *lpath;
	REBINT len;

	len = OS_GET_CURRENT_DIR(&lpath);
	ser = To_REBOL_Path(lpath, len, OS_WIDE, TRUE); // allocates extra for end /
	ASSERT1(ser, RP_MISC); // should never happen
	OS_FREE(lpath);
	Set_Series(REB_FILE, D_RET, ser);

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(change_dir)
/*
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);
	REBSER *ser;
	REBINT n;
	REBVAL val;

	ser = Value_To_OS_Path(arg);
	if (!ser) Trap_Arg(arg); // !!! ERROR MSG

	Set_String(&val, ser); // may be unicode or utf-8
	Check_Security(SYM_FILE, POL_EXEC, &val);

	n = OS_SET_CURRENT_DIR((void*)ser->data);  // use len for bool
	if (!n) Trap_Arg(arg); // !!! ERROR MSG

	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(browse)
/*
***********************************************************************/
{
	REBINT r;
	REBCHR *url = 0;
	REBVAL *arg = D_ARG(1);

	Check_Security(SYM_BROWSE, POL_EXEC, arg);

	if (IS_NONE(arg))
		return R_UNSET;

	url = Val_Str_To_OS(arg);

	r = OS_BROWSE(url, 0);

	if (r == 0) Trap1(RE_CALL_FAIL, Make_OS_Error());

	return R_UNSET;
}


/***********************************************************************
**
*/	REBNATIVE(call)
/*
***********************************************************************/
{
	REBINT r;
	REBCHR *cmd;
	REBVAL *arg = D_ARG(1);

	Check_Security(SYM_CALL, POL_EXEC, arg);

	cmd = Val_Str_To_OS(arg);
	r = OS_CREATE_PROCESS(cmd, D_REF(2) ? 1 : 0);

	if (D_REF(2)) {
		SET_INTEGER(D_RET, r);
		return R_RET;
	}
	
	if (r < 0) Trap1(RE_CALL_FAIL, Make_OS_Error());
	return R_NONE;
}


#ifdef not_used
/***********************************************************************
**
*/	REBNATIVE(launch)
/*
***********************************************************************/
{
	REBVAL *val = Get_System(SYS_OPTIONS, OPTIONS_BOOT);
	REBVAL *script = D_ARG(1);
	REBSER *cmd;
	REBINT r;

	Check_Security(SYM_CALL, POL_EXEC, script);

	if (ANY_STR(val)) {
		cmd = Make_Binary(VAL_LEN(val) + VAL_LEN(script) + 4);
		Append_Byte(cmd, '"');
		Append_Bytes(cmd, VAL_BIN_DATA(val));
		Append_Byte(cmd, '"');
		if (!IS_NONE(script)) {
			Append_Byte(cmd, ' ');
			Append_Bytes(cmd, VAL_BIN_DATA(script)); // !!! convert file
		}
		if (D_REF(2)) {
			Append_Byte(cmd, ' ');
			Append_Bytes(cmd, VAL_BIN_DATA(D_ARG(3)));
		}
		Print("Launching: %s", STR_HEAD(cmd));
		r = OS_CREATE_PROCESS(STR_HEAD(cmd), 0);
		if (r < 0) Trap1(RE_CALL_FAIL, Make_OS_Error());
	}
	return R_NONE;
}
#endif


/***********************************************************************
**
*/	static REBSER *String_List_To_Block(REBCHR *str)
/*
**		Convert a series of null terminated strings to
**		a block of strings separated with '='.
**
***********************************************************************/
{
	REBCNT n;
	REBCNT len = 0;
	REBCHR *start = str;
	REBCHR *eq;
	REBSER *blk;

	while (n = LEN_STR(str)) {
		len++;
		str += n + 1; // next
	}

	blk = Make_Block(len*2);

	str = start;
	while (NZ(eq = FIND_CHR(str+1, '=')) && NZ(n = LEN_STR(str))) {
		Set_Series(REB_STRING, Append_Value(blk), Copy_OS_Str(str, eq-str));
		Set_Series(REB_STRING, Append_Value(blk), Copy_OS_Str(eq+1, n-(eq-str)-1));
		str += n + 1; // next
	}

	Block_As_Map(blk);

	return blk;
}


#ifdef TO_WIN32
/***********************************************************************
**
*/	REBSER *Block_To_String_List(REBVAL *blk)
/*
**		Convert block of values to a string that holds
**		a series of null terminated strings, followed
**		by a final terminating string.
**
***********************************************************************/
{
	REB_MOLD mo = {0};
	REBVAL *value;

	Reset_Mold(&mo);

	for (value = VAL_BLK_DATA(blk); NOT_END(value); value++) {
		Mold_Value(&mo, value, 0);
		Append_Byte(mo.series, 0);
	}
	Append_Byte(mo.series, 0);

	return Copy_Series(mo.series); // Unicode
}


/***********************************************************************
**
*/	static REBSER *File_List_To_Block(REBCHR *str)
/*
**		Convert file directory and file name list to block.
**
***********************************************************************/
{
	REBCNT n;
	REBCNT len = 0;
	REBCHR *start = str;
	REBSER *blk;
	REBSER *dir;

	while (n = LEN_STR(str)) {
		len++;
		str += n + 1; // next
	}

	blk = Make_Block(len);

	// First is a dir path or full file path:
	str = start;
	n = LEN_STR(str);

	if (len == 1) {  // First is full file path
		dir = To_REBOL_Path(str, n, -1, 0);
		Set_Series(REB_FILE, Append_Value(blk), dir);
	}
	else {  // First is dir path for the rest of the files
		dir = To_REBOL_Path(str, n, -1, TRUE);
		str += n + 1; // next
		len = dir->tail;
		while (n = LEN_STR(str)) {
			dir->tail = len;
			Append_Uni_Uni(dir, str, n);
			Set_Series(REB_FILE, Append_Value(blk), Copy_String(dir, 0, -1));
			str += n + 1; // next
		}
	}

	return blk;
}
#endif


/***********************************************************************
**
*/	REBNATIVE(request_file)
/*
***********************************************************************/
{
#ifdef TO_WIN32
	REBRFR fr = {0};
	REBSER *ser;
	REBINT n;

	fr.files = OS_MAKE(MAX_FILE_REQ_BUF);
	fr.len = MAX_FILE_REQ_BUF/sizeof(REBCHR) - 2;  
	fr.files[0] = 0;

	DISABLE_GC;

	if (D_REF(ARG_REQUEST_FILE_SAVE)) SET_FLAG(fr.flags, FRF_SAVE);
	if (D_REF(ARG_REQUEST_FILE_MULTI)) SET_FLAG(fr.flags, FRF_MULTI);

	if (D_REF(ARG_REQUEST_FILE_FILE)) {
		ser = Value_To_OS_Path(D_ARG(ARG_REQUEST_FILE_NAME));
		fr.dir = (REBCHR*)(ser->data);
		n = ser->tail;
		if (fr.dir[n-1] != OS_DIR_SEP) {
			if (n+2 > fr.len) n = fr.len - 2;
			COPY_STR(fr.files, (REBCHR*)(ser->data), n);
			fr.files[n] = 0;
		}
	}

	if (D_REF(ARG_REQUEST_FILE_FILTER)) {
		ser = Block_To_String_List(D_ARG(ARG_REQUEST_FILE_LIST));
		fr.filter = (REBCHR*)(ser->data);
	}

	if (D_REF(ARG_REQUEST_FILE_TITLE))
		fr.title = Val_Str_To_OS(D_ARG(ARG_REQUEST_FILE_TEXT));

	if (OS_REQUEST_FILE(&fr)) {
		if (GET_FLAG(fr.flags, FRF_MULTI)) {
			ser = File_List_To_Block(fr.files);
			Set_Block(D_RET, ser);
		}
		else {
			ser = To_REBOL_Path(fr.files, LEN_STR(fr.files), OS_WIDE, 0);
			Set_Series(REB_FILE, D_RET, ser);
		}
	} else
		ser = 0;

	ENABLE_GC;
	OS_FREE(fr.files);

	return ser ? R_RET : R_NONE;
#else
	return R_NONE;
#endif
}


/***********************************************************************
**
*/	REBNATIVE(get_env)
/*
***********************************************************************/
{
	REBCHR *cmd;
	REBINT lenplus;
	REBCHR *buf;
	REBVAL *arg = D_ARG(1);

	Check_Security(SYM_ENVR, POL_READ, arg);

	if (ANY_WORD(arg)) Set_String(arg, Copy_Form_Value(arg, 0));
	cmd = Val_Str_To_OS(arg);

	lenplus = OS_GET_ENV(cmd, (REBCHR*)0, 0);
	if (lenplus == 0) return R_NONE;
	if (lenplus < 0) return R_UNSET;

	// Two copies...is there a better way?
	buf = MAKE_STR(lenplus);
	OS_GET_ENV(cmd, buf, lenplus);
	Set_String(D_RET, Copy_OS_Str(buf, lenplus - 1));
	FREE_MEM(buf);

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(set_env)
/*
***********************************************************************/
{
	REBCHR *cmd;
	REBVAL *arg1 = D_ARG(1);
	REBVAL *arg2 = D_ARG(2);
	REBOOL success;

	Check_Security(SYM_ENVR, POL_WRITE, arg1);

	if (ANY_WORD(arg1)) Set_String(arg1, Copy_Form_Value(arg1, 0));
	cmd = Val_Str_To_OS(arg1);
	
	if (ANY_STR(arg2)) {
		REBCHR *value = Val_Str_To_OS(arg2);
		success = OS_SET_ENV(cmd, value);
		if (success) {
			// What function could reuse arg2 as-is?
			Set_String(D_RET, Copy_OS_Str(value, LEN_STR(value)));
			return R_RET;
		}
		return R_UNSET;
	}

	if (IS_NONE(arg2)) {
		success = OS_SET_ENV(cmd, 0);
		if (success)
			return R_NONE;
		return R_UNSET;
	}

	// is there any checking that native interface has not changed
	// out from under the expectations of the code?

	return R_UNSET;
}


/***********************************************************************
**
*/	REBNATIVE(list_env)
/*
***********************************************************************/
{
	REBCHR *result = OS_LIST_ENV();

	Set_Series(REB_MAP, D_RET, String_List_To_Block(result));

	return R_RET;
}
