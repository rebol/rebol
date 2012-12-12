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
**  Summary: REBOL event definitions
**  Module:  reb-event.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

// Note: size must be 12 bytes!

#pragma pack(4)
typedef struct rebol_event {
	u8  type;		// event id (mouse-move, mouse-button, etc)
	u8  flags;		// special flags
	u8  win;		// window id
	u8  model;		// port, object, gui, callback
	u32 data;		// an x/y position or keycode (raw/decoded)
	union {
		REBREQ *req;	// request (for device events)
		void *ser;		// port or object
	};
} REBEVT;
#pragma pack()

// Special event flags:

enum {
	EVF_COPIED,		// event data has been copied
	EVF_HAS_XY,		// map-event will work on it
	EVF_DOUBLE,		// double click detected
	EVF_CONTROL,
	EVF_SHIFT,
};


// Event port data model

enum {
	EVM_DEVICE,		// I/O request holds the port pointer
	EVM_PORT,		// event holds port pointer
	EVM_OBJECT,		// event holds object frame pointer
	EVM_GUI,		// GUI event uses system/view/event/port
	EVM_CALLBACK,	// Callback event uses system/ports/callback port
};

// Special messages
#define WM_DNS (WM_USER+100)
