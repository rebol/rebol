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
**  Module:  p-event.c
**  Summary: event port interface
**  Section: ports
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
/*
  Basics:

	  Ports use requests to control devices.
	  Devices do their best, and return when no more is possible.
	  Progs call WAIT to check if devices have changed.
	  If devices changed, modifies request, and sends event.
	  If no devices changed, timeout happens.
	  On REBOL side, we scan event queue.
	  If we find an event, we call its port/awake function.

	  Different cases exist:

	  1. wait for time only

	  2. wait for ports and time.  Need a master wait list to
		 merge with the list provided this function.

	  3. wait for windows to close - check each time we process
		 a close event.

	  4. what to do on console ESCAPE interrupt? Can use catch it?

	  5. how dow we relate events back to their ports?

	  6. async callbacks
*/

#include "sys-core.h"

REBREQ *req;		//!!! move this global

#define EVENTS_LIMIT 0xFFFF //64k
#define EVENTS_CHUNK 128

/***********************************************************************
**
*/	REBVAL *Append_Event(void)
/*
**		Append an event to the end of the current event port queue.
**		Return a pointer to the event value.
**
**		Note: this function may be called from out of environment,
**		so do NOT extend the event queue here. If it does not have
**		space, return 0. (Should it overwrite or wrap???)
**
***********************************************************************/
{
	REBVAL *port;
	REBVAL *value;
	REBVAL *state;

	port = Get_System(SYS_PORTS, PORTS_SYSTEM);
	if (!IS_PORT(port)) return 0; // verify it is a port object

	// Get queue block:
	state = VAL_BLK_SKIP(port, STD_PORT_STATE);
	if (!IS_BLOCK(state)) return 0;

	// Append to tail if room:
	if (SERIES_FULL(VAL_SERIES(state))) {
		if (VAL_TAIL(state) > EVENTS_LIMIT) {
			Crash(RP_MAX_EVENTS);
		} else {
			Extend_Series(VAL_SERIES(state), EVENTS_CHUNK);
			//RL_Print("event queue increased to :%d\n", SERIES_REST(VAL_SERIES(state)));
		}
	}
	VAL_TAIL(state)++;
	value = VAL_BLK_TAIL(state);
	SET_END(value);
	value--;
	SET_NONE(value);

	//Dump_Series(VAL_SERIES(state), "state");
	//Print("Tail: %d %d", VAL_TAIL(state), nn++);

	return value;
}
/***********************************************************************
**
*/	REBVAL *Find_Event (REBINT model, REBINT type, void* ser)
/*
**		Find the event in the queue by the model, type and port/object (if any)
**		Return a pointer to the event value.
**
**
***********************************************************************/
{
	REBVAL *port;
	REBVAL *value;
	REBVAL *state;

	port = Get_System(SYS_PORTS, PORTS_SYSTEM);
	if (!IS_PORT(port)) return NULL; // verify it is a port object

	// Get queue block:
	state = VAL_BLK_SKIP(port, STD_PORT_STATE);
	if (!IS_BLOCK(state)) return NULL;
	for(value = VAL_BLK(state); value != VAL_BLK_TAIL(state); ++ value){
		if (VAL_EVENT_MODEL(value) == model
			&& VAL_EVENT_TYPE(value) == type
			&& (ser == NULL || VAL_EVENT_SER(value) == ser)){
			return value;
		}
	}

	return NULL;
}

/***********************************************************************
**
*/	static int Event_Actor(REBVAL *ds, REBVAL *port_value, REBCNT action)
/*
**		Internal port handler for events.
**
***********************************************************************/
{
	REBSER *port;
	REBVAL *spec;
	REBVAL *state;
	REBCNT result;
	REBVAL *arg;
	REBVAL save_port;

	port = Validate_Port_Value(port_value);

	arg = D_ARG(2);
	*D_RET = *D_ARG(1);

	// Validate and fetch relevant PORT fields:
	state = BLK_SKIP(port, STD_PORT_STATE);
	spec  = BLK_SKIP(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap1(RE_INVALID_SPEC, spec);

	// Get or setup internal state data:
	if (!IS_BLOCK(state)) Set_Block(state, Make_Block(EVENTS_CHUNK - 1));

	switch (action) {

	case A_UPDATE:
		return R_NONE;

	// Normal block actions done on events:
	case A_POKE:
		if (!IS_EVENT(D_ARG(3))) Trap_Arg(D_ARG(3));
		goto act_blk;
	case A_INSERT:
	case A_APPEND:
	//case A_PATH:		// not allowed: port/foo is port object field access
	//case A_PATH_SET:	// not allowed: above
		if (!IS_EVENT(arg)) Trap_Arg(arg);
	case A_PICK:
act_blk:
		save_port = *D_ARG(1); // save for return
		*D_ARG(1) = *state;
		result = T_Block(ds, action);
		SET_SIGNAL(SIG_EVENT_PORT);
		if (action == A_INSERT || action == A_APPEND || action == A_REMOVE) {
			*D_RET = save_port;
			break;
		}
		return result; // return condition

	case A_CLEAR:
		VAL_TAIL(state) = 0;
		VAL_BLK_TERM(state);
		CLR_SIGNAL(SIG_EVENT_PORT);
		break;

	case A_LENGTHQ:
		SET_INTEGER(D_RET, VAL_TAIL(state));
		break;

	case A_OPEN:
		if (!req) { //!!!
			req = OS_MAKE_DEVREQ(RDI_EVENT);
			if (req) {
				SET_OPEN(req);
				OS_DO_DEVICE(req, RDC_CONNECT);		// stays queued
			}
		}
		break;

	case A_CLOSE:
		OS_ABORT_DEVICE(req);
		OS_DO_DEVICE(req, RDC_CLOSE);
		OS_FREE(req);
		req = 0;
		break;

	case A_FIND: // add it

	default:
		Trap1(RE_NO_PORT_ACTION, Get_Action_Word(action));
	}

	return R_RET;
}


/***********************************************************************
**
*/	void Init_Event_Scheme(void)
/*
***********************************************************************/
{
	req = 0; // move to port struct
	Register_Scheme(SYM_SYSTEM, 0, Event_Actor);
	Register_Scheme(SYM_EVENT, 0, Event_Actor);
	Register_Scheme(SYM_CALLBACK, 0, Event_Actor);
}
