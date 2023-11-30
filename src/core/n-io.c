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
	REBINT err;

	Echo_File(0);

	if (IS_FILE(val))
		ser = Value_To_OS_Path(val, TRUE);
	else if (IS_LOGIC(val) && IS_TRUE(val))
		ser = To_Local_Path("output.txt", 10, FALSE, TRUE);

	if (ser) {
		err = Echo_File((REBCHR *)(ser->data));
		if (err != DR_DONE) {
			SET_INTEGER(D_RET, -err);
			Trap2(RE_CANNOT_OPEN, val, D_RET);
		}
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
**		/part	"Limit the length of the result"
**		 limit [integer!]
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REB_MOLD mo = {0};
	REBCNT  len;

	if (D_REF(3)) SET_FLAG(mo.opts, MOPT_MOLD_ALL);
	if (D_REF(4)) SET_FLAG(mo.opts, MOPT_INDENT);
	if (D_REF(5)) {
		if (VAL_INT64(D_ARG(6)) > (i64)MAX_U32)
			len = MAX_U32;
		else if (VAL_INT64(D_ARG(6)) <= 0)
			len = 0;
		else
			len = VAL_UNT32(D_ARG(6));
	}
	else {
		len = NO_LIMIT;
	}

	Reset_Mold(&mo);
	mo.limit = len;

	if (D_REF(2) && IS_BLOCK(val)) SET_FLAG(mo.opts, MOPT_ONLY);

	Mold_Value(&mo, val, TRUE);
	if (len > mo.series->tail) len = mo.series->tail;
	Set_String(D_RET, Copy_String(mo.series, 0, len));

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
	Print_Value(DS_TOP, NO_LIMIT, 0, FALSE);
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
	Prin_Value(DS_TOP, 0, 0, FALSE);
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

	Assert_Max_Refines(ds, D_REF(9) ? 2 : 1); // prevent too many refines like: now/year/month

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
			if (IS_INTEGER(val) || IS_DECIMAL(val) || IS_TIME(val)) break;
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
	if (!Wait_Ports(ports, timeout, D_REF(3))) {
		Sieve_Ports(NULL); /* just reset the waked list */
		return R_NONE;
	}
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
		//  Makes the port object fully consistent with internal native structures (e.g. the actual length of data read).
		Do_Port_Action(D_ARG(1), A_UPDATE); // uses current stack frame
	}

	val = OFV(port, STD_PORT_AWAKE);
	if (ANY_FUNC(val)) {
		// Calls the port's awake function with the event as an argument.
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
*/	REBNATIVE(to_real_file)
/*
//	to-real-file: native [
//		"Returns canonicalized absolute pathname. On Posix resolves symbolic links and returns NONE if file does not exists!"
//		path [file! string!]
//	]
***********************************************************************/
{
	REBVAL *path = D_ARG(1);
	REBSER *ser = NULL;
	REBSER *new;
	REBCHR *tmp;
	// First normalize to OS native wide string
	ser = Value_To_OS_Path(path, FALSE);
	// Above function does . and .. pre-processing, which does also the OS_REAL_PATH.
	// So it could be replaced with version, which just prepares the input to required OS wide! 
	if (!ser) {
		FREE_SERIES(ser);
		return R_NONE;
	}
	// Try to call realpath on posix or _fullpath on Windows
	// Returned string must be released once done!
	tmp = OS_REAL_PATH((REBCHR*)SERIES_DATA(ser));
	if (!tmp) return R_NONE;

	// Convert OS native wide string back to Rebol file type
	new = To_REBOL_Path(tmp, 0, OS_WIDE, FALSE);
	OS_FREE(tmp);
	if (!new) return R_NONE;
	
	Set_Series(REB_FILE, D_RET, new);
	return R_RET;
}

// Blog: http://www.rebol.net/cgi-bin/r3blog.r?view=0319
/***********************************************************************
**
*/	REBNATIVE(dirq)
/*
//	dir?: native [
//		"Returns TRUE if the value looks like a directory spec (ends with a slash (or backslash))."
//		target [file! url! none!]
//		/check "If the file is a directory on local storage (don't have to end with a slash)"
//	]
***********************************************************************/
{
	REBVAL *path = D_ARG(1);
	REBINT len;
	REBSER *ser;
	REBREQ file;
	REBUNI ch;

	if (!ANY_STR(path) || VAL_LEN(path) == 0) return R_FALSE;

	if (D_REF(2)) {
	// use OS check if path really exists and is a directory
		if (IS_FILE(path)) {
			CLEARS(&file); 
			ser = Value_To_OS_Path(path, TRUE);
			file.file.path = (REBCHR*)(ser->data);
			file.device = RDI_FILE;
			len = OS_DO_DEVICE(&file, RDC_QUERY);
			FREE_SERIES(ser);
			if (len == DR_DONE && GET_FLAG(file.modes, RFM_DIR)) return R_TRUE;
		}
	}
	// without OS check
	ch = GET_ANY_CHAR(VAL_SERIES(path), VAL_TAIL(path)-1);
	if (ch == '/' || ch == '\\') return R_TRUE;

	return R_FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(wildcardq)
/*
//	wildcard?: native [
//		"Return true if file contains wildcard chars (* or ?)"
//		path [file!]
//	]
***********************************************************************/
{
	REBVAL *path = D_ARG(1);
	REBCNT index = Find_Str_Wild(VAL_SERIES(path), VAL_INDEX(path), VAL_TAIL(path));
	return index == NOT_FOUND ? R_FALSE : R_TRUE;
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
#ifdef TO_WINDOWS
	ser = To_REBOL_Path(lpath, len, OS_WIDE, TRUE); // allocates extra for end /
#else
	ser = Decode_UTF_String(lpath, len, 8, 0, TRUE);
	ser = To_REBOL_Path(BIN_HEAD(ser), SERIES_TAIL(ser), TRUE, TRUE);
#endif
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

	ser = Value_To_OS_Path(arg, TRUE);
	// it should be safe not to check result from Value_To_OS_Path (it always succeeds)
	//if (!ser) Trap_Arg(arg); // !!! ERROR MSG

	Set_String(&val, ser); // may be unicode or utf-8
	Check_Security(SYM_FILE, POL_EXEC, &val);

	n = OS_SET_CURRENT_DIR((void*)ser->data);  // use len for bool

	// convert the full OS path back to Rebol format
	// used in error or as a result
#ifdef TO_WINDOWS
	ser = Value_To_REBOL_Path(&val, TRUE);
#else
	ser = Decode_UTF_String(VAL_BIN(&val), VAL_LEN(&val), 8, 0, TRUE);
	ser = To_REBOL_Path(BIN_HEAD(ser), SERIES_TAIL(ser), TRUE, TRUE);
#endif

	
	SET_FILE(arg, ser);

	if (NZ(n)) {
		SET_INTEGER(D_RET, -n);
		Trap2(RE_CANNOT_OPEN, arg, D_RET);
	}
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

	if(IS_FILE(arg)) {
		// Convert file to full local os path
		url = (REBCHR*)SERIES_DATA(Value_To_OS_Path(arg, TRUE));
	} else {
		url = Val_Str_To_OS(arg);
	}

	r = OS_BROWSE(url, 0);

	if (r == 0) Trap1(RE_CALL_FAIL, Make_OS_Error(r));

	return R_UNSET;
}


/***********************************************************************
**
*/	REBNATIVE(call)
/*
 *
	/wait "Wait for command to terminate before returning"
    /console "Runs command with I/O redirected to console"
    /shell "Forces command to be run from shell"
	/info "Return process information object"
    /input in [string! file! none] "Redirects stdin to in"
    /output out [string! file! none] "Redirects stdout to out"
    /error err [string! file! none] "Redirects stderr to err"
***********************************************************************/
{
#define INHERIT_TYPE 0
#define NONE_TYPE 1
#define STRING_TYPE 2
#define FILE_TYPE 3
#define BINARY_TYPE 4

#define FLAG_WAIT 1
#define FLAG_CONSOLE 2
#define FLAG_SHELL 4
#define FLAG_INFO 8

	REBINT r;
	REBCHR *cmd = NULL;
	REBVAL *arg = D_ARG(1);
	REBU64 pid = 0;
	u32 flags = 0;
	int argc = 1;
	REBCHR ** argv = NULL;
	REBVAL *input = NULL;
	REBVAL *output = NULL;
	REBVAL *err = NULL;
	void *os_input = NULL;
	void *os_output = NULL;
	void *os_err = NULL;
	int input_type = INHERIT_TYPE;
	int output_type = INHERIT_TYPE;
	int err_type = INHERIT_TYPE;
	REBCNT input_len = 0;
	REBCNT output_len = 0;
	REBCNT err_len = 0;
	REBOOL flag_wait = FALSE;
	REBOOL flag_console = FALSE;
	REBOOL flag_shell = FALSE;
	REBOOL flag_info = FALSE;
	int exit_code = 0;

	Check_Security(SYM_CALL, POL_EXEC, arg);

	if (D_REF(2)) flag_wait = TRUE;
	if (D_REF(3)) flag_console = TRUE;
	if (D_REF(4)) flag_shell = TRUE;
	if (D_REF(5)) flag_info = TRUE;
	if (D_REF(6)) { /* input */
		input = D_ARG(7);
		if (IS_STRING(input)) {
			input_type = STRING_TYPE;
			os_input = Val_Str_To_OS(input);
			input_len = VAL_LEN(input);
		} else if (IS_BINARY(input)) {
			input_type = BINARY_TYPE;
			os_input = VAL_BIN_DATA(input);
			input_len = VAL_LEN(input);
		} else if (IS_FILE(input)) {
			REBSER *path = Value_To_OS_Path(input, FALSE);
			input_type = FILE_TYPE;
			os_input = SERIES_DATA(path);
			input_len = SERIES_TAIL(path);
		} else if (IS_NONE(input)) {
			input_type = NONE_TYPE;
		} else {
			Trap_Arg(input);
		}
	}

	if (D_REF(8)) { /* output */
		output = D_ARG(9);
		if (IS_STRING(output)) {
			output_type = STRING_TYPE;
		} else if (IS_BINARY(output)) {
			output_type = BINARY_TYPE;
		} else if (IS_FILE(output)) {
			REBSER *path = Value_To_OS_Path(output, FALSE);
			output_type = FILE_TYPE;
			os_output = SERIES_DATA(path);
			output_len = SERIES_TAIL(path);
		} else if (IS_NONE(output)) {
			output_type = NONE_TYPE;
		} else {
			Trap_Arg(output);
		}
	}

	if (D_REF(10)) { /* err */
		err = D_ARG(11);
		if (IS_STRING(err)) {
			err_type = STRING_TYPE;
		} else if (IS_BINARY(err)) {
			err_type = BINARY_TYPE;
		} else if (IS_FILE(err)) {
			REBSER *path = Value_To_OS_Path(err, FALSE);
			err_type = FILE_TYPE;
			os_err = SERIES_DATA(path);
			err_len = SERIES_TAIL(path);
		} else if (IS_NONE(err)) {
			err_type = NONE_TYPE;
		} else {
			Trap_Arg(err);
		}
	}

	/* I/O redirection implies /wait */
	if (input_type == STRING_TYPE
		|| input_type == BINARY_TYPE
		|| output_type == STRING_TYPE
		|| output_type == BINARY_TYPE
		|| err_type == STRING_TYPE
		|| err_type == BINARY_TYPE) {
		flag_wait = TRUE;
	}

	if (flag_wait) flags |= FLAG_WAIT;
	if (flag_console) flags |= FLAG_CONSOLE;
	if (flag_shell) flags |= FLAG_SHELL;
	if (flag_info) flags |= FLAG_INFO;

	if (IS_FILE(arg)) {
		REBSER* ser = NULL;
		REBSER* path = Value_To_OS_Path(arg, FALSE);
		argc = 1;
		ser = Make_Series(argc + 1, sizeof(REBCHR*), FALSE);
		argv = (REBCHR**)SERIES_DATA(ser);
		argv[0] = (REBCHR*)SERIES_DATA(path);
		argv[argc] = NULL;
		cmd = NULL;
	} else if (ANY_STR(arg)) {
		REBSER * ser = NULL;
		cmd = Val_Str_To_OS(arg);
		argc = 1;
		ser = Make_Series(argc + 1, sizeof(REBCHR*), FALSE);
		argv = (REBCHR**)SERIES_DATA(ser);
		argv[0] = cmd;
		argv[argc] = NULL;
	} else if (IS_BLOCK(arg)) {
		int i = 0;
		REBSER * ser = NULL;
		argc = VAL_LEN(arg);
		if (argc <= 0) {
			Trap0(RE_TOO_SHORT);
		}
		ser = Make_Series(argc + 1, sizeof(REBCHR*), FALSE);
		argv = (REBCHR**)SERIES_DATA(ser);
		for (i = 0; i < argc; i ++) {
			REBVAL *param = VAL_BLK_SKIP(arg, i);
			// to avoid call to reduce, resolve get-word! or get-path! values if used
			if (IS_GET_WORD(param)) {
				param = Get_Var(param);
			}
			else if (IS_GET_PATH(param)) {
				Do_Path(&param, NULL);
				param = DS_POP; // volatile stack reference
			}
			if (IS_FILE(param)) {
				REBSER* path = Value_To_OS_Path(param, FALSE);
				argv[i] = (REBCHR*)SERIES_DATA(path);
			} else if (ANY_STR(param)) {
				argv[i] = Val_Str_To_OS(param);
			} else if (IS_WORD(param)) {
				Set_Series(REB_STRING, D_RET, Copy_Form_Value(param, TRUE));
				argv[i] = Val_Str_To_OS(D_RET);
			} else {
				Trap_Arg(param);
			}
		}
		argv[argc] = NULL;
		cmd = NULL;
	} else {
		Trap_Arg(arg);
	}

	r = OS_CREATE_PROCESS(cmd, argc, argv, flags, &pid, &exit_code,
						  input_type, os_input, input_len,
						  output_type, &os_output, &output_len,
						  err_type, &os_err, &err_len);

	if (output_type == STRING_TYPE) {
		if (output != NULL
			&& output_len > 0) {
			REBSER *ser = Copy_OS_Str(os_output, output_len);
			Append_String(VAL_SERIES(output), ser, 0, SERIES_TAIL(ser));
			OS_FREE(os_output);
		}
	} else if (output_type == BINARY_TYPE) {
		if (output != NULL
			&& output_len > 0) {
			Append_Bytes_Len(VAL_SERIES(output), os_output, output_len);
			OS_FREE(os_output);
		}
	}

	if (err_type == STRING_TYPE) {
		if (err != NULL
			&& err_len > 0) {
			REBSER *ser = Copy_OS_Str(os_err, err_len);
			Append_String(VAL_SERIES(err), ser, 0, SERIES_TAIL(ser));
			OS_FREE(os_err);
		}
	} else if (err_type == BINARY_TYPE) {
		if (err != NULL
			&& err_len > 0) {
			Append_Bytes_Len(VAL_SERIES(err), os_err, err_len);
			OS_FREE(os_err);
		}
	}

	if (flag_info) {
		REBSER *obj = Make_Frame(2);
		REBVAL *val = Append_Frame(obj, NULL, SYM_ID);
		SET_INTEGER(val, pid);

		if (flag_wait) {
			val = Append_Frame(obj, NULL, SYM_EXIT_CODE);
			SET_INTEGER(val, exit_code);
		}

		SET_OBJECT(D_RET, obj);
		return R_RET;
	}

	if (r == 0) {
		SET_INTEGER(D_RET, flag_wait ? exit_code : pid);
		return R_RET;
	} else {
		Trap1(RE_CALL_FAIL, Make_OS_Error(r));
		return R_NONE;
	}
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
		if (r < 0) Trap1(RE_CALL_FAIL, Make_OS_Error(r));
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

	while ((n = (REBCNT)LEN_STR(str))) {
		len++;
		str += n + 1; // next
	}

	blk = Make_Block(len*2);

	str = start;
	while (NZ(n = (REBCNT)LEN_STR(str)) && NZ(eq = FIND_CHR(str+1, '='))) {
		Set_Series(REB_STRING, Append_Value(blk), Copy_OS_Str(str, eq-str));
		Set_Series(REB_STRING, Append_Value(blk), Copy_OS_Str(eq+1, n-(eq-str)-1));
		str += n + 1; // next
	}

	Block_As_Map(blk);

	return blk;
}


#ifdef TO_WINDOWS
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

	while ((n = (REBCNT)LEN_STR(str))) {
		len++;
		str += n + 1; // next
	}

	blk = Make_Block(len);

	// First is a dir path or full file path:
	str = start;
	n = (REBCNT)LEN_STR(str);

	if (len == 1) {  // First is full file path
		dir = To_REBOL_Path(str, n, -1, 0);
		Set_Series(REB_FILE, Append_Value(blk), dir);
	}
	else {  // First is dir path for the rest of the files
		dir = To_REBOL_Path(str, n, -1, TRUE);
		str += n + 1; // next
		len = dir->tail;
		while ((n = (REBCNT)LEN_STR(str))) {
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
#ifdef TO_WINDOWS
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
		ser = Value_To_OS_Path(D_ARG(ARG_REQUEST_FILE_NAME), TRUE);
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
			ser = To_REBOL_Path(fr.files, (REBCNT)LEN_STR(fr.files), OS_WIDE, 0);
			Set_Series(REB_FILE, D_RET, ser);
		}
	} else
		ser = 0;

	ENABLE_GC;
	OS_FREE(fr.files);

	return ser ? R_RET : R_NONE;
#else
	Trap0(RE_FEATURE_NA);
	return R_NONE;
#endif
}


/***********************************************************************
**
*/	REBNATIVE(request_dir)
/*
***********************************************************************/
{
#ifdef TO_WINDOWS
	REBRFR fr = {0};
	REBSER *ser;
	REBINT n;

	fr.files = OS_MAKE(MAX_DIR_REQ_BUF);
	fr.len = MAX_DIR_REQ_BUF/sizeof(REBCHR) - 4; // reserve for trailing slash and null 
	fr.files[0] = 0;

	DISABLE_GC;

	if (D_REF(ARG_REQUEST_DIR_DIR)) {
		ser = Value_To_OS_Path(D_ARG(ARG_REQUEST_DIR_NAME), TRUE);
		fr.dir = (REBCHR*)(ser->data);
		n = ser->tail;
		if (fr.dir[n-1] != OS_DIR_SEP) {
			if (n+2 > fr.len) n = fr.len - 2;
			COPY_STR(fr.files, (REBCHR*)(ser->data), n);
			fr.files[n] = 0;
		}
	}

	if (D_REF(ARG_REQUEST_DIR_TITLE))
		fr.title = Val_Str_To_OS(D_ARG(ARG_REQUEST_DIR_TEXT));
	else
		fr.title = L"Select Folder";

	if (D_REF(ARG_REQUEST_DIR_KEEP)) SET_FLAG(fr.flags, FRF_KEEP);

	if (OS_REQUEST_DIR(&fr)) {
		REBCNT len = (REBCNT)LEN_STR(fr.files);
		// Windows may return path without trailing slash, so just add it if missing
		if (fr.files[len-1] != '\\') {
			fr.files[len++]  = '\\';
			fr.files[len  ]  = 0;
		}
		ser = To_REBOL_Path(fr.files, len, OS_WIDE, 0);
		Set_Series(REB_FILE, D_RET, ser);
	} else
		ser = 0;

	ENABLE_GC;
	OS_FREE(fr.files);

	return ser ? R_RET : R_NONE;
#else
	Trap0(RE_FEATURE_NA);
	return R_NONE;
#endif
}


/***********************************************************************
**
*/	REBNATIVE(request_password)
/*
***********************************************************************/
{
	REBREQ  req = {0};
	REBSER *str;

	OS_REQUEST_PASSWORD(&req);

	if (req.data == NULL) return R_NONE;

	str = Copy_OS_Str(req.data, req.actual);

	FREE_MEM(req.data);
	SET_STRING(D_RET, str);
	return R_RET;
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
			Set_String(D_RET, Copy_OS_Str(value, (REBCNT)LEN_STR(value)));
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

	if(result == NULL) Trap0(RE_NO_MEMORY);

	Set_Series(REB_MAP, D_RET, String_List_To_Block(result));
	FREE_MEM(result);
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(access_os)
/*
//	access-os: native [
//		{Access to various operating system functions (getuid, setuid, getpid, kill, etc.)}
//		field [word!] "Valid words: uid, euid, gid, egid, pid"
//		/set          "To set or kill pid (sig 15)"
//		value [integer! block!] "Argument, such as uid, gid, or pid (in which case, it could be a block with the signal no)"
//	]
//	
***********************************************************************/
{
	REBVAL *field = D_ARG(1);
	REBOOL set = D_REF(2);
	REBVAL *val = D_ARG(3);
	REBINT ret = 0;
	REBVAL *pid = 0;

	switch (VAL_WORD_CANON(field)) {
		case SYM_UID:
			if (set) {
				if (IS_INTEGER(val)) {
					ret = OS_SET_UID(VAL_INT32(val));
				} else {
					Trap_Arg(val);
				}
			} else {
				ret = OS_GET_UID();
			}
			break;
		case SYM_GID:
			if (set) {
				if (IS_INTEGER(val)) {
					ret = OS_SET_GID(VAL_INT32(val));
				} else {
					Trap_Arg(val);
				}
			} else {
				ret = OS_GET_GID();
			}
			break;
		case SYM_EUID:
			if (set) {
				if (IS_INTEGER(val)) {
					ret = OS_SET_EUID(VAL_INT32(val));
				} else {
					Trap_Arg(val);
				}
			} else {
				ret = OS_GET_EUID();
			}
			break;
		case SYM_EGID:
			if (set) {
				if (IS_INTEGER(val)) {
					ret = OS_SET_EGID(VAL_INT32(val));
				} else {
					Trap_Arg(val);
				}
			} else {
				ret = OS_GET_EGID();
			}
			break;
		case SYM_PID:
			if (set) {
				pid = val;
				//REBVAL *arg = val;
				if (IS_INTEGER(val)) {
					ret = OS_KILL(VAL_INT32(pid));
				} else if (IS_BLOCK(val)) {
					REBVAL *sig = NULL;

					if (VAL_LEN(val) != 2) {
						Trap_Arg(val);
					}
					pid = VAL_BLK_SKIP(val, 0);
					sig = VAL_BLK_SKIP(val, 1);
					if (!IS_INTEGER(pid)) {
						Trap_Arg(pid);
					}
					if (!IS_INTEGER(sig)) {
						Trap_Arg(sig);
					}
					ret = OS_SEND_SIGNAL(VAL_INT32(pid), VAL_INT32(sig));
					//arg = sig;
				} else {
					Trap_Arg(val);
				}
			} else {
				ret = OS_GET_PID();
			}
			break;
		default:
			Trap_Arg(field);
	}

	if(ret > 0) {
		SET_INTEGER(D_RET, ret);
		return R_RET;
	}

	if (ret == 0) {
		SET_TRUE(D_RET);
		return R_RET;
	}

	switch (ret) {
		case OS_ENA:
			Trap1(RE_NOT_HERE, field);
			break;
		case OS_EPERM:
			Trap0(RE_PERMISSION_DENIED);
			break;
		case OS_EINVAL:
			Trap_Arg(val);
			break;
		case OS_ESRCH:
			Trap1(RE_PROCESS_NOT_FOUND, pid);
			break;
		default:
			Trap_Arg(val);
			break;
	}
	// should not happen, but VS compiler wants to return value in all cases
	SET_FALSE(D_RET);
	return R_RET;
}
