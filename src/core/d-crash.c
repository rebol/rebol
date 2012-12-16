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
**  Module:  d-crash.c
**  Summary: low level crash output
**  Section: debug
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#define	CRASH_BUF_SIZE 512	// space for crash print string

extern const REBYTE * const Crash_Msgs[];

enum Crash_Msg_Nums {
	// Must align with Crash_Msgs[] array.
	CM_ERROR,
	CM_BOOT,
	CM_INTERNAL,
	CM_ASSERT,
	CM_DATATYPE,
	CM_DEBUG,
	CM_CONTACT
};


/***********************************************************************
**
*/	void Crash(REBINT id, ...)
/*
**		Print a failure message and abort.
**
**		LATIN1 ONLY!! (For now)
**
**		The error is identified by id number, which can reference an
**		error message string in the boot strings block.
**
**		Note that lower level error messages should not attempt to
**		use the %r (mold value) format (uses higher level functions).
**
**		See panics.h for list of crash errors.
**
***********************************************************************/
{
	va_list args;
	REBYTE buf[CRASH_BUF_SIZE];
	REBYTE *msg;
	REBINT n = 0;

	va_start(args, id);

	DISABLE_GC;
	if (Reb_Opts->crash_dump) {
		Dump_Info();
		Dump_Stack(0, 0);
	}

	// "REBOL PANIC #nnn:"
	COPY_BYTES(buf, Crash_Msgs[CM_ERROR], CRASH_BUF_SIZE);
	APPEND_BYTES(buf, " #", CRASH_BUF_SIZE);
	Form_Int(buf + LEN_BYTES(buf), id);
	APPEND_BYTES(buf, ": ", CRASH_BUF_SIZE);

	// "REBOL PANIC #nnn: put error message here"
	// The first few error types only print general error message.
	// Those errors > RP_STR_BASE have specific error messages (from boot.r).
	if      (id < RP_BOOT_DATA) n = CM_DEBUG;
	else if (id < RP_INTERNAL) n = CM_BOOT;
	else if (id < RP_ASSERTS)  n = CM_INTERNAL;
	else if (id < RP_DATATYPE) n = CM_ASSERT;
	else if (id < RP_STR_BASE) n = CM_DATATYPE;
	else if (id > RP_STR_BASE + RS_MAX - RS_ERROR) n = CM_DEBUG;

	// Use the above string or the boot string for the error (in boot.r):
	msg = (REBYTE*)(n >= 0 ? Crash_Msgs[n] : BOOT_STR(RS_ERROR, id - RP_STR_BASE - 1));
	Form_Var_Args(buf + LEN_BYTES(buf), CRASH_BUF_SIZE - 1 - LEN_BYTES(buf), msg, args);

	APPEND_BYTES(buf, Crash_Msgs[CM_CONTACT], CRASH_BUF_SIZE);

	// Convert to OS-specific char-type:
#ifdef disable_for_now //OS_WIDE_CHAR   /// win98 does not support it
	{
		REBCHR s1[512];
		REBCHR s2[2000];

		n = TO_OS_STR(s1, Crash_Msgs[CM_ERROR], LEN_BYTES(Crash_Msgs[CM_ERROR]));
		if (n > 0) s1[n] = 0; // terminate
		else OS_EXIT(200); // bad conversion

		n = TO_OS_STR(s2, buf, LEN_BYTES(buf));
		if (n > 0) s2[n] = 0;
		else OS_EXIT(200);

		OS_CRASH(s1, s2);
	}
#else
	OS_CRASH(Crash_Msgs[CM_ERROR], buf);
#endif
}

/***********************************************************************
**
*/	void NA(void)
/*
**		Feature not available.
**
***********************************************************************/
{
	Crash(RP_NA);
}
