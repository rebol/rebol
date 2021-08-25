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
**  Author: Carl Sassenrath, Oldes
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

#include "sys-net.h"
#include "reb-host.h"
#include "host-lib.h"


REBOOL GetClipboardStringData(REBREQ *req);
REBOOL SetClipboardStringData(REBREQ *req);

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
	req->actual = 0;
	
	if(!GetClipboardStringData(req)){
		req->error = 20;
		// just a random number... would be better to have some system for it
		return DR_ERROR;
	}
	
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_Clipboard(REBREQ *req)
/*
**		Expects data in UTF8 encoding in req->data!
**
***********************************************************************/
{
	if(!SetClipboardStringData(req)) {
		req->error = 20;
		// just a random number... would be better to have some system for it
		return DR_ERROR;
	}
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
