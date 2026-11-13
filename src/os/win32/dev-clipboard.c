/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
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
	REBYTE *bin;
	REBINT len;
	REBCNT ok;

	req->actual = 0;

	// If there is no clipboard data:
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
		req->error = 10;
		return DR_ERROR;
	}

	MSG msg;
	for (int i = 1; i < 4; i++) {
		ok = OpenClipboard(NULL);
		if (ok) break;
		Sleep(i);
		PeekMessage(&msg, NULL, 0, 0, 0);
	}
	if (!ok) {
		req->error = 20;
		return DR_ERROR;
	}

	// Read the UCS-2 data:
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
	// Convert to UTF-8
	len = WideCharToMultiByte(CP_UTF8, 0, cp, AS_INT(LEN_STR(cp)), NULL, 0, 0, 0);
	bin = OS_Make(len+1);
	WideCharToMultiByte(CP_UTF8, 0, cp, len, bin, len, 0, 0);

	GlobalUnlock(data);
	CloseClipboard();

	bin[len] = 0;

	//SET_FLAG(req->flags, RRF_WIDE);
	req->data = bin;
	req->actual = len;
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
	MSG msg;
	REBCHR *text;
	REBCNT ok;
	SIZE_T len;

	req->actual = 0;

	len = MultiByteToWideChar(CP_UTF8, 0, req->data, req->length, NULL, 0);

	data = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t));
	if (data == NULL) {
		req->error = 5;
		return DR_ERROR;
	}

	// Lock and copy the string:
	text = GlobalLock(data);
	if (text == NULL) {
		req->error = 10;
		return DR_ERROR;
	}
	
	len = MultiByteToWideChar(CP_UTF8, 0, req->data, req->length, text, len);
	text[len] = 0;
	
	for (int i = 1; i < 4; i++) {
		ok = OpenClipboard(NULL);
		if (ok) break;
		Sleep(i);
		PeekMessage(&msg, NULL, 0, 0, 0);
	}
	if (!ok) {
		GlobalUnlock(data);
		req->error = 20;
		return DR_ERROR;
	}

	EmptyClipboard();

	ok = (NULL != SetClipboardData(CF_UNICODETEXT, data));
	
	GlobalUnlock(data);
	CloseClipboard();

	if (!ok) {
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
