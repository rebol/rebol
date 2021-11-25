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
**  Module:  t-handle.c
**  Summary: handle datatype
**  Section: datatypes
**  Author:  Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

/***********************************************************************
**
*/	REBINT Cmp_Handle(REBVAL *a, REBVAL *b)
/*
***********************************************************************/
{
	REBYTE *sp;
	REBYTE *tp;
	if (IS_CONTEXT_HANDLE(a)) {
		if (IS_CONTEXT_HANDLE(b)) {
			if (VAL_HANDLE_SYM(a) != VAL_HANDLE_SYM(b)) {
				// comparing 2 context handles of different types
				sp = VAL_HANDLE_NAME(a);
				tp = VAL_HANDLE_NAME(b);
				return Compare_UTF8(sp, tp, (REBCNT)LEN_BYTES(tp)) + 2;
			}
		}
		else {
			// comparing context-handle with data-handle
			return -1;
		}
	}
	else if (IS_CONTEXT_HANDLE(b)) {
		// comparing data-handle with context-handle
		return 1;
	}
	// same ctx-handle types or both data handles
	return (VAL_HANDLE_I32(a) - VAL_HANDLE_I32(b));
}

/***********************************************************************
**
*/	REBINT CT_Handle(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT diff;
	if (mode > 0) {
		return ((VAL_HANDLE_FLAGS(a) == VAL_HANDLE_FLAGS(b))
			&&  (VAL_HANDLE_DATA(a)  == VAL_HANDLE_DATA(b)));
	}
	else if (mode == 0) {
		return (IS_CONTEXT_HANDLE(a) && IS_CONTEXT_HANDLE(b)
			&& (VAL_HANDLE_SYM(a) == VAL_HANDLE_SYM(b)));
	}
	else {
		diff = Cmp_Handle(a, b);
		if (mode == -1) return (diff >= 0);
		return (diff > 0);
	}
}


/***********************************************************************
**
*/	REBFLG MT_Handle(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	return FALSE;
}


/***********************************************************************
**
*/	REBTYPE(Handle)
/*
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);

	switch (action) {

	default:
		Trap_Action(VAL_TYPE(val), action);
	}

	return R_RET;
}
