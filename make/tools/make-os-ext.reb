REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Generate OS host API headers"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2025 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: ["Carl Sassenrath" "Oldes"]
	Version: 3.0.0
	Needs: 3.5.0
	Note: {
		Originaly Host was open-sourced part of Rebol3 while Core was closed.
		This script was used to generate headers for both parts.
		Now Host should be part of the Rebol library and so this script
		was simplified just to collect OS_* functions.
	}
]

context [ ; wrapped to prevent colisions with other build scripts

cnt: 0

xlib: make string! 20000

emit:  func [d] [append repend xlib d newline]

func-header: [
	[
		thru "/***" 10 100 "*" newline
		thru "*/"
		copy spec to newline
		(if all [
			spec
			trim spec
			not find spec "static"
			any [  ; make sure we got only functions with "OS_" at the beginning
				find spec " *OS_"
				find spec " OS_"
			]
			find spec #"("
		][
			emit [spec ";    // " the-file]
			cnt: cnt + 1
		]
		)
		newline
		[
			"/*" ; must be in func header section, not file banner
			any [
				thru "**"
				[#" " | #"^-"]
				copy line thru newline
			]
			thru "*/"
			| 
			none
		]
	] | 1 skip
]

process: func [file] [
	data: read-file file
	parse data [
		any func-header
	]
]

foreach file c-host-files [ process file ]

out: rejoin [
	form-header/gen "Host Access Library" %host-lib.h %make-os-ext.reb
	{#define Host_Crash(reason) OS_Crash(cb_cast("REBOL Host Failure"), cb_cast(reason))} LF
	xlib
]

print out ;read-key

if cnt > 0 [
	write-generated root-dir/src/include/host-lib.h out
]

] ; end of context