REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: To-Type Helpers"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

; These must be listed now, because there is no longer a global context for mezz functions:
; Are we sure we really want all these?? -Carl A108
to-logic: to-integer: to-decimal: to-percent: to-money: to-char: to-pair:
to-tuple: to-time: to-date: to-binary: to-string: to-file: to-email: to-url: to-tag:
to-bitset: to-image: to-vector: to-block: to-paren:
to-path: to-set-path: to-get-path: to-lit-path: to-map: to-datatype: to-typeset:
to-word: to-set-word: to-get-word: to-lit-word: to-refinement: to-issue:
to-command: to-closure: to-function: to-object: to-module: to-error: to-port: to-gob:
to-event:
	none

; Auto-build the functions for the above TO-* words.
use [word] [
	foreach type system/catalog/datatypes [
		; The list above determines what will be made here:
		if in lib word: make word! head remove back tail ajoin ["to-" type] [
			; Add doc line only if this build has autodocs:
			set in lib :word func either string? first spec-of :make [
				reduce [reform ["Converts to" form type "value."] 'value]
			][
				[value]
			] compose [to (type) :value]
		]
	]
]
