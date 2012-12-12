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
**  Title: Device management and command dispatch
**  Author: Carl Sassenrath
**  Caution: OS independent
**  Purpose:
**      This module implements a device management system for
**      REBOL devices and tracking their I/O requests.
**      It is intentionally kept very simple (makes debugging easy!)
**
**  Special note:
**      This module is parsed for function declarations used to
**      build prototypes, tables, and other definitions. To change
**      function arguments requires a rebuild of the REBOL library.
**
**  Design comments:
**      1. Not a lot of devices are needed (dozens, not hundreds).
**      2. Devices are referenced by integer (index into device table).
**      3. A single device can support multiple requests.
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
#include <string.h>

#include "reb-host.h"
#include "host-lib.h"


/***********************************************************************
**
**	REBOL Device Table
**
**		The table most be in same order as the RDI_ enums.
**		Table is in polling priority order.
**
***********************************************************************/

extern REBDEV Dev_StdIO;
extern REBDEV Dev_File;
extern REBDEV Dev_Event;
extern REBDEV Dev_Net;
extern REBDEV Dev_DNS;
#ifndef MIN_OS
extern REBDEV Dev_Clipboard;
#endif

REBDEV *Devices[RDI_LIMIT] =
{
	0,
	&Dev_StdIO,
	0,
	&Dev_File,
	&Dev_Event,
	&Dev_Net,
	&Dev_DNS,
#ifndef MIN_OS
	&Dev_Clipboard,
#endif
	0,
};


static int Poll_Default(REBDEV *dev)
{
	// The default polling function for devices.
	// Retries pending requests. Return TRUE if status changed.
	REBREQ **prior = &dev->pending;
	REBREQ *req;
	REBOOL change = FALSE;
	int result;

	for (req = *prior; req; req = *prior) {

		// Call command again:
		if (req->command < RDC_MAX)
			result = dev->commands[req->command](req);
		else {
			result = -1;	// invalid command, remove it
			req->error = ((REBCNT)-1);
		}

		// If done or error, remove command from list:
		if (result <= 0) {
			*prior = req->next;
			req->next = 0;
			CLR_FLAG(req->flags, RRF_PENDING);
			change = TRUE;
		}
		else prior = &req->next;
	}

	return change;
}


/***********************************************************************
**
*/	void Attach_Request(REBREQ **node, REBREQ *req)
/*
**		Attach a request to a device's pending or accept list.
**		Node is a pointer to the head pointer of the req list.
**
***********************************************************************/
{
	REBREQ *r;

#ifdef special_debug
	if (req->device == 5)
		Debug_Fmt("Attach: %x %x %x %x", req, req->device, req->port, req->next);
#endif

	// See if its there, and get last req:
	for (r = *node; r; r = *node) {
		if (r == req) return; // already in list
		node = &r->next;
	}

	// Link the new request to end:
	*node = req;
	req->next = 0;
	SET_FLAG(req->flags, RRF_PENDING);
}


/***********************************************************************
**
*/	void Detach_Request(REBREQ **node, REBREQ *req)
/*
**		Detach a request to a device's pending or accept list.
**		If it is not in list, then no harm done.
**
***********************************************************************/
{
	REBREQ *r;

#ifdef special_debug
	if (req->device == 5)
		Debug_Fmt("Detach= n: %x r: %x p: %x %x", *node, req, req->port, &req->next);
#endif

	// See if its there, and get last req:
	for (r = *node; r; r = *node) {
#ifdef special_debug
	if (req->device == 5)
		Debug_Fmt("Detach: r: %x n: %x", r, r->next);
#endif
		if (r == req) {
			*node = req->next;
			req->next = 0;
			CLR_FLAG(req->flags, RRF_PENDING);
			return;
		}
		node = &r->next;
	}
}


/***********************************************************************
**
*/	void Done_Device(int handle, int error)
/*
**		Given a handle mark the related request as done.
**		(Used by DNS device).
**
***********************************************************************/
{
	REBINT d;
	REBDEV *dev;
	REBREQ **prior;
	REBREQ *req;

	for (d = RDI_NET; d <= RDI_DNS; d++) {
		dev = Devices[d];
		prior = &dev->pending;
		// Scan the pending requests, mark the one we got:
		for (req = *prior; req; req = *prior) {
			if ((int)(req->handle) == handle) {
				req->error = error; // zero when no error
				SET_FLAG(req->flags, RRF_DONE);
				return;
			}
			prior = &req->next;
		}
	}
}


/***********************************************************************
**
*/	void Signal_Device(REBREQ *req, REBINT type)
/*
**		Generate a device event to awake a port on REBOL.
**
***********************************************************************/
{
	REBEVT evt;

	CLEARS(&evt);

	evt.type = (REBYTE)type;
	evt.model = EVM_DEVICE;
	evt.req  = req;
	if (type == EVT_ERROR) evt.data = req->error;

	RL_Event(&evt);	// (returns 0 if queue is full, ignored)
}


/***********************************************************************
**
*/	int OS_Call_Device(REBINT device, REBCNT command)
/*
**		Shortcut for non-request calls to device.
**
**		Init - Initialize any device-related resources (e.g. libs).
**		Quit - Cleanup any device-related resources.
**		Make - Create and initialize a request for a device.
**		Free - Free a device request structure.
**		Poll - Poll device for activity.
**
***********************************************************************/
{
	REBDEV *dev;

	// Validate device:
	if (device >= RDI_MAX || !(dev = Devices[device]))
		return -1;

	// Validate command:
	if (command > dev->max_command || dev->commands[command] == 0)
		return -2;

	// Do command, return result:
	return dev->commands[command]((REBREQ*)dev);
}


/***********************************************************************
**
*/	int OS_Do_Device(REBREQ *req, REBCNT command)
/*
**		Tell a device to perform a command. Non-blocking in many
**		cases and will attach the request for polling.
**
**		Returns:
**			=0: for command success
**			>0: for command still pending
**			<0: for command error
**
***********************************************************************/
{
	REBDEV *dev;
	REBINT result;

	req->error = 0; // A94 - be sure its cleared

	// Validate device:
	if (req->device >= RDI_MAX || !(dev = Devices[req->device])) {
		req->error = RDE_NO_DEVICE;
		return -1;
	}

	// Confirm device is initialized. If not, return an error or init
	// it if auto init option is set.
	if (!GET_FLAG(dev->flags, RDF_INIT)) {
		if (GET_FLAG(dev->flags, RDO_MUST_INIT)) {
			req->error = RDE_NO_INIT;
			return -1;
		}
		if (!dev->commands[RDC_INIT] || !dev->commands[RDC_INIT]((REBREQ*)dev))
		SET_FLAG(dev->flags, RDF_INIT);
	}

	// Validate command:
	if (command > dev->max_command || dev->commands[command] == 0) {
		req->error = RDE_NO_COMMAND;
		return -1;
	}

	// Do the command:
	req->command = command;
	result = dev->commands[command](req);

	// If request is pending, attach it to device for polling:
	if (result > 0) Attach_Request(&dev->pending, req);
	else if (dev->pending) {
		Detach_Request(&dev->pending, req); // often a no-op
		if (result == DR_ERROR && GET_FLAG(req->flags, RRF_ALLOC)) { // not on stack
			Signal_Device(req, EVT_ERROR);
		}
	}

	return result;
}


/***********************************************************************
**
*/	REBREQ *OS_Make_Devreq(int device)
/*
***********************************************************************/
{
	REBDEV *dev;
	REBREQ *req;
	int size;

	// Validate device:
	if (device >= RDI_MAX || !(dev = Devices[device]))
		return 0;

	size = dev->req_size ? dev->req_size : sizeof(REBREQ);
	req = OS_Make(size);
	CLEARS(req);
	SET_FLAG(req->flags, RRF_ALLOC);
	req->clen = size;
	req->device = device;

	return req;
}


/***********************************************************************
**
*/	int OS_Abort_Device(REBREQ *req)
/*
**		Ask device to abort prior request.
**
***********************************************************************/
{
	REBDEV *dev;

	if ((dev = Devices[req->device]) != 0) Detach_Request(&dev->pending, req);
	return 0;
}


/***********************************************************************
**
*/	int OS_Poll_Devices(void)
/*
**		Poll devices for activity.
**
**		Returns count of devices that changed status.
**
**		Devices with pending lists will be called to see if
**		there is a change in status of those requests. If so,
**		those devices are allowed to change the state of those
**		requests or call-back into special REBOL functions
**		(e.g. Add_Event for GUI) to invoke special actions.
**
***********************************************************************/
{
	int d;
	int cnt = 0;
	REBDEV *dev;
	//int cc = 0;

	//printf("Polling Devices\n");

	// Check each device:
	for (d = 0; d < RDI_MAX; d++) {
		dev = Devices[d];
		if (dev && (dev->pending || GET_FLAG(dev->flags, RDO_AUTO_POLL))) {
			// If there is a custom polling function, use it:
			if (dev->commands[RDC_POLL]) {
				if (dev->commands[RDC_POLL]((REBREQ*)dev)) cnt++;
			}
			else {
				if (Poll_Default(dev)) cnt++;
			}
		}
		//if (cc != cnt) {printf("dev=%s ", dev->title); cc = cnt;}
	}

	return cnt;
}


/***********************************************************************
**
*/	int OS_Quit_Devices(int flags)
/*
**		Terminate all devices in preparation to quit.
**
**		Allows devices to perform cleanup and resource freeing.
**
**		Set flags to zero for now. (May later be used to indicate
**		a device query check or a brute force quit.)
**
**		Returns: 0 for now.
**
***********************************************************************/
{
	int d;
	REBDEV *dev;

	for (d = RDI_MAX-1; d >= 0; d--) {
		dev = Devices[d];
		if (dev && GET_FLAG(dev->flags, RDF_INIT) && dev->commands[RDC_QUIT]) {
			dev->commands[RDC_QUIT]((REBREQ*)dev);
		}
	}

	return 0;
}


/***********************************************************************
**
*/	REBINT OS_Wait(REBCNT millisec, REBCNT res)
/*
**		Check if devices need attention, and if not, then wait.
**		The wait can be interrupted by a GUI event, otherwise
**		the timeout will wake it.
**
**		Res specifies resolution. (No wait if less than this.)
**
**		Returns:
**			-1: Devices have changed state.
**		     0: past given millsecs
**			 1: wait in timer
**
**		The time it takes for the devices to be scanned is
**		subtracted from the timer value.
**
***********************************************************************/
{
	REBREQ req;		// OK: QUERY below does not store it
	REBCNT delta;
	i64 base;

	// printf("OS_Wait %d\n", millisec);

	base = OS_Delta_Time(0, 0); // start timing

	// Setup for timing:
	CLEARS(&req);
	req.device = RDI_EVENT;

	// Let any pending device I/O have a chance to run:
	if (OS_Poll_Devices()) return -1;

	// Nothing, so wait for period of time
	delta = (REBCNT)OS_Delta_Time(base, 0)/1000 + res;
	if (delta >= millisec) return 0;
	millisec -= delta;  // account for time lost above
	req.length = millisec;

	// printf("Wait: %d ms\n", millisec);
	OS_Do_Device(&req, RDC_QUERY); // wait for timer or other event

	return 1;  // layer above should check delta again
}
