REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Internal Dialect: Rich Text"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Note: "Modification requires recompiling affected source files."
]

system/dialects/text: context [

	type-spec:		[string! tuple!]

	bold:			[logic!]
	italic:			[logic!]
	underline:		[logic!]
	font:			[object!]
	para:			[object!]
	size:			[integer!]
	shadow:			[pair! tuple! integer!]
	scroll:			[pair!]
	drop:			[integer!]
	anti-alias:		[logic!]
	newline:		[]
	caret:			[object!]
	center:			[]
	left:			[]
	right:			[]

	; Aliases
	b:
	i:
	u:
	nl:
		none

]
