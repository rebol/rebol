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

; as security was enabled recently, make sure that
; we can access files outside the home dir as before 
secure [ file allow ]

; Change back to the main souce directory:
change-dir %../ ;- make sure you call it just once at start!

; Output directory for temp files:
make-dir temp-dir: join what-dir %tmp/

do %tools/form-header.reb


;-- Options:
verbose: false

platform: none
version:  none
os-name:  none
os-base:  none
product:  none

if exists? opts: temp-dir/tmp-options.reb [
	opts: load opts
	platform: opts/platform
	version:  opts/version
	os-name:  opts/os-name
	os-base:  opts/os-base
	product:  opts/product
]




;-- UTILITIES ----------------------------------------------------------

error: func[msg [string! block!]][
	if block? msg [msg: reform msg]
	make error! msg
]

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

write-if: func [file data] [
	if data <> attempt [read file][
		print ["UPDATE:" file]
		write file data
	]
]

enbase-16: func[data][
	either system/version >= 3.1.0 [
		enbase data 16
	][	enbase/base data 16]
]
