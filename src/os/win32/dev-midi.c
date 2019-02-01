/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2019 Rebol Open Source Developers
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
**  Title: Device: MIDI access for Win32
**  Author: Oldes
**  Purpose:
**      Provides a very simple interface to the MIDI devices
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

#include <SDKDDKVer.h>
#include <Windows.h>
#include <stdio.h>
#include <conio.h>

#include "reb-host.h"
#include "host-lib.h"
#include "sys-net.h"


#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


static void PrintMidiDevices()
{
	UINT nMidiDeviceNum;
	MIDIINCAPS caps;

	nMidiDeviceNum = midiInGetNumDevs();
	if (nMidiDeviceNum == 0) {
		fprintf(stderr, "midiInGetNumDevs() return 0...");
		return;
	}

	printf("== PrintMidiDevices() == \n");
	for (unsigned int i = 0; i < nMidiDeviceNum; ++i) {
		midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
		printf("\t%d : name = %S\n", i, caps.szPname);
		printf("\t\tManufacturer ID: %d\n", caps.wMid);
		printf("\t\tProduct ID:      %d\n", caps.wPid);
		printf("\t\tDriver version:  %d\n", caps.vDriverVersion);
	}
	printf("=====\n");
}

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	int status, channel, param1, param2;

	switch (wMsg) {
	case MIM_OPEN:
		printf("wMsg=MIM_OPEN\n");
		break;
	case MIM_CLOSE:
		printf("wMsg=MIM_CLOSE\n");
		break;
	case MIM_DATA:
		status = (0xF0 & dwParam1);
		channel = (0x0F & dwParam1);
		param1 = 0xFF & (dwParam1 >> 8);
		param2 = 0xFF & (dwParam1 >> 16);
		printf("wMsg=MIM_DATA, dwInstance=%08x, dwParam1=%08x, dwParam2=%08x = status=%08x, channel=%u\n", dwInstance, dwParam1, dwParam2, status, channel);
		switch (status) {
		case 0x80: //Note Off event.
			printf("Release key: %u velocity: %u\n", param1, param2);
			break;
		case 0x90: //Note On event.
			printf("Pressed key: %u velocity: %u\n", param1, param2);
			break;
		case 0xb0: //control change.
			printf("Control num: %u value: %u\n", param1, param2);
			break;

		}
		break;
	case MIM_LONGDATA:
		printf("wMsg=MIM_LONGDATA\n");
		break;
	case MIM_ERROR:
		printf("wMsg=MIM_ERROR\n");
		break;
	case MIM_LONGERROR:
		printf("wMsg=MIM_LONGERROR\n");
		break;
	case MIM_MOREDATA:
		printf("wMsg=MIM_MOREDATA\n");
		break;
	default:
		printf("wMsg = unknown\n");
		break;
	}
	return;
}


/***********************************************************************
**
*/	DEVICE_CMD Open_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	HMIDIIN hMidiDevice = NULL;;
	DWORD nMidiPort = 0;
	UINT nMidiDeviceNum;
	MMRESULT rv;

	puts("Open_MIDI");
	SET_OPEN(req);
	PrintMidiDevices();

	nMidiDeviceNum = midiInGetNumDevs();
	if (nMidiDeviceNum == 0) {
		printf("midiInGetNumDevs() return 0...");
		return DR_ERROR;
	}

	rv = midiInOpen(&hMidiDevice, nMidiPort, (DWORD_PTR)MidiInProc, 0, CALLBACK_FUNCTION);
	if (rv != MMSYSERR_NOERROR) {
		printf("midiInOpen() failed...rv=%d", rv);
		return DR_ERROR;
	}

	midiInStart(hMidiDevice);

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	puts("Close_MIDI");
	SET_CLOSED(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	HANDLE data;
	REBUNI *cp;
	REBUNI *bin;
	REBINT len;

	puts("Read_MIDI");

	req->actual = 0;
#ifdef unused

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

	len = (REBINT)LEN_STR(cp); // wide chars
	bin = OS_Make((len+1) * sizeof(REBCHR));
	COPY_STR(bin, cp, len);

	GlobalUnlock(data);

	CloseClipboard();

	SET_FLAG(req->flags, RRF_WIDE);
	req->data = (REBYTE *)bin;
	req->actual = len * sizeof(REBCHR);
#endif
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_MIDI(REBREQ *req)
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

	puts("Write_MIDI");

	req->actual = 0;
#ifdef unused
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
#endif
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Poll_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	puts("Poll_MIDI");
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Query_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	puts("Query_MIDI");
	PrintMidiDevices();
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
	Open_MIDI,
	Close_MIDI,
	Read_MIDI,
	Write_MIDI,
	Poll_MIDI,
	0,	// connect
	Query_MIDI,
	0,	// modify
	0,	// create
};

DEFINE_DEV(Dev_MIDI, "MIDI", 1, Dev_Cmds, RDC_MAX, 0);
