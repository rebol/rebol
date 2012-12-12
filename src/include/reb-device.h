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
**  Summary: External REBOL Devices (OS Independent)
**  Module:  reb-device.h
**  Author:  Carl Sassenrath
**  Notes:
**      Critical: all struct alignment must be 4 bytes (see compile options)
**
***********************************************************************/

// REBOL Device Identifiers:
// Critical: Must be in same order as Device table in host-device.c
enum {
	RDI_SYSTEM,
	RDI_STDIO,
	RDI_CONSOLE,
	RDI_FILE,
	RDI_EVENT,
	RDI_NET,
	RDI_DNS,
	RDI_CLIPBOARD,
	RDI_MAX,
	RDI_LIMIT = 32
};


// REBOL Device Commands:
enum {
	RDC_INIT,		// init device driver resources
	RDC_QUIT,		// cleanup device driver resources

	RDC_OPEN,		// open device unit (port)
	RDC_CLOSE,		// close device unit

	RDC_READ,		// read from unit
	RDC_WRITE,		// write to unit

	RDC_POLL,		// check for activity
	RDC_CONNECT,	// connect (in or out)

	RDC_QUERY,		// query unit info
	RDC_MODIFY,		// set modes (also get modes)

	RDC_CREATE,		// create unit target
	RDC_DELETE,		// delete unit target
	RDC_RENAME,
	RDC_LOOKUP,
	RDC_MAX,

	RDC_CUSTOM=32	// start of custom commands
};

// Device Request (Command) Return Codes:
#define DR_PEND   1	// request is still pending
#define DR_DONE   0	// request is complete w/o errors
#define DR_ERROR -1	// request had an error

// REBOL Device Flags and Options (bitnums):
enum {
	// Status flags:
	RDF_INIT,		// Device is initialized
	RDF_OPEN,		// Global open (for devs that cannot multi-open)
	// Options:
	RDO_MUST_INIT = 16,	// Do not allow auto init (manual init required)
	RDO_AUTO_POLL,	// Poll device, even if no requests (e.g. interrupts)
};

// REBOL Request Flags (bitnums):
enum {
	RRF_OPEN,		// Port is open
	RRF_DONE,		// Request is done (used when extern proc changes it)
	RRF_FLUSH,		// Flush WRITE
//	RRF_PREWAKE,	// C-callback before awake happens (to update port object)
	RRF_PENDING,	// Request is attached to pending list
	RRF_ALLOC,		// Request is allocated, not a temp on stack
	RRF_WIDE,		// Wide char IO
};

// REBOL Device Errors:
enum {
	RDE_NONE,
	RDE_NO_DEVICE,	// command did not provide device
	RDE_NO_COMMAND,	// command past end
	RDE_NO_INIT,	// device has not been inited
};

enum {
	RDM_NULL,		// Null device
};

#pragma pack(4)

// Forward references:
typedef struct rebol_device REBDEV;
typedef struct rebol_devreq REBREQ;

// Commands:
typedef i32 (*DEVICE_CMD_FUNC)(REBREQ *req);
#define DEVICE_CMD i32 // Used to define

// Device structure:
struct rebol_device {
	char *title;			// title of device
	u32 version;			// version, revision, release
	u32 date;				// year, month, day, hour
	DEVICE_CMD_FUNC *commands;	// command dispatch table
	u32 max_command;		// keep commands in bounds
	REBREQ *pending;		// pending requests
	u32 flags;				// state: open, signal
	i32 req_size;			// size of request struct
};

// Inializer (keep ordered same as above)
#define DEFINE_DEV(w,t,v,c,m,s) REBDEV w = {t, v, 0, c, m, 0, 0, s}

// Request structure:		// Allowed to be extended by some devices
struct rebol_devreq {
	u32 clen;				// size of extended structure

	// Linkages:
	u32 device;				// device id (dev table)
	REBREQ *next;			// linked list (pending or done lists)
	void *port;				// link back to REBOL port object
	union {
		void *handle;		// OS object
		int socket;			// OS identifier
		int id;
	};

	// Command info:
	i32  command;			// command code
	u32  error;				// error code
	u32  modes;				// special modes, types or attributes
	u16  flags;				// request flags
	u16  state;				// device process flags
	i32  timeout;			// request timeout
//	int (*prewake)(void *);	// callback before awake

	// Common fields:
	union {
		REBYTE *data;		// data to transfer
		REBREQ *sock;		// temp link to related socket
	};
	u32  length;			// length to transfer
	u32  actual;			// length actually transferred

	// Special fields for common IO uses:
	union {
		struct {
			REBCHR *path;			// file string (in OS local format)
			i64  size;				// file size
			i64  index;				// file index position
			I64  time;				// file modification time (struct)
		} file;
		struct {
			u32  local_ip;			// local address used
			u32  local_port;		// local port used
			u32  remote_ip;			// remote address
			u32  remote_port;		// remote port
			void *host_info;		// for DNS usage
		} net;
	};
};
#pragma pack()

// Simple macros for common OPEN? test (for some but not all ports):
#define SET_OPEN(r)		SET_FLAG(((REBREQ*)(r))->flags, RRF_OPEN)
#define SET_CLOSED(r)	CLR_FLAG(((REBREQ*)(r))->flags, RRF_OPEN)
#define IS_OPEN(r)		GET_FLAG(((REBREQ*)(r))->flags, RRF_OPEN)
