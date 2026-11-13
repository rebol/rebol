/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
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
**  Module:  p-dir.c
**  Summary: file directory port interface
**  Section: ports
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

// Special policy: Win32 does not wanting tail slash for dir info
#define REMOVE_TAIL_SLASH (1<<10)

#define WILD_PATH(p) (Find_Str_Wild(VAL_SERIES(p), VAL_INDEX(p), VAL_TAIL(p)) != NOT_FOUND)


/***********************************************************************
**
*/	static int Read_Dir(REBREQ *dir, REBSER *files)
/*
**		Provide option to get file info too.
**		Provide option to prepend dir path.
**		Provide option to use wildcards.
**
***********************************************************************/
{
	REBINT result;
	REBCNT len;
	REBSER *fname;
	REBSER *name;
	REBREQ file;

	RESET_TAIL(files);
	CLEARS(&file);

	// Temporary filename storage:
	fname = BUF_OS_STR;
	file.file.path = (REBCHR*)Reset_Buffer(fname, MAX_FILE_NAME);

	SET_FLAG(dir->modes, RFM_DIR);

	dir->data = (REBYTE*)(&file);

#ifdef TO_WINDOWS
	if (dir->file.path[0] == 0) {
		// special case: reading drive letters -> read %/
		// https://github.com/Oldes/Rebol-issues/issues/2031
		SET_FLAG(dir->modes, RFM_DRIVES);
	}
#endif

	while ((result = OS_Do_Device(dir, RDC_READ)) == 0 && !GET_FLAG(dir->flags, RRF_DONE)) {
		len = (REBCNT)LEN_STR(file.file.path);
		if (GET_FLAG(file.modes, RFM_DIR)) len++;
		name = Copy_OS_Str(file.file.path, len);
		if (GET_FLAG(file.modes, RFM_DIR)) {
			SET_ANY_CHAR(name, name->tail-1, '/');
		}
		Set_Series(REB_FILE, Append_Value(files), name);
	}

	return result;
}


/***********************************************************************
**
*/	static void Init_Dir_Path(REBREQ *dir, REBVAL *path, REBINT wild, REBCNT policy)
/*
**		Convert REBOL dir path to file system path.
**		On Windows, we will also need to append a * if necessary.
**
**	ARGS:
**		Wild:
**			0 - no wild cards, path must end in / else error
**			1 - accept wild cards * and ?, and * if need
**		   -1 - not wild, if path does not end in /, add it
**
***********************************************************************/
{
	REBINT len;
	REBSER *os_path;
	REBSER *ser;
	REBVAL val = { 0 };
	//REBYTE *flags;

	SET_FLAG(dir->modes, RFM_DIR);
	SET_FLAG(dir->modes, policy);

	// We depend on To_Local_Path giving us 2 extra chars for / and *
	os_path = Value_To_OS_Path(path, TRUE);
	len = SERIES_TAIL(os_path);
	dir->file.path = (REBCHR*)(os_path->data);

	// Convert full OS path back to Rebol format.
	ser = To_REBOL_Path(BIN_HEAD(os_path), BIN_LEN(os_path), OS_WIDE, !wild);
	if (wild) {
		for (REBLEN i = 0; i < ser->tail; i++) {
			if (ser->data[i] == '*') {
				ser->tail = i;
				ser->data[0];
				break;
			}
		}
	}

	SET_FILE(&val, ser);

	Secure_Port(SYM_FILE, dir, &val);

	if (len == 0) return;
	if (len == 1 && dir->file.path[0] == '.') {
		if (wild > 0) {
			dir->file.path[0] = '*';
			dir->file.path[1] = 0;
		}
	}
	else if (len == 2 && dir->file.path[0] == '.' && dir->file.path[1] == '.') {
		// Insert * if needed:
		if (wild > 0) {
			dir->file.path[len++] = '/';
			dir->file.path[len++] = '*';
			dir->file.path[len] = 0;
		}
	}
	else if (dir->file.path[len-1] == '/' || dir->file.path[len-1] == '\\') {
		if (policy & REMOVE_TAIL_SLASH) {
			dir->file.path[len-1] = 0;
		}
		else {
			// Insert * if needed:
			if (wild > 0) {
				dir->file.path[len++] = '*';
				dir->file.path[len] = 0;
			}
		}
	} else {
		// Path did not end with /, so we better be wild:
		if (wild == 0) {
			///OS_FREE(dir->file.path);
			Trap1(RE_BAD_FILE_PATH, path);
		}
		else if (wild < 0) {
			dir->file.path[len++] = OS_DIR_SEP;
			dir->file.path[len] = 0;
		}
	}
}


/***********************************************************************
**
*/	static int Dir_Actor(REBVAL *ds, REBVAL *port_value, REBCNT action)
/*
**		Internal port handler for file directories.
**
**		Note: as Read_Dir is currently using static buffer for the path
**		there is no need to free dir.file.path
**
**		Using port/data to store intermediate file names.
**
***********************************************************************/
{
	REBSER *port;
	REBSER *target;
	REBVAL *spec;
	REBVAL *path;
	REBVAL *data;
	REBREQ *dir;
	REBINT result;

	//Debug_Fmt_("DIR ACTION: %r\n", Get_Action_Word(action));

	port = Validate_Port_With_Request(port_value, RDI_FILE, &dir);

	// Validate PORT fields:
	spec = OFV(port, STD_PORT_SPEC);
	path = Obj_Value(spec, STD_PORT_SPEC_HEAD_REF);
	if (!path) Trap1(RE_INVALID_SPEC, spec);
	if (IS_URL(path)) path = Obj_Value(spec, STD_PORT_SPEC_FILE_PATH);
	else if (!IS_FILE(path)) Trap1(RE_INVALID_SPEC, path);

	*D_RET = *D_ARG(1);

	data = BLK_SKIP(port, STD_PORT_DATA);

	switch (action) {

	case A_READ:
		// !!! ignores /SKIP and /PART, for now !!!
		if (!IS_BLOCK(data)) {
			Init_Dir_Path(dir, path, 1, POL_READ);
			Set_Block(data, Make_Block(7)); // initial guess
			result = Read_Dir(dir, VAL_SERIES(data));

			// don't throw an error if the original path contains wildcard chars * or ?
			if (result < 0 && !(result == -RFE_OPEN_FAIL && WILD_PATH(path)) ) {
				Trap_Port(RE_CANNOT_OPEN, port, dir->error);
			}
		}
		*D_RET = *data;
		SET_NONE(data);
		break;

	case A_CREATE:
create:
		if (IS_OPEN(dir)) Trap1(RE_ALREADY_OPEN, path); // already open
		Init_Dir_Path(dir, path, 0, POL_WRITE | REMOVE_TAIL_SLASH); // Sets RFM_DIR too
		SET_NONE(data);
		result = OS_Do_Device(dir, RDC_CREATE);
		if (result < 0) Trap1(RE_NO_CREATE, path);
		SET_OPEN(dir);
		if (action == A_CREATE) return R_ARG2;
		break;

	case A_RENAME:
		Init_Dir_Path(dir, path, 0, POL_WRITE | REMOVE_TAIL_SLASH); // Sets RFM_DIR too
		// Convert file name to OS format:
		if (!(target = Value_To_OS_Path(D_ARG(2), TRUE))) Trap1(RE_BAD_FILE_PATH, D_ARG(2));
		dir->data = BIN_DATA(target);
		OS_Do_Device(dir, RDC_RENAME);
		Free_Series(target);
		if (dir->error) Trap1(RE_NO_RENAME, path);
		break;

	case A_DELETE:
		SET_NONE(data);
		Init_Dir_Path(dir, path, 0, POL_WRITE);
		result = OS_Do_Device(dir, RDC_DELETE);
		SET_CLOSED(dir);
		if (result >=  0) return R_ARG2;
		if (result == -2) return R_FALSE;
		// else...
		Trap1(RE_NO_DELETE, path);
		break;

	case A_OPEN:
		if (D_REF(ARG_OPEN_NEW)) goto create;
		// If port is already open, just ignore it. 
		Init_Dir_Path(dir, path, 1, POL_READ);
		Set_Block(data, Make_Block(7));
		result = Read_Dir(dir, VAL_SERIES(data));
		if (result < 0) Trap_Port(RE_CANNOT_OPEN, port, dir->error);
		SET_OPEN(dir);
		break;

	case A_OPENQ:
		// as read reopens ports, does it make sense to return false here?
		if (IS_OPEN(dir)) return R_TRUE;
		return R_FALSE;

	case A_CLOSE:
		if (IS_OPEN(dir)) {
			if (dir->handle) OS_Do_Device(dir, RDC_CLOSE);
			SET_NONE(data);
			SET_CLOSED(dir);
		}
		break;

	case A_QUERY:
		if (IS_NONE(D_ARG(ARG_QUERY_FIELD))) {
			Ret_File_Modes(port, D_RET);
			return R_RET;
		}
		SET_NONE(data);
		Init_Dir_Path(dir, path, -1, POL_READ);
		if (OS_Do_Device(dir, RDC_QUERY) < 0) return R_NONE;
		Ret_Query_File(port, dir, D_RET, D_ARG(ARG_QUERY_FIELD));
		break;

	//-- Port Series Actions (only called if opened as a port)

	case A_LENGTHQ:
		SET_INTEGER(D_RET, IS_BLOCK(data) ? VAL_BLK_LEN(data) : 0);
		break;

	case A_TAILQ:
		if(IS_BLOCK(data)) {
			return (VAL_BLK_LEN(data) > 0) ? R_FALSE : R_TRUE;
		}
		Trap_Port(RE_NOT_OPEN, port, 0);

	default:
		Trap1(RE_NO_PORT_ACTION, Get_Action_Word(action));
	}

	if (!IS_OPEN(dir)) Release_Port_State(port);

	return R_RET;
}


/***********************************************************************
**
*/	void Init_Dir_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_DIR, 0, Dir_Actor);
}
