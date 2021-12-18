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

// some modes may not be defined
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT      0x0200
#endif
#ifndef ENABLE_INSERT_MODE
#define ENABLE_INSERT_MODE                 0x0020
#endif
#ifndef ENABLE_QUICK_EDIT_MODE
#define ENABLE_QUICK_EDIT_MODE             0x0040
#endif
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif


#define CONSOLE_MODES ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT \
		| ENABLE_EXTENDED_FLAGS | ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE

static HANDLE Std_Out = 0;
static HANDLE Std_Err = 0;
static HANDLE Std_Inp = 0;
static HANDLE Std_Echo = 0;
static REBCHR *Std_Buf = 0;		// for input and output

static BOOL Redir_Out = 0;
static BOOL Redir_Inp = 0;

static BOOL Handled_Break = 0;

// Since some Windows10 version it's possible to use the new terminal processing,
// of ANSI escape sequences. From my tests its not faster than my emulation, but
// may be used for functionalities which are not handled in the emulation.
// If the terminal processing is not available, there is a fallback to emulation
// so basic ANSI is supported also on older Windows.
//
// Known issues with the new MS terminal processing:
//	* it does not process sequences to switch output echo (^[[8m and ^[[28m)
//	  currently used in ASK/HIDE 
//
// Documentation: https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
//
// It's possible to force the emulation with following compilation definition:
#ifdef FORCE_ANSI_ESC_EMULATION_ON_WINDOWS
static BOOL Emulate_ANSI = 1; // forces backward compatible ANSI emulation (not using VIRTUAL_TERMINAL_PROCESSING)
#else
static BOOL Emulate_ANSI = 0;
#endif

// Special access:
extern REBDEV *Devices[];
extern void Close_StdIO(void);


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

DWORD dwOriginalOutMode = 0;
DWORD dwOriginalInpMode = 0;
WORD wOriginalAttributes = 0;

int Update_Graphic_Mode(int attribute, int value, boolean set);
const REBYTE* Parse_ANSI_sequence(const REBYTE *cp, const REBYTE *ep);

//**********************************************************************

BOOL WINAPI Handle_Break(DWORD dwCtrlType)
{
	//printf("\nHandle_Break %i\n", dwCtrlType);
	if(Handled_Break) {
		// CTRL-C was catched durring ReadConsoleW and was already processed
		Handled_Break = FALSE;
		return TRUE;
	}
	// Handle the MS CMD console CTRL-C, BREAK, and other events:
	if (dwCtrlType >= CTRL_CLOSE_EVENT) OS_Exit(100); // close button, shutdown, etc.
	RL_Escape(0);
	return TRUE;	// We handled it
}

void Set_Input_Mode(DWORD mode, boolean set) {
	DWORD modes = 0;
	GetConsoleMode(Std_Inp, &modes);
	if (set) 
		SetConsoleMode(Std_Inp, modes | mode);
	else
		SetConsoleMode(Std_Inp, modes & (~mode));
}
void Set_Cursor_Visible(boolean visible) {
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(Std_Out, &cursorInfo);  // Get cursorinfo from output
	cursorInfo.bVisible = visible;               // Set flag visible.
	SetConsoleCursorInfo(Std_Out, &cursorInfo);  // Apply changes
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

static void Close_StdIO_Local(void)
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

	// reset original modes on exit
	if (Std_Inp) SetConsoleMode(Std_Inp, dwOriginalInpMode);
	if (Std_Out) {
		SetConsoleMode(Std_Out, dwOriginalOutMode);
		SetConsoleTextAttribute(Std_Out, wOriginalAttributes);
	}

	Close_StdIO_Local();
	Close_StdIO();  // frees host's input buffer
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

	GetConsoleTitle((LPWSTR)pszOldWindowTitle, MY_BUFSIZE>>1); // size is in wide chars, not bytes!

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
		SET_FLAG(req->modes, RDM_READ_LINE);
		return DR_DONE; // Do not do it again
	}

	if (!GET_FLAG(req->modes, RDM_NULL)) {

		// Get the raw stdio handles:
		Std_Out = GetStdHandle(STD_OUTPUT_HANDLE);
		Std_Inp = GetStdHandle(STD_INPUT_HANDLE);
		//Std_Err = GetStdHandle(STD_ERROR_HANDLE);
		Std_Echo = 0;

		// store original text attributes:
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
		GetConsoleScreenBufferInfo(Std_Out, &csbiInfo);
		wOriginalAttributes = csbiInfo.wAttributes;

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

		// store original modes
		GetConsoleMode(Std_Out, &dwOriginalOutMode);
		GetConsoleMode(Std_Inp, &dwOriginalInpMode);

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
			DWORD dwInpMode = CONSOLE_MODES;
			DWORD dwOutMode = dwOriginalOutMode;

			if (!SetConsoleMode(Std_Inp, dwInpMode)) {
				printf("Failed to set input ConsoleMode! Error: %li\n", GetLastError());
				return DR_ERROR;
			}

			if (!Emulate_ANSI) {
				//dwInpMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
				dwOutMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				if (SetConsoleMode(Std_Out, dwOutMode)) {
					//SetConsoleMode(Std_Inp, dwInpMode);
				} else {
					Emulate_ANSI = 1; // failed to use VIRTUAL_TERMINAL_PROCESSING, so force emulation
				}
			}

			// Try some Set Graphics Rendition (SGR) terminal escape sequences
			/*
			wprintf(L"\x1b[31mThis text has a red foreground using SGR.31.\r\n");
			wprintf(L"\x1b[1mThis text has a bright (bold) red foreground using SGR.1 to affect the previous color setting.\r\n");
			wprintf(L"\x1b[mThis text has returned to default colors using SGR.0 implicitly.\r\n");
			wprintf(L"\x1b[34;46mThis text shows the foreground and background change at the same time.\r\n");
			wprintf(L"\x1b[0mThis text has returned to default colors using SGR.0 explicitly.\r\n");
			wprintf(L"\x1b[31;32;33;34;35;36;101;102;103;104;105;106;107mThis text attempts to apply many colors in the same command. Note the colors are applied from left to right so only the right-most option of foreground cyan (SGR.36) and background bright white (SGR.107) is effective.\r\n");
			wprintf(L"\x1b[39mThis text has restored the foreground color only.\r\n");
			wprintf(L"\x1b[49mThis text has restored the background color only.\r\n");
			*/
		}

		// Handle stdio CTRL-C interrupt:
		SetConsoleCtrlHandler(Handle_Break, TRUE);
	}
	else
		SET_FLAG(dev->flags, SF_DEV_NULL);

	//SetConsoleOutputCP(65001); // Don't use, cause of crash on Win7! https://github.com/Oldes/Rebol3/issues/25

	SET_FLAG(req->flags, RRF_OPEN);
	SET_FLAG(dev->flags, RDF_OPEN);
	SET_FLAG(req->modes, RDM_READ_LINE);

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
	long len;
	unsigned long total = 0;
	BOOL ok = FALSE;
	const REBYTE *bp;
	const REBYTE *cp;
	const REBYTE *ep;
	HANDLE hOutput;

	if (GET_FLAG(req->modes, RDM_NULL)) {
		req->actual = req->length;
		return DR_DONE;
	}
	if (GET_FLAG(req->flags, RRF_ERROR)) {
		hOutput = GetStdHandle(STD_ERROR_HANDLE);
	}
	else {
		// use stderr if requested by user
		hOutput = Std_Err ? Std_Err : Std_Out;
	}

	if (hOutput) {

		bp = req->data;
		ep = bp + req->length;

		if(Emulate_ANSI) {
			// Using this loop for seeking escape char and processing ANSI sequence
			do {
				if (ANSI_State >= 0) // there is pending ansi sequence state
					bp = Parse_ANSI_sequence(bp - 1, ep); // the pointer is incremented back in the parser

				cp = Skip_To_Char(bp, ep, (REBYTE)27); //find ANSI escape char "^["

				if (Redir_Out) { // for Console SubSystem (always UTF-8)
					if (cp) {
						ok = WriteFile(hOutput, bp, cp - bp, &total, 0);
						bp = Parse_ANSI_sequence(cp, ep);
					}
					else {
						ok = WriteFile(hOutput, bp, ep - bp, &total, 0);
						bp = ep;
					}
					if (!ok) {
						req->error = GetLastError();
						return DR_ERROR;
					}
				}
				else { // for Windows SubSystem - must be converted to Win32 wide-char format
					//if found, write to the console content before it starts, else everything
					if (cp) {
						len = MultiByteToWideChar(CP_UTF8, 0, cs_cast(bp), cp - bp, Std_Buf, BUF_SIZE);
					}
					else {
						len = MultiByteToWideChar(CP_UTF8, 0, cs_cast(bp), ep - bp, Std_Buf, BUF_SIZE);
						bp = ep;
					}
					if (len > 0) {// no error
						ok = WriteConsoleW(hOutput, Std_Buf, len, &total, 0);
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
		} else {
			// using MS built in ANSI processing
			if (Redir_Out) { // Always UTF-8
				ok = WriteFile(hOutput, req->data, req->length, &total, 0);
			}
			else {
				// Convert UTF-8 buffer to Win32 wide-char format for console.
				// Thankfully, MS provides something other than mbstowcs();
				// however, if our buffer overflows, it's an error. There's no
				// efficient way at this level to split-up the input data,
				// because its UTF-8 with variable char sizes.
				len = MultiByteToWideChar(CP_UTF8, 0, cs_cast(req->data), req->length, Std_Buf, BUF_SIZE);
				if (len > 0) // no error
					ok = WriteConsoleW(hOutput, Std_Buf, len, &total, 0);
			}
		}
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
	unsigned long total = 0;
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
			//printf("%li\n", total);
		}
		else if (GET_FLAG(req->modes, RDM_READ_LINE)) {
			ok = ReadConsoleW(Std_Inp, Std_Buf, BUF_SIZE-1, &total, 0);
			if (ok) {
				if (total == 0) {
					// CTRL-C pressed
					Handled_Break = TRUE; // let the break handler (which is signaled later) to know,
					                      // that we handled it already
					HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
					SetConsoleTextAttribute(hErr, FOREGROUND_INTENSITY | FOREGROUND_MAGENTA);
					WriteConsoleW(hErr, L"[ESC]\r\n", 7, NULL, 0);
					SetConsoleTextAttribute(hErr, FOREGROUND_GREY);
					req->data[0] = '\x1B'; // ESC char
					req->actual = 1;
					return DR_DONE;
				}
				total = WideCharToMultiByte(CP_UTF8, 0, Std_Buf, total, s_cast(req->data), req->length, 0, 0);
				if (!total) ok = FALSE;
			}
		}
		else {
			DWORD cNumRead, i; 
			INPUT_RECORD irInBuf[128]; 
			int counter=0;
			ok = ReadConsoleInputW(Std_Inp, irInBuf, 128, &cNumRead);
			for (i = 0; i < cNumRead; i++) 
			{
				printf("et: %u\n", irInBuf[i].EventType);
				switch(irInBuf[i].EventType) 
				{ 
					case KEY_EVENT: // keyboard input 
						puts("key");
				}
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
*/	DEVICE_CMD Poll_IO(REBREQ *req)
/*
**		Read console input and convert it to system events
**
***********************************************************************/
{
	REBEVT evt;
	DWORD  cNumRead, i, repeat; 
	INPUT_RECORD irInBuf[8]; 
	if (ReadConsoleInput(Std_Inp, irInBuf, 8, &cNumRead)) {
		//printf("cNumRead: %u\n", cNumRead);
		for (i = 0; i < cNumRead; i++) 
		{
			//printf("peek: %u\n", irInBuf[i].EventType);
			evt.flags = 0;
			evt.model = EVM_CONSOLE;
			switch (irInBuf[i].EventType) 
			{ 
				case KEY_EVENT:
				{ 
					KEY_EVENT_RECORD ker = irInBuf[i].Event.KeyEvent;
					//printf("key: %u %u %u %u %u\n", ker.uChar.UnicodeChar, ker.bKeyDown, ker.wRepeatCount, ker.wVirtualKeyCode, sizeof(REBEVT));
					evt.data  = (u32)ker.uChar.UnicodeChar;
					if (ker.bKeyDown) {
						if (evt.data == VK_CANCEL) {
							RL_Escape(0);
							return DR_DONE; // so stop sending other events
						}
						evt.type = EVT_KEY;
					} else {
						evt.type = EVT_KEY_UP;
					}
					
					repeat = ker.wRepeatCount;
					break;
				}
				case MOUSE_EVENT:
				{
					MOUSE_EVENT_RECORD mer = irInBuf[i].Event.MouseEvent;
					repeat = 1;
					evt.data = (u32)MAKELONG(mer.dwMousePosition.X, mer.dwMousePosition.Y);
					SET_FLAG(evt.flags, EVF_HAS_XY);
					//printf("mou: %u %u\n", mer.dwEventFlags, mer.dwButtonState);
					switch (mer.dwEventFlags) {
						case MOUSE_MOVED:
							evt.type = EVT_MOVE;
							break;
						case 0:
							if (!mer.dwButtonState) {
								continue;
							} else if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
								evt.type = EVT_DOWN;
							} else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {
								evt.type = EVT_ALT_DOWN;
							} else {
								evt.type = EVT_AUX_DOWN;
							}
							break;
						case DOUBLE_CLICK:
							evt.type = EVT_DOWN;
							SET_FLAG(evt.flags, EVF_DOUBLE);
							break;
						case MOUSE_HWHEELED:
							//printf("horizontal mouse wheel\n");
							continue;
						case MOUSE_WHEELED:
							//printf("vertical mouse wheel\n");
							continue;
						default:
							//printf("unknown\n");
							continue;
					}
					break;
				}
				case WINDOW_BUFFER_SIZE_EVENT:
				{
					WINDOW_BUFFER_SIZE_RECORD bsr = irInBuf[i].Event.WindowBufferSizeEvent;
					evt.type = EVT_RESIZE;
					evt.data = (u32)MAKELONG(bsr.dwSize.X, bsr.dwSize.Y);
					SET_FLAG(evt.flags, EVF_HAS_XY);
					repeat = 1;
					break;
				}
				case FOCUS_EVENT:
				{
					FOCUS_EVENT_RECORD fer = irInBuf[i].Event.FocusEvent;
					evt.type  = fer.bSetFocus ? EVT_FOCUS : EVT_UNFOCUS;
					evt.data  = 0;
					repeat = 1;
					break;
				}
                //case MENU_EVENT:
                //    break; 
				default: // ignore other events
					continue;
			}
			while (repeat-->0) 
				if (!RL_Event(&evt)) // returns 0 if queue is full
					return DR_DONE; // so stop sending other events
		}
	}

	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Query_IO(REBREQ *req)
/*
**		Resolve console port information. Currently just size of console.
**
***********************************************************************/
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	if(0 == GetConsoleScreenBufferInfo(Std_Out, &csbiInfo)) {
		req->error = GetLastError();
		return DR_ERROR;
	}
	req->console.buffer_rows = csbiInfo.dwSize.Y;
	req->console.buffer_cols = csbiInfo.dwSize.X;
	req->console.window_rows = csbiInfo.srWindow.Bottom - csbiInfo.srWindow.Top + 1;
	req->console.window_cols = csbiInfo.srWindow.Right - csbiInfo.srWindow.Left + 1;
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
	boolean value = req->modify.value;
	switch (req->modify.mode) {
	case MODE_CONSOLE_ECHO:
		Set_Input_Mode(ENABLE_ECHO_INPUT, value);
		break;
	case MODE_CONSOLE_LINE:
		Set_Input_Mode(ENABLE_LINE_INPUT | ENABLE_QUICK_EDIT_MODE | ENABLE_PROCESSED_INPUT, value);
		Set_Input_Mode(ENABLE_MOUSE_INPUT, !value);
		if (value) {
			SET_FLAG(req->modes, RDM_READ_LINE);
		}
		else {
			CLR_FLAG(req->modes, RDM_READ_LINE);
			SET_FLAG(req->flags, RRF_PENDING);
			SET_FLAG(Devices[1]->flags, RDO_AUTO_POLL);
		}
		break;
	case MODE_CONSOLE_ERROR:
		Std_Err = value ? GetStdHandle(STD_ERROR_HANDLE) : 0;
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
	Poll_IO,	// poll
	0,	// connect
	Query_IO,
	Modify_IO,	// modify
	Open_Echo,	// CREATE used for opening echo file
};

DEFINE_DEV(Dev_StdIO, "Standard IO", 1, Dev_Cmds, RDC_MAX, 0);



/***********************************************************************
**
*/	int Update_Graphic_Mode(int attribute, int value, boolean set)
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
		case 1: attribute = attribute | FOREGROUND_INTENSITY;          break;
		case 4: attribute = attribute | COMMON_LVB_UNDERSCORE;         break;
		case 7: tmp = (attribute & 0xFFF0) >> 4;
				attribute = ((attribute & 0xFF0F) << 4) | tmp;           break; //reverse
		case 8: Set_Input_Mode(ENABLE_ECHO_INPUT, FALSE);              break; //Conceal (turn off echo)
		case 22: attribute = attribute & 0xFFF7;                         break; //FOREGROUND_INTENSITY reset
		case 24: attribute = attribute & 0x7FFF;                         break; //reset underscore
		case 28: Set_Input_Mode(ENABLE_ECHO_INPUT, TRUE);              break; //Reveal (Conceal off)
		case 30: attribute =  attribute & 0xFFF8;                        break;
		case 31: attribute = (attribute & 0xFFF8) | FOREGROUND_RED;      break;
		case 32: attribute = (attribute & 0xFFF8) | FOREGROUND_GREEN;    break;
		case 33: attribute = (attribute & 0xFFF8) | FOREGROUND_YELLOW;   break;
		case 34: attribute = (attribute & 0xFFF8) | FOREGROUND_BLUE;     break;
		case 35: attribute = (attribute & 0xFFF8) | FOREGROUND_MAGENTA;  break;
		case 36: attribute = (attribute & 0xFFF8) | FOREGROUND_CYAN;     break;
		case 37: attribute = (attribute & 0xFFF8) | FOREGROUND_GREY;     break;	
		case 39: attribute =  attribute & 0xFFF7;                        break;  //FOREGROUND_INTENSITY reset	
		case 40: attribute =  attribute & 0xFF8F;                        break;
		case 41: attribute = (attribute & 0xFF8F) | BACKGROUND_RED;      break;
		case 42: attribute = (attribute & 0xFF8F) | BACKGROUND_GREEN;    break;
		case 43: attribute = (attribute & 0xFF8F) | BACKGROUND_YELLOW;   break;
		case 44: attribute = (attribute & 0xFF8F) | BACKGROUND_BLUE;     break;
		case 45: attribute = (attribute & 0xFF8F) | BACKGROUND_MAGENTA;  break;
		case 46: attribute = (attribute & 0xFF8F) | BACKGROUND_CYAN;     break;
		case 47: attribute = (attribute & 0xFF8F) | BACKGROUND_GREY;     break;
		case 49: attribute =  attribute & 0xFF7F;                        break; //BACKGROUND_INTENSITY reset
		//bright foreground colors
		case 90: attribute = (attribute & 0xFFF8) | FOREGROUND_INTENSITY;                       break;
		case 91: attribute = (attribute & 0xFFF8) | FOREGROUND_INTENSITY | FOREGROUND_RED;      break;
		case 92: attribute = (attribute & 0xFFF8) | FOREGROUND_INTENSITY | FOREGROUND_GREEN;    break;
		case 93: attribute = (attribute & 0xFFF8) | FOREGROUND_INTENSITY | FOREGROUND_YELLOW;   break;
		case 94: attribute = (attribute & 0xFFF8) | FOREGROUND_INTENSITY | FOREGROUND_BLUE;     break;
		case 95: attribute = (attribute & 0xFFF8) | FOREGROUND_INTENSITY | FOREGROUND_MAGENTA;  break;
		case 96: attribute = (attribute & 0xFFF8) | FOREGROUND_INTENSITY | FOREGROUND_CYAN;     break;
		case 97: attribute = (attribute & 0xFFF8) | FOREGROUND_INTENSITY | FOREGROUND_GREY;     break;
		//bright background colors
		case 100: attribute = (attribute & 0xFF8F) | BACKGROUND_INTENSITY;                      break;
		case 101: attribute = (attribute & 0xFF8F) | BACKGROUND_INTENSITY | BACKGROUND_RED;     break;
		case 102: attribute = (attribute & 0xFF8F) | BACKGROUND_INTENSITY | BACKGROUND_GREEN;   break;
		case 103: attribute = (attribute & 0xFF8F) | BACKGROUND_INTENSITY | BACKGROUND_YELLOW;  break;
		case 104: attribute = (attribute & 0xFF8F) | BACKGROUND_INTENSITY | BACKGROUND_BLUE;    break;
		case 105: attribute = (attribute & 0xFF8F) | BACKGROUND_INTENSITY | BACKGROUND_MAGENTA; break;
		case 106: attribute = (attribute & 0xFF8F) | BACKGROUND_INTENSITY | BACKGROUND_CYAN;    break;
		case 107: attribute = (attribute & 0xFF8F) | BACKGROUND_INTENSITY | BACKGROUND_GREY;    break;

		//default: attribute = value1;
	}
	if(set) SetConsoleTextAttribute(Std_Out, attribute);
	return attribute;
}

/***********************************************************************
**
*/	const REBYTE* Parse_ANSI_sequence(const REBYTE *cp, const REBYTE *ep)
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
			else if (*cp == 'm') {
				ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, 0, TRUE);
				ANSI_State = -1;
			}
			else if (*cp == '?' && (cp+3)<ep && cp[1] == '2' && cp[2] == '5') {
				if(cp[3] == 'l') {
					Set_Cursor_Visible(FALSE);
					cp += 4;
				} else if(cp[3] == 'h') {
					Set_Cursor_Visible(TRUE);
					cp += 4;
				} else {
					ANSI_State = -1;
				}
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
				ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value1, TRUE);
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
			else if (*cp == 'm' || *cp == ';') {
				if(ANSI_Value2 == 5 && (ANSI_Value1 == 38 || ANSI_Value1 == 48)) {
					// 8-bit colors... try to emulate it at least partially
					if(*cp == 'm') {
						// ignored
						ANSI_State = -1;
						break;
					} else {
						// 256-color lookup; fetch one more value
						ANSI_Value1 = -ANSI_Value1;
						ANSI_Value2 = 0;
						ANSI_State = 3;
						break;
					}
				}
				else if(ANSI_Value1 < 0) {
					// 256-color lookup
					ANSI_Value1 = -ANSI_Value1 - 8;
					if(ANSI_Value2 < 8) {
						// standard colors (as in ESC [ 30�37 m)
						ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value2 + ANSI_Value1, TRUE);
						
					} else if(ANSI_Value2 < 16) {
						// high intensity colors (as in ESC [ 90�97 m)
						ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value2 + ANSI_Value1 + 60 - 8, TRUE);
					} else {
						// on POSIX it is 216 colors and or 24 grey colors
						// this is not possible on Windows.. so just ignore it.
					}
				}
				else {
					ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value1, FALSE);
					ANSI_Attr = Update_Graphic_Mode(ANSI_Attr, ANSI_Value2, TRUE);
				}
				if (*cp == ';') {
					ANSI_Value1 = 0;
					ANSI_Value2 = 0;
					ANSI_State = 1;
				}
				else {
					ANSI_State = -1;
				}
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
