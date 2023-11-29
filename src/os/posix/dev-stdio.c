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
#include <locale.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>

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


static struct termios original_settings;
struct pollfd poller;

static int Get_Console_Size(int *cols, int *rows)
{
	#ifdef TIOCGWINSZ
		struct winsize w;
		if (ioctl(Std_Out, TIOCGWINSZ, &w) != 0) {
			return errno;
		}
		*rows = w.ws_row;
		*cols = w.ws_col;
	#else
	#ifdef WIOCGETD
		struct uwdata w;
		if (ioctl(Std_Out, WIOCGETD, &w) != 0) {
			return errno;
		}
		*rows = w.uw_height / w.uw_vs;
		*cols = w.uw_width / w.uw_hs;
	#endif
	#endif
	return 0;
}

static void Handle_Signal(int sig)
{
	//printf("sig: %i %i %i %i\n", sig, SIGINT, SIGHUP, SIGTERM);
	//REBYTE buf[] = "\x1B[1;35;49m[escape]\x1B[0m\n";
	//Put_Str(buf);

//	REBEVT evt;
//	evt.flags = 0;
//	evt.model = EVM_CONSOLE;
//	evt.type = EVT_INTERRUPT;
//	evt.data = sig;
//	SET_FLAG(evt.flags, EVF_HAS_CODE);
//	RL_Event(&evt);

	// Start escape sequence...
	RL_Escape(0);
}

static void Handle_Resize(int sig)
{
	int cols, rows;

	if(Get_Console_Size(&cols, &rows) != 0) return;
	//printf("cols: %i rows: %i\n", cols, rows);

	REBEVT evt;
	evt.flags = 0;
	evt.model = EVM_CONSOLE;
	evt.type = EVT_RESIZE;
	evt.data = (rows << 16) | (cols & 0xFFFF);
	SET_FLAG(evt.flags, EVF_HAS_XY);
	RL_Update_Event(&evt);
}

static void Init_Signals(void)
{
	signal(SIGINT, Handle_Signal);
	signal(SIGHUP, Handle_Signal);
	signal(SIGTERM, Handle_Signal);

	// Set up the signal handler for SIGWINCH (terminal window resize)
    signal(SIGWINCH, Handle_Resize);
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

	setlocale(LC_ALL, ""); // Enable wide character support
	
	Init_Signals();

	poller.fd = STDIN_FILENO;
    poller.events = POLLIN;

	if (!GET_FLAG(req->modes, RDM_NULL)) {

#ifndef HAS_SMART_CONSOLE
		if (isatty(Std_Inp))
			Term_IO = Init_Terminal();
#endif
		//printf("%x\r\n", req->handle);
	}
	else
		SET_FLAG(dev->flags, SF_DEV_NULL);

	tcgetattr(Std_Inp, &original_settings);

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
			//O: returning error from here means crash (RP_IO_ERROR)!
			//O: handle (errno == EAGAIN || errno == EWOULDBLOCK) ???
			req->error = errno;
			return DR_ERROR;
		}

		if (GET_FLAG(req->flags, RRF_FLUSH)) {
			CLR_FLAG(req->flags, RRF_FLUSH);
			fflush(Std_Out == STDOUT_FILENO ? stdout : stderr);
		}

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

	//puts("Read_IO");

	if (Std_Inp >= 0) {

		// Perform a processed read or a raw read?
#ifndef HAS_SMART_CONSOLE
		if (Term_IO && GET_FLAG(req->modes, RDM_READ_LINE)) 
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

//static REBYTE read_char(){
//	REBYTE c;
//	if (poll(&poller, 1, 0) > 0 && read(Std_Inp, &c, 1)) {
//		return c;
//	}
//	return (REBYTE)-1;
//}
/***********************************************************************
**
*/	DEVICE_CMD Poll_IO(REBREQ *req)
/*
**		Read console input and convert it to system events
**
***********************************************************************/
{
	REBEVT evt;
	REBYTE c[8];
	REBINT len;

	evt.flags = 1 << EVF_HAS_CODE;
	evt.model = EVM_CONSOLE;

	while (poll(&poller, 1, 0) > 0) {
		if ( read(Std_Inp, &c, 1) > 0 ) {
			evt.type = EVT_KEY;
			//printf("%u\n", c);
			if (c[0] == '\e') {
				evt.type = EVT_CONTROL;
				// Escape sequences... is there really some system in it?!
				// This may be helpful: https://stackoverflow.com/a/71659748
				if (poll(&poller, 1, 0) <= 0) {
					// no any other char
					evt.data = EVK_ESCAPE;
					goto throw_event;
				}
				if (2 != read(Std_Inp, &c, 2)) break;
				//printf(" %s ", c);
				if (c[0] == '[') {
					switch(c[1]){
					case 'A': evt.data = EVK_UP;    goto throw_event; //== "\e[A~"
					case 'B': evt.data = EVK_DOWN;  goto throw_event;
					case 'C': evt.data = EVK_RIGHT; goto throw_event;
					case 'D': evt.data = EVK_LEFT;  goto throw_event;
					case 'F': evt.data = EVK_END;   goto throw_event;
					case 'H': evt.data = EVK_HOME;  goto throw_event;
					}
					if (c[1] == '1') {
						if (2 != read(Std_Inp, &c[2], 2)) break;
						//printf("%s ", c);
						if(c[3] == '~' || c[3] == '^') {
							if (c[3] == '^') SET_FLAG(evt.flags, EVF_CONTROL);
							switch(c[2]){
							case '1': evt.data = EVK_F1; goto throw_event; //== "\e[11~"
							case '2': evt.data = EVK_F2; goto throw_event; //== "\e[12~"
							case '3': evt.data = EVK_F3; goto throw_event;
							case '4': evt.data = EVK_F4; goto throw_event;
							case '5': evt.data = EVK_F5; goto throw_event;
							case '7': evt.data = EVK_F6; goto throw_event;
							case '8': evt.data = EVK_F7; goto throw_event;
							case '9': evt.data = EVK_F8; goto throw_event;
							}
						}
						else if(c[2] == ';' && c[3] == '2') {
							SET_FLAG(evt.flags, EVF_SHIFT);
							if (1 != read(Std_Inp, &c[4], 1)) break;
							switch(c[4]){
							case 'C': evt.data = EVK_RIGHT; goto throw_event; //== "\e[1;2C"
							case 'D': evt.data = EVK_LEFT;  goto throw_event; //== "\e[1;2D"
							}
						}
					}
					else if (c[1] == '2') {
						if (1 != read(Std_Inp, &c[2], 1)) break;
						//printf("%s ", c);
						if (c[2] == '~') {
							evt.data = EVK_INSERT; goto throw_event; //== "\e[2~"
						}
						if (1 != read(Std_Inp, &c[3], 1)) break;
						//printf("%s ", c);
						if (c[3] == '~') {
							switch(c[2]){
							case '0': evt.data = EVK_F9;  goto throw_event; //== "\e[20~"
							case '1': evt.data = EVK_F10; goto throw_event;
							case '3': evt.data = EVK_F11; goto throw_event;
							case '4': evt.data = EVK_F12; goto throw_event;
							}
							SET_FLAG(evt.flags, EVF_SHIFT);
							switch(c[2]){
							case '5': evt.data = EVK_F5;  goto throw_event; //== "\e[25~"
							case '6': evt.data = EVK_F6;  goto throw_event;
							case '8': evt.data = EVK_F7;  goto throw_event;
							case '9': evt.data = EVK_F8;  goto throw_event;
							}
						}
						if (c[3] == '^') {
							SET_FLAG(evt.flags, EVF_CONTROL);
							switch(c[2]){
							case '0': evt.data = EVK_F9;  goto throw_event; //== "\e[20^"
							case '1': evt.data = EVK_F10; goto throw_event;
							case '3': evt.data = EVK_F11; goto throw_event;
							case '4': evt.data = EVK_F12; goto throw_event;
							}
							SET_FLAG(evt.flags, EVF_SHIFT);
							switch(c[2]){
							case '5': evt.data = EVK_F5;  goto throw_event; //== "\e[25^"
							case '6': evt.data = EVK_F6;  goto throw_event;
							case '8': evt.data = EVK_F7;  goto throw_event;
							case '9': evt.data = EVK_F8;  goto throw_event;
							}
						}
						else {
							if (1 != read(Std_Inp, &c[4], 1)) break;
							if (c[4] == '~' && c[2] == '0') {
								switch(c[3]){
								case '0': evt.data = EVK_PASTE_START; goto throw_event; //== "\e[200~"
								case '1': evt.data = EVK_PASTE_END;   goto throw_event; //== "\e[201~"
								}
							}
						}
					}
					else if (c[1] > '2' && c[1] <= '8') {
						if (1 != read(Std_Inp, &c[2], 1)) break;
						//printf("%s ", c);
						if (c[2] == '~') {
							switch(c[1]){
							case '3': evt.data = EVK_DELETE;    goto throw_event; //== "\e[3~"
							case '5': evt.data = EVK_PAGE_UP;   goto throw_event;
							case '6': evt.data = EVK_PAGE_DOWN; goto throw_event;
							case '7': evt.data = EVK_HOME;      goto throw_event;
							case '4':
							case '8': evt.data = EVK_END;       goto throw_event;
							}
						}
						else if (c[1] == '3') {
							if (1 != read(Std_Inp, &c[3], 1)) break;
							//printf("%s ", c);
							if (c[3] == '~') {
								SET_FLAG(evt.flags, EVF_SHIFT);
								switch(c[2]){
								case '1': evt.data = EVK_F9;    goto throw_event; //== "\e[31~"
								case '2': evt.data = EVK_F10;   goto throw_event;
								case '3': evt.data = EVK_F11;   goto throw_event;
								case '4': evt.data = EVK_F12;   goto throw_event;
								}
							}
						}
					}
				}
				else if (c[0] == 'O') {
					switch(c[1]){
					case 'P': evt.data = EVK_F1; goto throw_event;
					case 'Q': evt.data = EVK_F2; goto throw_event;
					case 'R': evt.data = EVK_F3; goto throw_event;
					case 'S': evt.data = EVK_F4; goto throw_event;
					}
				}
				else if (c[0] == 'b' || c[0] == 'f') {
					SET_FLAG(evt.flags, EVF_CONTROL);
					evt.data = c[0] == 'b' ? EVK_LEFT : EVK_RIGHT;
					goto throw_event;
				}
				// what to do with unrecognized sequencies?
			}
			else if ((c[0] & 0x80) == 0) evt.data = c[0];
			else {
				     if ((c[0] & 0xE0) == 0xC0) len = 1; // `len` as a number of missing bytes!
				else if ((c[0] & 0xF0) == 0xE0) len = 2;
				else if ((c[0] & 0xF8) == 0xF0) len = 3;
				if (len != read(Std_Inp, &c[1], len)) break;
				evt.data = RL_Decode_UTF8_Char(c, &len);
			}
throw_event:
			RL_Event(&evt); // returns 0 if queue is full
		}
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
	int cols, rows, err;
	err = Get_Console_Size(&cols, &rows);
	if ( err ) {
		req->error = errno;
		return DR_ERROR;
	}
	req->console.window_rows =
	req->console.buffer_rows = rows;
	req->console.window_cols =
	req->console.buffer_cols = cols;
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
	struct termios settings;
	REBDEV *dev;
	int flags;

	dev = Devices[req->device];

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
		case MODE_CONSOLE_LINE:
    		//flags = fcntl(Std_Inp, F_GETFL, 0);
    		//settings = original_settings;
			if (req->modify.value) {
				SET_FLAG(req->modes, RDM_READ_LINE);
				CLR_FLAG(dev->flags, RDO_AUTO_POLL);
				CLR_FLAG(req->flags, RRF_PENDING);
				settings.c_lflag |= ICANON | ECHO;
				//flags &= ~O_NONBLOCK;

				tcsetattr(Std_Inp, TCSANOW, &original_settings);

				// Turn off bracketed paste - https://cirw.in/blog/bracketed-paste
				printf("\e[?2004l");
			}
			else {
				//printf("char inp %s\n", dev->title);
				settings = original_settings;
				CLR_FLAG(req->modes, RDM_READ_LINE);
				SET_FLAG(req->flags, RRF_PENDING);
				SET_FLAG(dev->flags, RDO_AUTO_POLL);
				settings.c_lflag &= ~(ICANON | ECHO);
				// Set stdin to non-blocking mode
  				//flags |= O_NONBLOCK;
				tcsetattr(Std_Inp, TCSANOW, &settings);

				// Turn on bracketed paste - https://cirw.in/blog/bracketed-paste
				printf("\e[?2004h");
			}
			//fcntl(Std_Inp, F_SETFL, flags);

			
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
*/	DEVICE_CMD Flush_IO(REBREQ *req)
/*
**		Flushes output buffers.
**
***********************************************************************/
{
	fflush(Std_Out == STDOUT_FILENO ? stdout : stderr);
	if (Std_Echo) {
		fflush(Std_Echo);
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
	Poll_IO,
	0,	// connect
	Query_IO,
	Modify_IO,	// modify
	Open_Echo,	// CREATE used for opening echo file
	0, // delete
	0, // rename
	0, // lookup
	Flush_IO
};

DEFINE_DEV(Dev_StdIO, "Standard IO", 1, Dev_Cmds, RDC_MAX, 0);
