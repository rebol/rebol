/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2019 Rebol Open Source Developers
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
**  Module:  p-midi.c
**  Summary: MIDI port interface
**  Section: ports
**  Author:  Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

/***********************************************************************
**
*/	static int MIDI_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
***********************************************************************/
{
	REBREQ *req;
	REBINT result;
	REBVAL *arg;
	REBCNT refs;	// refinement argument flags
	REBINT len;
	REBSER *ser;

	printf("MIDI_Actor action: %i\n", action);

	Validate_Port(port, action);

	arg = D_ARG(2);

	req = Use_Port_State(port, RDI_MIDI, sizeof(REBREQ));

	switch (action) {

	case A_READ:
		// This device is opened on the READ:
#ifdef unused
		if (!IS_OPEN(req)) {
			if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
		}
		// Issue the read request:
		CLR_FLAG(req->flags, RRF_WIDE); // allow byte or wide chars
		result = OS_DO_DEVICE(req, RDC_READ);
		if (result < 0) Trap_Port(RE_READ_ERROR, port, req->error);

		// Copy and set the string result:
		arg = OFV(port, STD_PORT_DATA);

		// If wide, correct length:
		len = req->actual;
		if (GET_FLAG(req->flags, RRF_WIDE)) len /= sizeof(REBUNI);

		// Copy the string (convert to latin-8 if it fits):
		Set_String(arg, Copy_OS_Str(req->data, len));

		OS_FREE(req->data); // release the copy buffer
		req->data = 0;
#endif
		*D_RET = *arg;
		return R_RET;

	case A_WRITE:

		break;

	case A_OPEN:
		if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
		break;

	case A_CLOSE:
		OS_DO_DEVICE(req, RDC_CLOSE);
		break;

	case A_QUERY:
		OS_DO_DEVICE(req, RDC_QUERY);
		break;

	case A_OPENQ:
		if (IS_OPEN(req)) return R_TRUE;
		return R_FALSE;

	default:
		Trap_Action(REB_PORT, action);
	}

	return R_ARG1; // port
}


/***********************************************************************
**
*/	void Init_MIDI_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_MIDI, 0, MIDI_Actor);
}
