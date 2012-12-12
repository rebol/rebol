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
**  Title: Device: DNS access
**  Author: Carl Sassenrath
**  Purpose: Calls local DNS services for domain name lookup.
**  Notes:
**      See MS WSAAsyncGetHost* details regarding multiple requests.
**
************************************************************************
**
**  NOTE to PROGRAMMERS:
**
**    1. Keep code clear and simple.
**    2. Document unusual code, reasoning, or gotchas.
**    3. Use same style for code, vars, indent(4), comments, etc.
**    4. Keep in mind Linux, OS X, BSD, big/little endian CPUs.
**    5. Test everything, then test it again.
**
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reb-host.h"
#include "host-lib.h"
#include "sys-net.h"

extern DEVICE_CMD Init_Net(REBREQ *); // Share same init
extern DEVICE_CMD Quit_Net(REBREQ *);

extern void Signal_Device(REBREQ *req, REBINT type);

#ifdef HAS_ASYNC_DNS
// Async DNS requires a window handle to signal completion (WSAASync)
extern HWND Event_Handle;
#endif

/***********************************************************************
**
*/	DEVICE_CMD Open_DNS(REBREQ *sock)
/*
***********************************************************************/
{
	SET_OPEN(sock);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_DNS(REBREQ *sock)
/*
**		Note: valid even if not open.
**
***********************************************************************/
{
	// Terminate a pending request:
#ifdef HAS_ASYNC_DNS
	if (GET_FLAG(sock->flags, RRF_PENDING)) {
		CLR_FLAG(sock->flags, RRF_PENDING);
		if (sock->handle) WSACancelAsyncRequest(sock->handle);
	}
#endif
	if (sock->net.host_info) OS_Free(sock->net.host_info);
	sock->net.host_info = 0;
	sock->handle = 0;
	SET_CLOSED(sock);
	return DR_DONE; // Removes it from device's pending list (if needed)
}


/***********************************************************************
**
*/	DEVICE_CMD Read_DNS(REBREQ *sock)
/*
**		Initiate the GetHost request and return immediately.
**		Note the temporary results buffer (must be freed later).
**
***********************************************************************/
{
	void *host;
#ifdef HAS_ASYNC_DNS
	HANDLE handle;
#else
	HOSTENT *he;
#endif

	host = OS_Make(MAXGETHOSTSTRUCT); // be sure to free it

#ifdef HAS_ASYNC_DNS
	if (!GET_FLAG(sock->modes, RST_REVERSE)) // hostname lookup
		handle = WSAAsyncGetHostByName(Event_Handle, WM_DNS, sock->data, host, MAXGETHOSTSTRUCT);
	else
		handle = WSAAsyncGetHostByAddr(Event_Handle, WM_DNS, (char*)&(sock->net.remote_ip), 4, AF_INET, host, MAXGETHOSTSTRUCT);

	if (handle != 0) {
		sock->net.host_info = host;
		sock->handle = handle;
		return DR_PEND; // keep it on pending list
	}
#else
	// Use old-style blocking DNS (mainly for testing purposes):
	if (GET_FLAG(sock->modes, RST_REVERSE)) {
		he = gethostbyaddr((char*)&sock->net.remote_ip, 4, AF_INET);
		if (he) {
			sock->net.host_info = host; //???
			sock->data = he->h_name;
			SET_FLAG(sock->flags, RRF_DONE);
			return DR_DONE;
		}
	}
	else {
		he = gethostbyname(sock->data);
		if (he) {
			sock->net.host_info = host; // ?? who deallocs?
			COPY_MEM((char*)&(sock->net.remote_ip), (char *)(*he->h_addr_list), 4); //he->h_length);
			SET_FLAG(sock->flags, RRF_DONE);
			return DR_DONE;
		}
	}
#endif

	OS_Free(host);
	sock->net.host_info = 0;

	sock->error = GET_ERROR;
	//Signal_Device(sock, EVT_ERROR);
	return DR_ERROR; // Remove it from pending list
}


/***********************************************************************
**
*/	DEVICE_CMD Poll_DNS(REBREQ *dr)
/*
**		Check for completed DNS requests. These are marked with
**		RRF_DONE by the windows message event handler (dev-event.c).
**		Completed requests are removed from the pending queue and
**		event is signalled (for awake dispatch).
**
***********************************************************************/
{
	REBDEV *dev = (REBDEV*)dr;  // to keep compiler happy
	REBREQ **prior = &dev->pending;
	REBREQ *req;
	REBOOL change = FALSE;
	HOSTENT *host;

	// Scan the pending request list:
	for (req = *prior; req; req = *prior) {

		// If done or error, remove command from list:
		if (GET_FLAG(req->flags, RRF_DONE)) { // req->error may be set
			*prior = req->next;
			req->next = 0;
			CLR_FLAG(req->flags, RRF_PENDING);

			if (!req->error) { // success!
				host = (HOSTENT*)req->net.host_info;
				if (GET_FLAG(req->modes, RST_REVERSE))
					req->data = host->h_name;
				else
					COPY_MEM((char*)&(req->net.remote_ip), (char *)(*host->h_addr_list), 4); //he->h_length);
				Signal_Device(req, EVT_READ);
			}
			else
				Signal_Device(req, EVT_ERROR);
			change = TRUE;
		}
		else prior = &req->next;
	}

	return change;
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] =
{
	Init_Net,	// Shared init - called only once
	Quit_Net,	// Shared
	Open_DNS,
	Close_DNS,
	Read_DNS,
	0,	// write
	Poll_DNS,
};

DEFINE_DEV(Dev_DNS, "DNS", 1, Dev_Cmds, RDC_MAX, 0);
