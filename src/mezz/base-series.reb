REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Base: Series Functions"
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

repend: func [
	"Appends a reduced value to a series and returns the series head."
	series [series! port! map! gob! object! bitset!] {Series at point to insert (modified)}
	value {The value to insert}
	/part {Limits to a given length or position}
	length [number! series! pair!]
	/only {Appends a block value as a block}
	/dup {Duplicates the insert a specified number of times}
	count [number! pair!]
][
	apply :append [series reduce :value part length only dup count]
]

join: func [
	"Concatenates values."
	value "Base value"
	rest "Value or block of values"
][
	append either series? :value [copy value] [form :value] reduce :rest
]

reform: func [
	"Forms a reduced block and returns a string."
	value "Value to reduce and form"
	;/with "separator"
][
	form reduce :value
]

ellipsize: func [
	"Truncate and add ellipsis if str is longer than len"
	str [string!] "(modified)"
	len [integer!] "Max length"
	/one-line "Escape line breaks"
	/local chars
][
	if one-line [
		chars: #[bitset! [not bits #{0024}]]
		parse str [
			any [
				some chars
				| change #"^/" "^^/"
				| change #"^M" "^^M"
			]
		]
	]
	if len < length? str [
		append clear skip str (len - 3) "..."
	]

	str
]
