/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
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
**  Author: Carl Sassenrath, Oldes
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


static struct termios settings_original;
static struct termios settings_raw;
struct pollfd poller;

//#define DEBUG_STDIO
#ifdef DEBUG_STDIO
// Output debug messages to stderr (redirect to file like: 2>debug.log)
static void Debug_Dump_Bytes(const char *label, const REBYTE *buf, int len) {
	fprintf(stderr, "%s [%d bytes]:", label, len);
	for (int i = 0; i < len; i++) {
		if (buf[i] >= 0x20 && buf[i] < 0x7F)
			fprintf(stderr, " '%c'", buf[i]);
		else
			fprintf(stderr, " %02X", buf[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
}
static int _read_byte(REBYTE *c) {
	int res = read(Std_Inp, c, 1);
	Debug_Dump_Bytes("read1", c, 1);
	return res == 1;
}
#define READ_BYTE(c) _read_byte(c) 
#else
#define READ_BYTE(c) (1 == read(Std_Inp, c, 1))
#endif

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
		if (w.uw_vs == 0 || w.uw_hs == 0) return EINVAL;
		*rows = w.uw_height / w.uw_vs;
		*cols = w.uw_width / w.uw_hs;
	#else
		*rows = 24; // sensible defaults
		*cols = 80;
	#endif
	#endif
	return 0;
}

static int bExitReadKeyLoop = 0;

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

	bExitReadKeyLoop = 1;
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

static int Parse_CSI_Sequence(REBEVT *evt, REBYTE *c) {
	// CSI sequences start with ESC [ and are followed by parameter bytes,
	// an optional intermediate byte, and a final byte.
	// Reference: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html

	evt->type = EVT_CONTROL;
	if (!READ_BYTE(&c[1])) return DR_ERROR;

	// Single-byte final sequences ESC [ <final>
	switch (c[1]) {
	case 'A': evt->data = EVK_UP;        return DR_DONE; // ESC[A
	case 'B': evt->data = EVK_DOWN;      return DR_DONE; // ESC[B
	case 'C': evt->data = EVK_RIGHT;     return DR_DONE; // ESC[C
	case 'D': evt->data = EVK_LEFT;      return DR_DONE; // ESC[D
	case 'E': evt->data = EVK_BEGIN;     return DR_DONE; // ESC[E  keypad Begin
	case 'F': evt->data = EVK_END;       return DR_DONE; // ESC[F
	case 'H': evt->data = EVK_HOME;      return DR_DONE; // ESC[H
	case 'Z': evt->data = EVK_BACKTAB;   return DR_DONE; // ESC[Z  Shift+Tab
	}

	// All remaining sequences need at least one more byte
	if (!READ_BYTE(&c[2])) return DR_ERROR;

	switch (c[1]) {
	case '1':
		if (!READ_BYTE(&c[3])) return DR_ERROR;
		if (c[2] == ';') {
			// Modifier sequences ESC[1;<mod><final>
			// Modifier: 2=Shift, 3=Alt, 4=Alt+Shift, 5=Ctrl, 6=Ctrl+Shift, 7=Ctrl+Alt, 8=Ctrl+Alt+Shift
			switch (c[3]) {
			case '2': evt->flags |= (1 << EVF_SHIFT);                                       break;
			case '3': evt->flags |= (1 << EVF_ALT);                                         break;
			case '4': evt->flags |= (1 << EVF_ALT)     | (1 << EVF_SHIFT);                  break;
			case '5': evt->flags |= (1 << EVF_CONTROL);                                     break;
			case '6': evt->flags |= (1 << EVF_CONTROL) | (1 << EVF_SHIFT);                  break;
			case '7': evt->flags |= (1 << EVF_CONTROL) | (1 << EVF_ALT);                    break;
			case '8': evt->flags |= (1 << EVF_CONTROL) | (1 << EVF_ALT) | (1 << EVF_SHIFT); break;
			}
			if (!READ_BYTE(&c[4])) return DR_ERROR;
			switch (c[4]) {
			case 'A': evt->data = EVK_UP;    return DR_DONE; // ESC[1;2A etc.
			case 'B': evt->data = EVK_DOWN;  return DR_DONE;
			case 'C': evt->data = EVK_RIGHT; return DR_DONE;
			case 'D': evt->data = EVK_LEFT;  return DR_DONE;
			case 'F': evt->data = EVK_END;   return DR_DONE;
			case 'H': evt->data = EVK_HOME;  return DR_DONE;
			case 'P': evt->data = EVK_F1;    return DR_DONE;
			case 'Q': evt->data = EVK_F2;    return DR_DONE;
			case 'R': evt->data = EVK_F3;    return DR_DONE;
			case 'S': evt->data = EVK_F4;    return DR_DONE;
			}
		}
		else if (c[3] == '~' || c[3] == '^') {
			// F1-F8
			if (c[3] == '^') SET_FLAG(evt->flags, EVF_CONTROL);
			switch (c[2]) {
			case '1': evt->data = EVK_F1; return DR_DONE; // ESC[11~
			case '2': evt->data = EVK_F2; return DR_DONE; // ESC[12~
			case '3': evt->data = EVK_F3; return DR_DONE; // ESC[13~
			case '4': evt->data = EVK_F4; return DR_DONE; // ESC[14~
			case '5': evt->data = EVK_F5; return DR_DONE; // ESC[15~
			case '7': evt->data = EVK_F6; return DR_DONE; // ESC[17~
			case '8': evt->data = EVK_F7; return DR_DONE; // ESC[18~
			case '9': evt->data = EVK_F8; return DR_DONE; // ESC[19~
			}
		}
		break;
	case '2':
		if (c[2] == '~') {
			evt->data = EVK_INSERT; return DR_DONE;        // ESC[2~
		}
		if (!READ_BYTE(&c[3])) return DR_ERROR;
		if (c[3] == '~' || c[3] == '^') {
			if (c[3] == '^') SET_FLAG(evt->flags, EVF_CONTROL);
			switch (c[2]) {
			case '0': evt->data = EVK_F9;  return DR_DONE; // ESC[20~
			case '1': evt->data = EVK_F10; return DR_DONE; // ESC[21~
			case '3': evt->data = EVK_F11; return DR_DONE; // ESC[23~
			case '4': evt->data = EVK_F12; return DR_DONE; // ESC[24~
			}
			SET_FLAG(evt->flags, EVF_SHIFT);
			switch (c[2]) {
			case '5': evt->data = EVK_F5;  return DR_DONE; // ESC[25~
			case '6': evt->data = EVK_F6;  return DR_DONE; // ESC[26~
			case '8': evt->data = EVK_F7;  return DR_DONE; // ESC[28~
			case '9': evt->data = EVK_F8;  return DR_DONE; // ESC[29~
			}
		}
		else if (c[2] == '0') {
			// Bracketed paste mode markers ESC[200~ and ESC[201~
			if (!READ_BYTE(&c[4])) return DR_ERROR;
			if (c[4] == '~') {
				switch (c[3]) {
				case '0': evt->data = EVK_PASTE_START; return DR_DONE; // ESC[200~
				case '1': evt->data = EVK_PASTE_END;   return DR_DONE; // ESC[201~
				}
			}
		}
		break;
	case '3':
		if (c[2] == '~') {
			evt->data = EVK_DELETE; return DR_DONE;        // ESC[3~
		}
		if (c[2] == '^') {
			SET_FLAG(evt->flags, EVF_CONTROL);
			evt->data = EVK_DELETE; return DR_DONE;        // ESC[3^
		}
		if (!READ_BYTE(&c[3])) return DR_ERROR;
		if (c[2] == ';' && c[3] == '5') {
			if (!READ_BYTE(&c[4])) return DR_ERROR;
			if (c[4] == '~') {
				SET_FLAG(evt->flags, EVF_CONTROL);
				evt->data = EVK_DELETE; return DR_DONE;    // ESC[3;5~
			}
		}
		if (c[3] == '~') {
			SET_FLAG(evt->flags, EVF_SHIFT);
			switch (c[2]) {
			case '1': evt->data = EVK_F9;  return DR_DONE; // ESC[31~
			case '2': evt->data = EVK_F10; return DR_DONE; // ESC[32~
			case '3': evt->data = EVK_F11; return DR_DONE; // ESC[33~
			case '4': evt->data = EVK_F12; return DR_DONE; // ESC[34~
			}
		}
		else if (c[3] == '^') {
			SET_FLAG(evt->flags, EVF_CONTROL);
			switch (c[2]) {
			case '1': evt->data = EVK_F9;  return DR_DONE; // ESC[31^
			case '2': evt->data = EVK_F10; return DR_DONE; // ESC[32^
			case '3': evt->data = EVK_F11; return DR_DONE; // ESC[33^
			case '4': evt->data = EVK_F12; return DR_DONE; // ESC[34^
			}
		}
		break;
	case '4': evt->data = EVK_END;       return DR_DONE;  // ESC[4~
	case '5':
		if (c[2] == '~') {
			evt->data = EVK_PAGE_UP;   return DR_DONE;     // ESC[5~
		}
		if (!READ_BYTE(&c[3])) return DR_ERROR;
		if (c[2] == ';' && c[3] == '2') {
			if (!READ_BYTE(&c[4])) return DR_ERROR;
			if (c[4] == '~') {
				SET_FLAG(evt->flags, EVF_SHIFT);
				evt->data = EVK_PAGE_UP;   return DR_DONE; // ESC[5;2~
			}
		}
		break;
	case '6':
		if (c[2] == '~') {
			evt->data = EVK_PAGE_DOWN; return DR_DONE;     // ESC[6~
		}
		if (!READ_BYTE(&c[3])) return DR_ERROR;
		if (c[2] == ';' && c[3] == '2') {
			if (!READ_BYTE(&c[4])) return DR_ERROR;
			if (c[4] == '~') {
				SET_FLAG(evt->flags, EVF_SHIFT);
				evt->data = EVK_PAGE_DOWN; return DR_DONE; // ESC[6;2~
			}
		}
		break;
	case '7': evt->data = EVK_HOME;      return DR_DONE;  // ESC[7~
	case '8': evt->data = EVK_END;       return DR_DONE;  // ESC[8~
	}
	return DR_IGNORE;
}

static int Parse_SS3_Sequence(REBEVT *evt, REBYTE *c) {
	// SS3 sequences start with ESC O, used by many terminals for function
	// keys and keypad keys. Some terminals send these instead of or in
	// addition to CSI sequences depending on their keypad mode (DECCKM).
	// Reference: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
	if (!READ_BYTE(&c[1])) return DR_ERROR;
	evt->type = EVT_CONTROL;
	switch (c[1]) {
	// Arrow keys (sent instead of CSI sequences in application cursor key mode)
	case 'A': evt->data = EVK_UP;    return DR_DONE; // ESC O A
	case 'B': evt->data = EVK_DOWN;  return DR_DONE; // ESC O B
	case 'C': evt->data = EVK_RIGHT; return DR_DONE; // ESC O C
	case 'D': evt->data = EVK_LEFT;  return DR_DONE; // ESC O D

	// Home/End (sent by some terminals instead of CSI sequences)
	case 'H': evt->data = EVK_HOME;  return DR_DONE; // ESC O H
	case 'F': evt->data = EVK_END;   return DR_DONE; // ESC O F

	// F1-F4 (most common SS3 usage)
	case 'P': evt->data = EVK_F1;    return DR_DONE; // ESC O P
	case 'Q': evt->data = EVK_F2;    return DR_DONE; // ESC O Q
	case 'R': evt->data = EVK_F3;    return DR_DONE; // ESC O R
	case 'S': evt->data = EVK_F4;    return DR_DONE; // ESC O S
	}
	evt->type = EVT_KEY;
	switch (c[1]) {
	// Keypad keys (application keypad mode)
	case 'M': evt->data = '\r'; return DR_DONE; // ESC O M  keypad Enter
	case 'X': evt->data = '=';  return DR_DONE; // ESC O X  keypad =
	case 'j': evt->data = '*';  return DR_DONE; // ESC O j  keypad *
	case 'k': evt->data = '+';  return DR_DONE; // ESC O k  keypad +
	case 'l': evt->data = ',';  return DR_DONE; // ESC O l  keypad ,
	case 'm': evt->data = '-';  return DR_DONE; // ESC O m  keypad -
	case 'n': evt->data = '.';  return DR_DONE; // ESC O n  keypad .
	case 'o': evt->data = '/';  return DR_DONE; // ESC O o  keypad /
	case 'p': evt->data = '0';  return DR_DONE; // ESC O p  keypad 0
	case 'q': evt->data = '1';  return DR_DONE; // ESC O q  keypad 1
	case 'r': evt->data = '2';  return DR_DONE; // ESC O r  keypad 2
	case 's': evt->data = '3';  return DR_DONE; // ESC O s  keypad 3
	case 't': evt->data = '4';  return DR_DONE; // ESC O t  keypad 4
	case 'u': evt->data = '5';  return DR_DONE; // ESC O u  keypad 5
	case 'v': evt->data = '6';  return DR_DONE; // ESC O v  keypad 6
	case 'w': evt->data = '7';  return DR_DONE; // ESC O w  keypad 7
	case 'x': evt->data = '8';  return DR_DONE; // ESC O x  keypad 8
	case 'y': evt->data = '9';  return DR_DONE; // ESC O y  keypad 9
	}
	return DR_IGNORE; // unrecognized SS3 sequence
}
#undef READ_BYTE

static int Parse_Escape_Sequence(REBEVT *evt, REBYTE *c) {
	evt->type = EVT_CONTROL;

	if (poll(&poller, 1, 0) <= 0) {
		evt->data = EVK_ESCAPE;
		return DR_DONE;
	}
	if (1 != read(Std_Inp, &c[0], 1)) return DR_ERROR;
#ifdef DEBUG_STDIO
	Debug_Dump_Bytes("esc[0]", c, 1);
#endif

	if (c[0] == '[') return Parse_CSI_Sequence(evt, c);
	if (c[0] == 'O') return Parse_SS3_Sequence(evt, c);
	if (c[0] == 'b' || c[0] == 'f') {
		SET_FLAG(evt->flags, EVF_CONTROL);
		evt->data = (c[0] == 'b') ? EVK_LEFT : EVK_RIGHT;
		return DR_DONE;
	}

	// Unrecognized ESC <char> — treat as Alt+char
	// Only applies to single printable or control characters
	if (c[0] >= 0x20 && c[0] < 0x7F) {
		// Alt+printable: ESC a = Alt+a, ESC A = Alt+A, etc.
		evt->type = EVT_KEY;
		evt->data = c[0];
		SET_FLAG(evt->flags, EVF_ALT);
		return DR_DONE;
	}
	if (c[0] >= 0x01 && c[0] < 0x20) {
		// Alt+control: ESC ^A = Alt+Ctrl+A, etc.
		evt->type = EVT_CONTROL;
		evt->data = c[0];
		SET_FLAG(evt->flags, EVF_ALT);
		SET_FLAG(evt->flags, EVF_CONTROL);
		return DR_DONE;
	}
	return DR_IGNORE; // unrecognized sequence
}

// Returns DR_DONE if a valid event was parsed, DR_ERROR on read failure, DR_IGNORE if unrecognized
static int Read_Key_Event(REBEVT *evt) {
	REBYTE c[8];
	REBINT len;

#ifdef DEBUG_STDIO
	memset(c,0,8);
#endif

	if (read(Std_Inp, c, 1) <= 0) return DR_ERROR;

	evt->type = EVT_KEY;
	evt->data = 0;
	evt->flags = 0;

	if (c[0] == '\e') {
		int res = Parse_Escape_Sequence(evt, c);
#ifdef DEBUG_STDIO
		switch(res) {
		case DR_DONE:   Debug_Dump_Bytes("esc_ok", c, strlen(c)); break;
		case DR_IGNORE: Debug_Dump_Bytes("esc_ig", c, strlen(c)); break;
		case DR_ERROR:  Debug_Dump_Bytes("esc_er", c, strlen(c)); break;
		}
#endif
		return res;
	}
	else if ((c[0] & 0x80) == 0) {
#ifdef DEBUG_STDIO
		 Debug_Dump_Bytes("ascii", c, strlen(c));
#endif
		// plain ASCII
		// Normalize backspace
		if (c[0] == 0x7F || c[0] == 0x08) {
			evt->type = EVT_CONTROL;
			evt->data = EVK_BACKSPACE;
			if (c[0] != settings_original.c_cc[VERASE])
				SET_FLAG(evt->flags, EVF_CONTROL);
			return DR_DONE;
		}
		if (c[0] > 0 && c[0] <= 0x1F) {
			SET_FLAG(evt->flags, EVF_CONTROL);
		}
		// else..
		evt->data = c[0];
	}
	else {
		int extra;        // number of continuation bytes to read
		int seq_len;      // total sequence length including lead byte

			 if ((c[0] & 0xE0) == 0xC0) extra = 1; // 2-byte sequence
		else if ((c[0] & 0xF0) == 0xE0) extra = 2; // 3-byte sequence
		else if ((c[0] & 0xF8) == 0xF0) extra = 3; // 4-byte sequence
		else return DR_IGNORE;

		// Read continuation bytes
		int remaining = extra, offset = 1;
		while (remaining > 0) {
			int n = read(Std_Inp, &c[offset], remaining);
			if (n <= 0) return DR_ERROR;
			offset += n;
			remaining -= n;
		}

		// Validate continuation bytes
		for (int i = 1; i <= extra; i++) {
			if ((c[i] & 0xC0) != 0x80) return DR_IGNORE;
		}

		seq_len = extra + 1;
		evt->data = RL_Decode_UTF8_Char(c, &seq_len);
	}
	return DR_DONE;
}



/***********************************************************************
**
*/	DEVICE_CMD Quit_IO(REBREQ *dr)
/*
***********************************************************************/
{
	REBDEV *dev = (REBDEV*)dr; // just to keep compiler happy above

	Close_StdIO_Local();
	OS_Close_StdIO(); // frees host's input buffer

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

	// If the device is already open (by a previous request), there is no need
	// to reinitialize the console. Just mark this request as open and return.
	if (GET_FLAG(dev->flags, RDF_OPEN)) {
		// If the device was opened in null mode, propagate that to this request:
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

	tcgetattr(Std_Inp, &settings_original);
	settings_raw = settings_original;
	settings_raw.c_lflag &= ~(ICANON | ECHO);// | ISIG);
	settings_raw.c_iflag &= ~(IXON | IXOFF); // disable XON/XOFF flow control so Ctrl+Q and Ctrl+S reach the app
	settings_raw.c_cc[VMIN]  = 1;
	settings_raw.c_cc[VTIME] = 0;
	// Keep ISIG enabled so Ctrl+C still generates SIGINT for RL_Escape
	// but disable Ctrl+Z suspension specifically
	settings_raw.c_cc[VSUSP] = _POSIX_VDISABLE; // disable Ctrl+Z -> SIGTSTP
	settings_raw.c_cc[VQUIT] = _POSIX_VDISABLE; // optionally disable Ctrl+\ -> SIGQUIT

	SET_FLAG(req->flags, RRF_OPEN);
	SET_FLAG(dev->flags, RDF_OPEN);

	return DR_DONE;
}


#ifdef UNUSED
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
#endif


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
**		The request buffer must be long enough to hold result.
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
		if (Term_IO) {
			if (GET_FLAG(req->modes, RDM_READ_LINE)) {
				// readline
				total = Read_Line(Term_IO, req->data, len);
			}
			else {
				// read-key
				REBEVT evt = {0};
				int result = DR_IGNORE;

				req->key.uchar = 0;
				req->key.virtu = 0;
				req->key.flags = 0;

				bExitReadKeyLoop = 0;

				do {
					// Wait for input with timeout, allowing SIGINT to be checked periodically
					struct pollfd pfd = { Std_Inp, POLLIN, 0 };
					int ready = poll(&pfd, 1, 40); // 40ms timeout
					if (ready < 0 && errno == EINTR) {
						// Signal received (e.g. SIGINT) - check if we should exit
						if (bExitReadKeyLoop) {
				ctrl_c:
							req->key.uchar = 0x03; // Ctrl+C
							req->key.flags = 1 << EVF_CONTROL;
							return DR_DONE;
						}
						continue;
					}

					if (ready <= 0) {
						// Timeout or error - check escape flag
						if (bExitReadKeyLoop) goto ctrl_c;
						continue;
					}
					// Data is available - Read_Key_Event can now read safely.
					// Parse_Escape_Sequence uses poll(0) internally to check if more
					// bytes follow the ESC byte, which is still non-blocking as intended.
					result = Read_Key_Event(&evt);

				} while (result == DR_IGNORE);

				if (result == DR_ERROR) {
					req->error = errno;
					return DR_ERROR;
				}

				// Map parsed event back to req->key
				req->key.flags = evt.flags;
				if (evt.type == EVT_CONTROL) {
					req->key.uchar = 0;
					req->key.virtu = evt.data;
				} else {
					req->key.uchar = evt.data;
					req->key.virtu = 0;
				}

				return DR_DONE;
			}
		}
		else {
			// raw read
			total = read(Std_Inp, req->data, len);
		}
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
	evt.flags = 1 << EVF_HAS_CODE;
	evt.model = EVM_CONSOLE;

	while (poll(&poller, 1, 0) > 0) {
		int result = Read_Key_Event(&evt);
		if (result == DR_DONE)
			RL_Event(&evt);
		else if (result == DR_ERROR)
			break;
		// DR_IGNORE: sequence was consumed but unrecognized, just continue
	}
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Query_IO(REBREQ *req)
/*
**		Resolve console port information. Currently just:
**		- size of console
**		- number of bytes available in the stdin
**		Note: Windows console have BUFFER size, which may be bigger than
**		visible window size. There seems to be nothing like it on POSIX,
**		so the `buffer-size` info is reported same as `window-size`
**
***********************************************************************/
{
	int cols = 0, rows = 0, bytes = 0;
	Get_Console_Size(&cols, &rows); // possible error is ignored (sizes will be zero in zhis case)
	req->console.window_rows =
	req->console.buffer_rows = rows;
	req->console.window_cols =
	req->console.buffer_cols = cols;

	ioctl(Std_Inp, FIONREAD, &bytes); // how many bytes is available in the stdin
	req->console.length = bytes;

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
	REBDEV *dev;
	REBOOL value;
	int flags;

	dev = Devices[req->device];
	value = (REBOOL)req->modify.value;

	switch (req->modify.mode) {
		case MODE_CONSOLE_ECHO:
			if (Std_Out >= 0) {
				if (value
					? (write(Std_Out, "\x1B[28m", 5)) != 5 // echo ON
					: (write(Std_Out, "\x1B[8m",  4)) != 4 // echo OFF 
				) {
					req->error = errno;
					return DR_ERROR;
				}
			}
			break;
		case MODE_CONSOLE_LINE:
			if (value ^ GET_FLAG(req->modes, RDM_READ_LINE)) {
				ASSIGN_FLAG(req->modes, RDM_READ_LINE, value);

				tcsetattr(Std_Inp, TCSADRAIN , value ? &settings_original : &settings_raw);
				if(!GET_FLAG(req->modes, RDM_CGI)) {
					// Set bracketed paste mode - https://cirw.in/blog/bracketed-paste
					write(Std_Out, value ? "\e[?2004l" : "\e[?2004h", 8);
				}
			}
			// Turn autopolling on when not in the line mode (required for async key reading).
			ASSIGN_FLAG(req->modes, RRF_PENDING, !value);
			ASSIGN_FLAG(dev->flags, RDO_AUTO_POLL, !value);
			break;
		case MODE_CONSOLE_ERROR:
			Std_Out = value ? STDERR_FILENO : STDOUT_FILENO;
			break;
		default:
			req->error = 1;
			return DR_ERROR;
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
	0, //Close_IO,
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
