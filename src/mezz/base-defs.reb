REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Base: Other Definitions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Note: {
		This code is evaluated just after actions, natives, sysobj, and other lower
		levels definitions. This file intializes a minimal working environment
		that is used for the rest of the boot.
	}
]

;-- Create the reflector functions (e.g. spec-of, body-of, ...)

; Must be defined in A108 (no forward refs)
spec-of:
body-of:
words-of:
values-of:
types-of:
title-of:
	none

use [word title pos] [
	foreach [name types] system/catalog/reflectors [
		word: make word! ajoin [name "-of"]
		word: bind/new word 'reflect
		title: form types
		remove back tail title ; removes last #"!"
		; there is no replace function in this boot phase yet,
		; so lets do it this way:
		while [pos: find title "any-"][pos/4: #" "] ; replace/all "any-" to "any "
		if pos: find/last title #"!" [change/part pos " or" 1]
		while [pos: find title #"!"][pos/1: #","]   ; replace/all "!" to ","
		insert title ajoin ["Returns a copy of the " name " of "]
		set word func
			reduce [title 'value types]
			compose [reflect :value (to lit-word! name)]
	]
]

decode-url: none ; set in sys init

;-- Setup Codecs -------------------------------------------------------------

foreach [codec handler] system/codecs [
	if handle? handler [
		; Change boot handle (for internal native codecs) into object:
		codec: set codec make object! [			
			name: codec
			type: switch codec [
				text markup      ['text ]
				gif bmp jpeg png ['image]
				wav              ['sound]
			]
			title: form reduce ["Internal codec for" codec "media type"]
			suffixes: select [
				text [%.txt %.cgi]
				markup [%.html %.htm %.xsl %.wml %.sgml %.asp %.php]
			] codec
			entry: handler
		]
		; Media-types block format: [.abc .def type ...]
		if codec/suffixes [ ;append to file-types only if there is any suffix
			append append system/options/file-types codec/suffixes codec/name
		]
	]
]

; Special import case for extensions:
append system/options/file-types switch/default fourth system/version [
	3 [[%.rebx %.dll extension]]  ; Windows
	2 [[%.rebx %.dylib %.so extension]]  ; OS X
	4 7 [[%.rebx %.so extension]]  ; Other Posix
] [[%.rebx extension]]

internal!: make typeset! [
	end! unset! frame! handle!
]

; refinement! and issue! are special words which cannot be set
; all-word!: union any-word! make typeset! [refinement! issue!]

immediate!: make typeset! [
	; Does not include internal datatypes
	none! logic! scalar! date! any-word! datatype! typeset! event!
]

system/options/result-types: make typeset! [
	immediate! series! bitset! image! object! map! gob! handle! struct! ;module!
]

;-- Create "To-Datatype" conversion functions early in bootstrap:

any-block?: func [
	"Return TRUE if value is any type of block."
	value [any-type!]
][find any-block! type? :value]

any-string?: func [
	"Return TRUE if value is any type of string."
	value [any-type!]
][find any-string! type? :value]

any-function?: func [
	"Return TRUE if value is any type of function."
	value [any-type!]
][find any-function! type? :value]

any-word?: func [
	"Return TRUE if value is any type of word."
	value [any-type!]
][find any-word! type? :value]

any-path?: func [
	"Return TRUE if value is any type of path."
	value [any-type!]
][find any-path! type? :value]

any-object?: func [
	"Return TRUE if value is any type of object."
	value [any-type!]
][find any-object! type? :value]

;@@ Oldes: replaced with native version - n-math.c (which handles NaN values too)
;number?: func [
;	"Return TRUE if value is a number (integer or decimal)."
;	value [any-type!]
;][find number! type? :value]

series?: func [
	"Return TRUE if value is any type of series."
	value [any-type!]
][find series! type? :value]

scalar?: func [
	"Return TRUE if value is any type of scalar."
	value [any-type!]
][find scalar! type? :value]

true?: func [
	"Returns true if an expression can be used as true."
	val [any-type!] ;- we want unset! not to fail.
] [not not :val]

quote: func [
	"Returns the value passed to it without evaluation."
	:value [any-type!]
] [
	:value
]
