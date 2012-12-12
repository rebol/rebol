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

	while ((result = OS_DO_DEVICE(dir, RDC_READ)) == 0 && !GET_FLAG(dir->flags, RRF_DONE)) {
		len = LEN_STR(file.file.path);
		if (GET_FLAG(file.modes, RFM_DIR)) len++;
		name = Copy_OS_Str(file.file.path, len);
		if (GET_FLAG(file.modes, RFM_DIR))
			SET_ANY_CHAR(name, name->tail-1, '/');
		Set_Series(REB_FILE, Append_Value(files), name);
	}

	if (result < 0 && dir->error != -RFE_OPEN_FAIL
		&& (FIND_CHR(dir->file.path, '*') || FIND_CHR(dir->file.path, '?')))
		result = 0;  // no matches found, but not an error

	return result;
}


#ifdef REMOVED
// It's problematic. See blog. Moved to mezz.

/***********************************************************************
**
*/	REBNATIVE(dirq)
/*
**	Refinements:
**		/any -- allow * and ? wildcards
**
**	Patterns:
**		abc/ is true
**		abc/*.r is true
**		abc/?.r is true
**		abc - ask the file system
**
***********************************************************************/
{
	REBVAL *path = D_ARG(1);
	REBINT len;
	REBINT i;
	REBCNT dot;
	REBUNI c;
	REBSER *ser = VAL_SERIES(path);

	if (!ANY_STR(path)) return R_FALSE;

	len = (REBINT)VAL_LEN(path);
	if (len == 0) return R_FALSE;

	// We cannot tell from above, so we must check it (if file):
	if (IS_FILE(path)) {
		REBSER *ser;
		REBREQ file;

		CLEARS(&file); 
		ser = Value_To_OS_Path(path);
		file.file.path = (REBCHR*)(ser->data);
		file.device = RDI_FILE;
		len = OS_DO_DEVICE(&file, RDC_QUERY);
		FREE_SERIES(ser);
		if (len == DR_DONE && GET_FLAG(file.modes, RFM_DIR)) return R_TRUE;
	}

	// Search backward for abc/, abc/def, abc/*, etc:
	len = (REBINT)VAL_LEN(path);
	dot = 0;
	for (i = 0; i < len; i++) {
		c = GET_ANY_CHAR(ser, VAL_TAIL(path)-1-i);
		if (c == '/' || c == '\\') {
			if (i == 0 || dot) return R_TRUE;
			break;
		}
		if (c == '.') {
			if (i == 0 || dot) dot = 1;
		}
		else dot = 0;
		if ((c == '*' || c == '?') && D_REF(2)) return R_TRUE;
	}

	return R_FALSE;
}
#endif


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
	REBSER *ser;
	//REBYTE *flags;

	SET_FLAG(dir->modes, RFM_DIR);

	// We depend on To_Local_Path giving us 2 extra chars for / and *
	ser = Value_To_OS_Path(path);
	len = ser->tail;
	dir->file.path = (REBCHR*)(ser->data);

	Secure_Port(SYM_FILE, dir, path, ser);

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
*/	static int Dir_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
**		Internal port handler for file directories.
**
***********************************************************************/
{
	REBVAL *spec;
	REBVAL *path;
	REBVAL *state;
	REBREQ dir;
	REBCNT args = 0;
	REBINT result;
	REBCNT len;
	//REBYTE *flags;

	Validate_Port(port, action);

	*D_RET = *D_ARG(1);
	CLEARS(&dir);

	// Validate and fetch relevant PORT fields:
	spec  = BLK_SKIP(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap1(RE_INVALID_SPEC, spec);
	path = Obj_Value(spec, STD_PORT_SPEC_HEAD_REF);
	if (!path) Trap1(RE_INVALID_SPEC, spec);

	if (IS_URL(path)) path = Obj_Value(spec, STD_PORT_SPEC_HEAD_PATH);
	else if (!IS_FILE(path)) Trap1(RE_INVALID_SPEC, path);
	
	state = BLK_SKIP(port, STD_PORT_STATE); // if block, then port is open.

	//flags = Security_Policy(SYM_FILE, path);

	// Get or setup internal state data:
	dir.port = port;
	dir.device = RDI_FILE;

	switch (action) {

	case A_READ:
		//Trap_Security(flags[POL_READ], POL_READ, path);
		args = Find_Refines(ds, ALL_READ_REFS);
		if (!IS_BLOCK(state)) {		// !!! ignores /SKIP and /PART, for now
			Init_Dir_Path(&dir, path, 1, POL_READ);
			Set_Block(state, Make_Block(7)); // initial guess
			result = Read_Dir(&dir, VAL_SERIES(state));
			///OS_FREE(dir.file.path);
			if (result < 0) Trap_Port(RE_CANNOT_OPEN, port, dir.error);
			*D_RET = *state;
			SET_NONE(state);
		} else {
			len = VAL_BLK_LEN(state);
			// !!? Why does this need to copy the block??
			Set_Block(D_RET, Copy_Block_Values(VAL_SERIES(state), 0, len, TS_STRING));
		}
		break;

	case A_CREATE:
		//Trap_Security(flags[POL_WRITE], POL_WRITE, path);
		if (IS_BLOCK(state)) Trap1(RE_ALREADY_OPEN, path); // already open
create:
		Init_Dir_Path(&dir, path, 0, POL_WRITE | REMOVE_TAIL_SLASH); // Sets RFM_DIR too
		result = OS_DO_DEVICE(&dir, RDC_CREATE);
		///OS_FREE(dir.file.path);
		if (result < 0) Trap1(RE_NO_CREATE, path);
		if (action == A_CREATE) return R_ARG2;
		SET_NONE(state);
		break;

	case A_RENAME:
		if (IS_BLOCK(state)) Trap1(RE_ALREADY_OPEN, path); // already open
		else {
			REBSER *target;

			Init_Dir_Path(&dir, path, 0, POL_WRITE | REMOVE_TAIL_SLASH); // Sets RFM_DIR too
			// Convert file name to OS format:
			if (!(target = Value_To_OS_Path(D_ARG(2)))) Trap1(RE_BAD_FILE_PATH, D_ARG(2));
			dir.data = BIN_DATA(target);
			OS_DO_DEVICE(&dir, RDC_RENAME);
			Free_Series(target);
			if (dir.error) Trap1(RE_NO_RENAME, path);
		}
		break;

	case A_DELETE:
		//Trap_Security(flags[POL_WRITE], POL_WRITE, path);
		SET_NONE(state);
		Init_Dir_Path(&dir, path, 0, POL_WRITE);
		// !!! add *.r deletion
		// !!! add recursive delete (?)
		result = OS_DO_DEVICE(&dir, RDC_DELETE);
		///OS_FREE(dir.file.path);
		if (result < 0) Trap1(RE_NO_DELETE, path);
		return R_ARG2;

	case A_OPEN:
		// !! If open fails, what if user does a READ w/o checking for error?
		if (IS_BLOCK(state)) Trap1(RE_ALREADY_OPEN, path); // already open
		//Trap_Security(flags[POL_READ], POL_READ, path);
		args = Find_Refines(ds, ALL_OPEN_REFS);
		if (args & AM_OPEN_NEW) goto create;
		//if (args & ~AM_OPEN_READ) Trap1(RE_INVALID_SPEC, path);
		Set_Block(state, Make_Block(7));
		Init_Dir_Path(&dir, path, 1, POL_READ);
		result = Read_Dir(&dir, VAL_SERIES(state));
		///OS_FREE(dir.file.path);
		if (result < 0) Trap_Port(RE_CANNOT_OPEN, port, dir.error);
		break;

	case A_OPENQ:
		if (IS_BLOCK(state)) return R_TRUE;
		return R_FALSE;

	case A_CLOSE:
		SET_NONE(state);
		break;

	case A_QUERY:
		//Trap_Security(flags[POL_READ], POL_READ, path);
		SET_NONE(state);
		Init_Dir_Path(&dir, path, -1, REMOVE_TAIL_SLASH | POL_READ);
		if (OS_DO_DEVICE(&dir, RDC_QUERY) < 0) return R_NONE;
		Ret_Query_File(port, &dir, D_RET);
		///OS_FREE(dir.file.path);
		break;

	//-- Port Series Actions (only called if opened as a port)

	case A_LENGTHQ:
		len = IS_BLOCK(state) ? VAL_BLK_LEN(state) : 0;
		SET_INTEGER(D_RET, len);
		break;

	default:
		Trap_Action(REB_PORT, action);
	}

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
