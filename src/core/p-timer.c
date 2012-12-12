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
**  Module:  p-timer.c
**  Summary: timer port interface
**  Section: ports
**  Author:  Carl Sassenrath
**  Notes:   NOT IMPLEMENTED
**
***********************************************************************/
/*
	General idea of usage:

	t: open timer://name
	write t 10	; set timer - also allow: 1.23 1:23
	wait t
	clear t		; reset or delete?
	read t		; get timer value
	t/awake: func [event] [print "timer!"]
	one-shot vs restart timer
*/

#include "sys-core.h"


/***********************************************************************
**
*/	static int Event_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
***********************************************************************/
{
	REBVAL *spec;
	REBVAL *state;
	REBCNT result;
	REBVAL *arg;
	REBVAL save_port;

	Validate_Port(port, action);

	arg = D_ARG(2);
	*D_RET = *D_ARG(1);

	// Validate and fetch relevant PORT fields:
	state = BLK_SKIP(port, STD_PORT_STATE);
	spec  = BLK_SKIP(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap1(RE_INVALID_SPEC, spec);

	// Get or setup internal state data:
	if (!IS_BLOCK(state)) Set_Block(state, Make_Block(127));

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
		SET_FLAG(Eval_Signals, SIG_EVENT_PORT);
		if (action == A_INSERT || action == A_APPEND || action == A_REMOVE) {
			*D_RET = save_port;
			break;
		}
		return result; // return condition

	case A_CLEAR:
		VAL_TAIL(state) = 0;
		VAL_BLK_TERM(state);
		CLR_FLAG(Eval_Signals, SIG_EVENT_PORT);
		break;

	case A_LENGTHQ:
		SET_INTEGER(D_RET, VAL_TAIL(state));
		break;

	case A_OPEN:
		if (!req) { //!!!
			req = OS_MAKE_DEVREQ(RDI_EVENT);
			SET_OPEN(req);
			OS_DO_DEVICE(req, RDC_CONNECT);		// stays queued
		}
		break;

	default:
		Trap_Action(REB_PORT, action);
	}

	return R_RET;
}


/***********************************************************************
**
*/	void Init_Timer_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_TIMER, 0, Event_Actor);
}
