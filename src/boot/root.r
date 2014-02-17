REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Root context"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		Root system values. This context is hand-made very early at boot time
		to allow it to hold key system values during boot up. Most of these
		are put here to prevent them from being garbage collected.
	}
	Note: "See Task Context for per-task globals"
]

self			; (hand-built CONTEXT! value - but, has no WORD table!)
root			; the root context as a block (for GC protection)

system			; system object
errobj			; error object template
strings			; low-level strings accessed via Boot_Strs[] (GC protection)
typesets		; block of TYPESETs used by system; expandable
noneval			; NONE value
noname			; noname function word

boot			; boot block defined in boot.r (GC'd after boot is done)

