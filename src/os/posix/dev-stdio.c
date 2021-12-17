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
**  Title: Device: Standard I/O for Posix
**  Author: Carl Sassenrath
**  Purpose:
**      Provides basic I/O streams support for redirection and
**      opening a console window if necessary.
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
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "reb-host.h"

#define SF_DEV_NULL 31		// local flag to mark NULL device

// Temporary globals: (either move or remove?!)
static int Std_Inp = STDIN_FILENO;
static int Std_Out = STDOUT_FILENO;
static FILE *Std_Echo = NULL;

extern REBDEV *Devices[];
extern void Put_Str(REBYTE *buf);

#ifndef HAS_SMART_CONSOLE	// console line-editing and recall needed
typedef struct term_data {
	char *buffer;
	char *residue;
	char *out;
	int pos;
	int end;
	int hist;
} STD_TERM;

STD_TERM *Term_IO;

extern STD_TERM *Init_Terminal(void);
extern void Quit_Terminal(STD_TERM*);
extern int  Read_Line(STD_TERM*, char*, int);
#endif
extern void Close_StdIO(void);


static void Handle_Signal(int sig)
{
	REBYTE buf[] = "\x1B[1;35;49m[escape]\x1B[0m\n";
	Put_Str(buf);
	RL_Escape(0);
}

static void Init_Signals(void)
{
	signal(SIGINT, Handle_Signal);
	signal(SIGHUP, Handle_Signal);
	signal(SIGTERM, Handle_Signal);
}

static void Close_StdIO_Local(void)
{
#ifndef HAS_SMART_CONSOLE
	if (Term_IO) {
		Quit_Terminal(Term_IO);
		Term_IO = 0;
	}
#endif
	if (Std_Echo) {
		fclose(Std_Echo);
		Std_Echo = 0;
	}
}

/***********************************************************************
**
*/	DEVICE_CMD Quit_IO(REBREQ *dr)
/*
***********************************************************************/
{
	REBDEV *dev = (REBDEV*)dr; // just to keep compiler happy above

	Close_StdIO_Local();
	Close_StdIO(); // frees host's input buffer

	CLR_FLAG(dev->flags, RDF_OPEN);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Open_IO(REBREQ *req)
/*
***********************************************************************/
{
	REBDEV *dev;

	dev = Devices[req->device];

	// Avoid opening the console twice (compare dev and req flags):
	if (GET_FLAG(dev->flags, RDF_OPEN)) {
		// Device was opened earlier as null, so req must have that flag:
		if (GET_FLAG(dev->flags, SF_DEV_NULL))
			SET_FLAG(req->modes, RDM_NULL);
		SET_FLAG(req->flags, RRF_OPEN);
		return DR_DONE; // Do not do it again
	}

	Init_Signals();

	if (!GET_FLAG(req->modes, RDM_NULL)) {

#ifndef HAS_SMART_CONSOLE
		if (isatty(Std_Inp))
			Term_IO = Init_Terminal();
#endif
		//printf("%x\r\n", req->handle);
	}
	else
		SET_FLAG(dev->flags, SF_DEV_NULL);

	SET_FLAG(req->flags, RRF_OPEN);
	SET_FLAG(dev->flags, RDF_OPEN);

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_IO(REBREQ *req)
/*
 ***********************************************************************/
{
	REBDEV *dev = Devices[req->device];

	Close_StdIO_Local();

	CLR_FLAG(dev->flags, RRF_OPEN);

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_IO(REBREQ *req)
/*
**		Low level "raw" standard output function.
**
**		Allowed to restrict the write to a max OS buffer size.
**
**		Returns the number of chars written.
**
***********************************************************************/
{
	long total;
	int output;

	if (GET_FLAG(req->modes, RDM_NULL)) {
		req->actual = req->length;
		return DR_DONE;
	}
	output = GET_FLAG(req->flags, RRF_ERROR) ? STDERR_FILENO : Std_Out;

	if (output >= 0) {

		total = write(output, req->data, req->length);

		if (total < 0) {
			req->error = errno;
			return DR_ERROR;
		}

		//if (GET_FLAG(req->flags, RRF_FLUSH)) {
			//FLUSH();
		//}

		req->actual = (u32)total;
	}

	if (Std_Echo) {
		fwrite(req->data, req->length, 1, Std_Echo);
		//fflush(Std_Echo); //slow!
	}

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_IO(REBREQ *req)
/*
**		Low level "raw" standard input function.
**
**		The request buffer must be long enough to hold result.
**
**		Result is NOT terminated (the actual field has length.)
**
***********************************************************************/
{
	long total = 0;
	int len = req->length;

	if (GET_FLAG(req->modes, RDM_NULL)) {
		req->data[0] = 0;
		return DR_DONE;
	}

	req->actual = 0;

	if (Std_Inp >= 0) {

		// Perform a processed read or a raw read?
#ifndef HAS_SMART_CONSOLE
		if (Term_IO)
			total = Read_Line(Term_IO, req->data, len);
		else
#endif
			total = read(Std_Inp, req->data, len);

		if (total < 0) {
			req->error = errno;
			return DR_ERROR;
		}

		req->actual = total;
	}

	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Query_IO(REBREQ *req)
/*
**		Resolve port information. Currently just size of console.
**		Note: Windows console have BUFFER size, which may be bigger than
**		visible window size. There seems to be nothing like it on POSIX,
**		so the `buffer-size` info is reported same as `window-info`
**
***********************************************************************/
{
#ifdef TIOCGWINSZ
	struct winsize w;
	if (ioctl(Std_Out, TIOCGWINSZ, &w) != 0) {
		req->error = errno;
		return DR_ERROR;
	}
	req->console.window_rows =
	req->console.buffer_rows = w.ws_row;
	req->console.window_cols =
	req->console.buffer_cols = w.ws_col;
#else
#ifdef WIOCGETD
	struct uwdata w;
	if (ioctl(Std_Out, WIOCGETD, &w) != 0) {
		req->error = errno;
		return DR_ERROR;
	}
	req->console.window_rows =
	req->console.buffer_rows = w.uw_height / w.uw_vs;
	req->console.window_cols =
	req->console.buffer_cols = w.uw_width / w.uw_hs;
#endif
#endif
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Modify_IO(REBREQ *req)
/*
**		Change console's mode.
**
***********************************************************************/
{
	long total;
	switch (req->modify.mode) {
		case MODE_CONSOLE_ECHO:
			if (Std_Out >= 0) {
				if(req->modify.value) {
					total = write(Std_Out, "\x1B[28m", 5);
				} else {
					total = write(Std_Out, "\x1B[8m", 4);
				}
				if (total < 0) {
					req->error = errno;
					return DR_ERROR;
				}
			}
			break;
		case MODE_CONSOLE_ERROR:
			Std_Out = req->modify.value ? STDERR_FILENO : STDOUT_FILENO;
			break;
	}
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Open_Echo(REBREQ *req)
/*
**		Open a file for low-level console echo (output).
**
***********************************************************************/
{
	if (Std_Echo) {
		fclose(Std_Echo);
		Std_Echo = 0;
	}

	if (req->file.path) {
		Std_Echo = fopen(req->file.path, "w");  // null on error
		if (!Std_Echo) {
			req->error = errno;
			return DR_ERROR;
		}
	}

	return DR_DONE;
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] =
{
	0,	// init
	Quit_IO,
	Open_IO,
	Close_IO,
	Read_IO,
	Write_IO,
	0,	// poll
	0,	// connect
	Query_IO,
	Modify_IO,	// modify
	Open_Echo,	// CREATE used for opening echo file
};

DEFINE_DEV(Dev_StdIO, "Standard IO", 1, Dev_Cmds, RDC_MAX, 0);
