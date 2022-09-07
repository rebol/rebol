/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2022 Rebol Open Source Developers
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
*/	static REBOOL Query_MIDI_Field(REBSER *obj, REBCNT field, REBVAL *ret)
/*
**		Set a value with MIDI related data according specified field
**		The `obj` must be system/standard/midi-info with filled info
**
***********************************************************************/
{
	switch (field) {
	case SYM_DEVICES_IN:
		*ret = *Get_Field(obj, STD_MIDI_INFO_DEVICES_IN);
		return TRUE;
	case SYM_DEVICES_OUT:
		*ret = *Get_Field(obj, STD_MIDI_INFO_DEVICES_OUT);
		return TRUE;
	default:
		return FALSE;
	}
}

/***********************************************************************
**
*/	static int MIDI_Actor(REBVAL *ds, REBVAL *port_value, REBCNT action)
/*
***********************************************************************/
{
	REBSER *port;
	REBREQ *req;
	REBINT result;
	REBVAL *arg;
	REBCNT args = 0;
	REBVAL *spec;
	REBVAL *val1, *val2;

	//printf("MIDI_Actor action: %i\n", action);

	port = Validate_Port_Value(port_value);

	req = Use_Port_State(port, RDI_MIDI, sizeof(REBREQ));

	switch (action) {

	case A_READ:
		// This device is opened on the READ:
		if (!IS_OPEN(req)) Trap_Port(RE_NOT_OPEN, port, -12);

		result = OS_DO_DEVICE(req, RDC_READ);
		if (result < 0) Trap_Port(RE_READ_ERROR, port, req->error);

		// Copy and set the string result:
		arg = OFV(port, STD_PORT_DATA);
		//arg = D_ARG(1);
		SET_BINARY(arg, Copy_Bytes(req->data, req->actual));
		req->data = 0;
		*D_RET = *arg;
		return R_RET;

	case A_WRITE:
		if (!IS_OPEN(req)) Trap_Port(RE_NOT_OPEN, port, -12);
		arg = D_ARG(2);
		if(IS_BINARY(arg)) {
			req->data = VAL_BIN(arg);
			req->length = VAL_LEN(arg);

			result = OS_DO_DEVICE(req, RDC_WRITE);
			if (result < 0) Trap_Port(RE_WRITE_ERROR, port, req->error);
		}
		break;

	case A_OPEN:
		spec = OFV(port, STD_PORT_SPEC);
		args = Find_Refines(ds, AM_OPEN_READ | AM_OPEN_WRITE);
		req->modes = args; 
		if (IS_OBJECT(spec)) {
			val1 = Obj_Value(spec, STD_PORT_SPEC_MIDI_DEVICE_IN);
			if (IS_INTEGER(val1)) {
				req->midi.device_in = VAL_INT32(val1);
				//printf(";;;;;;;; requested device-in: %i\n", req->midi.device_in);
			}
			val2 = Obj_Value(spec, STD_PORT_SPEC_MIDI_DEVICE_OUT);
			if (IS_INTEGER(val2)) {
				req->midi.device_out = VAL_INT32(val2);
				//printf(";;;;;;;; requested device-out: %i\n", req->midi.device_out);
			} else if ((args & AM_OPEN_WRITE) && !(args & AM_OPEN_READ)) {
				// it was opened using: open/WRITE midi://device:NUM
				req->midi.device_out = req->midi.device_in;
				req->midi.device_in = 0;
				SET_NONE(val1);
				SET_INTEGER(val2, req->midi.device_out);
			}
		}
		if (OS_DO_DEVICE(req, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, req->error);

		break;

	case A_CLOSE:
		if (!IS_OPEN(req)) Trap_Port(RE_NOT_OPEN, port, -12);
		OS_DO_DEVICE(req, RDC_CLOSE);
		break;

	case A_QUERY:
		spec = Get_System(SYS_STANDARD, STD_MIDI_INFO);
		if (!IS_OBJECT(spec)) Trap_Arg(spec);
		if (D_REF(2) && IS_NONE(D_ARG(3))) {
			// query/mode midi:// none ;<-- lists possible fields to request
			Set_Block(D_RET, Get_Object_Words(spec));
			return R_RET;
		}

		REBSER *obj = CLONE_OBJECT(VAL_OBJ_FRAME(spec));
		Set_Block(Get_Field(obj, STD_MIDI_INFO_DEVICES_IN), Make_Block(7));
		Set_Block(Get_Field(obj, STD_MIDI_INFO_DEVICES_OUT), Make_Block(7));
		req->data = (REBYTE*)obj;
		OS_DO_DEVICE(req, RDC_QUERY);

		if (D_REF(2)) {
			// query/mode used
			REBVAL *field = D_ARG(3);
			if (IS_WORD(field)) {
				if (!Query_MIDI_Field(obj, VAL_WORD_SYM(field), D_RET))
					Trap_Reflect(VAL_TYPE(D_ARG(1)), field); // better error?
			}
			else if (IS_BLOCK(field)) {
				REBVAL *val;
				REBSER *values = Make_Block(2 * BLK_LEN(VAL_SERIES(field)));
				REBVAL *word = VAL_BLK_DATA(field);
				for (; NOT_END(word); word++) {
					if (ANY_WORD(word)) {
						if (IS_SET_WORD(word)) {
							// keep the set-word in result
							val = Append_Value(values);
							*val = *word;
							VAL_SET_LINE(val);
						}
						val = Append_Value(values);
						if (!Query_MIDI_Field(obj, VAL_WORD_SYM(word), val))
							Trap1(RE_INVALID_ARG, word);
					}
					else  Trap1(RE_INVALID_ARG, word);
				}
				Set_Series(REB_BLOCK, D_RET, values);
			}
			return R_RET;
		}
		Set_Object(D_RET, obj);
		return R_RET;

	case A_OPENQ:
		if (IS_OPEN(req)) return R_TRUE;
		return R_FALSE;

	case A_UPDATE:
		// Update the port object after a READ or WRITE operation.
		// This is normally called by the WAKE-UP function.
		arg = OFV(port, STD_PORT_DATA);
		//printf("MIDI Update action, req->command = %d\n", req->command);
		if (req->command == RDC_READ) {
			if (ANY_BINSTR(arg)) VAL_TAIL(arg) += req->actual;
		}
		else if (req->command == RDC_WRITE) {
			SET_NONE(arg);  // Write is done.
		}
		return R_NONE;

	default:
		Trap1(RE_NO_PORT_ACTION, Get_Action_Word(action));
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
