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
**  Title: Device: TCP/IP network access
**  Author: Carl Sassenrath
**  Purpose: Supports TCP and UDP (but not raw socket modes.)
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

#include "reb-host.h"
#include "host-lib.h"
#include "sys-net.h"

#if (0)
#define WATCH1(s,a) printf(s, a)
#define WATCH2(s,a,b) printf(s, a, b)
#define WATCH4(s,a,b,c,d) printf(s, a, b, c, d)
#else
#define WATCH1(s,a)
#define WATCH2(s,a,b)
#define WATCH4(s,a,b,c,d)
#endif

void Signal_Device(REBREQ *req, REBINT type);
DEVICE_CMD Listen_Socket(REBREQ *sock);

#ifdef TO_WIN32
extern HWND Event_Handle; // For WSAAsync API
#endif


/***********************************************************************
**
**	Local Functions
**
***********************************************************************/

static void Set_Addr(SOCKAI *sa, long ip, int port)
{
	// Set the IP address and port number in a socket_addr struct.
	sa->sin_family = AF_INET;
	sa->sin_addr.s_addr = ip;  //htonl(ip); NOTE: REBOL stays in network byte order
	sa->sin_port = htons((unsigned short)port);
}

static void Get_Local_IP(REBREQ *sock)
{
	// Get the local IP address and port number.
	// This code should be fast and never fail.
	SOCKAI sa;
	int len = sizeof(sa);

	getsockname(sock->socket, (struct sockaddr *)&sa, &len);
	sock->net.local_ip = sa.sin_addr.s_addr; //htonl(ip); NOTE: REBOL stays in network byte order
	sock->net.local_port = ntohs(sa.sin_port);
}

static REBOOL Nonblocking_Mode(SOCKET sock)
{
	// Set non-blocking mode. Return TRUE if no error.
#ifdef FIONBIO
	long mode = 1;
	return !IOCTL(sock, FIONBIO, &mode);
#else
	int flags;
	flags = fcntl(sock, F_GETFL, 0);
	flags |= O_NONBLOCK;
	//else flags &= ~O_NONBLOCK;
	fcntl(sock, F_SETFL, flags);
	return TRUE;
#endif
}


/***********************************************************************
**
*/	DEVICE_CMD Init_Net(REBREQ *dr)
/*
**		Intialize networking libraries and related interfaces.
**		This function will be called prior to any socket functions.
**
***********************************************************************/
{
	REBDEV *dev = (REBDEV*)dr; // just to keep compiler happy
#ifdef TO_WIN32
	WSADATA wsaData;
	// Initialize Windows Socket API with given VERSION.
	// It is ok to call twice, as long as WSACleanup twice.
	if (WSAStartup(0x0101, &wsaData)) return DR_ERROR;
#endif
	SET_FLAG(dev->flags, RDF_INIT);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Quit_Net(REBREQ *dr)
/*
**		Close and cleanup networking libraries and related interfaces.
**
***********************************************************************/
{
	REBDEV *dev = (REBDEV*)dr; // just to keep compiler happy
#ifdef TO_WIN32
	if (GET_FLAG(dev->flags, RDF_INIT)) WSACleanup();
#endif
	CLR_FLAG(dev->flags, RDF_INIT);
	return DR_DONE;
}


/***********************************************************************
**
*/	int Host_Address(char *hostname, char *hostaddr)
/*
**		Simple lookup of a host address.
**		The hostaddr must be at least 16 bytes in size (IPv6).
**		This is a synchronous function and blocks during access.
**
**		On success, returns length of address.
**		On failure, returns 0.
**
**		Current version is IPv4 only.
**
***********************************************************************/
{
	struct hostent *he;

	if (!(he = gethostbyname(hostname))) return DR_DONE;

	COPY_MEM(hostaddr, (char *)(*he->h_addr_list), he->h_length);

	return he->h_length;
}


/***********************************************************************
**
*/	DEVICE_CMD Open_Socket(REBREQ *sock)
/*
**		Setup a socket with the specified protocol and bind it to
**		the related transport service.
**
**		Returns 0 on success.
**		On failure, error code is OS local.
**
**		Note: This is an intialization procedure and no actual
**		connection is made at this time. The IP address and port
**		number are not needed, only the type of service required.
**
**		After usage:
**			Close_Socket() - to free OS allocations
**
***********************************************************************/
{
	int type;
    int	protocol;
	long result;

	sock->error = 0;
	sock->state = 0;  // clear all flags

	// Setup for correct type and protocol:
	if (GET_FLAG(sock->modes, RST_UDP)) {
		type = SOCK_DGRAM;
		protocol = IPPROTO_UDP;
	}
	else {	// TCP is default
		type = SOCK_STREAM;
		protocol = IPPROTO_TCP;
	}

	// Bind to the transport service, return socket handle or error:
	result = (int)socket(AF_INET, type, protocol);

	// Failed, get error code (os local):
	if (result == BAD_SOCKET) {
		sock->error = GET_ERROR;
		return DR_ERROR;
	}

	sock->socket = result;
	SET_FLAG(sock->state, RSM_OPEN);

	// Set socket to non-blocking async mode:
	if (!Nonblocking_Mode(sock->socket)) {
		sock->error = GET_ERROR;
		return DR_ERROR;
	}

	return DR_DONE;
}


/***********************************************************************
**
*/	 DEVICE_CMD Close_Socket(REBREQ *sock)
/*
**		Close a socket.
**
**		Returns 0 on success.
**		On failure, error code is OS local.
**
***********************************************************************/
{
	sock->error = 0;

	if (GET_FLAG(sock->state, RSM_OPEN)) {

		sock->state = 0;  // clear: RSM_OPEN, RSM_CONNECT

		// If DNS pending, abort it:
		if (sock->net.host_info) {  // indicates DNS phase active
#ifdef HAS_ASYNC_DNS
			if (sock->handle) WSACancelAsyncRequest(sock->handle);
#endif
			OS_Free(sock->net.host_info);
			sock->socket = sock->length; // Restore TCP socket (see Lookup)
		}

		if (CLOSE_SOCKET(sock->socket)) {
			sock->error = GET_ERROR;
			return DR_ERROR;
		}
	}

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Lookup_Socket(REBREQ *sock)
/*
**		Initiate the GetHost request and return immediately.
**		This is very similar to the DNS device.
**		The request will pend until the main event handler gets WM_DNS.
**		Note the temporary results buffer (must be freed later).
**		Note we use the sock->handle for the DNS handle. During use,
**		we store the TCP socket in the length field.
**
***********************************************************************/
{
#ifdef TO_WIN32
	HANDLE handle;
#endif
	HOSTENT *host;

#ifdef HAS_ASYNC_DNS
	// Check if we are polling for completion:
	if (host = (HOSTENT*)(sock->net.host_info)) {
		// The windows main event handler will change this when it gets WM_DNS event:
		if (!GET_FLAG(sock->flags, RRF_DONE)) return DR_PEND; // still waiting
		CLR_FLAG(sock->flags, RRF_DONE);
		if (!sock->error) { // Success!
			host = (HOSTENT*)sock->net.host_info;
			COPY_MEM((char*)&(sock->net.remote_ip), (char *)(*host->h_addr_list), 4); //he->h_length);
			Signal_Device(sock, EVT_LOOKUP);
		}
		else
			Signal_Device(sock, EVT_ERROR);
		OS_Free(host);	// free what we allocated earlier
		sock->socket = sock->length; // Restore TCP socket saved below
		sock->net.host_info = 0;
		return DR_DONE;
	}

	// Else, make the lookup request:
	host = OS_Make(MAXGETHOSTSTRUCT); // be sure to free it
	handle = WSAAsyncGetHostByName(Event_Handle, WM_DNS, sock->data, (char*)host, MAXGETHOSTSTRUCT);
	if (handle != 0) {
		sock->net.host_info = host;
		sock->length = sock->socket; // save TCP socket temporarily
		sock->handle = handle;
		return DR_PEND; // keep it on pending list
	}
	OS_Free(host);
#else
	// Use old-style blocking DNS (mainly for testing purposes):
	host = gethostbyname(sock->data);
	sock->net.host_info = 0; // no allocated data

	if (host) {
		COPY_MEM((char*)&(sock->net.remote_ip), (char *)(*host->h_addr_list), 4); //he->h_length);
		CLR_FLAG(sock->flags, RRF_DONE);
		Signal_Device(sock, EVT_LOOKUP);
		return DR_DONE;
	}
#endif

	sock->error = GET_ERROR;
	//Signal_Device(sock, EVT_ERROR);
	return DR_ERROR; // Remove it from pending list
}


/***********************************************************************
**
*/	DEVICE_CMD Connect_Socket(REBREQ *sock)
/*
**		Connect a socket to a service.
**		Only required for connection-based protocols (e.g. not UDP).
**		The IP address must already be resolved before calling.
**
**		This function is asynchronous. It will return immediately.
**		You can call this function again to check the pending connection.
**
**		The function will return:
**			=0: connection succeeded (or already is connected)
**			>0: in-progress, still trying
**		    <0: error occurred, no longer trying
**
**		Before usage:
**			Open_Socket() -- to allocate the socket
**
***********************************************************************/
{
	int result;
	SOCKAI sa;

	if (GET_FLAG(sock->modes, RST_LISTEN))
		return Listen_Socket(sock);

	if (GET_FLAG(sock->state, RSM_CONNECT)) return DR_DONE; // already connected

	Set_Addr(&sa, sock->net.remote_ip, sock->net.remote_port);
	result = connect(sock->socket, (struct sockaddr *)&sa, sizeof(sa));

	if (result != 0) result = GET_ERROR;

	WATCH2("connect() error: %d - %s\n", result, strerror(result));

	switch (result) {

	case 0: // no error
	case NE_ISCONN:
		// Connected, set state:
		CLR_FLAG(sock->state, RSM_ATTEMPT);
		SET_FLAG(sock->state, RSM_CONNECT);
		Get_Local_IP(sock);
		Signal_Device(sock, EVT_CONNECT);
		return DR_DONE; // done

#ifdef TO_WIN32
	case NE_INVALID:	// Corrects for Microsoft bug
#endif
	case NE_WOULDBLOCK:
	case NE_INPROGRESS:
	case NE_ALREADY:
		// Still trying:
		SET_FLAG(sock->state, RSM_ATTEMPT);
		return DR_PEND;

	default:
		// An error happened:
		CLR_FLAG(sock->state, RSM_ATTEMPT);
		sock->error = result;
		//Signal_Device(sock, EVT_ERROR);
		return DR_ERROR;
	}
}


/***********************************************************************
**
*/	DEVICE_CMD Transfer_Socket(REBREQ *sock)
/*
**		Write or read a socket (for connection-based protocols).
**
**		This function is asynchronous. It will return immediately.
**		You can call this function again to check the pending connection.
**
**		The mode is RSM_RECEIVE or RSM_SEND.
**
**		The function will return:
**			=0: succeeded
**			>0: in-progress, still trying
**		    <0: error occurred, no longer trying
**
**		Before usage:
**			Open_Socket()
**			Connect_Socket()
**			Verify that RSM_CONNECT is true
**			Setup the sock->data and sock->length
**
**		Note that the mode flag is cleared by the caller, not here.
**
***********************************************************************/
{
	int result;
	long len;
	int mode = (sock->command == RDC_READ ? RSM_RECEIVE : RSM_SEND);

	if (!GET_FLAG(sock->state, RSM_CONNECT)) {
		sock->error = -18;
		return DR_ERROR;
	}

	SET_FLAG(sock->state, mode);

	// Limit size of transfer:
	len = MIN(sock->length, MAX_TRANSFER);

	if (mode == RSM_SEND) {
		// If host is no longer connected:
		result = send(sock->socket, sock->data, len, 0);
		WATCH2("send() len: %d actual: %d\n", len, result);

		if (result >= 0) {
			sock->data += result;
			sock->actual += result;
			if (sock->actual >= sock->length) {
				Signal_Device(sock, EVT_WROTE);
				return DR_DONE;
			}
			return DR_PEND;
		}
		// if (result < 0) ...
	}
	else {
		result = recv(sock->socket, sock->data, len, 0);
		WATCH2("recv() len: %d result: %d\n", len, result);

		if (result > 0) {
			sock->actual = result;
			Signal_Device(sock, EVT_READ);
			return DR_DONE;
		}
		if (result == 0) {		// The socket gracefully closed.
			sock->actual = 0;
			CLR_FLAG(sock->state, RSM_CONNECT); // But, keep RRF_OPEN true
			Signal_Device(sock, EVT_CLOSE);
			return DR_DONE;
		}
		// if (result < 0) ...
	}

	// Check error code:
	result = GET_ERROR;
	WATCH2("get error: %d %s\n", result, strerror(result));
	if (result == NE_WOULDBLOCK) return DR_PEND; // still waiting

	WATCH4("ERROR: recv(%d %x) len: %d error: %d\n", sock->socket, sock->data, len, result);
	// A nasty error happened:
	sock->error = result;
	//Signal_Device(sock, EVT_ERROR);
	return DR_ERROR;
}


/***********************************************************************
**
*/	DEVICE_CMD Listen_Socket(REBREQ *sock)
/*
**		Setup a server (listening) socket (TCP or UDP).
**
**		Before usage:
**			Open_Socket();
**			Set local_port to desired port number.
**
**		Use this instead of Connect_Socket().
**
***********************************************************************/
{
	int result;
	int len = 1;
	SOCKAI sa;

	// Setup socket address range and port:
	Set_Addr(&sa, INADDR_ANY, sock->net.local_port);

	// Allow listen socket reuse:
	result = setsockopt(sock->socket, SOL_SOCKET, SO_REUSEADDR, (char*)(&len), sizeof(len));
	if (result) {
lserr:
		sock->error = GET_ERROR;
		return DR_ERROR;
	}

	// Bind the socket to our local address:
	result = bind(sock->socket, (struct sockaddr *)&sa, sizeof(sa));
	if (result) goto lserr;

	SET_FLAG(sock->state, RSM_BIND);

	// For TCP connections, setup listen queue:
	if (!GET_FLAG(sock->modes, RST_UDP)) {
		result = listen(sock->socket, SOMAXCONN);
		if (result) goto lserr;
		SET_FLAG(sock->state, RSM_LISTEN);
	}

	Get_Local_IP(sock);
	sock->command = RDC_CREATE;	// the command done on wakeup

	return DR_PEND;
}


/***********************************************************************
**
*/	 DEVICE_CMD Accept_Socket(REBREQ *sock)
/*
**		Accept an inbound connection on a TCP listen socket.
**
**		The function will return:
**			=0: succeeded
**			>0: in-progress, still trying
**		    <0: error occurred, no longer trying
**
**		Before usage:
**			Open_Socket();
**			Set local_port to desired port number.
**			Listen_Socket();
**
***********************************************************************/
{
	SOCKAI sa;
	REBREQ *news;
	int len = sizeof(sa);
	int result;
	extern void Attach_Request(REBREQ **prior, REBREQ *req);

	// Accept a new socket, if there is one:
	result = accept(sock->socket, (struct sockaddr *)&sa, &len);

	if (result == BAD_SOCKET) {
		result = GET_ERROR;
		if (result == NE_WOULDBLOCK) return DR_PEND;
		sock->error = result;
		//Signal_Device(sock, EVT_ERROR);
		return DR_ERROR;
	}

	// To report the new socket, the code here creates a temporary
	// request and copies the listen request to it. Then, it stores
	// the new values for IP and ports and links this request to the
	// original via the sock->data.
	news = MAKE_NEW(*news);	// Be sure to deallocate it
	CLEARS(news);
//	*news = *sock;
	news->device = sock->device;

	SET_OPEN(news);
	SET_FLAG(news->state, RSM_OPEN);
	SET_FLAG(news->state, RSM_CONNECT);

	news->socket = result;
	news->net.remote_ip   = sa.sin_addr.s_addr; //htonl(ip); NOTE: REBOL stays in network byte order
	news->net.remote_port = ntohs(sa.sin_port);
	Get_Local_IP(news);

	//Nonblocking_Mode(news->socket);  ???Needed?

	Attach_Request((REBREQ**)&sock->data, news);
	Signal_Device(sock, EVT_ACCEPT);

	// Even though we signalled, we keep the listen pending to
	// accept additional connections.
	return DR_PEND;
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] = {
	Init_Net,
	Quit_Net,
	Open_Socket,
	Close_Socket,
	Transfer_Socket,		// Read
	Transfer_Socket,		// Write
	0,	// poll
	Connect_Socket,
	0,	// query
	0,	// modify
	Accept_Socket,			// Create
	0,	// delete
	0,	// rename
	Lookup_Socket
};

DEFINE_DEV(Dev_Net, "TCP/IP Network", 1, Dev_Cmds, RDC_MAX, sizeof(REBREQ));
