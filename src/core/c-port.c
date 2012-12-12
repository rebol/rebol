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
**  Module:  c-port.c
**  Summary: support for I/O ports
**  Section: core
**  Author:  Carl Sassenrath
**  Notes:
**     See comments in Init_Ports for startup.
**     See www.rebol.net/wiki/Event_System for full details.
**
***********************************************************************/

#include "sys-core.h"

#define MAX_WAIT_MS 64 // Maximum millsec to sleep

/***********************************************************************
**
*/	REBVAL *Make_Port(REBVAL *spec)
/*
**		Create a new port. This is done by calling the MAKE_PORT
**		function stored in the system/intrinsic object.
**
***********************************************************************/
{
	REBVAL *value;

	value = Do_Sys_Func(SYS_CTX_MAKE_PORT_P, spec, 0); // volatile
	if (IS_NONE(value)) Trap1(RE_INVALID_SPEC, spec);

	return value;
}


/***********************************************************************
**
*/	REBFLG Is_Port_Open(REBSER *port)
/*
**		Standard method for checking if port is open.
**		A convention. Not all ports use this method.
**
***********************************************************************/
{
	REBVAL *state = BLK_SKIP(port, STD_PORT_STATE);
	if (!IS_BINARY(state)) return FALSE;
	return IS_OPEN(VAL_BIN_DATA(state));
}


/***********************************************************************
**
*/	void Set_Port_Open(REBSER *port, REBFLG flag)
/*
**		Standard method for setting a port open/closed.
**		A convention. Not all ports use this method.
**
***********************************************************************/
{
	REBVAL *state = BLK_SKIP(port, STD_PORT_STATE);
	if (IS_BINARY(state)) {
		if (flag) SET_OPEN(VAL_BIN_DATA(state));
		else SET_CLOSED(VAL_BIN_DATA(state));
	}
}


/***********************************************************************
**
*/	void *Use_Port_State(REBSER *port, REBCNT device, REBCNT size)
/*
**		Use private state area in a port. Create if necessary.
**		The size is that of a binary structure used by
**		the port for storing internal information.
**
***********************************************************************/
{
	REBVAL *state = BLK_SKIP(port, STD_PORT_STATE);

	// If state is not a binary structure, create it:
	if (!IS_BINARY(state)) {
		REBSER *data = Make_Binary(size);
		REBREQ *req = (REBREQ*)STR_HEAD(data);
		Guard_Series(data); // GC safe if no other references
		req->clen = size;
		CLEAR(STR_HEAD(data), size);
		//data->tail = size; // makes it easier for ACCEPT to clone the port
		SET_FLAG(req->flags, RRF_ALLOC); // not on stack
		req->port = port;
		req->device = device;
		Set_Binary(state, data);
	}

	return (void *)VAL_BIN(state);
}


/***********************************************************************
**
*/	void Free_Port_State(REBSER *port)
/*
***********************************************************************/
{
	REBVAL *state = BLK_SKIP(port, STD_PORT_STATE);

	// ??? check that this is the binary we think it is? !!!

	if (IS_BINARY(state)) {
		Loose_Series(VAL_SERIES(state));
		VAL_SET(state, REB_NONE);
	}
}


/***********************************************************************
**
*/	REBFLG Pending_Port(REBVAL *port)
/*
**		Return TRUE if port value is pending a signal.
**		Not valid for all ports - requires request struct!!!
**
***********************************************************************/
{
	REBVAL *state;
	REBREQ *req;

	if (IS_PORT(port)) {
		state = BLK_SKIP(VAL_PORT(port), STD_PORT_STATE);
		if (IS_BINARY(state)) {
			req = (REBREQ*)VAL_BIN(state);
			if (!GET_FLAG(req->flags, RRF_PENDING)) return FALSE;
		}
	}
	return TRUE;
}


/***********************************************************************
**
*/	REBINT Awake_System(REBSER *ports)
/*
**	Returns:
**		-1 for errors
**		 0 for nothing to do
**		 1 for wait is satisifed
**
***********************************************************************/
{
	REBVAL *port;
	REBVAL *state;
	REBVAL *waked;
	REBVAL *awake;
	REBVAL tmp;
	REBVAL *v;

	// Get the system port object:
	port = Get_System(SYS_PORTS, PORTS_SYSTEM);
	if (!IS_PORT(port)) return -10; // verify it is a port object

	// Get wait queue block (the state field):
	state = VAL_BLK_SKIP(port, STD_PORT_STATE);
	if (!IS_BLOCK(state)) return -10;
	//Debug_Num("S", VAL_TAIL(state));

	// Get waked queue block:
	waked = VAL_BLK_SKIP(port, STD_PORT_DATA);
	if (!IS_BLOCK(waked)) return -10;

	// If there is nothing new to do, return now:
	if (VAL_TAIL(state) == 0 && VAL_TAIL(waked) == 0) return -1;

	//Debug_Num("A", VAL_TAIL(waked));
	// Get the system port AWAKE function:
	awake = VAL_BLK_SKIP(port, STD_PORT_AWAKE);
	if (!ANY_FUNC(awake)) return -1;
	if (ports) Set_Block(&tmp, ports);
	else SET_NONE(&tmp);

	// Call the system awake function:
	v = Apply_Func(0, awake, port, &tmp, 0); // ds is return value

	// Awake function returns 1 for end of WAIT:
	return (IS_LOGIC(v) && VAL_LOGIC(v)) ? 1 : 0;
}


/***********************************************************************
**
*/	REBINT Wait_Ports(REBSER *ports, REBCNT timeout)
/*
**	Inputs:
**		Ports: a block of ports or zero (on stack to avoid GC).
**		Timeout: milliseconds to wait
**
**	Returns:
**		TRUE when port action happened, or FALSE for timeout.
**
***********************************************************************/
{
	REBI64 base = OS_DELTA_TIME(0, 0);
	REBCNT time;
	REBINT result;
	REBCNT wt = 1;
	REBCNT res = (timeout >= 1000) ? 0 : 16;  // OS dependent?

	while (wt) {
		if (GET_SIGNAL(SIG_ESCAPE)) {
			CLR_SIGNAL(SIG_ESCAPE);
			Halt_Code(RE_HALT, 0); // Throws!
		}

		// Process any waiting events:
		if ((result = Awake_System(ports)) > 0) return TRUE;

		// If activity, use low wait time, otherwise increase it:
		if (result == 0) wt = 1;
		else {
			wt *= 2;
			if (wt > MAX_WAIT_MS) wt = MAX_WAIT_MS;
		}

		if (timeout != ALL_BITS) {
			// Figure out how long that (and OS_WAIT) took:
			time = (REBCNT)(OS_DELTA_TIME(base, 0)/1000);
			if (time >= timeout) break;	  // done (was dt = 0 before)
			else if (wt > timeout - time) // use smaller residual time
				wt = timeout - time;
		}

		//printf("%d %d %d\n", dt, time, timeout);

		// Wait for events or time to expire:
		//Debug_Num("OSW", wt);
		OS_WAIT(wt, res);
	}

	//time = (REBCNT)OS_DELTA_TIME(base, 0);
	//Print("dt: %d", time);

	return FALSE; // timeout
}

#ifdef NDEF
/***********************************************************************
**
xx*/	REBINT Wait_Device(REBREQ *req, REBCNT timeout)
/*
**		Utility function for waiting on specific device.
**		(Near the main WAIT code above to keep in-sync.)
**		This still lets any GUI events continue.
**		Returns 0 when event occurs, else -1 for error.
**
***********************************************************************/
{
	REBI64 base = OS_DELTA_TIME(0);
	REBCNT time;
	REBCNT dt = DT;

	while (dt) {
		// Process any waiting events:
		Awake_System(0);
		if (!GET_FLAG(req->flags, RRF_PENDING)) return TRUE;

		// Figure out how long that (and OS_WAIT) took:
		time = (REBCNT)OS_DELTA_TIME(base);

		Use above method!

		// Did we use all our time?
		if (timeout == ALL_BITS) dt = DT;	// infinite time
		else if (time >= timeout) dt = 0;	// done
		else if (dt > timeout - time) // residual time
			dt = timeout - time;

		// Wait for events or time to expire:
		OS_WAIT(dt);
	}

	return FALSE; // timeout
}
#endif

/***********************************************************************
**
*/	void Sieve_Ports(REBSER *ports)
/*
**		Remove all ports not found in the WAKE list.
**
***********************************************************************/
{
	REBVAL *port;
	REBVAL *waked;
	REBVAL *val;
	REBCNT n;

	port = Get_System(SYS_PORTS, PORTS_SYSTEM);
	if (!IS_PORT(port)) return;
	waked = VAL_BLK_SKIP(port, STD_PORT_DATA);
	if (!IS_BLOCK(waked)) return;

	for (n = 0; n < SERIES_TAIL(ports);) {
		val = BLK_SKIP(ports, n);
		if (IS_PORT(val)) {
			if (VAL_TAIL(waked) != Find_Block_Simple(VAL_SERIES(waked), 0, val)) {
				Remove_Series(VAL_SERIES(waked), n, 1);
				continue;
			}
		}
		n++;
	}
}


#ifdef not_used
/***********************************************************************
**
*/	REBVAL *Form_Write(REBVAL *arg, REBYTE *newline)
/*
**		Converts REBOL values to strings to use as data in WRITE.
**		Will also add newlines for conversions of blocks of lines.
**
***********************************************************************/
{
	REBSER *series;
	REBVAL *val;
	REBCNT n = 0;
	//REB_MOLD mo = {0 --- more here needed};

	if (IS_BLOCK(arg)) {

		if (newline) n = LEN_BYTES(newline);

		mo.series = series = Make_Binary(VAL_BLK_LEN(arg) * 10);

		for (val = VAL_BLK_DATA(arg); NOT_END(val); val++) {
			Mold_Value(&mo, val, 0);
			if (newline) Append_Series(series, newline, n);
		}

		Set_String(arg, series);
	}

	if (!ANY_STRING(arg)) {
		Set_String(arg, Copy_Form_Value(arg, 0));
	}

	return arg;
}
#endif


/***********************************************************************
**
*/	REBCNT Find_Action(REBVAL *object, REBCNT action)
/*
**		Given an action number, return the action's index in
**		the specified object. If not found, a zero is returned.
**
***********************************************************************/
{
	return Find_Word_Index(VAL_OBJ_FRAME(object), VAL_BIND_SYM(Get_Action_Word(action)), FALSE);
}


/***********************************************************************
**
*/	int Do_Port_Action(REBSER *port, REBCNT action)
/*
**		Call a PORT actor (action) value. Search PORT actor
**		first. If not found, search the PORT scheme actor.
**
**		NOTE: stack must already be setup correctly for action, and
**		the caller must cleanup the stack.
**
***********************************************************************/
{
	REBVAL *actor;
	REBCNT n = 0;

	ASSERT2(action < A_MAX_ACTION, RP_BAD_PORT_ACTION);

	// Verify valid port (all of these must be false):
	if (
		// Must be = or larger than std port:
		(SERIES_TAIL(port) < STD_PORT_MAX) ||
		// Must be an object series:
		!IS_FRAME(BLK_HEAD(port)) ||
		// Must have a spec object:
		!IS_OBJECT(BLK_SKIP(port, STD_PORT_SPEC))
	)
		Trap0(RE_INVALID_PORT);

	// Get actor for port, if it has one:
	actor = BLK_SKIP(port, STD_PORT_ACTOR);

	if (IS_NONE(actor)) return R_NONE;

	// If actor is a native function:
	if (IS_NATIVE(actor))
		return ((REBPAF)VAL_FUNC_CODE(actor))(DS_RETURN, port, action);

	// actor must be an object:
	if (!IS_OBJECT(actor)) Trap0(RE_INVALID_ACTOR);

	// Dispatch object function:
	n = Find_Action(actor, action);
	actor = Obj_Value(actor, n);
	if (!n || !actor || !ANY_FUNC(actor)) {
		Trap1(RE_NO_PORT_ACTION, Get_Action_Word(action));
	}
	Redo_Func(actor);
	return R_RET;

	// If not in PORT actor, use the SCHEME actor:
#ifdef no_longer_used
	if (n == 0) {
		actor = Obj_Value(scheme, STD_SCHEME_actor);
		if (!actor) goto err;
		if (IS_NATIVE(actor)) goto fun;
		if (!IS_OBJECT(actor)) goto err; //Trap_Expect(value, STD_PORT_actor, REB_OBJECT);
		n = Find_Action(actor, action);
		if (n == 0) goto err;
	}
#endif

}


/***********************************************************************
**
*/	void Secure_Port(REBCNT kind, REBREQ *req, REBVAL *name, REBSER *path)
/*
**		kind: word that represents the type (e.g. 'file)
**		req:  I/O request
**		name: value that holds the original user spec
**		path: the local path to compare with
**		
***********************************************************************/
{
	REBYTE *flags;
	REBVAL val;

	Set_String(&val, path);
	flags = Security_Policy(kind, &val); // policy flags

	// Check policy integer:
	// Mask is [xxxx wwww rrrr] - each holds the action
	if (GET_FLAG(req->modes, RFM_READ))  Trap_Security(flags[POL_READ], kind, name);
	if (GET_FLAG(req->modes, RFM_WRITE)) Trap_Security(flags[POL_WRITE], kind, name);
}


/***********************************************************************
**
*/	void Validate_Port(REBSER *port, REBCNT action)
/*
**		Because port actors are exposed to the user level, we must
**		prevent them from being called with invalid values.
**
***********************************************************************/
{
	if (
		action >= A_MAX_ACTION
		|| port->tail > 50
		|| SERIES_WIDE(port) != sizeof(REBVAL)
		|| !IS_FRAME(BLK_HEAD(port))
		|| !IS_OBJECT(BLK_SKIP(port, STD_PORT_SPEC))
	)
		Trap0(RE_INVALID_PORT);
}

/***********************************************************************
**
**  Scheme Native Action Support
**
**		This array is used to associate a scheme word with its
**		native action functions.
**
**		Each native port scheme must be listed here. This list is
**		created by each native scheme calling Register_Scheme()
**		during initialization.
**
**	Example of defining actions:
**
**		static const PORT_ACTION File_Actions[] = {
**			A_OPEN,		P_open,
**			A_CLOSE,	P_close,
**			0, 0
**		}
**
**		Register_Scheme(SYM_FILE, &File_Actions[0], 0);
**
**
***********************************************************************/

#define MAX_SCHEMES 10		// max native schemes

typedef struct rebol_scheme_actions {
	REBCNT sym;
	const PORT_ACTION *map;
	REBPAF fun;
} SCHEME_ACTIONS;

SCHEME_ACTIONS *Scheme_Actions;	// Initial Global (not threaded)


/***********************************************************************
**
*/	void Register_Scheme(REBCNT sym, const PORT_ACTION *map, REBPAF fun)
/*
**		Associate a scheme word (e.g. FILE) with a set of native
**		scheme actions. This will be used by the Set_Scheme native
**
***********************************************************************/
{
	REBINT n;

	for (n = 0; n < MAX_SCHEMES && Scheme_Actions[n].sym; n++);
	ASSERT2(n < MAX_SCHEMES, RP_MAX_SCHEMES);

	Scheme_Actions[n].sym = sym;
	Scheme_Actions[n].map = map;
	Scheme_Actions[n].fun = fun;
}


/***********************************************************************
**
*/	REBNATIVE(set_scheme)
/*
***********************************************************************/
{
	REBVAL *scheme;
	REBVAL *actor;
	REBVAL *func;
	REBVAL *act;
	REBCNT n;
	const PORT_ACTION *map = 0;

	scheme = D_ARG(1);

	act = Obj_Value(scheme, STD_SCHEME_NAME);
	if (!IS_WORD(act)) return R_NONE;
	actor = Obj_Value(scheme, STD_SCHEME_ACTOR);
	if (!actor) return R_NONE;

	// Does this scheme have native actor or actions?
	for (n = 0; Scheme_Actions[n].sym; n++) {
		if (Scheme_Actions[n].sym == VAL_WORD_SYM(act)) break;
	}
	if (!Scheme_Actions[n].sym) return R_NONE;

	// The scheme uses a native actor:
	if (Scheme_Actions[n].fun) {
		//Make_Native(actor, Make_Block(0), (REBFUN)(Scheme_Actions[n].fun), REB_NATIVE);
		// Hand build a native function that will be used to reach native scheme actors.
		REBSER *ser = Make_Block(1);
		act = Append_Value(ser);
		Init_Word(act, REB_PORT+1); // any word will do
		VAL_TYPESET(act) = TYPESET(REB_END); // don't let it get called normally
		VAL_FUNC_SPEC(actor) = ser;
		VAL_FUNC_ARGS(actor) = ser;
		VAL_FUNC_CODE(actor) = (REBFUN)(Scheme_Actions[n].fun);
		VAL_SET(actor, REB_NATIVE);
		return R_TRUE;
	}

	// The scheme has an array of action natives:
	if (!IS_OBJECT(actor)) return R_NONE;

	// Map action natives to scheme actor words:
	for (; map->func; map++) {
		// Find the action in the scheme actor:
		n = Find_Action(actor, map->action);
		if (n) {
			// Get standard action's spec block:
			act = Get_Action_Value(map->action);

			// Make native function for action:
			func = Obj_Value(actor, n); // function
			Make_Native(func, VAL_FUNC_SPEC(act), (REBFUN)(map->func), REB_NATIVE);
		}
	}
	return R_TRUE;
}


/***********************************************************************
**
*/	void Init_Ports(void)
/*
**		Initialize port scheme related subsystems.
**
**	In order to add a port scheme:
**
**		In mezz-ports.r add a make-scheme.
**		Add an Init_*_Scheme() here.
**		Be sure host-devices.c has the device enabled.
**
***********************************************************************/
{
	Scheme_Actions = Make_Mem(sizeof(SCHEME_ACTIONS) * MAX_SCHEMES);

	Init_Console_Scheme();
	Init_File_Scheme();
	Init_Dir_Scheme();
	Init_Event_Scheme();
	Init_TCP_Scheme();
	Init_DNS_Scheme();
#ifndef MIN_OS
	Init_Clipboard_Scheme();
#endif
}
