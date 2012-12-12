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
**  Summary: System network definitions
**  Module:  sys-net.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

//----- Windows - "Network standards? What network standards?" -Bill G.
#ifdef TO_WIN32

#include <winsock.h>

#define GET_ERROR		WSAGetLastError()
#define IOCTL			ioctlsocket
#define CLOSE_SOCKET	closesocket

#define NE_ISCONN		WSAEISCONN
#define NE_WOULDBLOCK	WSAEWOULDBLOCK
#define NE_INPROGRESS	WSAEINPROGRESS
#define NE_ALREADY		WSAEALREADY
#define NE_NOTCONN		WSAENOTCONN
#define NE_INVALID		WSAEINVAL

//----- BSD - The network standard the rest of the world uses
#else

#ifdef TO_AMIGA
typedef char __BYTE;
typedef unsigned char __UBYTE;
typedef char * __STRPTR;
typedef long __LONG;
#endif

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define GET_ERROR		errno
#define IOCTL			ioctl
#define CLOSE_SOCKET	close
#define SOCKET			unsigned int

#define NE_ISCONN		EISCONN
#define NE_WOULDBLOCK	EAGAIN		// see include/asm/errno.h
#define NE_INPROGRESS	EINPROGRESS
#define NE_ALREADY		EALREADY
#define NE_NOTCONN		ENOTCONN
#define NE_INVALID		EINVAL

// Null Win32 functions:
#define WSADATA int

// FreeBSD mystery define:
#ifndef u_int32_t
#define u_int32_t long
#endif

#ifndef HOSTENT
typedef struct hostent HOSTENT;
#endif

#ifndef MAXGETHOSTSTRUCT
#define MAXGETHOSTSTRUCT ((sizeof(struct hostent)+15) & ~15)
#endif

#endif // BSD

typedef struct sockaddr_in SOCKAI; // Internet extensions

#define BAD_SOCKET (~0)
#define MAX_TRANSFER 32000		// Max send/recv buffer size
#define MAX_HOST_NAME 256		// Max length of host name
