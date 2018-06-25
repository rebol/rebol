REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Common make-* code"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Carl Sassenrath"
]

do %form-header.r

;-- UTILITIES ----------------------------------------------------------

up-word: func [w] [
	w: uppercase form w
	foreach [f t] [
		#"-" #"_"
	][replace/all w f t]
	w
]

to-c-name: func [word] [
	word: form word
	foreach [f t] [
		#"-" #"_"
		#"." #"_"
		#"?" #"q"
		#"!" #"x"
		#"~" ""
		#"*" "_p"
		#"+" "_add"
		#"|" "or_bar"
	][replace/all word f t]
	word
]
