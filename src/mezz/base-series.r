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

;; Deprecated - use JOIN which is now functionally equivalent and native.
;; (or ADJOIN if you specifically wish to join one value with a value or series)
rejoin: :join 

;; Deprecated - instead of REPEND X Y use APPEND X REDUCE Y
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

;; Deprecated - instead of REFORM X use FORM REDUCE X
reform: func [
	"Forms a reduced block and returns a string."
	value "Value to reduce and form"
	;/with "separator"
][
	form reduce :value
]

;; Deprecated - instead of REMOLD X use MOLD REDUCE X
remold: func [
	{Reduces and converts a value to a REBOL-readable string.}
	value {The value to reduce and mold}
	/only {For a block value, mold only its contents, no outer []}
	/all  {Mold in serialized format}
	/flat {No indentation}
][
	apply :mold [reduce :value only all flat]
]

;; Deprecated - AJOIN X was really just FORM REDUCE X internally
;; Use ADJOIN {} X or update to be expressed as a JOIN, which
;; is now faster by virtue of being native.
ajoin: func [
	{Reduces and joins a block of values into a new string}
	block [block!]
] [
	adjoin {} block
]