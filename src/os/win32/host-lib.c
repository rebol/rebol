/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
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

#if !defined(REBOL_OPTIONS_FILE)
#include "opt-config.h"
#else
#include REBOL_OPTIONS_FILE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>   // used for OS_Request_Password
#include <windows.h>
#include <process.h>
#include <ShlObj.h>  // used for OS_Request_Dir

#include "reb-host.h"
#include "host-lib.h"

RL_LIB *RL; // Link back to reb-lib from embedded extensions (like for now: host-window, host-ext-test..)

// Semaphore lock to sync sub-task launch:
static void *Task_Ready;

#ifdef REB_VIEW
void Dispose_Windows(void);
#endif


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
		hold[HOLD_SIZE] = 0;

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

#ifdef removing_this_code
// this function is not needed. Now is possible to use RL_GET_STRING with WIDE flag
/***********************************************************************
**
*/	REBOOL As_OS_Str(REBSER *series, REBCHR **string)
/*
**	If necessary, convert a string series to Win32 wide-chars.
**  (Handy for GOB/TEXT handling).
**  If the string series is empty the resulting string is set to NULL
**
**  Function returns:
**      TRUE - if the resulting string needs to be deallocated by the caller code
**      FALSE - if REBOL string is used (no dealloc needed)
**
**  Note: REBOL strings are allowed to contain nulls.
**
***********************************************************************/
{
	int len, n;
	void *str;
	wchar_t *wstr;

	if ((len = RL_Get_String(series, 0, &str)) < 0) {
		// Latin1 byte string - convert to wide chars
		len = -len;
		wstr = OS_Make((len + 1) * sizeof(wchar_t));
		for (n = 0; n < len; n++)
			wstr[n] = (wchar_t)((unsigned char*)str)[n];
		wstr[len] = 0;
		//note: following string needs be deallocated in the code that uses this function
		*string = (REBCHR*)wstr;
		return TRUE;
	}
	*string = (len == 0) ? NULL : str; //empty string check
	return FALSE;
}
#endif

/***********************************************************************
**
**	OS Library Functions
**
***********************************************************************/

/***********************************************************************
**
*/	REBINT OS_Get_PID()
/*
**		Return the current process ID
**
***********************************************************************/
{
	return GetCurrentProcessId();
}

/***********************************************************************
**
*/	REBINT OS_Get_UID()
/*
**		Return the real user ID
**
***********************************************************************/
{
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Set_UID(REBINT uid)
/*
**		Set the user ID, see setuid manual for its semantics
**
***********************************************************************/
{
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Get_GID()
/*
**		Return the real group ID
**
***********************************************************************/
{
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Set_GID(REBINT gid)
/*
**		Set the group ID, see setgid manual for its semantics
**
***********************************************************************/
{
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Get_EUID()
/*
**		Return the effective user ID
**
***********************************************************************/
{
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Set_EUID(REBINT uid)
/*
**		Set the effective user ID
**
***********************************************************************/
{
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Get_EGID()
/*
**		Return the effective group ID
**
***********************************************************************/
{
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Set_EGID(REBINT gid)
/*
**		Set the effective group ID
**
***********************************************************************/
{
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Send_Signal(REBINT pid, REBINT signal)
/*
**		Send signal to a process
**
***********************************************************************/
{
	if (signal == 9 || signal == 15) { //SIGKILL || SIGTERM
		return OS_Kill(pid);
	}
	return OS_ENA;
}

/***********************************************************************
**
*/	REBINT OS_Kill(REBINT pid)
/*
**		Try to kill the process
**
***********************************************************************/
{
	REBINT err = 0;
	HANDLE ph = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (ph == NULL) {
		err = GetLastError();
		switch (err) {
			case ERROR_ACCESS_DENIED:
				return OS_EPERM;
			case ERROR_INVALID_PARAMETER:
				return OS_ESRCH;
			default:
				return OS_ESRCH;
		}
	}
	if (TerminateProcess(ph, 0)) {
		CloseHandle(ph);
		return 0;
	}
	err = GetLastError();
	CloseHandle(ph);
	switch (err) {
		case ERROR_INVALID_HANDLE:
			return OS_EINVAL;
		default:
			return -err;
	}
}

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
*/	REB_NORETURN void OS_Exit(int code)
/*
**		Called in all cases when REBOL quits
**
**		If there would be case when freeing resources is not wanted,
**		it should be signalised by a new argument.
**
***********************************************************************/
{
	//OS_Call_Device(RDI_STDIO, RDC_CLOSE); // close echo
	
	OS_Quit_Devices(0);
#ifdef INCLUDE_IMAGE_OS_CODEC
	OS_Release_Codecs();
#endif
#ifdef REB_VIEW
	//Dispose_Graphics();
	Dispose_Windows();
#endif
	RL_Dispose();
	exit(code);
}


/***********************************************************************
**
*/	REB_NORETURN void OS_Crash(const REBYTE *title, const REBYTE *content)
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
		MessageBoxA(NULL, (LPCSTR)content, (LPCSTR)title, MB_ICONHAND);
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
		str[len] = 0;
		len = (int)LEN_STR(str);
		if (str[len-2] == '\r' && str[len-1] == '\n') str[len-2] = 0; // trim CRLF
		LocalFree(lpMsgBuf);
	}
	return str;
}


/***********************************************************************
**
*/	REBOOL OS_Get_Boot_Path(REBCHR **name)
/*
**		Used to determine the program file path for REBOL.
**		This is the path stored in system->options->boot and
**		it is used for finding default boot files.
**
***********************************************************************/
{
	*name = MAKE_STR(MAX_FILE_NAME);
	return (GetModuleFileName(0, *name, MAX_FILE_NAME) > 0);
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
**		Returns NULL on error.
**
***********************************************************************/
{
	REBCHR *env = GetEnvironmentStrings();
	REBCNT n, len = 0;
	REBCHR *str;

	str = env;
	while ((n = (REBCNT)LEN_STR(str))) {
		len += n + 1;
		str = env + len; // next
	}
	len++;

	str = OS_Make(len * sizeof(REBCHR)); // Must be released by caller!
	if(!str) return NULL;

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
		OS_Crash(cb_cast("Missing resource"), cb_cast("High performance timer"));

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
*/	int OS_Set_Current_Dir(REBCHR *path)
/*
**		Set the current directory to local path.
**		Return 0 on success else error number.
**
***********************************************************************/
{
	if (SetCurrentDirectory(path[0] == 0 ? L"\\" : path)) {
		// directory changed... update PWD
		// https://github.com/Oldes/Rebol-issues/issues/2448
		SetEnvironmentVariable(L"PWD", path);
		return 0;
	} else
		return GetLastError();
}

/***********************************************************************
**
*/	REBCHR* OS_Real_Path(const REBCHR *path)
/*
**		Returns a null-terminated string containing the canonicalized
**		absolute pathname corresponding to path. In the returned string,
**		symbolic links are resolved, as are . and .. pathname components.
**		Consecutive slash (/) characters are replaced by a single slash.
**
**		The result should be freed after copy/conversion.
**
***********************************************************************/
{
	return _wfullpath(NULL, path, MAX_PATH);
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
*/	void *OS_Find_Function(void *dll, const char *funcname)
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
*/	int OS_Create_Process(REBCHR *call, int argc, REBCHR* argv[], u32 flags, u64 *pid, int *exit_code, u32 input_type, void *input, u32 input_len, u32 output_type, void **output, u32 *output_len, u32 err_type, void **err, u32 *err_len)
/*
** flags:
**      1: wait, is implied when I/O redirection is enabled
**      2: console
**      4: shell
**      8: info
**      16: show
** input_type/output_type/err_type:
**      0: none
**      1: string
**      2: file
**
**      Return -1 on error, otherwise the process return code.
**
**  NOTE:
**		Taken from Atronix version, written mostly by Shixin Zeng
**
***********************************************************************/
{
#define INHERIT_TYPE 0
#define NONE_TYPE 1
#define STRING_TYPE 2
#define FILE_TYPE 3
#define BINARY_TYPE 4

#define FLAG_WAIT 1
#define FLAG_CONSOLE 2
#define FLAG_SHELL 4
#define FLAG_INFO 8

	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
//	REBOOL				is_NT;
//	OSVERSIONINFO		info;
	REBINT				result = -1;
	REBINT				ret = 0;
	HANDLE hOutputRead = 0, hOutputWrite = 0;
	HANDLE hInputWrite = 0, hInputRead = 0;
	HANDLE hErrorWrite = 0, hErrorRead = 0;
	REBCHR *cmd = NULL;
	char *oem_input = NULL;
	void *tmp;

	SECURITY_ATTRIBUTES sa;

	unsigned char flag_wait = FALSE;
	unsigned char flag_console = FALSE;
	unsigned char flag_shell = FALSE;
	unsigned char flag_info = FALSE;

	if (flags & FLAG_WAIT) flag_wait = TRUE;
	if (flags & FLAG_CONSOLE) flag_console = TRUE;
	if (flags & FLAG_SHELL) flag_shell = TRUE;
	if (flags & FLAG_INFO) flag_info = TRUE;

	// Set up the security attributes struct.
	sa.nLength= sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	si.cb = sizeof(si);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.wShowWindow = SW_SHOWNORMAL;
	si.cbReserved2 = 0;
	si.lpReserved2 = NULL;

//	GetVersionEx(&info);
//	is_NT = info.dwPlatformId >= VER_PLATFORM_WIN32_NT;

	/* initialize output/error */
	if (output_type != NONE_TYPE
		&& output_type != INHERIT_TYPE
		&& (output == NULL
			|| output_len == NULL)) {
		return -1;
	}
	if (output != NULL) *output = NULL;
	if (output_len != NULL) *output_len = 0;

	if (err_type != NONE_TYPE
		&& err_type != INHERIT_TYPE
		&& (err == NULL
			|| err_len == NULL)) {
		return -1;
	}
	if (err != NULL) *err = NULL;
	if (err_len != NULL) *err_len = 0;

	switch (input_type) {
		case STRING_TYPE:
		case BINARY_TYPE:
			if (!CreatePipe(&hInputRead, &hInputWrite, NULL, 0)) {
				goto input_error;
			}
			/* make child side handle inheritable */
			if (!SetHandleInformation(hInputRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
				goto input_error;
			}
			si.hStdInput = hInputRead;
			break;
		case FILE_TYPE:
			hInputRead = CreateFile(input,
				GENERIC_READ, /* desired mode*/
				0, /* shared mode*/
				&sa, /* security attributes */
				OPEN_EXISTING, /* Creation disposition */
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, /* flag and attributes */
				NULL /* template */);
			si.hStdInput = hInputRead;
			break;
		case NONE_TYPE:
			si.hStdInput = 0;
			break;
		default:
			si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
			break;
	}

	switch (output_type) {
		case STRING_TYPE:
		case BINARY_TYPE:
			if (!CreatePipe(&hOutputRead, &hOutputWrite, NULL, 0)) {
				goto output_error;
			}
			/* make child side handle inheritable */
			if (!SetHandleInformation(hOutputWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
				goto output_error;
			}
			si.hStdOutput = hOutputWrite;
			break;
		case FILE_TYPE:
			si.hStdOutput = CreateFile(*(LPCTSTR*)output,
				GENERIC_WRITE, /* desired mode*/
				0, /* shared mode*/
				&sa, /* security attributes */
				CREATE_NEW, /* Creation disposition */
				FILE_ATTRIBUTE_NORMAL, /* flag and attributes */
				NULL /* template */);

			if (si.hStdOutput == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_EXISTS) {
				si.hStdOutput = CreateFile(*(LPCTSTR*)output,
					GENERIC_WRITE, /* desired mode*/
					0, /* shared mode*/
					&sa, /* security attributes */
					OPEN_EXISTING, /* Creation disposition */
					FILE_ATTRIBUTE_NORMAL, /* flag and attributes */
					NULL /* template */);
			}
			break;
		case NONE_TYPE:
			si.hStdOutput = 0;
			break;
		default:
			si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
			break;
	}

	switch (err_type) {
		case STRING_TYPE:
		case BINARY_TYPE:
			if (!CreatePipe(&hErrorRead, &hErrorWrite, NULL, 0)) {
				goto error_error;
			}
			/* make child side handle inheritable */
			if (!SetHandleInformation(hErrorWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
				goto error_error;
			}
			si.hStdError = hErrorWrite;
			break;
		case FILE_TYPE:
			si.hStdError = CreateFile(*(LPCTSTR*)err,
				GENERIC_WRITE, /* desired mode*/
				0, /* shared mode*/
				&sa, /* security attributes */
				CREATE_NEW, /* Creation disposition */
				FILE_ATTRIBUTE_NORMAL, /* flag and attributes */
				NULL /* template */);

			if (si.hStdError == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_EXISTS) {
				si.hStdError = CreateFile(*(LPCTSTR*)err,
					GENERIC_WRITE, /* desired mode*/
					0, /* shared mode*/
					&sa, /* security attributes */
					OPEN_EXISTING, /* Creation disposition */
					FILE_ATTRIBUTE_NORMAL, /* flag and attributes */
					NULL /* template */);
			}
			break;
		case NONE_TYPE:
			si.hStdError = 0;
			break;
		default:
			si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
			break;
	}

	if (call == NULL) {
		/* command in argv */
		goto cleanup; /* NOT IMPLEMENTED*/
	} else {
		if (flag_shell) {
			// command to cmd.exe needs to be surrounded by quotes to preserve the inner quotes
			const wchar_t *sh = L"cmd.exe /C \"";
			size_t len = wcslen(sh) + wcslen(call) + 3;

			cmd = cast(wchar_t*, malloc(len * sizeof(wchar_t)));
			cmd[0] = L'\0';
			wcscat(cmd, sh);
			wcscat(cmd, call);
			wcscat(cmd, L"\"");
		} else {
			cmd = _wcsdup(call); /* CreateProcess might write to this memory, so duplicate it to be safe */
		}
	}

	result = CreateProcess(
		NULL,						// Executable name
		cmd,						// Command to execute
		NULL,						// Process security attributes
		NULL,						// Thread security attributes
		TRUE,						// Inherit handles, must be TRUE for I/O redirection
		NORMAL_PRIORITY_CLASS		// Creation flags
		| CREATE_DEFAULT_ERROR_MODE,
		NULL,						// Environment
		NULL,						// Current directory
		&si,						// Startup information
		&pi							// Process information
	);

	OS_Free(cmd);

	if (pid != NULL) *pid = pi.dwProcessId;

	if (hInputRead != NULL)
		CloseHandle(hInputRead);

	if (hOutputWrite != NULL)
		CloseHandle(hOutputWrite);

	if (hErrorWrite != NULL)
		CloseHandle(hErrorWrite);

	// Wait for termination:
	if (result && flag_wait) {
		HANDLE handles[3];
		int count = 0;
		DWORD wait_result = 0;
		DWORD output_size = 0;
		DWORD err_size = 0;

#define BUF_SIZE_CHUNK 4096

		if (hInputWrite != NULL && input_len > 0) {
			if (input_type == STRING_TYPE) {
				DWORD dest_len = 0;
				/* convert input encoding from UNICODE to OEM */
				dest_len = WideCharToMultiByte(CP_OEMCP, 0, input, input_len, oem_input, dest_len, NULL, NULL);
				if (dest_len > 0) {
					oem_input = OS_Make(dest_len);
					if (oem_input != NULL) {
						WideCharToMultiByte(CP_OEMCP, 0, input, input_len, oem_input, dest_len, NULL, NULL);
						input_len = dest_len;
						input = oem_input;
						handles[count ++] = hInputWrite;
					}
				}
			} else { /* BINARY_TYPE */
				handles[count ++] = hInputWrite;
			}
		}
		if (hOutputRead != NULL) {
			output_size = BUF_SIZE_CHUNK;
			*output_len = 0;
			*output = OS_Make(output_size);
			handles[count ++] = hOutputRead;
		}
		if (hErrorRead != NULL) {
			err_size = BUF_SIZE_CHUNK;
			*err_len = 0;
			*err = OS_Make(err_size);
			handles[count++] = hErrorRead;
		}

		while (count > 0) {
			wait_result = WaitForMultipleObjects(count, handles, FALSE, INFINITE);
			if (wait_result >= WAIT_OBJECT_0
				&& wait_result < WAIT_OBJECT_0 + count) {
				int i = wait_result - WAIT_OBJECT_0;
				DWORD input_pos = 0;
				DWORD n = 0;

				if (handles[i] == hInputWrite) {
					if (!WriteFile(hInputWrite, (char*)input + input_pos, input_len - input_pos, &n, NULL)) {
						if (i < count - 1) {
							memmove(&handles[i], &handles[i + 1], (count - i - 1) * sizeof(HANDLE));
						}
						count--;
					} else {
						input_pos += n;
						if (input_pos >= input_len) {
							/* done with input */
							CloseHandle(hInputWrite);
							hInputWrite = NULL;
							OS_Free(oem_input);
							oem_input = NULL;
							if (i < count - 1) {
								memmove(&handles[i], &handles[i + 1], (count - i - 1) * sizeof(HANDLE));
							}
							count--;
						}
					}
				} else if (handles[i] == hOutputRead) {
					if (!ReadFile(hOutputRead, *(char**)output + *output_len, output_size - *output_len, &n, NULL)) {
						if (i < count - 1) {
							memmove(&handles[i], &handles[i + 1], (count - i - 1) * sizeof(HANDLE));
						}
						count--;
					} else {
						*output_len += n;
						if (*output_len >= output_size) {
							output_size += BUF_SIZE_CHUNK;
							tmp = realloc(*output, output_size);
							if (tmp == NULL) goto kill;
							*output = tmp;
						}
					}
				} else if (handles[i] == hErrorRead) {
					if (!ReadFile(hErrorRead, *(char**)err + *err_len, err_size - *err_len, &n, NULL)) {
						if (i < count - 1) {
							memmove(&handles[i], &handles[i + 1], (count - i - 1) * sizeof(HANDLE));
						}
						count--;
					} else {
						*err_len += n;
						if (*err_len >= err_size) {
							err_size += BUF_SIZE_CHUNK;
							tmp = realloc(*err, err_size);
							if (tmp == NULL) goto kill;
							*err = tmp;
						}
					}
				} else {
					//RL_Print("Error READ");
					if (!ret) ret = GetLastError();
					goto kill;
				}
			} else if (wait_result == WAIT_FAILED) { /* */
				//RL_Print("Wait Failed\n");
				if (!ret) ret = GetLastError();
				goto kill;
			} else {
				//RL_Print("Wait returns unexpected result: %d\n", wait_result);
				if (!ret) ret = GetLastError();
				goto kill;
			}
		}

		WaitForSingleObject(pi.hProcess, INFINITE); // check result??
		if (exit_code != NULL) {
			GetExitCodeProcess(pi.hProcess, (PDWORD)exit_code);
		}
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		if (output_type == STRING_TYPE && *output != NULL && *output_len > 0) {
			/* convert to wide char string */
			int dest_len = 0;
			wchar_t *dest = NULL;
			dest_len = MultiByteToWideChar(CP_OEMCP, 0, *output, *output_len, dest, 0);
			if (dest_len <= 0) {
				OS_Free(*output);
				*output = NULL;
				*output_len = 0;
			}
			dest = OS_Make(*output_len * sizeof(wchar_t));
			if (dest == NULL) goto cleanup;
			MultiByteToWideChar(CP_OEMCP, 0, *output, *output_len, dest, dest_len);
			OS_Free(*output);
			*output = dest;
			*output_len = dest_len;
		}

		if (err_type == STRING_TYPE && *err != NULL && *err_len > 0) {
			/* convert to wide char string */
			int dest_len = 0;
			wchar_t *dest = NULL;
			dest_len = MultiByteToWideChar(CP_OEMCP, 0, *err, *err_len, dest, 0);
			if (dest_len <= 0) {
				OS_Free(*err);
				*err = NULL;
				*err_len = 0;
			}
			dest = OS_Make(*err_len * sizeof(wchar_t));
			if (dest == NULL) goto cleanup;
			MultiByteToWideChar(CP_OEMCP, 0, *err, *err_len, dest, dest_len);
			OS_Free(*err);
			*err = dest;
			*err_len = dest_len;
		}
	} else if (result) {
		/* no wait */
		/* Close handles to avoid leaks */
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	} else {
		/* CreateProcess failed */
		ret = GetLastError();
	}

	goto cleanup;

kill:
	if (TerminateProcess(pi.hProcess, 0)) {
		WaitForSingleObject(pi.hProcess, INFINITE);
		if (exit_code != NULL) {
			GetExitCodeProcess(pi.hProcess, (PDWORD)exit_code);
		}
	} else if (ret == 0) {
		ret = GetLastError();
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

cleanup:
	if (oem_input != NULL) {
		OS_Free(oem_input);
	}

	if (output != NULL && *output != NULL && *output_len == 0) {
		OS_Free(*output);
	}

	if (err != NULL && *err != NULL && *err_len == 0) {
		OS_Free(*err);
	}

	if (hInputWrite != NULL)
		CloseHandle(hInputWrite);

	if (hOutputRead != NULL)
		CloseHandle(hOutputRead);

	if (hErrorRead != NULL)
		CloseHandle(hErrorRead);

	if (err_type == FILE_TYPE) {
		CloseHandle(si.hStdError);
	}

error_error:
	if (output_type == FILE_TYPE) {
		CloseHandle(si.hStdOutput);
	}

output_error:
	if (input_type == FILE_TYPE && si.hStdInput != NULL) {
		CloseHandle(si.hStdInput);
	}

input_error:
	return ret;  // meaning depends on flags
}

/***********************************************************************
**
*/	int OS_Reap_Process(int pid, int *status, int flags)
/*
 * pid: 
 * 		> 0, a signle process
 * 		-1, any child process
 * flags:
 * 		0: return immediately
 * 		1: wait until one of child processes exits
 *
**		Return -1 on error, otherwise process ID
***********************************************************************/
{
	/* It seems that process doesn't need to be reaped on Windows */
	return 0;
}

/***********************************************************************
**
*/	int OS_Browse(REBCHR *url, int reserved)
/*
**		Return FALSE on error else TRUE (like on Posix)
**
***********************************************************************/
{
	#define MAX_BRW_PATH 2044
	long flag;
	DWORD len;
	DWORD type;
	HKEY key;
	REBCHR *path;
	HWND hWnd = GetFocus();
	int exit_code = 0;

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("http\\shell\\open\\command"), 0, KEY_READ, &key) != ERROR_SUCCESS)
		return FALSE;

	if (!url) url = TEXT("");

	path = MAKE_STR(MAX_BRW_PATH+4);
	len = MAX_BRW_PATH;

	flag = RegQueryValueEx(key, TEXT(""), 0, &type, (LPBYTE)path, &len);
	RegCloseKey(key);
	if (flag != ERROR_SUCCESS) {
		FREE_MEM(path);
		return FALSE;
	}
	//if (ExpandEnvironmentStrings(&str[0], result, len))

	Insert_Command_Arg(path, url, MAX_BRW_PATH);

	REBCHR* const argv[] = {path, NULL};
	len = OS_Create_Process(path, 1, (REBCHR**)argv, 0, 
							NULL, /* pid */
							&exit_code,
							INHERIT_TYPE, NULL, 0, /* input_type, void *input, u32 input_len, */
							INHERIT_TYPE, NULL, NULL, /* output_type, void **output, u32 *output_len, */
							INHERIT_TYPE, NULL, NULL); /* u32 err_type, void **err, u32 *err_len */

	FREE_MEM(path);
	return (len < 0) ? FALSE : TRUE;
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
	REBCHR *filters = TEXT("All files\0*.*\0REBOL scripts\0*.reb\0Text files\0*.txt\0"	);

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

static LPITEMIDLIST lpLastBrowseFolder = NULL;
static BOOL bBrowseFolderInit = FALSE;
static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	// Using this callback to set default folder.
	// The SendMessage in BFFM_SELCHANGED is there to update focus on the directory
	// Without it it would select the folder on init, but the folder could not be visible.
	REBOOL set = FALSE;

	switch (uMsg) {
	case BFFM_INITIALIZED:
		bBrowseFolderInit = TRUE;
		set = TRUE;
		break;
	case BFFM_SELCHANGED:
		if (bBrowseFolderInit) {
			bBrowseFolderInit = FALSE;
			set = TRUE;
		}
		break;
	}
	if (lpData && set) {
		SendMessage(hwnd, BFFM_SETSELECTION, lpLastBrowseFolder != (LPITEMIDLIST)lpData, lpData);
	}
	return 0;
}
/***********************************************************************
**
*/	REBOOL OS_Request_Dir(REBRFR *fr)
/*
***********************************************************************/
{
	BROWSEINFO bInfo = {0};
	REBOOL keep = GET_FLAG(fr->flags, FRF_KEEP);
	REBCHR *dir = fr->dir;

	//bInfo.hwndOwner = Owner window // Must find a way to set this
	bInfo.pidlRoot = NULL; 
	bInfo.pszDisplayName = fr->files; // Address of a buffer to receive the name of the folder selected by the user
	bInfo.lpszTitle = fr->title;      // Title of the dialog
	bInfo.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
	bInfo.lpfn = BrowseCallbackProc;
	bInfo.iImage = -1;
	// start in dir location is used /dir
	if (dir) {
		bInfo.lParam = (LPARAM)dir;
	}
	// else use last keeped result if used /keep
	else if (keep) {
		bInfo.lParam = (LPARAM)lpLastBrowseFolder;
	}

	LPITEMIDLIST lpItem = SHBrowseForFolder( &bInfo);
	if(lpItem == NULL) {
		// nothing selected
		return FALSE;
	}
	if (keep) {
		// release last result if there was any
		if(lpLastBrowseFolder)
			CoTaskMemFree((LPVOID)lpLastBrowseFolder);
		// and store result for next request
		lpLastBrowseFolder = lpItem;
	}
	SHGetPathFromIDList(lpItem, fr->files);
	return TRUE;
}


/***********************************************************************
**
*/	void OS_Request_Password(REBREQ *req)
/*
***********************************************************************/
{
	REBCNT size = 64;
	REBCNT  pos = 0;
	REBCHR *str = (REBUNI*)malloc(size * sizeof(REBCHR));
	REBCHR  c;

	req->data = NULL;

	while ((c = _getwch()) != '\r') {
		if (c ==  27) { // ESC
			free(str);
			return; 
		}
		if (c == '\b') { // backspace
			if (pos > 0) pos--;
			continue;
		}
		str[pos++] = c;
		if (pos+1 == size) {
			size += 64;
			str = (REBCHR *)realloc(str, size * sizeof(REBCHR));
		}
	}
	req->data = (REBYTE*)str;
	req->actual = pos;
	str[pos++] = 0;
}

