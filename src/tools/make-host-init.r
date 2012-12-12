REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Make REBOL host initialization code"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Package: "REBOL 3 Host Kit"
	Version: 1.1.1
	Needs: 2.100.100
	Author: "Carl Sassenrath"
	Purpose: {
		Build a single init-file from a collection of scripts.
		This is used during the REBOL host startup sequence.
	}
]

print "--- Make Host Init Code ---"
;print ["REBOL version:" system/version]

; Options:
include-vid: off
proof: off

do %form-header.r

; Output directory for temp files:
dir: %os/

; Files to include in the host program:
files: [
	%mezz/prot-http.r
;	%mezz/view-colors.r
]

vid-files: [
	%mezz/dial-draw.r
	%mezz/dial-text.r
	%mezz/dial-effect.r
	%mezz/view-funcs.r
	%mezz/vid-face.r
	%mezz/vid-events.r
	%mezz/vid-styles.r
	%mezz/mezz-splash.r
]

if include-vid [append files vid-files]

; Change back to the main souce directory:
change-dir %../
make-dir dir

;** Utility Functions **************************************************

out: make string! 100000
emit: func [data] [repend out data]

emit-head: func [title file] [
	clear out
	emit form-header/gen title file %make-host-init.r
]

emit-end: func [/easy] [
	if not easy [remove find/last out #","]
	append out {^};^/}
]

; Convert binary to C code depending on the compiler requirements.
; (Some compilers cannot create long string concatenations.)
binary-to-c: either system/version/4 = 3 [
	; Windows MSVC 6 compatible format (as integer chars):
	func [comp-data /local out] [
		out: make string! 4 * (length? comp-data)
		forall comp-data [
			out: insert out reduce [to-integer first comp-data ", "]
			if zero? ((index? comp-data) // 10) [out: insert out "^/^-"]
		]
		;remove/part out either (pick out -1) = #" " [-2][-4]
		head out
	]
][
	; Other compilers (as hex-escaped char strings "\x00"):
	func [comp-data /local out] [
		out: make string! 4 * (length? comp-data)
		forall comp-data [
			data: copy/part comp-data 16
			comp-data: skip comp-data 15
			data: enbase/base data 16
			forall data [
				insert data "\x"
				data: skip data 3
			]
			data: tail data
			insert data {"^/}
			append out {"}
			append out head data
		]
		head out
	]
]

;** Main Functions *****************************************************

write-c-file: func [
	c-file
	code
	/local data comp-data comp-size
][
	;print "writing C code..."
	emit-head "Host custom init code" c-file

	data: either system/version > 2.7.5 [
		mold/flat/only/all code ; crashes 2.7
	][
		mold/only/all code
	]
	append data newline ; BUG? why does MOLD not provide it?

	insert data reduce ["; Copyright REBOL Technologies " now newline]
	insert tail data make char! 0 ; zero termination required

	if proof [
		write %tmp.r to-binary data
		;ask "wrote tmp.r for proofreading (press return)"
		;probe data
	]

	comp-data: compress data
	comp-size: length? comp-data

	emit ["#define REB_INIT_SIZE " comp-size newline newline]

	emit "const unsigned char Reb_Init_Code[REB_INIT_SIZE] = {^/^-"

	;-- Convert to C-encoded string:
	;print "converting..."
	emit binary-to-c comp-data
	emit-end/easy

	print ["writing" c-file]
	write c-file to-binary out
;	write h-file to-binary reform [
;		form-header "Host custom init header" second split-path h-file newline  
;		"#define REB_INIT_SIZE" comp-size newline
;		"extern REBYTE Reb_Init_Code[REB_INIT_SIZE];" newline
;	]

	;-- Output stats:
	print [
		newline
		"Compressed" length? data "to" comp-size "bytes:"
		to-integer (comp-size / (length? data) * 100)
		"percent of original"
	]

	return comp-size
]

load-files: func [
	file-list
	/local data
][
	data: make block! 100
	;append data [print "REBOL Host-Init"] ; for startup debug only
	foreach file file-list [
		print ["loading:" file]
		file: load/header file
		header: file/1
		remove file
		if header/type = 'module [
			file: compose/deep [
				import module
				[
					title: (header/title)
					version: (header/version)
					name: (header/name)
				][
					(file)
				]
			]
			;probe file/2
		]
		append data file
	]
	data
]

code: load-files files

save %boot/host-init.r code

write-c-file %include/host-init.h code

print "   " ; (separate the output for build watch window)
