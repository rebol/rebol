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
**  Title: Device: Event handler for Posix
**  Author: Carl Sassenrath
**  Purpose:
**      Processes events to pass to REBOL. Note that events are
**      used for more than just windowing.
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
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <poll.h>

#include "reb-host.h"
#include "host-lib.h"

#ifdef REB_VIEW
#include <gtk/gtk.h>
extern GMainContext *GTKCtx;
extern REBINT exit_loop;
#else
REBINT exit_loop = 0;
#endif

extern struct pollfd poller; // currently in dev-stdio.c

void Done_Device(int handle, int error);

/***********************************************************************
**
*/	DEVICE_CMD Init_Events(REBREQ *dr)
/*
**		Initialize the event device.
**
**		Create a hidden window to handle special events,
**		such as timers and async DNS.
**
***********************************************************************/
{
	REBDEV *dev = (REBDEV*)dr; // just to keep compiler happy
	SET_FLAG(dev->flags, RDF_INIT);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Poll_Events(REBREQ *req)
/*
**		Poll for events and process them.
**		Returns 1 if event found, else 0.
**
***********************************************************************/
{
	int flag = DR_DONE;
#ifdef REB_VIEW
	//X_Event_Loop(-1);
	//puts("Poll_Events");
	if (exit_loop > 0) {
		while(g_main_context_pending(GTKCtx)) {
			flag = DR_PEND;
			//printf(".");
			g_main_context_iteration(GTKCtx, FALSE);
		}
	}
#endif
	return flag;	// different meaning compared to most commands
}


/***********************************************************************
**
*/	DEVICE_CMD Query_Events(REBREQ *req)
/*
**		Wait for an event, or a timeout (in milliseconds) specified by
**		req->length. The latter is used by WAIT as the main timing
**		method.
**
***********************************************************************/
{
	poll(&poller, 1, req->length);

//	struct timeval tv;

//#ifdef REB_VIEW
	//int max_priority;
	//GPollFD poll_fds[10];
	//gint timeout = tv.tv_usec;

	//if (g_main_context_acquire(GTKCtx)) {
	//	if (g_main_context_prepare(GTKCtx, &max_priority)) {
	//		result = g_main_context_query (GTKCtx, max_priority, &timeout, poll_fds, 10);
	//		//printf("g_main_context_query: %i timeout: %i\n", result, timeout);
	//	}
	//	g_main_context_release(GTKCtx);
	//	if (result >= 0) return DR_DONE;
	//}
//#endif	

//	int result = select(STDIN_FILENO+1, &readfds, 0, 0, &tv);
//	if (result < 0) {
//		//
//		// !!! In R3-Alpha this had a TBD that said "set error code" and had a
//		// printf that said "ERROR!!!!".  However this can happen when a
//		// Ctrl-C interrupts a timer on a WAIT.  As a patch this is tolerant
//		// of EINTR, but still returns the error code.  :-/
//		//
//		if (errno == EINTR)
//			return DR_ERROR;
//
//		printf("select() returned -1 in dev-event.c (I/O error!)\n");
//		return DR_ERROR;
//	}

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Connect_Events(REBREQ *req)
/*
**		Simply keeps the request pending for polling purposes.
**		Use Abort_Device to remove it.
**
***********************************************************************/
{
	return DR_PEND;	// keep pending
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] = {
	Init_Events,			// init device driver resources
	0,	// RDC_QUIT,		// cleanup device driver resources
	0,	// RDC_OPEN,		// open device unit (port)
	0,	// RDC_CLOSE,		// close device unit
	0,	// RDC_READ,		// read from unit
	0,	// RDC_WRITE,		// write to unit
	Poll_Events,
	Connect_Events,
	Query_Events,
};

DEFINE_DEV(Dev_Event, "OS Events", 1, Dev_Cmds, RDC_MAX, 0);
