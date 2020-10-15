REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Low-level strings"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		This section holds lower-level C strings used in various parts
		of the system. This section is unique, because it constructs a
		single C string that contains all the specified strings below.
		This is done to eliminate series headers required for normal
		REBOL strings. The strings are referenced using Boot_Strs[RS_*]
		where * is the set-word (and is zero based). For example:
		RS_SCAN+3 refers to "end-of-paren"
	}
]

scan: ; Used by scanner. Keep in sync with Value_Types in scan.h file!
	"end-of-script"
	"line"
	"end-of-block"
	"end-of-paren"
	"word"
	"word-set"
	"word-get"
	"word-lit"
	"none"
	"logic"
	"integer"
	"decimal"
	"percent"
	"money"
	"time"
	"date"
	"char"
	"block"
	"paren"
	"string"
	"binary"
	"pair"
	"tuple"
	"file"
	"email"
	"url"
	"issue"
	"tag"
	"path"
	"refine"
	"construct"

info:
	"Booting..."

;secure:
;	"Script requests permission to "
;	"Script requests permission to lower security level"
;	"REBOL - Security Violation"
;	"unknown"

;secopts:
;	"open a port for read only on: "
;	"open a port for read/write on: "
;	"delete: "
;	"rename: "
;	"make a directory named: "
;	"lower security"
;	"execute a system shell command: "

trace:
	"trace"
	"%-02d: %50r"
	" : %50r"
	" : %s %50m"
	" : %s"
	"--> %s"
	"<-- %s =="
	"Parse match: %r"
	"Parse input: %s"
	"Parse back: %r"
	"**: error : %r %r" ; 10

stack:
	"STACK Expanded - DSP: %d MAX: %d"
	"^/STACK[%d] %s[%d] %s"

dump:
	"^/--REBOL Kernel Dump--"
	"Evaluator:"
	"    Cycles:  %d" ; only lower bits
	"    Counter: %d"
	"    Dose:    %d"
	"    Signals: %x"
	"    Sigmask: %x"
	"    DSP:     %d"
	"    DSF:     %d"
	"Memory/GC:"
	"    Ballast: %d"
	"    Disable: %d"
	"    Protect: %d"
	"    Infants: %d"

;stats:
;	"Stats: bad series value: %d in: %x offset: %d size: %d"

error:
	"out of memory (req %d bytes)"
	"invalid series width %d got %d type %d"
;		"error catalog object out of range"
;		"error num in category out of range"
	"error already caught"
	"stack overflow"
	"I/O error"
	"too many words"
	"word list buffer in use"
	"locked series"
	"error recycled"
	"top level error not caught"
	"error state underflow"
	"event queue overflow (WAIT recursion?)"
	"not available (NA)"

errs:
	" error: "
	"(improperly formatted error)"
	"** Where: "
	"** Near: "

watch:
	"RECYCLING: "
	"%d series"
	"obj-copy: %d %m"

extension:
	"RX_Init"
	"RX_Quit"
	"RX_Call"

;plugin:
;	"cannot open"
;	"missing function"
;	"wrong version"
;	"no header"
;	"bad header"
;	"boot code failed"
