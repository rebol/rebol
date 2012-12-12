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
**  Title: Simple helper functions for host-side standard I/O
**  Author: Carl Sassenrath
**  Caution: OS independent
**  Purpose:
**      Interfaces to the stdio device for standard I/O on the host.
**      All stdio within REBOL uses UTF-8 encoding so the functions
**      shown here operate on UTF-8 bytes, regardless of the OS.
**      The conversion to wide-chars for OSes like Win32 is done in
**      the StdIO Device code.
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
#include <string.h>
#include "reb-host.h"
#include "host-lib.h"

void Host_Crash(REBYTE *reason);

// Temporary globals: (either move or remove?!)
REBREQ Std_IO_Req;
static REBYTE *inbuf;
static REBCNT inbuf_len = 32*1024;

static REBYTE *Get_Next_Line()
{
	REBYTE *bp = inbuf;
	REBYTE *out;
	REBCNT len;

	// Scan for line terminator or end:
	for (bp = inbuf; *bp != CR && *bp != LF && *bp != 0; bp++);

	// If found, copy the line and remove it from buffer:
	if (*bp) {
		if (*bp == CR && bp[1] == LF) bp++;
		len = bp - inbuf;
		out = OS_Make(len + 2);
		COPY_BYTES(out, inbuf, len+1);
		out[len+1] = 0;
		MOVE_MEM(inbuf, bp+1, 1+strlen(bp+1));
		return out;
	}

	return 0; // more input needed
}

static int Fetch_Buf()
{
	REBCNT len = strlen(inbuf);

	Std_IO_Req.data   = inbuf + len;
	Std_IO_Req.length = inbuf_len - len - 1;
	Std_IO_Req.actual = 0;

	OS_Do_Device(&Std_IO_Req, RDC_READ);

	// If error, don't crash, just ignore it:
	if (Std_IO_Req.error) return 0; //Host_Crash("stdio read");

	// Terminate (LF) last line?
	if (len > 0 && Std_IO_Req.actual == 0) {
		inbuf[len++] = LF;
		inbuf[len] = 0;
		return TRUE;
	}

	// Null terminate buffer:
	len = Std_IO_Req.actual;
	Std_IO_Req.data[len] = 0;
	return len > 0;
}


/***********************************************************************
**
*/	void Open_StdIO(void)
/*
**		Open REBOL's standard IO device. This same device is used
**		by both the host code and the R3 DLL itself.
**
**		This must be done before any other initialization is done
**		in order to output banners or errors.
**
***********************************************************************/
{
	CLEARS(&Std_IO_Req);
	Std_IO_Req.clen = sizeof(Std_IO_Req);
	Std_IO_Req.device = RDI_STDIO;

	OS_Do_Device(&Std_IO_Req, RDC_OPEN);

	if (Std_IO_Req.error) Host_Crash("stdio open");

	inbuf = OS_Make(inbuf_len);
	inbuf[0] = 0;
}


/***********************************************************************
**
*/	REBYTE *Get_Str()
/*
**		Get input of a null terminated UTF-8 string.
**		Divides the input into lines.
**		Buffers multiple lines if needed.
**		Returns NULL on end of stream.
**
***********************************************************************/
{
	REBYTE *line;

	if ((line = Get_Next_Line())) return line;

	if (Fetch_Buf()) return Get_Next_Line();

	return 0;
}


/***********************************************************************
**
*/	void Put_Str(REBYTE *buf)
/*
**		Outputs a null terminated UTF-8 string.
**		If buf is larger than StdIO Device allows, error out.
**		OS dependent line termination must be done prior to call.
**
***********************************************************************/
{
	Std_IO_Req.length = strlen(buf);
	Std_IO_Req.data = (REBYTE*)buf;
	Std_IO_Req.actual = 0;

	OS_Do_Device(&Std_IO_Req, RDC_WRITE);

	if (Std_IO_Req.error) Host_Crash("stdio write");
}
