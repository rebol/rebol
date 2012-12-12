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
**  Title: Device: Clipboard access for Win32
**  Author: Carl Sassenrath
**  Purpose:
**      Provides a very simple interface to the clipboard for text.
**      May be expanded in the future for images, etc.
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

#include "reb-host.h"
#include "host-lib.h"
#include "sys-net.h"


/***********************************************************************
**
*/	DEVICE_CMD Open_Clipboard(REBREQ *req)
/*
***********************************************************************/
{
	SET_OPEN(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_Clipboard(REBREQ *req)
/*
***********************************************************************/
{
	SET_CLOSED(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_Clipboard(REBREQ *req)
/*
***********************************************************************/
{
	HANDLE data;
	REBUNI *cp;
	REBUNI *bin;
	REBINT len;

	req->actual = 0;

	// If there is no clipboard data:
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
		req->error = 10;
		return DR_ERROR;
	}

	if (!OpenClipboard(NULL)) {
		req->error = 20;
		return DR_ERROR;
	}

	// Read the UTF-8 data:
	if ((data = GetClipboardData(CF_UNICODETEXT)) == NULL) {
		CloseClipboard();
		req->error = 30;
		return DR_ERROR;
	}

	cp = GlobalLock(data);
	if (!cp) {
		GlobalUnlock(data);
		CloseClipboard();
		req->error = 40;
		return DR_ERROR;
	}

	len = LEN_STR(cp); // wide chars
	bin = OS_Make((len+1) * sizeof(REBCHR));
	COPY_STR(bin, cp, len);

	GlobalUnlock(data);

	CloseClipboard();

	SET_FLAG(req->flags, RRF_WIDE);
	req->data = (REBYTE *)bin;
	req->actual = len * sizeof(REBCHR);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_Clipboard(REBREQ *req)
/*
**		Works for Unicode and ASCII strings.
**		Length is number of bytes passed (not number of chars).
**
***********************************************************************/
{
	HANDLE data;
	REBYTE *bin;
	REBCNT err;
	REBINT len = req->length; // in bytes

	req->actual = 0;

	data = GlobalAlloc(GHND, len + 4);
	if (data == NULL) {
		req->error = 5;
		return DR_ERROR;
	}

	// Lock and copy the string:
	bin = GlobalLock(data);
	if (bin == NULL) {
		req->error = 10;
		return DR_ERROR;
	}

	COPY_MEM(bin, req->data, len);
	bin[len] = 0;
	GlobalUnlock(data);

	if (!OpenClipboard(NULL)) {
		req->error = 20;
		return DR_ERROR;
	}

	EmptyClipboard();

	err = !SetClipboardData(GET_FLAG(req->flags, RRF_WIDE) ? CF_UNICODETEXT : CF_TEXT, data);

	CloseClipboard();

	if (err) {
		req->error = 50;
		return DR_ERROR;
	}

	req->actual = len;
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Poll_Clipboard(REBREQ *req)
/*
***********************************************************************/
{
	return DR_DONE;
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] =
{
	0,
	0,
	Open_Clipboard,
	Close_Clipboard,
	Read_Clipboard,
	Write_Clipboard,
	Poll_Clipboard,
};

DEFINE_DEV(Dev_Clipboard, "Clipboard", 1, Dev_Cmds, RDC_MAX, 0);
