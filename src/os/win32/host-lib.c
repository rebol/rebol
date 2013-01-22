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
**  Title: OS API function library called by REBOL interpreter
**  Author: Carl Sassenrath
**  Purpose:
**      This module provides the functions that REBOL calls
**      to interface to the native (host) operating system.
**      REBOL accesses these functions through the structure
**      defined in host-lib.h (auto-generated, do not modify).
**
**	Flags: compile with -DUNICODE for Win32 wide char API
**
**  Special note:
**      This module is parsed for function declarations used to
**      build prototypes, tables, and other definitions. To change
**      function arguments requires a rebuild of the REBOL library.
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

/* WARNING:
**     The function declarations here cannot be modified without
**     also modifying those found in the other OS host-lib files!
**     Do not even modify the argument names.
*/

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>

#include "reb-host.h"
#include "host-lib.h"

// Semaphore lock to sync sub-task launch:
static void *Task_Ready;


/***********************************************************************
**
*/	void Convert_Date(SYSTEMTIME *stime, REBOL_DAT *dat, long zone)
/*
**		Convert local format of system time into standard date
**		and time structure.
**
***********************************************************************/
{
	dat->year  = stime->wYear;
	dat->month = stime->wMonth;
	dat->day   = stime->wDay;
	dat->time  = stime->wHour * 3600 + stime->wMinute * 60 + stime->wSecond;
	dat->nano  = 1000000 * stime->wMilliseconds;
	dat->zone  = zone;
}

/***********************************************************************
**
*/	static void Insert_Command_Arg(REBCHR *cmd, REBCHR *arg, REBINT limit)
/*
**		Insert an argument into a command line at the %1 position,
**		or at the end if there is no %1. (An INSERT action.)
**		Do not exceed the specified limit length.
**
**		Too bad std Clib does not provide INSERT or REPLACE functions.
**
***********************************************************************/
{
	#define HOLD_SIZE 2000
	REBCHR *spot;
	REBCHR hold[HOLD_SIZE+4];

	if ((REBINT)LEN_STR(cmd) >= limit) return; // invalid case, ignore it.

	// Find %1:
	spot = FIND_STR(cmd, TEXT("%1"));

	if (spot) {
		// Save rest of cmd line (such as end quote, -flags, etc.)
		COPY_STR(hold, spot+2, HOLD_SIZE);

		// Terminate at the arg location:
		spot[0] = 0;

		// Insert the arg:
		JOIN_STR(spot, arg, limit - LEN_STR(cmd) - 1);

		// Add back the rest of cmd:
		JOIN_STR(spot, hold, limit - LEN_STR(cmd) - 1);
	}
	else {
		JOIN_STR(cmd, TEXT(" "), 1);
		JOIN_STR(cmd, arg, limit - LEN_STR(cmd) - 1);
	}
}


/***********************************************************************
**
**	OS Library Functions
**
***********************************************************************/

/***********************************************************************
**
*/	REBINT OS_Config(int id, REBYTE *result)
/*
**		Return a specific runtime configuration parameter.
**
***********************************************************************/
{
#define OCID_STACK_SIZE 1  // needs to move to .h file

	switch (id) {
	case OCID_STACK_SIZE:
		return 0;  // (size in bytes should be returned here)
	}

	return 0;
}


/***********************************************************************
**
*/	void *OS_Make(size_t size)
/*
**		Allocate memory of given size.
**
**		This is necessary because some environments may use their
**		own specific memory allocation (e.g. private heaps).
**
***********************************************************************/
{
	return malloc(size);
}


/***********************************************************************
**
*/	void OS_Free(void *mem)
/*
**		Free memory allocated in this OS environment. (See OS_Make)
**
***********************************************************************/
{
	free(mem);
}


/***********************************************************************
**
*/	void OS_Exit(int code)
/*
**		Called in cases where REBOL needs to quit immediately
**		without returning from the main() function.
**
***********************************************************************/
{
	//OS_Call_Device(RDI_STDIO, RDC_CLOSE); // close echo
	OS_Quit_Devices(0);
	exit(code);
}


/***********************************************************************
**
*/	void OS_Crash(const REBYTE *title, const REBYTE *content)
/*
**		Tell user that REBOL has crashed. This function must use
**		the most obvious and reliable method of displaying the
**		crash message.
**
**		If the title is NULL, then REBOL is running in a server mode.
**		In that case, we do not want the crash message to appear on
**		the screen, because the system may be unattended.
**
**		On some systems, the error may be recorded in the system log.
**
***********************************************************************/
{
	// Echo crash message if echo file is open:
	///PUTE(content);
	OS_Call_Device(RDI_STDIO, RDC_CLOSE); // close echo

	// A title tells us we should alert the user:
	if (title) {
	//	OS_Put_Str(title);
	//	OS_Put_Str(":\n");
		// Use ASCII only (in case we are on non-unicode win32):
		MessageBoxA(NULL, content, title, MB_ICONHAND);
	}
	//	OS_Put_Str(content);
	exit(100);
}


/***********************************************************************
**
*/	REBCHR *OS_Form_Error(int errnum, REBCHR *str, int len)
/*
**		Translate OS error into a string. The str is the string
**		buffer and the len is the length of the buffer.
**
***********************************************************************/
{
	LPVOID lpMsgBuf;
	int ok;

	if (!errnum) errnum = GetLastError();

	ok = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errnum,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL);

	len--; // termination

	if (!ok) COPY_STR(str, TEXT("unknown error"), len);
	else {
		COPY_STR(str, lpMsgBuf, len);
		LocalFree(lpMsgBuf);
	}
	return str;
}


/***********************************************************************
**
*/	REBOOL OS_Get_Boot_Path(REBCHR *name)
/*
**		Used to determine the program file path for REBOL.
**		This is the path stored in system->options->boot and
**		it is used for finding default boot files.
**
***********************************************************************/
{
	return (GetModuleFileName(0, name, MAX_FILE_NAME) > 0);
}


/***********************************************************************
**
*/	REBCHR *OS_Get_Locale(int what)
/*
**		Used to obtain locale information from the system.
**		The returned value must be freed with OS_FREE_MEM.
**
***********************************************************************/
{
	LCTYPE type;
	int len;
	REBCHR *data;
	LCTYPE types[] = {
		LOCALE_SENGLANGUAGE,
		LOCALE_SNATIVELANGNAME,
		LOCALE_SENGCOUNTRY,
		LOCALE_SCOUNTRY,
	};

	type = types[what];

	len = GetLocaleInfo(0, type, 0, 0);
	data = MAKE_STR(len);
	len = GetLocaleInfo(0, type, data, len);

	return data;
}


/***********************************************************************
**
*/	REBINT OS_Get_Env(REBCHR *envname, REBCHR* envval, REBINT valsize)
/*
**		Get a value from the environment.
**		Returns size of retrieved value for success or zero if missing.
**		If return size is greater than valsize then value contents
**		are undefined, and size includes null terminator of needed buf
**
***********************************************************************/
{
	// Note: The Windows variant of this API is NOT case-sensitive

	REBINT result = GetEnvironmentVariable(envname, envval, valsize);
	if (result == 0) { // some failure...
		if (GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
			return 0; // not found
		}
		return -1; // other error
	}
	return result;
}


/***********************************************************************
**
*/	REBOOL OS_Set_Env(REBCHR *envname, REBCHR *envval)
/*
**		Set a value from the environment.
**		Returns >0 for success and 0 for errors.
**
***********************************************************************/
{
	return SetEnvironmentVariable(envname, envval);
}


/***********************************************************************
**
*/	REBCHR *OS_List_Env(void)
/*
***********************************************************************/
{
	REBCHR *env = GetEnvironmentStrings();
	REBCNT n, len = 0;
	REBCHR *str;

	str = env;
	while (n = LEN_STR(str)) {
		len += n + 1;
		str = env + len; // next
	}
	len++;

	str = OS_Make(len * sizeof(REBCHR));
	MOVE_MEM(str, env, len * sizeof(REBCHR));

	FreeEnvironmentStrings(env);

	return str;
}


/***********************************************************************
**
*/	void OS_Get_Time(REBOL_DAT *dat)
/*
**		Get the current system date/time in UTC plus zone offset (mins).
**
***********************************************************************/
{
	SYSTEMTIME stime;
	TIME_ZONE_INFORMATION tzone;

	GetSystemTime(&stime);

	if (TIME_ZONE_ID_DAYLIGHT == GetTimeZoneInformation(&tzone))
		tzone.Bias += tzone.DaylightBias;

	Convert_Date(&stime, dat, -tzone.Bias);
}


/***********************************************************************
**
*/	i64 OS_Delta_Time(i64 base, int flags)
/*
**		Return time difference in microseconds. If base = 0, then
**		return the counter. If base != 0, compute the time difference.
**
**		Note: Requires high performance timer.
** 		Q: If not found, use timeGetTime() instead ?!
**
***********************************************************************/
{
	LARGE_INTEGER freq;
	LARGE_INTEGER time;

	if (!QueryPerformanceCounter(&time))
		OS_Crash("Missing resource", "High performance timer");

	if (base == 0) return time.QuadPart; // counter (may not be time)

	QueryPerformanceFrequency(&freq);

	return ((time.QuadPart - base) * 1000) / (freq.QuadPart / 1000);
}


/***********************************************************************
**
*/	int OS_Get_Current_Dir(REBCHR **path)
/*
**		Return the current directory path as a string and
**		its length in chars (not bytes).
**
**		The result should be freed after copy/conversion.
**
***********************************************************************/
{
	int len;

	len = GetCurrentDirectory(0, NULL); // length, incl terminator.
	*path = MAKE_STR(len);
	GetCurrentDirectory(len, *path);
	len--; // less terminator

	return len; // Be sure to call free() after usage
}


/***********************************************************************
**
*/	REBOOL OS_Set_Current_Dir(REBCHR *path)
/*
**		Set the current directory to local path. Return FALSE
**		on failure.
**
***********************************************************************/
{
	return SetCurrentDirectory(path);
}


/***********************************************************************
**
*/	void OS_File_Time(REBREQ *file, REBOL_DAT *dat)
/*
**		Convert file.time to REBOL date/time format.
**		Time zone is UTC.
**
***********************************************************************/
{
	SYSTEMTIME stime;
	TIME_ZONE_INFORMATION tzone;

	if (TIME_ZONE_ID_DAYLIGHT == GetTimeZoneInformation(&tzone))
		tzone.Bias += tzone.DaylightBias;

	FileTimeToSystemTime((FILETIME *)(&(file->file.time)), &stime);
	Convert_Date(&stime, dat, -tzone.Bias);
}


/***********************************************************************
**
*/	void *OS_Open_Library(REBCHR *path, REBCNT *error)
/*
**		Load a DLL library and return the handle to it.
**		If zero is returned, error indicates the reason.
**
***********************************************************************/
{
	void *dll = LoadLibraryW(path);
	*error = GetLastError();

	return dll;
}


/***********************************************************************
**
*/	void OS_Close_Library(void *dll)
/*
**		Free a DLL library opened earlier.
**
***********************************************************************/
{
	FreeLibrary((HINSTANCE)dll);
}


/***********************************************************************
**
*/	void *OS_Find_Function(void *dll, char *funcname)
/*
**		Get a DLL function address from its string name.
**
***********************************************************************/
{
	void *fp = GetProcAddress((HMODULE)dll, funcname);
	//DWORD err = GetLastError();

	return fp;
}


/***********************************************************************
**
*/	REBINT OS_Create_Thread(CFUNC init, void *arg, REBCNT stack_size)
/*
**		Creates a new thread for a REBOL task datatype.
**
**	NOTE:
**		For this to work, the multithreaded library option is
**		needed in the C/C++ code generation settings.
**
**		The Task_Ready stops return until the new task has been
**		initialized (to avoid unknown new thread state).
**
***********************************************************************/
{
	REBINT thread;

	Task_Ready = CreateEvent(NULL, TRUE, FALSE, TEXT("REBOL_Task_Launch"));
	if (!Task_Ready) return -1;

	thread = _beginthread(init, stack_size, arg);

	if (thread) WaitForSingleObject(Task_Ready, 2000);
	CloseHandle(Task_Ready);

	return 1;
}


/***********************************************************************
**
*/	void OS_Delete_Thread(void)
/*
**		Can be called by a REBOL task to terminate its thread.
**
***********************************************************************/
{
	_endthread();
}


/***********************************************************************
**
*/	void OS_Task_Ready(REBINT tid)
/*
**		Used for new task startup to resume the thread that
**		launched the new task.
**
***********************************************************************/
{
	SetEvent(Task_Ready);
}


/***********************************************************************
**
*/	int OS_Create_Process(REBCHR *call, u32 flags)
/*
**		Return -1 on error.
**		For right now, set flags to 1 for /wait.
**
***********************************************************************/
{
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
//	REBOOL				is_NT;
//	OSVERSIONINFO		info;
	REBINT				result;

//	GetVersionEx(&info);
//	is_NT = info.dwPlatformId >= VER_PLATFORM_WIN32_NT;

	si.cb = sizeof(si);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.wShowWindow = SW_SHOWNORMAL;
	si.cbReserved2 = 0;
	si.lpReserved2 = NULL;

	si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

	result = CreateProcess(
		NULL,						// Executable name
		call,						// Command to execute
		NULL,						// Process security attributes
		NULL,						// Thread security attributes
		FALSE,						// Inherit handles
		NORMAL_PRIORITY_CLASS		// Creation flags
		| CREATE_DEFAULT_ERROR_MODE,
		NULL,						// Environment
		NULL,						// Current directory
		&si,						// Startup information
		&pi							// Process information
	);

	// Wait for termination:
	if (result && (flags & 1)) {
		result = 0;
		WaitForSingleObject(pi.hProcess, INFINITE); // check result??
		GetExitCodeProcess(pi.hProcess, (PDWORD)&result);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	return result;  // meaning depends on flags
}


/***********************************************************************
**
*/	int OS_Browse(REBCHR *url, int reserved)
/*
***********************************************************************/
{
	#define MAX_BRW_PATH 2044
	long flag;
	long len;
	long type;
	HKEY key;
	REBCHR *path;
	HWND hWnd = GetFocus();

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("http\\shell\\open\\command"), 0, KEY_READ, &key) != ERROR_SUCCESS)
		return 0;

	if (!url) url = TEXT("");

	path = MAKE_STR(MAX_BRW_PATH+4);
	len = MAX_BRW_PATH;

	flag = RegQueryValueEx(key, TEXT(""), 0, &type, (LPBYTE)path, &len);
	RegCloseKey(key);
	if (flag != ERROR_SUCCESS) {
		FREE_MEM(path);
		return 0;
	}
	//if (ExpandEnvironmentStrings(&str[0], result, len))

	Insert_Command_Arg(path, url, MAX_BRW_PATH);

	len = OS_Create_Process(path, 0);

	FREE_MEM(path);
	return len;
}


/***********************************************************************
**
*/	REBOOL OS_Request_File(REBRFR *fr)
/*
***********************************************************************/
{
	OPENFILENAME ofn = {0};
	BOOL ret;
	//int err;
	REBCHR *filters = TEXT("All files\0*.*\0REBOL scripts\0*.r\0Text files\0*.txt\0"	);

	ofn.lStructSize = sizeof(ofn);

	// ofn.hwndOwner = WIN_WIN(win); // Must find a way to set this

	ofn.lpstrTitle = fr->title;
	ofn.lpstrInitialDir = fr->dir;
	ofn.lpstrFile = fr->files;
	ofn.lpstrFilter = fr->filter ? fr->filter : filters;
	ofn.nMaxFile = fr->len;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;

	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_NOCHANGEDIR; //|OFN_NONETWORKBUTTON; //;

	if (GET_FLAG(fr->flags, FRF_MULTI)) ofn.Flags |= OFN_ALLOWMULTISELECT;

	if (GET_FLAG(fr->flags, FRF_SAVE))
		ret = GetSaveFileName(&ofn);
	else
		ret = GetOpenFileName(&ofn);

	//if (!ret)
	//	err = CommDlgExtendedError(); // CDERR_FINDRESFAILURE

	return ret;
}


/***********************************************************************
**
*/	REBSER *OS_GOB_To_Image(REBGOB *gob)
/*
**		Render a GOB into an image. Returns an image or zero if
**		it cannot be done.
**
***********************************************************************/
{

#ifndef REB_CORE

#ifndef NO_COMPOSITOR
	return (REBSER*)Gob_To_Image(gob);
#else
	return 0;
#endif

#else
	return 0;
#endif

}
