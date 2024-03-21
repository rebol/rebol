/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2022 Rebol Open Source Contributors
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
**  Module:  p-clipboard.c
**  Summary: clipboard port interface
**  Section: ports
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	static int Clipboard_Actor(REBVAL *ds, REBVAL *port_value, REBCNT action)
/*
***********************************************************************/
{
	REBSER *port;
	REBREQ *req;
	REBINT result;
	REBVAL *arg;
	REBCNT refs = 0;	// refinement argument flags
	REBINT len;
	REBSER *ser;

	port = Validate_Port_Value(port_value);

	arg = D_ARG(2);

	req = Use_Port_State(port, RDI_CLIPBOARD, sizeof(REBREQ));

	switch (action) {
	case A_UPDATE:
		// Update the port object after a READ or WRITE operation.
		// This is normally called by the WAKE-UP function.
		arg = OFV(port, STD_PORT_DATA);
		if (req->command == RDC_READ) {
			len = req->actual;
			if (GET_FLAG(req->flags, RRF_WIDE)) {
				len /= sizeof(REBUNI); //correct length
				// Copy the string (convert to latin-8 if it fits):
				Set_Binary(arg, Copy_Wide_Str(req->data, len));
			} else {
				Set_Binary(arg, Copy_OS_Str(req->data, len));
			}
		}
		else if (req->command == RDC_WRITE) {
			SET_NONE(arg);  // Write is done.
		}
		return R_NONE;

	case A_READ:
		refs = Find_Refines(ds, ALL_READ_REFS);
		// This device is opened on the READ:
		if (!IS_OPEN(req)) {
			if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
		}
			
		// Handle /part refinement:
		if (refs & AM_READ_PART) {
			req->length = VAL_INT32(D_ARG(ARG_READ_LENGTH));
		} else {
			req->length = 0;
		}
			
		// Issue the read request:
		CLR_FLAG(req->flags, RRF_WIDE); // allow byte or wide chars
		result = OS_DO_DEVICE(req, RDC_READ);
		if (result < 0) Trap_Port(RE_READ_ERROR, port, req->error);

		// Copy and set the string result:
		arg = OFV(port, STD_PORT_DATA);
		
		len = req->actual;
		if (GET_FLAG(req->flags, RRF_WIDE)) {
			len /= sizeof(REBUNI); //correct length
			// Copy the string (convert to latin-8 if it fits):
			Set_String(arg, Copy_Wide_Str(req->data, len));
		} else {
			Set_String(arg, Copy_OS_Str(req->data, len));
		}

		OS_FREE(req->data); // release the copy buffer
		req->data = 0;

		if (refs & AM_READ_LINES) {
			Set_Block(D_RET, Split_Lines(arg));
		} else {
			*D_RET = *arg;
		}
		return R_RET;

	case A_WRITE:
		if (!(IS_STRING(arg) || IS_BINARY(arg))) {
#ifdef WRITE_ANY_VALUE_TO_CLIPBOARD
			REB_MOLD mo = {0};
			Reset_Mold(&mo);
			Mold_Value(&mo, arg, TRUE);
			Set_String(arg, mo.series);
#else
			Trap1(RE_INVALID_PORT_ARG, arg);
#endif
		}
		// This device is opened on the WRITE:
		if (!IS_OPEN(req)) {
			if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
		}

		refs = Find_Refines(ds, ALL_WRITE_REFS);

		// Handle /part refinement:
		len = VAL_LEN(arg);
		if (refs & AM_WRITE_PART && VAL_INT32(D_ARG(ARG_WRITE_LENGTH)) < len)
			len = VAL_INT32(D_ARG(ARG_WRITE_LENGTH));

#ifdef TO_WINDOWS
		// Oldes: this code is very old and should be revisited!!!
		// If bytes, see if we can fit it:
		if (SERIES_WIDE(VAL_SERIES(arg)) == 1) {
			#ifdef ARG_STRINGS_ALLOWED
				if (Is_Not_ASCII(VAL_BIN_DATA(arg), len)) {
					Set_String(arg, Copy_Bytes_To_Unicode(VAL_BIN_DATA(arg), len));
				} else
					req->data = VAL_BIN_DATA(arg);
			#endif

			// Temp conversion:!!!
			ser = Make_Unicode(len);
			len = Decode_UTF8(UNI_HEAD(ser), VAL_BIN_DATA(arg), len, FALSE);
			SERIES_TAIL(ser) = len = abs(len);
			UNI_TERM(ser);
			Set_String(arg, ser);
			req->data = (REBYTE*) UNI_HEAD(ser);
			SET_FLAG(req->flags, RRF_WIDE);
		}
		else
		// If unicode (may be from above conversion), handle it:
		if (SERIES_WIDE(VAL_SERIES(arg)) == sizeof(REBUNI)) {
			req->data = (REBYTE *)VAL_UNI_DATA(arg);
			SET_FLAG(req->flags, RRF_WIDE);
		}

		// Temp!!!
		req->length = len * sizeof(REBUNI);
		
#else // macOS requires UTF8 encoded data (no clipboard support on other oses yet)
		ser = Encode_UTF8_Value(arg, len, 0);
		Set_String(arg, ser);
		req->data = VAL_BIN_DATA(arg);
		req->length = len;
#endif
		// Setup the write:
		*OFV(port, STD_PORT_DATA) = *arg;	// keep it GC safe
		req->actual = 0;

		result = OS_DO_DEVICE(req, RDC_WRITE);
		SET_NONE(OFV(port, STD_PORT_DATA)); // GC can collect it

		if (result < 0) Trap_Port(RE_WRITE_ERROR, port, req->error);
		//if (result == DR_DONE) SET_NONE(OFV(port, STD_PORT_DATA));
		break;

	case A_OPEN:
		if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
		break;

	case A_CLOSE:
		OS_DO_DEVICE(req, RDC_CLOSE);
		break;

	case A_OPENQ:
		if (IS_OPEN(req)) return R_TRUE;
		return R_FALSE;

	default:
		Trap1(RE_NO_PORT_ACTION, Get_Action_Word(action));
	}

	return R_ARG1; // port
}


/***********************************************************************
**
*/	void Init_Clipboard_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_CLIPBOARD, 0, Clipboard_Actor);
}
