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

launch: function/with [
	{Runs a script as a separate process; return immediately.}
	script [file! string!] "The name of the script"
	/with args [string! block! none!] "Arguments to the script"
	/wait "Wait for the process to terminate"
][
	command: reduce [system/options/boot script]
	if args [
		unless block? args [args: reduce [:args]]
		foreach arg args [
			;; arguments are expected to be strings...
			append command escape-arg mold/flat arg
		]
	]
	sys/log/info 'REBOL ["Launch:" as-green reform next command]
	call/:wait command
][
	;-- just a simple argument escaping function
	;@@ needs test on other platforms... Linux seems to be ok without any escaping.
	escape-arg: func[arg] either/only system/platform = 'Windows [
		replace/all arg #"^"" {\"}
		append insert arg #"^"" #"^"" 
	][ arg ]
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
