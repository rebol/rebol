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
**  Title: Device: Audio access for Win32
**  Author: Oldes
**  Purpose:
**      Provides a very simple interface to the audio API
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
*/	DEVICE_CMD Open_Audio(REBREQ *req)
/*
***********************************************************************/
{
	puts("Open_Audio");
	SET_OPEN(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_Audio(REBREQ *req)
/*
***********************************************************************/
{
	puts("Close_Audio");
	SET_CLOSED(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_Audio(REBREQ *req)
/*
***********************************************************************/
{
	puts("Read_Audio");
	HANDLE data;
	REBUNI *cp;
	REBUNI *bin;
	REBINT len;

	req->actual = 0;
	
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_Audio(REBREQ *req)
/*
**		Works for Unicode and ASCII strings.
**		Length is number of bytes passed (not number of chars).
**
***********************************************************************/
{
	puts("Write_Audio");
	HANDLE data;
	REBYTE *bin;
	REBCNT err;
	REBINT len = req->length; // in bytes

	req->actual = 0;

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Poll_Audio(REBREQ *req)
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
	Open_Audio,
	Close_Audio,
	Read_Audio,
	Write_Audio,
	Poll_Audio,
};

DEFINE_DEV(Dev_Audio, "Audio", 1, Dev_Cmds, RDC_MAX, 0);
