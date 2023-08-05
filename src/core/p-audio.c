/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
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
**  Notes:   prototype
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
	REBCNT refs = 0;	// refinement argument flags
	REBCNT len;
	//REBOOL sync = FALSE; // act synchronously

	port = Validate_Port_Value(port_value);

	arg = D_ARG(2);

	req = Use_Port_State(port, RDI_AUDIO, sizeof(REBREQ));

	switch (action) {
	case A_WRITE:
	case A_INSERT:
		if (!(IS_BINARY(arg) || IS_VECTOR(arg) || IS_LOGIC(arg)))
			Trap1(RE_INVALID_PORT_ARG, arg);

		if (!IS_OPEN(req)) {
			if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
			//sync = TRUE;
		}

		REBVAL *data = BLK_SKIP(port, STD_PORT_DATA);
		if (IS_LOGIC(arg)) {
			if (VAL_LOGIC(arg) == FALSE) {
				req->modify.mode = MODE_AUDIO_PLAY;
				req->modify.value = FALSE;
				result = OS_DO_DEVICE(req, RDC_MODIFY);
				break;
			}
			if (IS_VECTOR(data) || IS_BINARY(data)) {
				req->data = VAL_BIN_DATA(data);
				req->length = VAL_TAIL(data) - VAL_INDEX(data);
				if (IS_VECTOR(data)) {
					req->length *= VAL_VEC_WIDTH(data); // length in raw bytes
				}
			}
			else break;
		}
		else {
			VAL_SERIES(data) = VAL_SERIES(arg);
			VAL_TYPE(data) = VAL_TYPE(arg);
			PROTECT_SERIES(VAL_SERIES(data));

			len = Partial(arg, 0, D_ARG(ARG_WRITE_LENGTH), 0);

			req->data = VAL_BIN_DATA(arg);
			req->length = len;

			if (IS_VECTOR(arg)) {
				// length in raw bytes
				req->length *= VAL_VEC_WIDTH(arg);
			}
		}

		result = OS_DO_DEVICE(req, RDC_WRITE);
		if (result < 0) Trap_Port(RE_WRITE_ERROR, port, req->error);
		//if (sync) OS_DO_DEVICE(req, RDC_CLOSE);
		break;
	case A_READ:
		break;

	case A_PICK:  // FIRST - return result
		break;

	case A_OPEN:
		spec = OFV(port, STD_PORT_SPEC);
		if (IS_OBJECT(spec)) {
			arg = Obj_Value(spec, STD_PORT_SPEC_AUDIO_CHANNELS);
			if (IS_INTEGER(arg)) {
				req->audio.channels = VAL_INT32(arg);
			}
			arg = Obj_Value(spec, STD_PORT_SPEC_AUDIO_RATE);
			if (IS_INTEGER(arg)) {
				req->audio.rate = VAL_INT32(arg);
			}
			arg = Obj_Value(spec, STD_PORT_SPEC_AUDIO_BITS);
			if (IS_INTEGER(arg)) {
				req->audio.bits = VAL_INT32(arg);
			}
			arg = Obj_Value(spec, STD_PORT_SPEC_AUDIO_SAMPLE_TYPE);
			if (IS_INTEGER(arg)) {
				req->audio.type = VAL_INT32(arg);
			}
			arg = Obj_Value(spec, STD_PORT_SPEC_AUDIO_LOOP_COUNT);
			if (IS_INTEGER(arg)) {
				req->audio.loop_count = MIN(MAX_U32, VAL_UNT64(arg));
			}
		}
		if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);
		break;

	case A_CLOSE:
		OS_DO_DEVICE(req, RDC_CLOSE);
		break;

	case A_OPENQ:
		if (IS_OPEN(req)) return R_TRUE;
		return R_FALSE;

	case A_UPDATE:
		// Update the port object after a READ or WRITE operation.
		// This is normally called by the WAKE-UP function.
		//arg = OFV(port, STD_PORT_DATA);
		//if (req->command == RDC_WRITE) {
		//	SET_NONE(arg);  // Write is done.
		//}
		return R_TRUE;
	default:
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
