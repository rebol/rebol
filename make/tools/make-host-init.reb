REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Make REBOL host initialization code"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2021 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Package: "REBOL 3 Host Kit"
	Version: 2.0.0
	Needs: 3.5.0
	Author: "Carl Sassenrath"
	Purpose: {
		Build a single init-file from a collection of scripts.
		This is used during the REBOL host startup sequence.
	}
]

context [ ; wrapped to prevent colisions with other build scripts
;** Utility Functions **************************************************

out: make string! 100000
emit: func [data] [repend out data]

emit-head: func [title file] [
	clear out
	emit form-header/gen title file %make-host-init.reb
]

emit-end: func [/easy] [
	if not easy [remove find/last out #","]
	append out {^};^/}
]

;** Main Functions *****************************************************

write-c-file: func [
	c-file
	code
	/local data comp-data comp-size
][
	;print "writing C code..."
	emit-head "Host custom init code" c-file

	data: mold/flat/only/all code

	append data newline ; BUG? why does MOLD not provide it?

	insert data reduce ["; Copyright REBOL Technologies " now newline]
	insert tail data make char! 0 ; zero termination required

	write-generated gen-dir/gen-host-init.reb data

	comp-data: compress/level data 'zlib 9
	comp-size: length? comp-data

	emit ["#define REB_INIT_SIZE " comp-size newline newline]

	emit "const unsigned char Reb_Init_Code[REB_INIT_SIZE] = {^/^-"

	;-- Convert to C-encoded string:
	;print "converting..."
	emit binary-to-c comp-data
	emit-end/easy

	write-generated c-file to-binary out
;	write h-file to-binary reform [
;		form-header "Host custom init header" second split-path h-file newline  
;		"#define REB_INIT_SIZE" comp-size newline
;		"extern REBYTE Reb_Init_Code[REB_INIT_SIZE];" newline
;	]

	;-- Output stats:
	print-info [
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
		file: load-file/header file
		header: file/1
		remove file
		if header/type = 'module [
			file: compose/deep [
				import module
				[
					title:   (header/title)
					version: (header/version)
					name:    (header/name)
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

code: load-files boot-host-files
;probe code

save-generated gen-dir/gen-host-init.reb code
write-c-file root-dir/src/include/host-init.h code

] ; end of context
