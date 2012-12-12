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
**  Module:  p-file.c
**  Summary: file port interface
**  Section: ports
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

// For reference to port/state series that holds the file structure:
#define AS_FILE(s) ((REBREQ*)VAL_BIN(s))
#define READ_MAX ((REBCNT)(-1))
#define HL64(v) (v##l + (v##h << 32))
#define MAX_READ_MASK 0x7FFFFFFF // max size per chunk


/***********************************************************************
**
*/	static void Setup_File(REBREQ *file, REBCNT args, REBVAL *path)
/*
**		Convert native action refinements to file modes.
**
***********************************************************************/
{
	REBSER *ser;

	if (args & AM_OPEN_WRITE) SET_FLAG(file->modes, RFM_WRITE);
	if (args & AM_OPEN_READ) SET_FLAG(file->modes, RFM_READ);
	if (args & AM_OPEN_SEEK) SET_FLAG(file->modes, RFM_SEEK);

	if (args & AM_OPEN_NEW) {
		SET_FLAG(file->modes, RFM_NEW);
		if (!(args & AM_OPEN_WRITE)) Trap1(RE_BAD_FILE_MODE, path);
	}

	// Convert file name to OS format, let it GC later.
	if (!(ser = Value_To_OS_Path(path)))
		Trap1(RE_BAD_FILE_PATH, path);
	
	file->file.path = (REBCHR*)(ser->data);

	SET_FLAG(file->modes, RFM_NAME_MEM);

	Secure_Port(SYM_FILE, file, path, ser);
}


/***********************************************************************
**
*/	static void Cleanup_File(REBREQ *file)
/*
***********************************************************************/
{
	if (GET_FLAG(file->modes, RFM_NAME_MEM)) {
		//NOTE: file->file.path will get GC'd
		file->file.path = 0;
		CLR_FLAG(file->modes, RFM_NAME_MEM);
	}
	SET_CLOSED(file);
}


/***********************************************************************
**
*/	static void Set_File_Date(REBREQ *file, REBVAL *val)
/*
**		Set a value with the UTC date of a file.
**
***********************************************************************/
{
	REBOL_DAT dat;

	OS_FILE_TIME(file, &dat);
	Set_Date(val, &dat);
}


/***********************************************************************
**
*/	void Ret_Query_File(REBSER *port, REBREQ *file, REBVAL *ret)
/*
**		Query file and set RET value to resulting STD_FILE_INFO object.
**
***********************************************************************/
{
	REBVAL *info = In_Object(port, STD_PORT_SCHEME, STD_SCHEME_INFO, 0);
	REBSER *obj;
	REBSER *ser;

	if (!info || !IS_OBJECT(info)) Trap_Port(RE_INVALID_SPEC, port, -10);

	obj = CLONE_OBJECT(VAL_OBJ_FRAME(info));

	SET_OBJECT(ret, obj);
	Init_Word(OFV(obj, STD_FILE_INFO_TYPE), GET_FLAG(file->modes, RFM_DIR) ? SYM_DIR : SYM_FILE);
	SET_INTEGER(OFV(obj, STD_FILE_INFO_SIZE), file->file.size);
	Set_File_Date(file, OFV(obj, STD_FILE_INFO_DATE));

	ser = To_REBOL_Path(file->file.path, 0, OS_WIDE, 0);

	Set_Series(REB_FILE, OFV(obj, STD_FILE_INFO_NAME), ser);
}


/***********************************************************************
**
*/	static void Open_File_Port(REBSER *port, REBREQ *file, REBVAL *path)
/*
**		Open a file port.
**
***********************************************************************/
{
	if (Is_Port_Open(port)) Trap1(RE_ALREADY_OPEN, path);

	if (OS_DO_DEVICE(file, RDC_OPEN) < 0) Trap_Port(RE_CANNOT_OPEN, port, file->error);

	Set_Port_Open(port, TRUE);
}


REBINT Mode_Syms[] = {
    SYM_OWNER_READ,
    SYM_OWNER_WRITE,
    SYM_OWNER_EXECUTE,
    SYM_GROUP_READ,
    SYM_GROUP_WRITE,
    SYM_GROUP_EXECUTE,
    SYM_WORLD_READ,
    SYM_WORLD_WRITE,
    SYM_WORLD_EXECUTE,
	0
};


/***********************************************************************
**
*/	static REBCNT Get_Mode_Id(REBVAL *word)
/*
***********************************************************************/
{
	REBCNT id = 0;
	if (IS_WORD(word)) {
		id = Find_Int(&Mode_Syms[0], VAL_WORD_CANON(word));
		if (id == NOT_FOUND) Trap_Arg(word);
	}
	return id;
}


/***********************************************************************
**
*/	static REBCNT Set_Mode_Value(REBREQ *file, REBCNT mode, REBVAL *val)
/*
***********************************************************************/
{
	return 0;
}


/***********************************************************************
**
*/	static void Read_File_Port(REBSER *port, REBREQ *file, REBVAL *path, REBCNT args, REBCNT len)
/*
**		Read from a file port.
**
***********************************************************************/
{
	REBSER *ser;
	REBVAL *ds = DS_RETURN;

	// Allocate read result buffer:
	ser = Make_Binary(len);
	Set_Series(REB_BINARY, ds, ser); //??? what if already set?

	// Do the read, check for errors:
	file->data = BIN_HEAD(ser);
	file->length = len;
	if (OS_DO_DEVICE(file, RDC_READ) < 0) Trap_Port(RE_READ_ERROR, port, file->error);
	SERIES_TAIL(ser) = file->actual;
	STR_TERM(ser);

	// Convert to string or block of strings.
	// NOTE: This code is incorrect for files read in chunks!!!
	if (args & (AM_READ_STRING | AM_READ_LINES)) {
		ser = Decode_UTF_String(BIN_HEAD(ser), file->actual, -1);
		Set_String(ds, ser);
		if (args & AM_READ_LINES) Set_Block(ds, Split_Lines(ds));
	}
}


/***********************************************************************
**
*/	static void Write_File_Port(REBREQ *file, REBVAL *data, REBCNT len, REBCNT args)
/*
***********************************************************************/
{
	REBSER *ser;

	if (IS_BLOCK(data)) {
		// Form the values of the block
		// !! Could be made more efficient if we broke the FORM
		// into 32K chunks for writing.
		REB_MOLD mo = {0};
		Reset_Mold(&mo);
		if (args & AM_WRITE_LINES) {
			mo.opts = 1 << MOPT_LINES;
		}
		Mold_Value(&mo, data, 0);
		Set_String(data, mo.series); // fall into next section
		len = SERIES_TAIL(mo.series);
	}

	// Auto convert string to UTF-8
	if (IS_STRING(data)) {
		ser = Encode_UTF8_Value(data, len, ENCF_OS_CRLF);
		file->data = ser? BIN_HEAD(ser) : VAL_BIN_DATA(data); // No encoding may be needed
		len = SERIES_TAIL(ser);
	}
	else {
		file->data = VAL_BIN_DATA(data);
	}
	file->length = len;
	OS_DO_DEVICE(file, RDC_WRITE);
}


/***********************************************************************
**
*/	static REBCNT Set_Length(const REBVAL *ds, const REBREQ *file, const REBCNT arg)
/*
**		Computes the length of data based on the argument number
**		provided for the ARG_*_PART stack value (which, when there,
**		is always followed by the size).
**
**		Note: converts 64bit number to 32bit. The requested size
**		can never be greater than 4GB.
**
***********************************************************************/
{
	REBI64 len;  // maximum size
	REBI64 cnt;
	int what_if_it_changed;

	// Compute and bound bytes remaining:
	len = file->file.size - file->file.index; // already read
	if (len < 0) return 0;
	len &= MAX_READ_MASK; // limit the size

	// Return requested length:
	if (!D_REF(arg)) return (REBCNT)len;

	// Limit size of requested read:
	cnt = VAL_INT64(D_ARG(arg+1));
	if (cnt > len) return (REBCNT)len;
	return (REBCNT)cnt;
}


/***********************************************************************
**
*/	static void Set_Seek(REBREQ *file, REBVAL *arg)
/*
**		Computes the number of bytes that should be skipped.
**
***********************************************************************/
{
	REBI64 cnt;

	cnt = Int64s(arg, 0);

	if (cnt > file->file.size) cnt = file->file.size;

	file->file.index = cnt;

	SET_FLAG(file->modes, RFM_RESEEK); // force a seek
}


/***********************************************************************
**
*/	static int File_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
**		Internal port handler for files.
**
***********************************************************************/
{
	REBVAL *spec;
	REBVAL *path;
	REBREQ *file = 0;
	REBCNT args = 0;
	REBCNT len;
	REBOOL opened = FALSE;	// had to be opened (shortcut case)

	//Print("FILE ACTION: %r", Get_Action_Word(action));

	Validate_Port(port, action);

	*D_RET = *D_ARG(1);

	// Validate PORT fields:
	spec = BLK_SKIP(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap1(RE_INVALID_SPEC, spec);
	path = Obj_Value(spec, STD_PORT_SPEC_HEAD_REF);
	if (!path) Trap1(RE_INVALID_SPEC, spec);

	if (IS_URL(path)) path = Obj_Value(spec, STD_PORT_SPEC_HEAD_PATH);
	else if (!IS_FILE(path)) Trap1(RE_INVALID_SPEC, path);

	// Get or setup internal state data:
	file = (REBREQ*)Use_Port_State(port, RDI_FILE, sizeof(*file));

	switch (action) {

	case A_READ:
		args = Find_Refines(ds, ALL_READ_REFS);

		// Handle the READ %file shortcut case:
		if (!IS_OPEN(file)) {
			REBCNT nargs = AM_OPEN_READ;
			if (args & AM_READ_SEEK) nargs |= AM_OPEN_SEEK;
			Setup_File(file, nargs, path);
			Open_File_Port(port, file, path);
			opened = TRUE;
		}

		if (args & AM_READ_SEEK) Set_Seek(file, D_ARG(ARG_READ_INDEX));
		len = Set_Length(ds, file, ARG_READ_PART);
		Read_File_Port(port, file, path, args, len);

		if (opened) {
			OS_DO_DEVICE(file, RDC_CLOSE);
			Cleanup_File(file);
			Free_Port_State(port);
		}

		if (file->error) Trap_Port(RE_READ_ERROR, port, file->error);
		break;

	case A_APPEND:
		file->file.index = file->file.size;
		SET_FLAG(file->modes, RFM_RESEEK);

	case A_WRITE:
		args = Find_Refines(ds, ALL_WRITE_REFS);
		spec = D_ARG(2); // data (binary, string, or block)

		// Handle the READ %file shortcut case:
		if (!IS_OPEN(file)) {
			REBCNT nargs = AM_OPEN_WRITE;
			if (args & AM_WRITE_SEEK || args & AM_WRITE_APPEND) nargs |= AM_OPEN_SEEK;
			else nargs |= AM_OPEN_NEW;
			Setup_File(file, nargs, path);
			Open_File_Port(port, file, path);
			opened = TRUE;
		}
		else {
			if (!GET_FLAG(file->modes, RFM_WRITE)) Trap1(RE_READ_ONLY, path);
		}

		// Setup for /append or /seek:
		if (args & AM_WRITE_APPEND) {
			file->file.index = -1; // append
			SET_FLAG(file->modes, RFM_RESEEK);
		}
		if (args & AM_WRITE_SEEK) Set_Seek(file, D_ARG(ARG_WRITE_INDEX));

		// Determine length. Clip /PART to size of string if needed.
		len = VAL_LEN(spec);
		if (args & AM_WRITE_PART) {
			REBCNT n = Int32s(D_ARG(ARG_WRITE_LENGTH), 0);
			if (n <= len) len = n;
		}

		Write_File_Port(file, spec, len, args);

		if (opened) {
			OS_DO_DEVICE(file, RDC_CLOSE);
			Cleanup_File(file);
			Free_Port_State(port);
		}

		if (file->error) Trap1(RE_WRITE_ERROR, path);
		break;

	case A_OPEN:
		args = Find_Refines(ds, ALL_OPEN_REFS);
		// Default file modes if not specified:
		if (!(args & (AM_OPEN_READ | AM_OPEN_WRITE))) args |= (AM_OPEN_READ | AM_OPEN_WRITE);
		Setup_File(file, args, path);
		Open_File_Port(port, file, path); // !!! needs to change file modes to R/O if necessary
		break;

	case A_COPY:
		if (!IS_OPEN(file)) Trap1(RE_NOT_OPEN, path); //!!!! wrong msg
		len = Set_Length(ds, file, 2);
		Read_File_Port(port, file, path, args, len);
		break;

	case A_OPENQ:
		if (IS_OPEN(file)) return R_TRUE;
		return R_FALSE;

	case A_CLOSE:
		if (IS_OPEN(file)) {
			OS_DO_DEVICE(file, RDC_CLOSE);
			Cleanup_File(file);
			Free_Port_State(port);
		}
		break;

	case A_DELETE:
		if (IS_OPEN(file)) Trap1(RE_NO_DELETE, path);
		Setup_File(file, 0, path);
		if (OS_DO_DEVICE(file, RDC_DELETE) < 0 ) Trap1(RE_NO_DELETE, path);
		break;

	case A_RENAME:
		if (IS_OPEN(file)) Trap1(RE_NO_RENAME, path);
		else {
			REBSER *target;

			Setup_File(file, 0, path);

			// Convert file name to OS format:
			if (!(target = Value_To_OS_Path(D_ARG(2))))
				Trap1(RE_BAD_FILE_PATH, D_ARG(2));
			file->data = BIN_DATA(target);
			OS_DO_DEVICE(file, RDC_RENAME);
			Free_Series(target);
			if (file->error) Trap1(RE_NO_RENAME, path);
		}
		break;

	case A_CREATE:
		// !!! should it leave file open???
		if (!IS_OPEN(file)) {
			Setup_File(file, AM_OPEN_WRITE | AM_OPEN_NEW, path);
			if (OS_DO_DEVICE(file, RDC_CREATE) < 0) Trap_Port(RE_CANNOT_OPEN, port, file->error);
			OS_DO_DEVICE(file, RDC_CLOSE);
		}
		break;

	case A_QUERY:
		if (!IS_OPEN(file)) {
			Setup_File(file, 0, path);
			if (OS_DO_DEVICE(file, RDC_QUERY) < 0) return R_NONE;
		}
		Ret_Query_File(port, file, D_RET);
		// !!! free file path?
		break;

	case A_MODIFY:
		Set_Mode_Value(file, Get_Mode_Id(D_ARG(2)), D_ARG(3));
		if (!IS_OPEN(file)) {
			Setup_File(file, 0, path);
			if (OS_DO_DEVICE(file, RDC_MODIFY) < 0) return R_NONE;
		}
		return R_TRUE;
		break;

	case A_INDEXQ:
		SET_INTEGER(D_RET, file->file.index + 1);
		break;

	case A_LENGTHQ:
		SET_INTEGER(D_RET, file->file.size - file->file.index); // !clip at zero
		break;

	case A_HEAD:
		file->file.index = 0;
		goto seeked;

    case A_TAIL:
		file->file.index = file->file.size;
		goto seeked;

	case A_NEXT:
		file->file.index++;
		goto seeked;

	case A_BACK:
		if (file->file.index > 0) file->file.index--;
		goto seeked;

	case A_SKIP:
		file->file.index += Get_Num_Arg(D_ARG(2));
		goto seeked;

    case A_HEADQ:
		DECIDE(file->file.index == 0);

    case A_TAILQ:
		DECIDE(file->file.index >= file->file.size);

    case A_PASTQ:
		DECIDE(file->file.index > file->file.size);

	case A_CLEAR:
		// !! check for write enabled?
		SET_FLAG(file->modes, RFM_RESEEK);
		SET_FLAG(file->modes, RFM_TRUNCATE);
		file->length = 0;
		if (OS_DO_DEVICE(file, RDC_WRITE) < 0) Trap1(RE_WRITE_ERROR, path);
		break;

	/* Not yet implemented:
		A_AT,					// 38
		A_PICK,					// 41
		A_PATH,					// 42
		A_PATH_SET,				// 43
		A_FIND,					// 44
		A_SELECT,				// 45
		A_TAKE,					// 49
		A_INSERT,				// 50
		A_REMOVE,				// 52
		A_CHANGE,				// 53
		A_POKE,					// 54
		A_QUERY,				// 64
		A_FLUSH,				// 65
	*/

	default:
		Trap_Action(REB_PORT, action);
	}

	return R_RET;

seeked:
	SET_FLAG(file->modes, RFM_RESEEK);
	return R_ARG1;

is_true:
	return R_TRUE;

is_false:
	return R_FALSE;
}


/***********************************************************************
**
*/	void Init_File_Scheme(void)
/*
**		Associate the FILE:// scheme with the above native
**		actions. This will later be used by SET-SCHEME when
**		the scheme is initialized.
**
***********************************************************************/
{
	Register_Scheme(SYM_FILE, 0, File_Actor);
}


#ifdef low_usage
	// was in Read_File above...
	if (args & AM_READ_LINES) {
		REBYTE *bp = BIN_HEAD(ser);
		REBYTE *lp;
		REBSER *blk = Make_Block(1 + Count_Lines(bp, len));
		REBVAL *val = Append_Value(blk);
		Set_Binary(val, ser); // temp - keep it save from GC
		Set_Block(ds, blk); // accounts for GC
		while (*bp) {
			lp = bp;
			len = Next_Line(&bp);
			val = Append_Value(blk);
			Set_String(val, Decode_UTF8_Series(lp, len));
		}
		Remove_Series(blk, 0, 1); // remove temp binary
	}
#endif
