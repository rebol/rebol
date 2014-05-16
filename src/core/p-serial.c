/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2013 REBOL Technologies
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
**  Module:  p-serial.c
**  Summary: serial port interface
**  Section: ports
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#include "reb-net.h"
#include "reb-evtypes.h"


/***********************************************************************
**
*/	static int Serial_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
***********************************************************************/
{
	REBREQ *req;	// IO request
	REBVAL *spec;	// port spec
	REBVAL *arg;	// action argument value
	REBVAL *val;	// e.g. port number value
	REBINT result;	// IO result
	REBCNT refs;	// refinement argument flags
	REBCNT len;		// generic length
	REBSER *ser;	// simplifier
	REBVAL *path;

	Validate_Port(port, action);

	*D_RET = *D_ARG(1);

	// Validate PORT fields:
	spec = OFV(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap0(RE_INVALID_PORT);
	path = Obj_Value(spec, STD_PORT_SPEC_HEAD_REF);
	if (!path) Trap1(RE_INVALID_SPEC, spec);

	//if (!IS_FILE(path)) Trap1(RE_INVALID_SPEC, path);

	req = Use_Port_State(port, RDI_SERIAL, sizeof(*req));

	// Actions for an unopened serial port:
	if (!IS_OPEN(req)) {

		switch (action) {

		case A_OPEN:
			arg = Obj_Value(spec, STD_PORT_SPEC_SERIAL_PATH);
			req->file.path = VAL_DATA(arg);
			arg = Obj_Value(spec, STD_PORT_SPEC_SERIAL_SPEED);
			req->file.size = VAL_INT32(arg); // used for baudrate
			//Secure_Port(SYM_SERIAL, ???, path, ser);
			if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, -12);
			SET_OPEN(req);
			return R_RET;

		case A_CLOSE:
			return R_RET;

		case A_OPENQ:
			return R_FALSE;

		default:
			Trap_Port(RE_NOT_OPEN, port, -12);
		}
	}

	// Actions for an open socket:
	switch (action) {

	case A_READ:
		refs = Find_Refines(ds, ALL_READ_REFS);

		// Setup the read buffer (allocate a buffer if needed):
		arg = OFV(port, STD_PORT_DATA);
		if (!IS_STRING(arg) && !IS_BINARY(arg)) {
			Set_Binary(arg, Make_Binary(32000));
		}
		ser = VAL_SERIES(arg);
		req->length = SERIES_AVAIL(ser); // space available
		if (req->length < 32000/2) Extend_Series(ser, 32000);
		req->length = SERIES_AVAIL(ser);
		req->data = STR_TAIL(ser); // write at tail
		//if (SERIES_TAIL(ser) == 0)
		req->actual = 0;  // Actual for THIS read, not for total.

		// printf("(max read length %d)", req->length);
		result = OS_DO_DEVICE(req, RDC_READ); // recv can happen immediately
		if (result < 0) Trap_Port(RE_READ_ERROR, port, req->error);
#ifdef DEBUG_SERIAL
		for (len = 0; len < req->actual; len++) {
			if (len % 16 == 0) printf("\n");
			printf("%02x ", req->data[len]);
		}
		printf("\n");
#endif
		Set_Binary(arg, Copy_Bytes(req->data, req->actual));
		*D_RET = *arg;
		return R_RET;

	case A_WRITE:
		refs = Find_Refines(ds, ALL_WRITE_REFS);

		// Determine length. Clip /PART to size of string if needed.
		spec = D_ARG(2);
		len = VAL_LEN(spec);
		if (refs & AM_WRITE_PART) {
			REBCNT n = Int32s(D_ARG(ARG_WRITE_LENGTH), 0);
			if (n <= len) len = n;
		}

		// Setup the write:
		*OFV(port, STD_PORT_DATA) = *spec;	// keep it GC safe
		req->length = len;
		req->data = VAL_BIN_DATA(spec);
		req->actual = 0;

		//Print("(write length %d)", len);
		result = OS_DO_DEVICE(req, RDC_WRITE); // send can happen immediately
		if (result < 0) Trap_Port(RE_WRITE_ERROR, port, req->error);
		if (result == DR_DONE) SET_NONE(OFV(port, STD_PORT_DATA));
		break;
#if 0
	case A_UPDATE:
		// Update the port object after a READ or WRITE operation.
		// This is normally called by the WAKE-UP function.
		arg = OFV(port, STD_PORT_DATA);
		if (req->command == RDC_READ) {
			if (ANY_BINSTR(arg)) VAL_TAIL(arg) += req->actual;
		}
		else if (req->command == RDC_WRITE) {
			SET_NONE(arg);  // Write is done.
		}
		return R_NONE;
#endif
	case A_OPENQ:
		return R_TRUE;

	case A_CLOSE:
		if (IS_OPEN(req)) {
			OS_DO_DEVICE(req, RDC_CLOSE);
			SET_CLOSED(req);
		}
		Free_Port_State(port);
		break;

	default:
		Trap_Action(REB_PORT, action);
	}

	return R_RET;
}


/***********************************************************************
**
*/	void Init_Serial_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_SERIAL, 0, Serial_Actor);
}
