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

#define PROMPT_STR ">> "
#define RESULT_STR "== "

REBARGS Main_Args;

#ifdef TO_WIN32
HINSTANCE App_Instance = 0;
#endif

#ifndef REB_CORE
extern void Init_Windows(void);
extern void Init_Graphics(void);
#endif

//#define TEST_EXTENSIONS
#ifdef TEST_EXTENSIONS
extern void Init_Ext_Test(void);	// see: host-ext-test.c
#endif

// Host bare-bones stdio functs:
extern void Open_StdIO(void);
extern void Put_Str(char *buf);
extern REBYTE *Get_Str();

void Host_Crash(REBYTE *reason) {
	OS_Crash("REBOL Host Failure", reason);
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

#ifdef TO_WIN32
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prior, LPSTR cmd, int show)
#else
int main(int argc, char **argv)
#endif
{
	REBYTE vers[8];
	REBYTE *line;
	REBINT n;

#ifdef TO_WIN32  // In Win32 get args manually:
	int argc;
	REBCHR **argv;
	// Fetch the win32 unicoded program arguments:
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	App_Instance = inst;
#endif

	Host_Lib = &Host_Lib_Init;

	Parse_Args(argc, (REBCHR **)argv, &Main_Args);

	vers[0] = 5; // len
	RL_Version(&vers[0]);

	// Must be done before an console I/O can occur. Does not use reb-lib,
	// so this device should open even if there are other problems.
	Open_StdIO();  // also sets up interrupt handler

	// Initialize the REBOL library (reb-lib):
	if (!CHECK_STRUCT_ALIGN) Host_Crash("Incompatible struct alignment");
	if (!Host_Lib) Host_Crash("Missing host lib");
	// !!! Second part will become vers[2] < RL_REV on release!!!
	if (vers[1] != RL_VER || vers[2] != RL_REV) Host_Crash("Incompatible reb-lib DLL");
	n = RL_Init(&Main_Args, Host_Lib);
	if (n == 1) Host_Crash("Host-lib wrong size");
	if (n == 2) Host_Crash("Host-lib wrong version/checksum");

#ifndef REB_CORE
	Init_Windows();
	Init_Graphics();
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

	// Console line input loop (just an example, can be improved):
	if (
		!(Main_Args.options & RO_CGI)
		&& (
			!Main_Args.script // no script was provided
			|| n  < 0         // script halted or had error
			|| Main_Args.options & RO_HALT  // --halt option
		)
	){
		n = 0;  // reset error code (but should be able to set it below too!)
		while (TRUE) {
			Put_Str(PROMPT_STR);
			if ((line = Get_Str())) {
				RL_Do_String(line, 0, 0);
				RL_Print_TOS(0, RESULT_STR);
				OS_Free(line);
			}
			else break; // EOS
		}
	}

	//OS_Call_Device(RDI_STDIO, RDC_CLOSE);
	OS_Quit_Devices(0);

	// A QUIT does not exit this way, so the only valid return code is zero.
	return 0;
}

