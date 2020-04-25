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
	{Inputs a string from the console.}
;	/hide "Mask input with a * character"
][
	if any [
		not port? system/ports/input
		not open? system/ports/input
	][
		system/ports/input: open [scheme: 'console]
	]
	if line: read system/ports/input [ line: to string! line ]
	line
]

ask: func [
	"Ask the user for input."
	question [series!] "Prompt to user"
	/hide "Turns off echoing inputs"
][
	prin question
	either hide [
		prin  "^[[8m"  ; ANSI conceal command
		also input
		print "^[[28m" ; ANSI reveal command
	][	input ]
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

dir-tree: func [
	;----------------------------------------------------------------------
	;-- This function is based on Toomas Vooglaid's %dir-tree.red script --
	;-- https://gist.github.com/toomasv/f2bcf320800ca340379457c1c4036338 --
	;----------------------------------------------------------------------
	"Prints a directory tree"
	'path [file! word! path! string! unset!] "Accepts %file, :variables, and just words (as dirs)"
	/d "Dirs only"
	/i indent [string! char!]
	/l max-depth
	/callback on-value [function!] "Function with [value depth] args - responsible to format value line"
	/local
		; using these as hidden args!
		value prefix changeprefix directory depth
		; --
		newprefix addprefix formed
		filtered contents str
][
	unless value [
		directory: dirize switch type?/word :path [
			unset!      [path: what-dir] ; Stay here
			file!       [path]
			string!     [to-rebol-file path]
			word! path! [to-file path]
		]
		if #"/" <> first directory [insert directory what-dir]
		value: contents: try/except [read directory][
			print ["Not found:" :directory]
			exit
		]
		set [directory value] split-path directory
		prin "^[[31;1m"
	]

	prefix:       any [prefix ""]
	changeprefix: any [changeprefix ""]
	directory: 	  any [directory none]
	depth:        any [depth 0]							
	indent:       any [indent ""]

	if file? value [
		all [
			any [none? max-depth max-depth >= depth]	; is depth limited?
			formed: either :on-value [
				on-value directory/:value depth
			][	join either dir? value ["^[[32;1m"]["^[[33;1m"][value "^[[m"] ]
			print ajoin [indent prefix "[^[[m " formed ]
		]
		all [
			dir? value									; if this is directory
			any [none? max-depth max-depth > depth]		; and and we have to dig deeper
			try [
				contents: read directory/:value			; and it is not a fake directory
				apply :dir-tree [
					path d i indent l max-depth callback :on-value
					/local
					contents
					changeprefix
					changeprefix
					directory/:value
					depth + 1
				]
			]
		]
	]
	unless block? value [exit]

	str: [
		"^[[31;1m├───"
		"^[[31;1m│   "
		"^[[31;1m└───"
		"^[[31;1m    "
	]

	if d [ ; are we considering directories only?
		filtered: make block! length? value
		forall value [
			if dir? value/1 [append filtered value/1]
		]
		value: :filtered
	]						
	forall value [
		either 1 = length? value [							; if this is last element
			newprefix: copy str/3							; set new prefix to 'corner'
			if dir? value/1 [								; and if this is a directory
				changeprefix: append copy prefix copy str/4	; append some empty space to previous prefix for next items
			]
		][													; if this is not last piece
			newprefix: copy str/1							; set new prefix to '|-'
			if dir? value/1 [								; and if this is a directory
				changeprefix: append copy prefix copy str/2	; append '| ' to previous prefix for next items
			]
		]
		addprefix: append copy prefix copy newprefix		; this is printed before the current item
		; send current item to the printing house
		if any [dir? value/1 not d][
			apply :dir-tree [
				path d i indent l max-depth callback :on-value
				/local
				value/1
				copy addprefix
				copy changeprefix
				directory
				depth
			]
		]
	]
	exit
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
