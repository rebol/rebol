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
**  Summary: Network device definitions
**  Module:  reb-net.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

// REBOL Socket types:
enum socket_types {
	RST_UDP,					// TCP or UDP
	RST_LISTEN = 8,				// LISTEN
	RST_REVERSE,				// DNS reverse
};

// REBOL Socket Modes (state flags)
enum {
	RSM_OPEN = 0,				// socket is allocated
	RSM_ATTEMPT,				// attempting connection
	RSM_CONNECT,				// connection is open
	RSM_BIND,					// socket is bound to port
	RSM_LISTEN,					// socket is listening (TCP)
	RSM_SEND,					// sending
	RSM_RECEIVE,				// receiving
	RSM_ACCEPT,					// an inbound connection
};

#define IPA(a,b,c,d) (a<<24 | b<<16 | c<<8 | d)
