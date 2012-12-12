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
**  Module:  p-console.c
**  Summary: console port interface
**  Section: ports
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


#define OUT_BUF_SIZE 32*1024

// Does OS use wide chars or byte chars (UTF-8):
#ifdef OS_WIDE_CHAR
#define MAKE_OS_BUFFER Make_Unicode
#else
#define MAKE_OS_BUFFER Make_Binary
#endif

/***********************************************************************
**
*/	static int Console_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
***********************************************************************/
{
	REBREQ *req;
	REBINT result;
	REBVAL *arg = D_ARG(2);
	REBSER *ser;

	Validate_Port(port, action);

	arg = D_ARG(2);
	*D_RET = *D_ARG(1);

	req = Use_Port_State(port, RDI_STDIO, sizeof(REBREQ));

	switch (action) {

	case A_READ:

		// If not open, open it:
		if (!IS_OPEN(req)) {
			if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
		}

		// If no buffer, create a buffer:
		arg = OFV(port, STD_PORT_DATA);
		if (!IS_STRING(arg) && !IS_BINARY(arg)) {
			Set_Binary(arg, MAKE_OS_BUFFER(OUT_BUF_SIZE));
		}
		ser = VAL_SERIES(arg);
		RESET_SERIES(ser);

		req->data = BIN_HEAD(ser);
		req->length = SERIES_AVAIL(ser);

#ifdef nono
		// Is the buffer large enough?
		req->length = SERIES_AVAIL(ser); // space available
		if (req->length < OUT_BUF_SIZE/2) Extend_Series(ser, OUT_BUF_SIZE);
		req->length = SERIES_AVAIL(ser);

		// Don't make buffer too large:  Bug #174   ?????
		if (req->length > 1024) req->length = 1024;  //???
		req->data = STR_TAIL(ser); // write at tail  //???
		if (SERIES_TAIL(ser) == 0) req->actual = 0;  //???
#endif

		result = OS_DO_DEVICE(req, RDC_READ);
		if (result < 0) Trap_Port(RE_READ_ERROR, port, req->error);

#ifdef nono
		// Does not belong here!!
		// Remove or replace CRs:
		result = 0;
		for (n = 0; n < req->actual; n++) {
			chr = GET_ANY_CHAR(ser, n);
			if (chr == CR) {
				chr = LF;
				// Skip LF if it follows:
				if ((n+1) < req->actual &&
					LF == GET_ANY_CHAR(ser, n+1)) n++;
			}
			SET_ANY_CHAR(ser, result, chr);
			result++;
		}
#endif
		// Another copy???
		//Set_String(ds, Copy_OS_Str((void *)(ser->data), result));
		Set_Binary(ds, Copy_Bytes(req->data, req->actual));
		break;

	case A_OPEN:
		// ?? why???
		//if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port);
		SET_OPEN(req);
		break;

	case A_CLOSE:
		SET_CLOSED(req);
		//OS_DO_DEVICE(req, RDC_CLOSE);
		break;

	case A_OPENQ:
		if (IS_OPEN(req)) return R_TRUE;
		return R_FALSE;

	default:
		Trap_Action(REB_PORT, action);
	}

	return R_RET;
}


/***********************************************************************
**
*/	void Init_Console_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_CONSOLE, 0, Console_Actor);
}
