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
	/only {Inserts a series as a series}
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
	value: either series? :value [copy value] [form :value]
	repend value :rest
]

reform: func [
	"Forms a reduced block and returns a string."
	value "Value to reduce and form"
	;/with "separator"
][
	form reduce :value
]
