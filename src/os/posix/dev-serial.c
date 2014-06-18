/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2013 REBOL Technologies
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
**  Title: Device: Serial port access for Posix
**  Author: Carl Sassenrath
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <termios.h>

#include "reb-host.h"
#include "host-lib.h"

#define MAX_SERIAL_PATH 128

/* BXXX constants are defined in termios.h */ 
const int speeds[] = {
	50, B50,
	75, B75,
	110, B110,
	134, B134,
	150, B150,
	200, B200,
	300, B300,
	600, B600,
	1200, B1200,
	1800, B1800,
	2400, B2400,
	4800, B4800,
	9600, B9600,
	19200, B19200,
	38400, B38400,
	57600, B57600,
	115200, B115200,
	230400, B230400,
	0
};

/***********************************************************************
**
**	Local Functions
**
***********************************************************************/

static struct termios *Get_Serial_Settings(int ttyfd)
{
	struct termios *attr = NULL;
	attr= (struct termios *) MAKE_NEW(struct termios);
	if (attr != NULL) {
        	if (tcgetattr(ttyfd,attr) == -1) {
          		FREE_MEM(attr);
          		attr = NULL;
        	}
      	}
	return attr;
}


static REBINT Set_Serial_Settings(int ttyfd, int speed)
{
	REBINT n;
	struct termios attr = {};
#ifdef DEBUG_SERIAL
	printf("setting attributes: speed %d\n", speed);
#endif
	for (n = 0; speeds[n]; n += 2) {
		if (speed == speeds[n]) {
			speed = speeds[n+1];
			break;
		}
	}
	if (speeds[n] == 0) speed = B115200; // invalid, use default

	cfsetospeed (&attr, speed);
	cfsetispeed (&attr, speed);

	// TTY has many attributes. Refer to "man tcgetattr" for descriptions.
	// C-flags - control modes:
	attr.c_cflag |= CREAD | CS8 | CLOCAL;

	// L-flags - local modes:
	attr.c_lflag = 0; // raw, not ICANON

	// I-flags - input modes:
	attr.c_iflag |= IGNPAR;
	
	// O-flags - output modes:
	attr.c_oflag = 0;

	// Control characters:
	// R3 devices are non-blocking (polled for changes):
	attr.c_cc[VMIN]  = 0;
	attr.c_cc[VTIME] = 0;

	// Make sure OS queues are empty:
	tcflush(ttyfd, TCIFLUSH);

	// Set new attributes:
	if (tcsetattr(ttyfd, TCSANOW, &attr)) return 2;

	return 0;
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
	REBCHR *path;
	REBCHR devpath[MAX_SERIAL_PATH];
	REBINT h;

	if (!(path = req->serial.path)) {
		req->error = -RFE_BAD_PATH;
		return DR_ERROR;
	}

	strcpy(&devpath[0], "/dev/");
	strncpy(&devpath[5], path, MAX_SERIAL_PATH-6);
	h = open(&devpath[0], O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (h < 0) {
		req->error = -RFE_OPEN_FAIL;
		return DR_ERROR;
	}

	//Getting prior atttributes:
	req->serial.prior_attr = Get_Serial_Settings(h);
	if (tcgetattr(h, req->serial.prior_attr)) return 1;	

	if (Set_Serial_Settings(h, req->serial.baud)) {
		close(h);
		req->error = -RFE_OPEN_FAIL;
		return DR_ERROR;
	}		

	req->id = h;
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_Serial(REBREQ *req)
/*
***********************************************************************/
{
	if (req->id) {
		//Warning: should free req->serial.prior_attr termios struct?
		tcsetattr(req->id, TCSANOW, req->serial.prior_attr);
		close(req->id);
		req->id = 0;
	}
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_Serial(REBREQ *req)
/*
***********************************************************************/
{
	if (!req->id) {
		req->error = -RFE_NO_HANDLE;
		return DR_ERROR;
	}

	req->actual = read(req->id, req->data, req->length);
#ifdef DEBUG_SERIAL
	printf("read %d ret: %d\n", req->length, req->actual);
#endif
	if (req->actual < 0) {
		req->error = -RFE_BAD_READ;
		return DR_ERROR;
	} else {
		req->serial.index += req->actual;
	}

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_Serial(REBREQ *req)
/*
***********************************************************************/
{
	if (!req->id) {
		req->error = -RFE_NO_HANDLE;
		return DR_ERROR;
	}

	if (req->length == 0) return DR_DONE;

	req->actual = write(req->id, req->data, req->length);
#ifdef DEBUG_SERIAL
	printf("write %d ret: %d\n", req->length, req->actual);
#endif
	if (req->actual < 0) {
		req->error = -RFE_BAD_WRITE;
		return DR_ERROR;
	}

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Query_Serial(REBREQ *req)
/*
***********************************************************************/
{
#ifdef QUERY_IMPLEMENTED
	struct pollfd pfd;

	if (req->id) {
		pfd.fd = req->id;
		pfd.events = POLLIN;
		n = poll(&pfd, 1, 0);
	}
#endif
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
	0,	// modify
	0,	// create
	0,	// delete
	0	// rename
};

DEFINE_DEV(Dev_Serial, "Serial IO", 1, Dev_Cmds, RDC_MAX, sizeof(REBREQ));

