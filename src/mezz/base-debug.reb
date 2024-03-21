REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Base: Debug Functions"
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

probe: func [
	{Debug print a molded value and returns that same value.}
	value [any-type!] {The output is truncated to size defined in: system/options/probe-limit}
	/local len
][
	len: system/options/probe-limit
	print either 0 < len [
		ellipsize (mold/part :value len + 1) len
	][
		mold :value
	]
	:value
]

??: func [
	{Debug print a word, path, or block of such, followed by its molded value.}
	'name "Word, path or block to obtain values."
][
	unless block? name [name: reduce [name]]
	foreach word name [
		either any [
			word? :word
			path? :word
		][
			prin ajoin ["^[[1;32m" mold :word "^[[0m: ^[[32m"]
			prin try/with [mold/all get/any :word][["^[[1;35mError:" system/state/last-error/id]] 
			print "^[[0m"
		][
			print ajoin ["^[[1;32m" mold/all word "^[[0m"]
		]
	]
	exit
]
