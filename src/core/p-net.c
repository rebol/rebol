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
**  Module:  p-net.c
**  Summary: network port interface
**  Section: ports
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#include "reb-net.h"
#include "reb-evtypes.h"

#define NET_BUF_SIZE 32*1024

/***********************************************************************
**
*/	static void Ret_Query_Net(REBSER *port, REBREQ *sock, REBVAL *ret)
/*
***********************************************************************/
{
	REBVAL *info = In_Object(port, STD_PORT_SCHEME, STD_SCHEME_INFO, 0);
	REBSER *obj;

	if (!info || !IS_OBJECT(info)) Trap_Port(RE_INVALID_SPEC, port, -10);

	obj = CLONE_OBJECT(VAL_OBJ_FRAME(info));

	SET_OBJECT(ret, obj);
	Set_Tuple(OFV(obj, STD_NET_INFO_LOCAL_IP), (REBYTE*)&sock->net.local_ip, 4);
	Set_Tuple(OFV(obj, STD_NET_INFO_REMOTE_IP), (REBYTE*)&sock->net.remote_ip, 4);
	SET_INTEGER(OFV(obj, STD_NET_INFO_LOCAL_PORT), sock->net.local_port);
	SET_INTEGER(OFV(obj, STD_NET_INFO_REMOTE_PORT), sock->net.remote_port);
}


/***********************************************************************
**
*/	static void Accept_New_Port(REBVAL *ds, REBSER *port, REBREQ *sock)
/*
**		Clone a listening port as a new accept port.
**
***********************************************************************/
{
	REBREQ *nsock;

	// Get temp sock struct created by the device:
	nsock = sock->sock;
	if (!nsock) return;  // false alarm
	sock->sock = nsock->next;
	nsock->data = 0;
	nsock->next = 0;

	// Create a new port using ACCEPT request passed by sock->sock:
	port = Copy_Block(port, 0);
	SET_PORT(DS_RETURN, port);	// Also for GC protect
	SET_NONE(OFV(port, STD_PORT_DATA)); // just to be sure.
	SET_NONE(OFV(port, STD_PORT_STATE)); // just to be sure.

	// Copy over the new sock data:
	sock = Use_Port_State(port, RDI_NET, sizeof(*sock));
	*sock = *nsock;
	sock->clen = sizeof(*sock);
	sock->port = port;
	OS_FREE(nsock); // allocated by dev_net.c (MT issues?)
}


/***********************************************************************
**
*/	static int TCP_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
***********************************************************************/
{
	REBREQ *sock;	// IO request
	REBVAL *spec;	// port spec
	REBVAL *arg;	// action argument value
	REBVAL *val;	// e.g. port number value
	REBINT result;	// IO result
	REBCNT refs;	// refinement argument flags
	REBCNT len;		// generic length
	REBSER *ser;	// simplifier

	Validate_Port(port, action);

	*D_RET = *D_ARG(1);
	arg = D_ARG(2);
	refs = 0;

	sock = Use_Port_State(port, RDI_NET, sizeof(*sock));
	//Debug_Fmt("Sock: %x", sock);
	spec = OFV(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap0(RE_INVALID_PORT);

	// sock->timeout = 4000; // where does this go? !!!

	// HOW TO PREVENT OVERWRITE DURING BUSY OPERATION!!!
	// Should it just ignore it or cause an error?

	// Actions for an unopened socket:
	if (!IS_OPEN(sock)) {

		switch (action) {	// Ordered by frequency

		case A_OPEN:

			arg = Obj_Value(spec, STD_PORT_SPEC_NET_HOST);
			val = Obj_Value(spec, STD_PORT_SPEC_NET_PORT_ID);

			if (OS_DO_DEVICE(sock, RDC_OPEN)) Trap_Port(RE_CANNOT_OPEN, port, -12);
			SET_OPEN(sock);

			// Lookup host name (an extra TCP device step):
			if (IS_STRING(arg)) {
				sock->data = VAL_BIN(arg);
				sock->net.remote_port = IS_INTEGER(val) ? VAL_INT32(val) : 80;
				result = OS_DO_DEVICE(sock, RDC_LOOKUP);  // sets remote_ip field
				if (result < 0) Trap_Port(RE_NO_CONNECT, port, sock->error);
				return R_RET;
			}

			// Host IP specified:
			else if (IS_TUPLE(arg)) {
				sock->net.remote_port = IS_INTEGER(val) ? VAL_INT32(val) : 80;
				memcpy(&sock->net.remote_ip, VAL_TUPLE(arg), 4);
				break;
			}

			// No host, must be a LISTEN socket:
			else if (IS_NONE(arg)) {
				SET_FLAG(sock->modes, RST_LISTEN);
				sock->data = 0; // where ACCEPT requests are queued
				sock->net.local_port = IS_INTEGER(val) ? VAL_INT32(val) : 8000;
				break;
			}
			else Trap_Port(RE_INVALID_SPEC, port, -10);

		case A_CLOSE:
			return R_RET;

		case A_OPENQ:
			return R_FALSE;

		case A_UPDATE:	// allowed after a close
			break;

		default:
			Trap_Port(RE_NOT_OPEN, port, -12);
		}
	}

	// Actions for an open socket:
	switch (action) {	// Ordered by frequency

	case A_UPDATE:
		// Update the port object after a READ or WRITE operation.
		// This is normally called by the WAKE-UP function.
		arg = OFV(port, STD_PORT_DATA);
		if (sock->command == RDC_READ) {
			if (ANY_BINSTR(arg)) VAL_TAIL(arg) += sock->actual;
		}
		else if (sock->command == RDC_WRITE) {
			SET_NONE(arg);  // Write is done.
		}
		return R_NONE;

	case A_READ:
		// Read data into a buffer, expanding the buffer if needed.
		// If no length is given, program must stop it at some point.
		refs = Find_Refines(ds, ALL_READ_REFS);
		if (!GET_FLAG(sock->state, RSM_CONNECT)) Trap_Port(RE_NOT_CONNECTED, port, -15);

		// Setup the read buffer (allocate a buffer if needed):
		arg = OFV(port, STD_PORT_DATA);
		if (!IS_STRING(arg) && !IS_BINARY(arg)) {
			Set_Binary(arg, Make_Binary(NET_BUF_SIZE));
		}
		ser = VAL_SERIES(arg);
		sock->length = SERIES_AVAIL(ser); // space available
		if (sock->length < NET_BUF_SIZE/2) Extend_Series(ser, NET_BUF_SIZE);
		sock->length = SERIES_AVAIL(ser);
		sock->data = STR_TAIL(ser); // write at tail
		//if (SERIES_TAIL(ser) == 0)
		sock->actual = 0;  // Actual for THIS read, not for total.

		//Print("(max read length %d)", sock->length);
		result = OS_DO_DEVICE(sock, RDC_READ); // recv can happen immediately
		if (result < 0) Trap_Port(RE_READ_ERROR, port, sock->error);
		break;

	case A_WRITE:
		// Write the entire argument string to the network.
		// The lower level write code continues until done.

		refs = Find_Refines(ds, ALL_WRITE_REFS);
		if (!GET_FLAG(sock->state, RSM_CONNECT)) Trap_Port(RE_NOT_CONNECTED, port, -15);

		// Determine length. Clip /PART to size of string if needed.
		spec = D_ARG(2);
		len = VAL_LEN(spec);
		if (refs & AM_WRITE_PART) {
			REBCNT n = Int32s(D_ARG(ARG_WRITE_LENGTH), 0);
			if (n <= len) len = n;
		}

		// Setup the write:
		*OFV(port, STD_PORT_DATA) = *spec;	// keep it GC safe
		sock->length = len;
		sock->data = VAL_BIN_DATA(spec);
		sock->actual = 0;

		//Print("(write length %d)", len);
		result = OS_DO_DEVICE(sock, RDC_WRITE); // send can happen immediately
		if (result < 0) Trap_Port(RE_WRITE_ERROR, port, sock->error);
		if (result == DR_DONE) SET_NONE(OFV(port, STD_PORT_DATA));
		break;

	case A_PICK:
		// FIRST server-port returns new port connection.
		len = Get_Num_Arg(arg); // Position
		if (len == 1 && GET_FLAG(sock->modes, RST_LISTEN) && sock->data)
			Accept_New_Port(ds, port, sock); // sets D_RET
		else
			Trap_Range(arg);
		break;

	case A_QUERY:
		// Get specific information - the scheme's info object.
		// Special notation allows just getting part of the info.
		Ret_Query_Net(port, sock, D_RET);
		break;

	case A_OPENQ:
		// Connect for clients, bind for servers:
		if (sock->state & ((1<<RSM_CONNECT) | (1<<RSM_BIND))) return R_TRUE;
		return R_FALSE;

	case A_CLOSE:
		if (IS_OPEN(sock)) {
			OS_DO_DEVICE(sock, RDC_CLOSE);
			SET_CLOSED(sock);
		}
		Free_Port_State(port);
		break;

	case A_LENGTHQ:
		arg = OFV(port, STD_PORT_DATA);
		len = ANY_SERIES(arg) ? VAL_TAIL(arg) : 0;
		SET_INTEGER(D_RET, len);
		break;

	case A_OPEN:
		result = OS_DO_DEVICE(sock, RDC_CONNECT);
		if (result < 0) Trap_Port(RE_NO_CONNECT, port, sock->error);
		break;
		//Trap_Port(RE_ALREADY_OPEN, port);

	case A_DELETE: // Temporary to TEST error handler!
		{
			REBVAL *event = Append_Event();		// sets signal
			VAL_SET(event, REB_EVENT);		// (has more space, if we need it)
			VAL_EVENT_TYPE(event) = EVT_ERROR;
			VAL_EVENT_DATA(event) = 101;
			VAL_EVENT_REQ(event) = sock;
		}
		break;

	default:
		Trap_Action(REB_PORT, action);
	}

	return R_RET;
}


/***********************************************************************
**
*/	void Init_TCP_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_TCP, 0, TCP_Actor);
}
