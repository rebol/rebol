REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Port & console modes"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2024 Rebol Open Source Developers
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

*port-modes* [
	created
	accessed
	modified
	owner-read
	owner-write
	owner-execute
	group-read
	group-write
	group-execute
	world-read
	world-write
	world-execute
]

*console-modes* [
	echo
	line
	error
]

*serial-modes* [
	brk ;break
	dtr ;data-terminal-ready
	rts ;request-to-send
]

*audio-modes* [
	play  ; used to pass play/pause commands into audio device
]

