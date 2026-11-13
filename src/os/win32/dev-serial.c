/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2013 REBOL Technologies
**  Copyright 2013-2023 Rebol Open Source Developers
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
**  Title: Device: Serial port access for Windows
**  Author: Carl Sassenrath, Joshua Shireman, Shixin Zeng
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


#include <windows.h>
#include <stdio.h>

#include "reb-host.h"
#include "host-lib.h"

#define MAX_SERIAL_DEV_PATH 128
//#define DEBUG_SERIAL



/***********************************************************************
**
**	Local Functions
**
***********************************************************************/
static REBOOL Set_Serial_Settings(HANDLE hComm, REBREQ *req)
{
	DCB dcbSerialParams = {0};
	COMMTIMEOUTS timeouts = {0};

	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	if (GetCommState(hComm, &dcbSerialParams) == 0)
		return FALSE;

	dcbSerialParams.BaudRate = req->serial.baud;
	dcbSerialParams.ByteSize = req->serial.data_bits;
	dcbSerialParams.StopBits = req->serial.stop_bits == 1? ONESTOPBIT : TWOSTOPBITS;
	switch (req->serial.parity) {
		case SERIAL_PARITY_ODD:
			dcbSerialParams.Parity = ODDPARITY;
			break;
		case SERIAL_PARITY_EVEN:
			dcbSerialParams.Parity = EVENPARITY;
			break;
		case SERIAL_PARITY_NONE:
		default:
			dcbSerialParams.Parity = NOPARITY;
			break;
	}

	if(SetCommState(hComm, &dcbSerialParams) == 0)
		return FALSE;

	// See: http://msdn.microsoft.com/en-us/library/windows/desktop/aa363190%28v=vs.85%29.aspx
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	if (SetCommTimeouts(hComm, &timeouts) == 0)
		return FALSE;

#ifdef DEBUG_SERIAL
	GetCommState(hComm, &dcbSerialParams);
	printf("BaudRate: %lu\n",  dcbSerialParams.BaudRate);
	printf("ByteSize: %hhu\n", dcbSerialParams.ByteSize);
	printf("StopBits: %hhu\n", dcbSerialParams.StopBits);
	printf("Parity:   %hhu\n", dcbSerialParams.Parity);

	puts("Timeouts:");
	GetCommTimeouts(hComm, &timeouts);
	printf("ReadIntervalTimeout: %lu\n", timeouts.ReadIntervalTimeout);
	printf("ReadTotalTimeoutMultiplier: %lu\n", timeouts.ReadTotalTimeoutMultiplier);
	printf("ReadTotalTimeoutConstant: %lu\n", timeouts.ReadTotalTimeoutConstant);
	printf("WriteTotalTimeoutMultiplier: %lu\n", timeouts.WriteTotalTimeoutMultiplier);
	printf("WriteTotalTimeoutConstant: %lu\n", timeouts.WriteTotalTimeoutConstant);
#endif

	PurgeComm(hComm,PURGE_RXCLEAR|PURGE_TXCLEAR);  //make sure buffers are clean
	EscapeCommFunction(hComm, CLRRTS);
	return TRUE;
}

/***********************************************************************
**
*/	DEVICE_CMD Open_Serial(REBREQ *req)
/*
**		serial.path = the /dev name for the serial port
**		serial.baud = speed (baudrate)
**
***********************************************************************/
{
	HANDLE hComm;

	// req->serial.path should be prefixed with "\\.\" to allow for higher com port numbers
	REBCHR fullpath [MAX_SERIAL_DEV_PATH]=TXT("\\\\.\\");

	if (!req->serial.path) {
		req->error = -RFE_BAD_PATH;
		return DR_ERROR;
	}

	JOIN_STR(fullpath,req->serial.path,MAX_SERIAL_DEV_PATH-5); // 5 because of "\\.\" and terminating byte

	hComm = CreateFile(fullpath, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
	if (hComm == INVALID_HANDLE_VALUE) {
		req->error = -RFE_OPEN_FAIL;
		return DR_ERROR;
	}

	if (!Set_Serial_Settings(hComm, req)) {
		CloseHandle(hComm);
		req->error = -RFE_OPEN_FAIL;
		return DR_ERROR;
	}

	req->handle = hComm;
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_Serial(REBREQ *req)
/*
***********************************************************************/
{
	if (req->handle) {
		//Warning: should free req->serial.prior_attr termios struct?
		CloseHandle(req->handle);
		req->handle = 0;
	}
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_Serial(REBREQ *req)
/*
***********************************************************************/
{
	DWORD result = 0;
	if (!req->handle) {
		req->error = -RFE_NO_HANDLE;
		return DR_ERROR;
	}

	//RL_Print("reading %d bytes\n", req->length);
	if (!ReadFile(req->handle, req->data, req->length, &result, 0) || result < 0) {
		req->error = -RFE_BAD_READ;
		OS_Signal_Device(req, EVT_ERROR);
		return DR_ERROR;
	} else {
		if (result == 0) {
			return DR_PEND;
		} else if (result > 0){
			//RL_Print("read %d bytes\n", req->actual);
			req->actual = result;
			OS_Signal_Device(req, EVT_READ);
		}
	}

#ifdef DEBUG_SERIAL
	printf("read %d ret: %d\n", req->length, req->actual);
#endif

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_Serial(REBREQ *req)
/*
***********************************************************************/
{
	DWORD result = 0, len = 0;
	len = req->length - req->actual;
	if (!req->handle) {
		req->error = -RFE_NO_HANDLE;
		return DR_ERROR;
	}

	if (len <= 0) return DR_DONE;


	if (!WriteFile(req->handle, req->data, len, &result, NULL)){
		req->error = -RFE_BAD_WRITE;
		return DR_ERROR;
	}

#ifdef DEBUG_SERIAL
	printf("%p = %u %u %u %u\n", req->data, req->data[0], req->data[1], req->data[2], req->data[3]);
	printf("write %d wrote: %lu ret: %d\n", req->length, result,  req->actual);
#endif

	if (result < 0) {
		req->error = -RFE_BAD_WRITE;
		OS_Signal_Device(req, EVT_ERROR);
		return DR_ERROR;
	}
	req->actual += result;
	//req->data += result;
	if (req->actual >= req->length) {
		OS_Signal_Device(req, EVT_WROTE);
		return DR_DONE;
	} else {
		SET_FLAG(req->flags, RRF_ACTIVE); /* notify OS_WAIT of activity */
		return DR_PEND;
	}
}


/***********************************************************************
**
*/	DEVICE_CMD Query_Serial(REBREQ *req)
/*
***********************************************************************/
{
#ifdef QUERY_IMPLEMENTED
	struct pollfd pfd;

	if (req->handle) {
		pfd.fd = req->handle;
		pfd.events = POLLIN;
		n = poll(&pfd, 1, 0);
	}
#endif
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Modify_Serial(REBREQ *req)
/*
***********************************************************************/
{
	boolean value = req->modify.value;
	switch (req->modify.mode) {
	case 1:	EscapeCommFunction(req->handle, value ? SETBREAK : CLRBREAK); break;
	case 2:	EscapeCommFunction(req->handle, value ? SETRTS : CLRRTS); break; // (request-to-send) signal
	case 3:	EscapeCommFunction(req->handle, value ? SETDTR : CLRDTR); break; // (data-terminal-ready) signal
	}
	return DR_DONE;
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] = {
	0,
	0,
	Open_Serial,
	Close_Serial,
	Read_Serial,
	Write_Serial,
	0,  // poll
	0,	// connect
	Query_Serial,
	Modify_Serial,
	0,	// create
	0,	// delete
	0	// rename
};

DEFINE_DEV(Dev_Serial, "Serial IO", 1, Dev_Cmds, RDC_MAX, sizeof(REBREQ));

