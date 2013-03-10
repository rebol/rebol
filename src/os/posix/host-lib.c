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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#ifndef timeval // for older systems
#include <sys/time.h>
#endif

#include "reb-host.h"
#include "host-lib.h"

#ifndef NO_DL_LIB
#include <dlfcn.h>
#endif

// Semaphore lock to sync sub-task launch:
static void *Task_Ready;

#ifndef PATH_MAX
#define PATH_MAX 4096  // generally lacking in Posix
#endif



/***********************************************************************
**
*/	static int Get_Timezone(struct tm *local_tm)
/*
**		Get the time zone in minutes from GMT.
**		NOT consistently supported in Posix OSes!
**		We have to use a few different methods.
**
***********************************************************************/
{
#ifdef HAS_SMART_TIMEZONE
	time_t rightnow;
	time(&rightnow);
	return (int)difftime(mktime(localtime(&rightnow)), mktime(gmtime(&rightnow))) / 60;
#else
	struct tm tm2;
	time_t rightnow;
	time(&rightnow);
	tm2 = *localtime(&rightnow);
	tm2.tm_isdst=0;
	return (int)difftime(mktime(&tm2), mktime(gmtime(&rightnow))) / 60;
#endif
//	 return local_tm->tm_gmtoff / 60;  // makes the most sense, but no longer used
}


/***********************************************************************
**
*/	void Convert_Date(time_t *stime, REBOL_DAT *dat, long zone)
/*
**		Convert local format of system time into standard date
**		and time structure (for date/time and file timestamps).
**
***********************************************************************/
{
	struct tm *time;

	CLEARS(dat);

	time = gmtime(stime);

	dat->year  = time->tm_year + 1900;
	dat->month = time->tm_mon + 1;
	dat->day   = time->tm_mday;
	dat->time  = time->tm_hour * 3600 + time->tm_min * 60 + time->tm_sec;
	dat->nano  = 0;
	dat->zone  = Get_Timezone(time);
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
		fputs(title, stderr);
		fputs(":\n", stderr);
	}
	fputs(content, stderr);
	fputs("\n\n", stderr);
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
	strerror_r(errnum, str, len);
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
	return FALSE; // not yet used
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
	return 0; // not yet used
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
	// Note: The Posix variant of this API is case-sensitive

	REBINT len;
	const REBCHR* value = getenv(envname);
	if (value == 0) return 0;

	len = LEN_STR(value);
	if (len == 0) return -1; // shouldn't have saved an empty env string

	if (len + 1 > valsize) {
		return len + 1;
	}

	COPY_STR(envval, value, len);
	return len;
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
	if (envval) {
#ifdef setenv
		// we pass 1 for overwrite (make call to OS_Get_Env if you 
		// want to check if already exists)

		if (setenv(envname, envval, 1) == -1)
			return FALSE;
#else
		// WARNING: KNOWN MEMORY LEAK!

		// putenv is *fatally flawed*, and was obsoleted by setenv
		// and unsetenv System V...

		// http://stackoverflow.com/a/5876818/211160

		// once you have passed a string to it you never know when that
		// string will no longer be needed.  Thus it may either not be
		// dynamic or you must leak it, or track a local copy of the 
		// environment yourself.

		// If you're stuck without setenv on some old platform, but
		// really need to set an environment variable, here's a way
		// that just leaks a string each time you call.  

		char* expr = MAKE_STR(LEN_STR(envname) + 1 + LEN_STR(envval) + 1);

		strcpy(expr, envname);
		strcat(expr, "=");
		strcat(expr, envval);

		if (putenv(expr) == -1)
			return FALSE;
#endif
		return TRUE;
	}

#ifdef unsetenv
	if (unsetenv(envname) == -1)
		return FALSE;
#else
	// WARNING: KNOWN PORTABILITY ISSUE

	// Simply saying putenv("FOO") will delete FOO from
	// the environment, but it's not consistent...does
	// nothing on NetBSD for instance.  But not all
	// other systems have unsetenv...
	//
	// http://julipedia.meroh.net/2004/10/portability-unsetenvfoo-vs-putenvfoo.html 

	// going to hope this case doesn't hold onto the string...
	if (putenv((char*)envname) == -1)
		return FALSE;
#endif
	return TRUE;
}


/***********************************************************************
**
*/	REBCHR *OS_List_Env(void)
/*
***********************************************************************/
{
	extern char **environ;
	int n, len = 0;
	char *str, *cp;

	// compute total size:
	for (n = 0; environ[n]; n++) len += 1 + LEN_STR(environ[n]);

	cp = str = OS_Make(len + 1); // +terminator
	*cp = 0;

	// combine all strings into one:
	for (n = 0; environ[n]; n++) {
		len = LEN_STR(environ[n]);
		strcat(cp, environ[n]);
		cp += len;
		*cp++ = 0;
		*cp = 0;
	}

	return str; // caller will free it
}


/***********************************************************************
**
*/	void OS_Get_Time(REBOL_DAT *dat)
/*
**		Get the current system date/time in UTC plus zone offset (mins).
**
***********************************************************************/
{
	struct timeval tv;
	time_t stime;

	gettimeofday(&tv, 0); // (tz field obsolete)
	stime = tv.tv_sec;
	Convert_Date(&stime, dat, -1);
	dat->nano  = tv.tv_usec * 1000;
}


/***********************************************************************
**
*/	i64 OS_Delta_Time(i64 base, int flags)
/*
**		Return time difference in microseconds. If base = 0, then
**		return the counter. If base != 0, compute the time difference.
**
**		NOTE: This needs to be precise, but many OSes do not
**		provide a precise time sampling method. So, if the target
**		posix OS does, add the ifdef code in here.
**
***********************************************************************/
{
	struct timeval tv;
	i64 time;

	gettimeofday(&tv,0);

	time = ((i64)tv.tv_sec * 1000000) + tv.tv_usec;

	if (base == 0) return time;

	return time - base;
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
	*path = MAKE_STR(PATH_MAX);
	if (!getcwd(*path, PATH_MAX-1)) *path[0] = 0;
	return LEN_STR(*path); // Be sure to call free() after usage
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
	return chdir(path) == 0;
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
	Convert_Date((time_t *)&(file->file.time.l), dat, 0);
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
#ifndef NO_DL_LIB
	void *dll = dlopen(path, RTLD_LAZY/*|RTLD_GLOBAL*/);
	*error = 0; // dlerror() returns a char* error message, so there's
				// no immediate way to return an "error code" in *error
	return dll;
#else
	return 0;
#endif
}


/***********************************************************************
**
*/	void OS_Close_Library(void *dll)
/*
**		Free a DLL library opened earlier.
**
***********************************************************************/
{
#ifndef NO_DL_LIB
	dlclose(dll);
#endif
}


/***********************************************************************
**
*/	void *OS_Find_Function(void *dll, char *funcname)
/*
**		Get a DLL function address from its string name.
**
***********************************************************************/
{
#ifndef NO_DL_LIB
	void *fp = dlsym(dll, funcname);
	return fp;
#else
	return 0;
#endif
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
/*
	Task_Ready = CreateEvent(NULL, TRUE, FALSE, "REBOL_Task_Launch");
	if (!Task_Ready) return -1;

	thread = _beginthread(init, stack_size, arg);

	if (thread) WaitForSingleObject(Task_Ready, 2000);
	CloseHandle(Task_Ready);
*/
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
	//_endthread();
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
	//SetEvent(Task_Ready);
}


/***********************************************************************
**
*/	int OS_Create_Process(REBCHR *call, u32 flags)
/*
**		Return -1 on error, otherwise the process return code.
**
***********************************************************************/
{
	return system(call); // returns -1 on system call error
}

static int Try_Browser(char *browser, REBCHR *url)
{
	pid_t pid;
	int result, status;

	switch (pid = fork()) {
		case -1:
			result = FALSE;
			break;
		case 0:
			execlp(browser, browser, url, NULL);
			exit(1);
			break;
		default:
			waitpid(pid, &status, WUNTRACED);
			result = WIFEXITED(status)
					&& (WEXITSTATUS(status) == 0);
	}

	return result;
}

/***********************************************************************
**
*/	int OS_Browse(REBCHR *url, int reserved)
/*
***********************************************************************/
{
	if (
#if defined(TO_OSX) || defined(TO_OSXI)
		Try_Browser("/usr/bin/open", url)
#else
		Try_Browser("xdg-open", url)
		|| Try_Browser("x-www-browser", url)
#endif
	) return TRUE;
	return FALSE;
}


/***********************************************************************
**
*/	REBOOL OS_Request_File(REBRFR *fr)
/*
***********************************************************************/
{
	return FALSE;
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
	return 0;
}
