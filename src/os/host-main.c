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
**	Title: Host environment main entry point
**	Note: OS independent
**  Author: Carl Sassenrath
**  Purpose:
**		Provides the outer environment that calls the REBOL lib.
**		This module is more or less just an example and includes
**		a very simple console prompt.
**
************************************************************************
**
**  WARNING to PROGRAMMERS:
**
**		This open source code is strictly managed to maintain
**		source consistency according to our standards, not yours.
**
**		1. Keep code clear and simple.
**		2. Document odd code, your reasoning, or gotchas.
**		3. Use our source style for code, indentation, comments, etc.
**		4. It must work on Win32, Linux, OS X, BSD, big/little endian.
**		5. Test your code really well before submitting it.
**
***********************************************************************/

#if !defined(REBOL_OPTIONS_FILE)
#include "opt-config.h"
#else
#include REBOL_OPTIONS_FILE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define OS_LIB_TABLE		// include the host-lib dispatch table

#include "reb-host.h"		// standard host include files
#include "host-lib.h"		// OS host library (dispatch table)

#ifdef CUSTOM_STARTUP
#include "host-init.h"
#endif

/**********************************************************************/

REBARGS Main_Args;

#define PROMPT_STR (REBYTE*)"\x1B[1;31;49m>>\x1B[1;33;49m "
#define RESULT_STR (REBYTE*)"\x1B[32m==\x1B[1;32;49m "

#ifdef TO_WINDOWS
#define MAX_TITLE_LENGTH  1024
HINSTANCE App_Instance = 0;
HWND      Focused_Window = 0;
WCHAR     App_Title[MAX_TITLE_LENGTH]; //will be filled later from the resources file
#endif

#ifdef REB_GTK

extern void Init_Windows(void);

#endif

#ifdef REB_VIEW
extern void Init_Windows(void);
//extern void Init_Graphics(void);
#endif

//#define TEST_EXTENSIONS
#ifdef TEST_EXTENSIONS
extern void Init_Ext_Test(void);	// see: host-ext-test.c
#endif

// Host bare-bones stdio functs:
extern REBREQ *Open_StdIO();
extern void Close_StdIO(void);
extern void Put_Str(REBYTE *buf);
extern REBYTE *Get_Str(void);

void Host_Crash(char *reason) {
	OS_Crash(cb_cast("REBOL Host Failure"), cb_cast(reason));
}

void Host_Repl() {
	REBOOL why_alert = TRUE;

#define MAX_CONT_LEVEL 1024
	REBYTE cont_str[] = "\x1B[1;31;49m  \x1B[1;33;49m ";
	REBCNT cont_level = 0;
	REBYTE cont_stack[MAX_CONT_LEVEL] = { 0 };

	REBCNT input_max = 32768;
	int input_len = 0;
	REBYTE *input = OS_Make(input_max);

	REBYTE *tmp;
	REBYTE *line;
	int line_len;

	REBYTE *utf8byte;
	BOOL inside_short_str = FALSE;
	int long_str_level = 0;

	while (TRUE) {
		if (cont_level > 0) {
			cont_str[11] = cont_level <= MAX_CONT_LEVEL ? cont_stack[cont_level - 1] : '-';
			Put_Str(cont_str);
		} else {
			Put_Str(PROMPT_STR);
		}

		line = Get_Str();

		if (!line) {
			// "end of stream" - for example on CTRL+C
			if(cont_level > 0) {
				// we were in multiline editing, so escape from it only
				cont_level = 0;
				input_len = 0;
				input[0] = 0;
				continue;
			}
			Put_Str(b_cast("\x1B[0m")); //reset console color before leaving
			goto cleanup_and_return;
		}

		line_len = 0;
		for (utf8byte = line; *utf8byte; utf8byte++) {
			line_len++;
			switch (*utf8byte) {
			case '^':
				if (*(utf8byte + 1) != 0) {
					line_len++;
					utf8byte++;
				}
				break;
			case '"':
				if (long_str_level == 0) inside_short_str = !inside_short_str;
				break;
			case '[':
			case '(':
				if (!inside_short_str && long_str_level == 0) {
					if (cont_level < MAX_CONT_LEVEL) cont_stack[cont_level] = *utf8byte;
					cont_level++;
				}
				break;
			case ']':
			case ')':
				if (!inside_short_str && long_str_level == 0) {
					cont_level--;
				}
				break;
			case '{':
				if (!inside_short_str) {
					if (cont_level < MAX_CONT_LEVEL) cont_stack[cont_level] = *utf8byte;
					cont_level++;
					long_str_level++;
				}
				break;
			case '}':
				if (!inside_short_str) {
					cont_level--;
					if (long_str_level > 0) long_str_level--;
				}
				break;
			}
		}
		inside_short_str = FALSE;

		if (input_len + line_len > input_max) {
			// limit maximum input size to 2GB (it should be more than enough)
			if (input_max >= 0x80000000) goto crash_buffer;
			input_max *= 2;
			tmp = OS_Make(input_max);
			if (!tmp) {
				crash_buffer:
				Put_Str(b_cast("\x1B[0m")); //reset console color;
				Host_Crash("Growing console input buffer failed!");
				return; // make VS compiler happy
			}
			memcpy(tmp, input, input_len);
			OS_Free(input);
			input = tmp;
		}

		memcpy(&input[input_len], line, line_len);
		input_len += line_len;
		input[input_len] = 0;

		OS_Free(line);

		if (cont_level > 0)
			continue;

		input_len = 0;
		cont_level = 0;

		Put_Str(b_cast("\x1B[0m")); //reset color

		RL_Do_String(input, 0, 0);
		RL_Print_TOS(TRUE, RESULT_STR);
	}

cleanup_and_return:
	OS_Free(input);
	return;
}


/***********************************************************************
**
**  MAIN ENTRY POINT
**
**	Win32 args:
**		inst:  current instance of the application (app handle)
**		prior: always NULL (use a mutex for single inst of app)
**		cmd:   command line string (or use GetCommandLine)
**	    show:  how app window is to be shown (e.g. maximize, minimize, etc.)
**
**	Win32 return:
**		If the function succeeds, terminating when it receives a WM_QUIT
**		message, it should return the exit value contained in that
**		message's wParam parameter. If the function terminates before
**		entering the message loop, it should return zero.
**
**  Posix args: as you would expect in C.
**  Posix return: ditto.
**
***********************************************************************/

#ifdef TO_WINDOWS

#ifdef _WINDOWS //use this define with Windows subsystem, by default use console subsystem
int APIENTRY WinMain(HINSTANCE inst, HINSTANCE prior, LPSTR cmd, int show) {
	int argc;
	REBCHR **argv;
	App_Instance = inst;
#else
int main(int argc, char **argv) {
	// Retrieves the window handle used by the console associated with the calling process
	HWND hwndC = GetConsoleWindow();
	// Then we could just get the HINSTANCE:
	App_Instance = GetModuleHandle(0); // HMODULE=HINSTANCE
#endif
	// Fetch the win32 unicoded program arguments:
	argv = (char**)CommandLineToArgvW(GetCommandLineW(), &argc);
	// Use title string as defined in resources file (.rc) with hardcoded ID 101
	LoadStringW(App_Instance, 101, App_Title, MAX_TITLE_LENGTH);
#ifdef INCLUDE_IMAGE_OS_CODEC 
	CoInitialize(0);
#endif

#else //non Windows platforms
int main(int argc, char **argv) {
#endif
	REBYTE vers[8];
	REBINT n;
	REBREQ *std_io;

	// Must be done before an console I/O can occur. Does not use reb-lib,
	// so this device should open even if there are other problems.
	std_io = Open_StdIO();  // also sets up interrupt handler

	Host_Lib = &Host_Lib_Init;

	Parse_Args(argc, (REBCHR **)argv, &Main_Args);

	vers[0] = 5; // len
	RL_Version(&vers[0]);

#ifdef TO_WINDOWS
	// Setting title after Open_StdIO, because with Windows subsystem the console is not by default opened.
	SetConsoleTitle((LPWSTR)App_Title);
#endif

	// Initialize the REBOL library (reb-lib):
	if (!CHECK_STRUCT_ALIGN) Host_Crash("Incompatible struct alignment");
	if (!Host_Lib) Host_Crash("Missing host lib");
	// !!! Second part will become vers[2] < RL_REV on release!!!
	if (vers[1] != RL_VER || vers[2] != RL_REV) Host_Crash("Incompatible reb-lib DLL");
	Host_Lib->std_io = std_io;
	n = RL_Init(&Main_Args, Host_Lib);
	if (n == 1) Host_Crash("Host-lib wrong size");
	if (n == 2) Host_Crash("Host-lib wrong version/checksum");

#ifdef REB_VIEW
	Init_Windows();
	//Init_Graphics();
#endif

#ifdef TEST_EXTENSIONS
	Init_Ext_Test();
#endif

// Call sys/start function. If a compressed script is provided, it will be 
// decompressed, stored in system/options/boot-host, loaded, and evaluated.
// Returns: 0: ok, -1: error, 1: bad data.
#ifdef CUSTOM_STARTUP
	// For custom startup, you can provide compressed script code here:
	n = RL_Start((REBYTE *)(&Reb_Init_Code[0]), REB_INIT_SIZE, 0); // TRUE on halt
#else
	n = RL_Start(0, 0, 0);
#endif

	if (
		!(Main_Args.options & RO_CGI)
		&& (
			!Main_Args.script // no script was provided
			|| n  < 0         // script halted or had error
			|| (Main_Args.options & RO_HALT)  // --halt option
		)
	){
		if (n < 0 && !(Main_Args.options & RO_HALT)) {
			RL_Do_String(b_cast("unless system/options/quiet [print {^[[mClosing in 3s!} wait 3] quit/return -1"), 0, 0);
		}
		Host_Repl();
	}

	OS_Exit(0);
	return 0;
}

