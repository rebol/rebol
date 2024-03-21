REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Task context"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		Globals used for each task. Prevents GC of these values.
		See also the Root Context (program-wide globals)
	}
]

self
stack			; data stack
ballast			; current memory ballast (used for GC)
max-ballast		; ballast reset value
this-error		; current error
this-value		; for holding an error argument during throw back
stack-error		; special stack error object
this-context	; current context
buf-emit		; temporary emit output block
buf-words		; temporary word cache
buf-utf8		; UTF8 reused buffer
buf-print		; temporary print output - used by raw print
buf-form		; temporary form buffer - used by raw print
buf-mold		; temporary mold buffer - used by mold
mold-loop		; mold loop detection
err-temps		; error temporaries

