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
**  Title: Device: Standard I/O for Win32
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
#include <windows.h>
#include <process.h>

#include <fcntl.h>
#include <io.h>

#include "reb-host.h"
#include "host-lib.h"
#include "sys-scan.h"

#define BUF_SIZE (16*1024)		// MS restrictions apply

#define SF_DEV_NULL 31			// local flag to mark NULL device

#define CONSOLE_MODES ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT \
		| ENABLE_EXTENDED_FLAGS | 0x0040 | 0x0020  // quick edit and insert mode (not defined in VC6)

static HANDLE Std_Out = 0;
static HANDLE Std_Inp = 0;
static HANDLE Std_Echo = 0;
static REBCHR *Std_Buf = 0;		// for input and output

static BOOL Redir_Out = 0;
static BOOL Redir_Inp = 0;

// Special access:
extern REBDEV *Devices[];


//** ANSI emulation definition ****************************************** 
#define FOREGROUND_BLACK           0x0000
//#define FOREGROUND_BLUE          0x0001
//#define FOREGROUND_GREEN         0x0002
#define FOREGROUND_CYAN            0x0003
//#define FOREGROUND_RED           0x0004
#define FOREGROUND_MAGENTA         0x0005
#define FOREGROUND_YELLOW          0x0006
#define FOREGROUND_GREY            0x0007
//#define FOREGROUND_INTENSITY     0x0008
#define FOREGROUND_WHITE           0x000F
//#define BACKGROUND_BLUE          0x0010
#define BACKGROUND_CYAN            0x0030
//#define BACKGROUND_GREEN         0x0020
//#define BACKGROUND_RED           0x0040
#define BACKGROUND_MAGENTA         0x0050
#define BACKGROUND_YELLOW          0x0060
#define BACKGROUND_GREY            0x0070
//#define BACKGROUND_INTENSITY     0x0080
#define COMMON_LVB_UNDERSCORE      0x8000

static COORD Std_coord = { 0, 0 };

static int ANSI_State = -1; // if >= 0, we are in the middle of the parsing ANSI sequence
static int ANSI_Value1 = 0;
static int ANSI_Value2 = 0;
static int ANSI_Attr  = -1;

int Update_Graphic_Mode(int attribute, int value);
REBYTE* Parse_ANSI_sequence(REBYTE *cp, REBYTE *ep);

//**********************************************************************

BOOL WINAPI Handle_Break(DWORD dwCtrlType)
{
	// Handle the MS CMD console CTRL-C, BREAK, and other events:
	if (dwCtrlType >= CTRL_CLOSE_EVENT) OS_Exit(100); // close button, shutdown, etc.
	RL_Escape(0);
	return TRUE;	// We handled it
}

#ifdef DEBUG_METHOD
// Because this file deals with stdio, we must avoid using stdio for debug.
// This funtion is of use wne needed.
static dbgout(char *fmt, int d, char *s)
{
	char buf[255];
	FILE *f = fopen("dbgout.txt", "a");
	sprintf(buf, fmt, d, s);
	fwrite(buf, strlen(buf), 1, f);
	fclose(f);
}
// example: dbgout("handle: %x %s\n", hdl, name);
#endif

#ifdef NOT_USED
static void attach_console(void) {
	void *h = LoadLibraryW(TEXT("kernel32.dll"));
	(BOOL (_stdcall *)(DWORD))GetProcAddress(h, "AttachConsole")(-1);
	FreeLibrary(h);
}
#endif

static void close_stdio(void)
{
	if (Std_Buf) {
		OS_Free(Std_Buf);
		Std_Buf = 0;
		//FreeConsole();  // problem: causes a delay
	}
	if (Std_Echo) {
		CloseHandle(Std_Echo);
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

	close_stdio();
	//if (GET_FLAG(dev->flags, RDF_OPEN)) FreeConsole();
	CLR_FLAG(dev->flags, RDF_OPEN);
	return DR_DONE;
}

/***********************************************************************
**
*/	HWND GetConsoleHwnd(void)
/*
**		Used to get handle of a newly created console
**		See: http://support.microsoft.com/kb/124103
**
***********************************************************************/
{
#define MY_BUFSIZE 1024 // Buffer size for console window titles.
	HWND hwndFound;     // This is what is returned to the caller.
	char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
										// WindowTitle.
	char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
										// WindowTitle.
										// Fetch current window title.

	GetConsoleTitle((LPWSTR)pszOldWindowTitle, MY_BUFSIZE);

	// Format a "unique" NewWindowTitle.
	wsprintf((LPWSTR)pszNewWindowTitle, (LPCWSTR)"%d/%d",
		GetTickCount(),
		GetCurrentProcessId());

	// Change current window title.
	SetConsoleTitle((LPWSTR)pszNewWindowTitle);

	// Ensure window title has been updated.
	Sleep(40);

	// Look for NewWindowTitle.
	hwndFound = FindWindow(NULL, (LPWSTR)pszNewWindowTitle);

	// Restore original window title.
	SetConsoleTitle((LPWSTR)pszOldWindowTitle);
	return(hwndFound);
}

/***********************************************************************
**
*/	DEVICE_CMD Open_IO(REBREQ *req)
/*
***********************************************************************/
{
	REBDEV *dev;
//	HANDLE win;

	dev = Devices[req->device];

	// Avoid opening the console twice (compare dev and req flags):
	if (GET_FLAG(dev->flags, RDF_OPEN)) {
		// Device was opened earlier as null, so req must have that flag:
		if (GET_FLAG(dev->flags, SF_DEV_NULL))
			SET_FLAG(req->modes, RDM_NULL);
		SET_FLAG(req->flags, RRF_OPEN);
		return DR_DONE; // Do not do it again
	}

	if (!GET_FLAG(req->modes, RDM_NULL)) {

		// Get the raw stdio handles:
		Std_Out = GetStdHandle(STD_OUTPUT_HANDLE);
		Std_Inp = GetStdHandle(STD_INPUT_HANDLE);
		//Std_Err = GetStdHandle(STD_ERROR_HANDLE);
		Std_Echo = 0;

		Redir_Out = (GetFileType(Std_Out) != FILE_TYPE_CHAR);
		Redir_Inp = (GetFileType(Std_Inp) != FILE_TYPE_CHAR);

#ifdef _WINDOWS
// This code is needed only when the app is not copiled with Console subsystem
 		// If output not redirected, open a console:
		if (!Redir_Out) {
			if (!AllocConsole()) {
				req->error = GetLastError();
				return DR_ERROR;
			}

			win = GetConsoleHwnd();
			if (win) {
				SetForegroundWindow(win);
				BringWindowToTop(win);
			}

			// Get the new stdio handles:
			Std_Out = GetStdHandle(STD_OUTPUT_HANDLE);

			if (!Redir_Inp) {
				Std_Inp = GetStdHandle(STD_INPUT_HANDLE);
			}
		}
#endif
		Std_Buf = OS_Make(BUF_SIZE * sizeof(REBCHR));

		if (!Redir_Inp) {
			//
			// Windows offers its own "smart" line editor (with history
			// management, etc.) in the form of the Windows Terminal.  These
			// modes only apply if a the input is coming from the terminal,
			// not if Rebol has a file redirection connected to the input.
			//
			// While the line editor is running with ENABLE_LINE_INPUT, there
			// are very few hooks offered.
			//
			SetConsoleMode(Std_Inp, CONSOLE_MODES);
		}

		// Handle stdio CTRL-C interrupt:
		SetConsoleCtrlHandler(Handle_Break, TRUE);
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

	close_stdio();

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
	long len;
	long total = 0;
	BOOL ok = FALSE;
	REBYTE *bp;
	REBYTE *cp;
	REBYTE *ep;

	if (GET_FLAG(req->modes, RDM_NULL)) {
		req->actual = req->length;
		return DR_DONE;
	}

	if (Std_Out) {

		bp = req->data;
		ep = bp + req->length;

		// Using this loop for seeking escape char and processing ANSI sequence
		do {

			if(ANSI_State >= 0) // there is pending ansi sequence state
				bp = Parse_ANSI_sequence(bp-1, ep); // the pointer is incremented back in the parser

			cp = Skip_To_Char(bp, ep, (REBYTE)27); //find ANSI escape char "^["

			if (Redir_Out) { // for Console SubSystem (always UTF-8)
				if (cp) {
					ok = WriteFile(Std_Out, bp, cp - bp, &total, 0);
					bp = Parse_ANSI_sequence(cp, ep);
				}
				else {
					ok = WriteFile(Std_Out, bp, ep - bp, &total, 0);
					bp = ep;
				}
				if (!ok) {
					req->error = GetLastError();
					return DR_ERROR;
				}
			}
			else { // for Windows SubSystem - must be converted to Win32 wide-char format

				// Thankfully, MS provides something other than mbstowcs();
				// however, if our buffer overflows, it's an error. There's no
				// efficient way at this level to split-up the input data,
				// because its UTF-8 with variable char sizes.

				//if found, write to the console content before it starts, else everything
				if (cp) {
					len = MultiByteToWideChar(CP_UTF8, 0, bp, cp - bp, Std_Buf, BUF_SIZE);
				}
				else {
					len = MultiByteToWideChar(CP_UTF8, 0, bp, ep - bp, Std_Buf, BUF_SIZE);
					bp = ep;
				}
				if (len > 0) {// no error
					ok = WriteConsoleW(Std_Out, Std_Buf, len, &total, 0);
					if (!ok) {
						req->error = GetLastError();
						return DR_ERROR;
					}
				}
				//is escape char was found, parse the ANSI sequence...
				if (cp) {
					bp = Parse_ANSI_sequence(cp, ep);
				}
			}
		} while (bp < ep);

		req->actual = req->length;  // do not use "total" (can be byte or wide)

		//if (GET_FLAG(req->flags, RRF_FLUSH)) {
		//	FLUSH();
		//}
	}

	if (Std_Echo) {	// always UTF-8
		WriteFile(Std_Echo, req->data, req->length, &total, 0);
		//FlushFileBuffers(Std_Echo);
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
	int len;
	BOOL ok;

	if (GET_FLAG(req->modes, RDM_NULL)) {
		req->data[0] = 0;
		return DR_DONE;
	}

	req->actual = 0;

	if (Std_Inp) {

		if (Redir_Inp) { // always UTF-8
			len = MIN(req->length, BUF_SIZE);
			ok = ReadFile(Std_Inp, req->data, len, &total, 0);
			printf("%i\n", total);
		}
		else {
			ok = ReadConsoleW(Std_Inp, Std_Buf, BUF_SIZE-1, &total, 0);
			if (ok) {
				total = WideCharToMultiByte(CP_UTF8, 0, Std_Buf, total, req->data, req->length, 0, 0);
				if (!total) ok = FALSE;
			}
		}

		if (!ok) {
			req->error = GetLastError();
			return DR_ERROR;
		}

		req->actual = total;
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
		CloseHandle(Std_Echo);
		Std_Echo = 0;
	}

	if (req->file.path) {
		Std_Echo = CreateFile(req->file.path, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
		if (Std_Echo == INVALID_HANDLE_VALUE) {
			Std_Echo = 0;
			req->error = GetLastError();
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
	0,	// query
	0,	// modify
	Open_Echo,	// CREATE used for opening echo file
};

DEFINE_DEV(Dev_StdIO, "Standard IO", 1, Dev_Cmds, RDC_MAX, 0);



/***********************************************************************
**
*/	int Update_Graphic_Mode(int attribute, int value)
/*
**
***********************************************************************/
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	int tmp;

	if (attribute < 0) {
		GetConsoleScreenBufferInfo(Std_Out, &csbiInfo);
		attribute = csbiInfo.wAttributes;
	}

	switch (value) {
		case 0: attribute = FOREGROUND_GREY;                           break;
		case 1: attribute = attribute | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY; break;
		case 4: attribute = attribute | COMMON_LVB_UNDERSCORE;         break;
		case 7: tmp = (attribute & 0xF0) >> 4;
				attribute = ((attribute & 0x0F) << 4) | tmp;           break; //reverse
		case 30: attribute =  attribute & 0xF8;                        break;
		case 31: attribute = (attribute & 0xF8) | FOREGROUND_RED;      break;
		case 32: attribute = (attribute & 0xF8) | FOREGROUND_GREEN;    break;
		case 33: attribute = (attribute & 0xF8) | FOREGROUND_YELLOW;   break;
		case 34: attribute = (attribute & 0xF8) | FOREGROUND_BLUE;     break;
		case 35: attribute = (attribute & 0xF8) | FOREGROUND_MAGENTA;  break;
		case 36: attribute = (attribute & 0xF8) | FOREGROUND_CYAN;     break;
		case 37: attribute = (attribute & 0xF8) | FOREGROUND_GREY;     break;
		case 39: attribute =  attribute & 0xF7;                        break;  //FOREGROUND_INTENSITY reset	
		case 40: attribute =  attribute & 0x8F;                        break;
		case 41: attribute = (attribute & 0x8F) | BACKGROUND_RED;      break;
		case 42: attribute = (attribute & 0x8F) | BACKGROUND_GREEN;    break;
		case 43: attribute = (attribute & 0x8F) | BACKGROUND_YELLOW;   break;
		case 44: attribute = (attribute & 0x8F) | BACKGROUND_BLUE;     break;
		case 45: attribute = (attribute & 0x8F) | BACKGROUND_MAGENTA;  break;
		case 46: attribute = (attribute & 0x8F) | BACKGROUND_CYAN;     break;
		case 47: attribute = (attribute & 0x8F) | BACKGROUND_GREY;     break;
		case 49: attribute =  attribute & 0x7F;                        break; //BACKGROUND_INTENSITY reset
		default: attribute = value;
	}
	return attribute;
}

/***********************************************************************
**
*/	REBYTE* Parse_ANSI_sequence(REBYTE *cp, REBYTE *ep)
/*
**		Parses ANSI sequence and return number of bytes used.
**      Based on http://ascii-table.com/ansi-escape-sequences.php
**
***********************************************************************/
{
	if (cp >= ep) return cp;
	long unsigned int num;
	int len;
	COORD coordScreen;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	if (ANSI_State < 0) {
		ANSI_State = 0;
		ANSI_Value1 = 0;
		ANSI_Value2 = 0;
		ANSI_Attr = -1;
	}

	do {
		if (++cp == ep) return cp;

		switch (ANSI_State) {

		case 0:
			ANSI_State = (*cp == '[') ? 1 : -1;
			break;

		case 1: //value1 start
			if (*cp >= (int)'0' && *cp <= (int)'9') {
				ANSI_Value1 = ((ANSI_Value1 * 10) + (*cp - (int)'0')) % 0xFFFF;
				ANSI_State = 2;
			}
			else if (*cp == ';') {
				//do nothing
			}
			else if (*cp == 's') {
				//Saves the current cursor position.
				GetConsoleScreenBufferInfo(Std_Out, &csbiInfo);
				Std_coord.X = csbiInfo.dwCursorPosition.X;
				Std_coord.Y = csbiInfo.dwCursorPosition.Y;
				ANSI_State = -1;
			}
			else if (*cp == 'u') {
				//Returns the cursor to the position stored by the Save Cursor Position sequence.
				SetConsoleCursorPosition(Std_Out, Std_coord);
				ANSI_State = -1;
			}
			else if (*cp == 'K') {
				//TODO: Erase Line.
				ANSI_State = -1;
			}
			else if (*cp == 'J') {
				//TODO: Clear screen from cursor down.
				ANSI_State = -1;
			}
			else if (*cp == 'H' || *cp == 'f') {
				coordScreen.X = 0;
				coordScreen.Y = 0;
				SetConsoleCursorPosition(Std_Out, coordScreen);
				ANSI_State = -1;
			}
			else {
				ANSI_State = -1;
			}
			break;
		case 2: //value1 continue
			if (*cp >= (int)'0' && *cp <= (int)'9') {
				ANSI_Value1 = ((ANSI_Value1 * 10) + (*cp - (int)'0')) % 0xFFFF;
				ANSI_State = 2;
			}
			else if (*cp == ';') {
				ANSI_State = 3;
			}
			else if (*cp == 'm') {
				ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value1);
				SetConsoleTextAttribute(Std_Out, ANSI_Attr);
				ANSI_State = -1;
			}
			else if (*cp == 'A') {
				//Cursor Up.
				GetConsoleScreenBufferInfo(Std_Out, &csbiInfo);
				csbiInfo.dwCursorPosition.Y = MAX(0, csbiInfo.dwCursorPosition.Y - ANSI_Value1);
				SetConsoleCursorPosition(Std_Out, csbiInfo.dwCursorPosition);
				ANSI_State = -1;
			}
			else if (*cp == 'B') {
				//Cursor Down.
				GetConsoleScreenBufferInfo(Std_Out, &csbiInfo);
				csbiInfo.dwCursorPosition.Y = MIN(csbiInfo.dwSize.Y, csbiInfo.dwCursorPosition.Y + ANSI_Value1);
				SetConsoleCursorPosition(Std_Out, csbiInfo.dwCursorPosition);
				ANSI_State = -1;
			}
			else if (*cp == 'C') {
				//Cursor Forward.
				GetConsoleScreenBufferInfo(Std_Out, &csbiInfo);
				csbiInfo.dwCursorPosition.X = MIN(csbiInfo.dwSize.X, csbiInfo.dwCursorPosition.X + ANSI_Value1);
				SetConsoleCursorPosition(Std_Out, csbiInfo.dwCursorPosition);
				ANSI_State = -1;
			}
			else if (*cp == 'D') {
				//Cursor Backward.
				GetConsoleScreenBufferInfo(Std_Out, &csbiInfo);
				csbiInfo.dwCursorPosition.X = MAX(0, csbiInfo.dwCursorPosition.X - ANSI_Value1);
				SetConsoleCursorPosition(Std_Out, csbiInfo.dwCursorPosition);
				ANSI_State = -1;
			}
			else if (*cp == 'J') {
				if (ANSI_Value1 == 2) {
					GetConsoleScreenBufferInfo(Std_Out, &csbiInfo);
					len = csbiInfo.dwSize.X * csbiInfo.dwSize.Y;
					coordScreen.X = 0;
					coordScreen.Y = 0;
					FillConsoleOutputCharacter(Std_Out, (TCHAR)' ', len, coordScreen, &num);
					FillConsoleOutputAttribute(Std_Out, csbiInfo.wAttributes, len, coordScreen, &num);
					SetConsoleCursorPosition(Std_Out, coordScreen);
				}
				ANSI_State = -1;
			}
			else {
				ANSI_State = -1;
			}
			break; //End CASE 2
		case 3: //value2 start
			if (*cp >= (int)'0' && *cp <= (int)'9') {
				ANSI_Value2 = ((ANSI_Value2 * 10) + (*cp - (int)'0')) % 0xFFFF;
				ANSI_State = 4;
			}
			else if (*cp == ';') {
				//do nothing
			}
			else {
				ANSI_State = -1;
			}
			break; //End CASE 3
		case 4: //value2 continue
			if (*cp >= (int)'0' && *cp <= (int)'9') {
				ANSI_Value2 = ((ANSI_Value2 * 10) + (*cp - (int)'0')) % 0xFFFF;
				ANSI_State = 4;
			}
			else if (*cp == 'm') {
				ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value1);
				ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value2);
				SetConsoleTextAttribute(Std_Out, ANSI_Attr);
				ANSI_State = -1;
			}
			else if (*cp == ';') {
				ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value1);
				ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value2);
				SetConsoleTextAttribute(Std_Out, ANSI_Attr);
				ANSI_Value1 = 0;
				ANSI_Value2 = 0;
				ANSI_State = 1;
			}
			else if (*cp == 'H' || *cp == 'f') {
				coordScreen.Y = ANSI_Value1;
				coordScreen.X = ANSI_Value2;
				SetConsoleCursorPosition(Std_Out, coordScreen);
				ANSI_State = -1;
			}
			else {
				ANSI_State = -1;
			}


		} //End: switch (state)
	} while (ANSI_State >= 0);

	return ++cp;
}

//*** Old fragments ***************************************************

#if OLD_CONSOLE_FILE_IO
	int cfh;	// C file handle
	FILE *file;

	cfh = _open_osfhandle((long)Std_Out, _O_TEXT);
	file = _fdopen(cfh, "w");
	*stdout = *file;
	setvbuf(stdout, NULL, _IONBF, 0);

	cfh = _open_osfhandle((long)Std_Inp, _O_TEXT);
	file = _fdopen(cfh, "r");
	*stdin = *file;
#endif
