REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Control"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

launch: func [
	{Runs a script as a separate process; return immediately.}
	script [file! string! none!] "The name of the script"
	/args arg [string! block! none!] "Arguments to the script"
	/wait "Wait for the process to terminate"
	/local exe
][
	if file? script [script: to-local-file any [to-real-file script script]]
	exe: to-local-file system/options/boot

	; Quote everything, just in case it has spaces:
	args: to-string reduce [{"} exe {" "} script {" }]
	if arg [append args arg]
	either wait [call/wait/shell args] [call/shell args]
]

wrap: func [
	"Evaluates a block, wrapping all set-words as locals."
	body [block!] "Block to evaluate"
][
	do bind/copy/set body make object! 0
]

any-of: func [
	"Returns the first value(s) for which the test is not FALSE or NONE."
	'word [word! block!] "Word or block of words to set each time (local)"
	data [series! any-object! map! none!] "The series to traverse"
	test [block!] "Condition to test each time"
][
	if data [
		foreach (word) data reduce [
			:if to paren! test compose [
				(to path! reduce [:break 'return]) (
					either word? word [to get-word! word] [
						reduce [:reduce map-each w word [to get-word! w]]
					]
				)
			]
		]
	]
]

all-of: func [
	"Returns TRUE if all value(s) pass the test, otherwise NONE."
	'word [word! block!] "Word or block of words to set each time (local)"
	data [series! any-object! map! none!] "The series to traverse"
	test [block!] "Condition to test each time"
][
	if data [
		foreach (word) data reduce [
			:unless to paren! test reduce [
				to path! reduce [:break 'return] none
			]
			true
		]
	]
]
