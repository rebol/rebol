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
**  Module:  p-audio.c
**  Summary: Audio port interface
**  Section: ports
**  Author:  Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "reb-net.h"


/***********************************************************************
**
*/	static int Audio_Actor(REBVAL *ds, REBVAL *port_value, REBCNT action)
/*
***********************************************************************/
{
	REBSER *port;
	REBVAL *spec;
	REBREQ *req;
	REBINT result;
	REBVAL *arg;
	REBCNT len;
	REBOOL sync = FALSE; // act synchronously
	REBVAL tmp;

	port = Validate_Port_Value(port_value);

	arg = D_ARG(2);

	req = Use_Port_State(port, RDI_AUDIO, sizeof(REBREQ));

	printf("Audio device action: %i - ", action);


	switch (action) {
	case A_WRITE:
	puts("write");
		break;
	case A_READ:
	puts("read");
		if (!IS_OPEN(req)) {
			if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
			sync = TRUE;
		}
		// Issue the read request:
		result = OS_DO_DEVICE(req, RDC_READ);
		if (result < 0) Trap_Port(RE_READ_ERROR, port, req->error);
		if (sync) OS_DO_DEVICE(req, RDC_CLOSE);
		break;

	case A_PICK:  // FIRST - return result
	puts("pick");
		break;

	case A_OPEN:
	puts("open");
		if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
		break;

	case A_CLOSE:
	puts("close");
		OS_DO_DEVICE(req, RDC_CLOSE);
		break;

	case A_OPENQ:
	puts("open?");
		if (IS_OPEN(req)) return R_TRUE;
		return R_FALSE;

	default:
	puts("not supported command");
		Trap_Action(REB_PORT, action);
	}
	return R_ARG1; //= port
}


/***********************************************************************
**
*/	void Init_Audio_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_AUDIO, 0, Audio_Actor);
}
