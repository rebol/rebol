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
**  Module:  t-none.c
**  Summary: none datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

/***********************************************************************
**
*/	REBINT CT_None(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode >= 0) return (VAL_TYPE(a) == VAL_TYPE(b));
	return -1;
}


/***********************************************************************
**
*/	REBFLG MT_None(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	VAL_SET(out, type);
	return TRUE;
}


/***********************************************************************
**
*/	REBTYPE(None)
/*
**		ALSO used for unset!
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);

	switch (action) {

	case A_MAKE:
	case A_TO:
		if (IS_DATATYPE(val))
			return VAL_DATATYPE(val) == REB_NONE ? R_NONE : R_UNSET;
		else
			return IS_NONE(val) ? R_NONE : R_UNSET;

	case A_TAILQ:
		if (IS_NONE(val)) return R_TRUE;
		goto trap_it;
	case A_INDEXQ:
	case A_LENGTHQ:
	case A_SELECT:
	case A_FIND:
	case A_REMOVE:
	case A_CLEAR:
	case A_TAKE:
		if (IS_NONE(val)) return R_NONE;
	default:
	trap_it:
		Trap_Action(VAL_TYPE(val), action);
	}

	return R_RET;
}
