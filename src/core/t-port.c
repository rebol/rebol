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
**  Module:  t-port.c
**  Summary: port datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	REBINT CT_Port(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode < 0) return -1;
	return VAL_OBJ_FRAME(a) == VAL_OBJ_FRAME(b);
}


/***********************************************************************
**
*/	REBFLG MT_Port(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	return FALSE;
}


/***********************************************************************
**
*/	static REBVAL *As_Port(REBVAL *value)
/*
**		Make the port object if necessary.
**
***********************************************************************/
{
	REBVAL *ds;

	if (IS_PORT(value)) return value;

	value = Make_Port(value);
	ds = DS_RETURN;
	*D_ARG(1) = *value;

	return D_ARG(1);
}


/***********************************************************************
**
*/	REBTYPE(Port)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);
	REBVAL *arg = D_ARG(2);

	switch (action) {

	case A_READ:
	case A_WRITE:
	case A_QUERY:
	case A_OPEN:
	case A_CREATE:
	case A_DELETE:
	case A_RENAME:
		value = As_Port(value);
	case A_UPDATE:
	default:
		return Do_Port_Action(VAL_PORT(value), action); // Result on stack

	case A_REFLECT:
		return T_Object(ds, action);
		break;

	case A_MAKE:
		if (IS_DATATYPE(value)) value = Make_Port(arg);
		else Trap_Make(REB_PORT, value);
		break;

	case A_TO:
		if (!(IS_DATATYPE(value) && IS_OBJECT(arg))) Trap_Make(REB_PORT, arg);
		value = arg;
		VAL_SET(value, REB_PORT);
		break;
	}

	DS_Ret_Val(value);
	return R_RET;
}
