REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: File Related"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

clean-path: func [
	"Returns new directory path with //, . and .. processed."
	file [file! url! string!]
	/only "Do not prepend current directory"
	/dir "Add a trailing / if missing"
	/local out cnt f
][
	case [
		any [only not file? file] [file: copy file]
		#"/" = first file [
			++ file
			out: next what-dir
			while [
				all [
					#"/" = first file
					f: find/tail out #"/"
				]
			][
				++ file
				out: f
			]
			file: append clear out file
		]
		file: append what-dir file
	]

	if all [dir not dir? file] [append file #"/"]

	out: make file length? file ; same datatype
	cnt: 0 ; back dir counter

	parse/all reverse file [
		some [
			;pp: (?? pp)
			"../" (++ cnt)
			| "./"
			| #"/" (if any [not file? file #"/" <> last out] [append out #"/"])
			| copy f [to #"/" | to end] (
				either cnt > 0 [
					-- cnt
				][
					unless find ["" "." ".."] to string! f [append out f]
				]
			)
		]
	]

	if all [#"/" = last out #"/" <> last file] [remove back tail out]
	reverse out
]

input: function [
	{Inputs a string from the console. New-line character is removed.}
;	/hide "Mask input with a * character"
][
	if any [
		not port? system/ports/input
		not open? system/ports/input
	][
		system/ports/input: open [scheme: 'console]
	]
	line: to-string read system/ports/input
	trim/with line newline
	line
]

ask: func [
	"Ask the user for input."
	question [series!] "Prompt to user"
	/hide "mask input with *"
][
	prin question
	trim either hide [input/hide] [input]
]

confirm: func [
	"Confirms a user choice."
	question [series!] "Prompt to user"
	/with choices [string! block!]
	/local response
][
	if all [block? choices 2 < length? choices] [
		cause-error 'script 'invalid-arg mold choices
	]
	response: ask question
	unless with [choices: [["y" "yes"] ["n" "no"]]]
	case [ ; returned
		empty? choices [true]
		string? choices [if find/match response choices [true]]
		2 > length? choices [if find/match response first choices [true]]
		find first choices response [true]
		find second choices response [false]
	]
]

list-dir: func [
	"Print contents of a directory (ls)."
	'path [file! word! path! string! unset!] "Accepts %file, :variables, and just words (as dirs)"
	/l "Line of info format"
	/f "Files only"
	/d "Dirs only"
;	/t "Time order"
	/r "Recursive"
	/i indent
	/local files save-dir info
][
	save-dir: what-dir
	switch type?/word :path [
		unset! [] ; Stay here
		file! [change-dir path]
		string! [change-dir to-rebol-file path]
		word! path! [change-dir to-file path]
	]
	if r [l: true]
	unless l [l: make string! 62] ; approx width
	unless indent [indent: ""]
	files: attempt [read %./]
	if not files [print ["Not found:" :path] change-dir save-dir exit]
	foreach file files [
		if any [
			all [f dir? file]
			all [d not dir? file]
		][continue]
		either string? l [
			append l file
			append/dup l #" " 15 - remainder length? l 15
			if greater? length? l 60 [print l clear l]
		][
			info: get query file
			change info second split-path info/1
			printf [indent 16 -8 #" " 24 #" " 6] info
			if all [r dir? file] [
				list-dir/l/r/i :file join indent "    "
			]
		]
	]
	if all [string? l not empty? l] [print l]
	change-dir save-dir
	exit
]

undirize: func [
	{Returns a copy of the path with any trailing "/" removed.}
	path [file! string! url!]
][
	path: copy path
	if #"/" = last path [clear back tail path]
	path
]

in-dir: func [
	"Evaluate a block while in a directory."
	dir [file!] "Directory to change to (changed back after)"
	block [block!] "Block to evaluate"
	/local old-dir
] [
	old-dir: what-dir
	change-dir dir
	also do block change-dir old-dir
] ; You don't want the block to be done if the change-dir fails, for safety.

to-relative-file: func [
	"Returns the relative portion of a file if in a subdirectory, or the original if not."
	file [file! string!] "File to check (local if string!)"
	/no-copy "Don't copy, just reference"
	/as-rebol "Convert to REBOL-style filename if not"
	/as-local "Convert to local-style filename if not"
] [
	either string? file [ ; Local file
		; Note: to-local-file drops trailing / in R2, not in R3
		; if tmp: find/match file to-local-file what-dir [file: next tmp]
		file: any [find/match file to-local-file what-dir  file]
		if as-rebol [file: to-rebol-file file  no-copy: true]
	] [
		file: any [find/match file what-dir  file]
		if as-local [file: to-local-file file  no-copy: true]
	]
	unless no-copy [file: copy file]
	file
]
