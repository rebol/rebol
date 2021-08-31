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
					unless find ["" "." ".."] as string! f [append out f]
				]
			)
		]
	]

	if all [#"/" = last out #"/" <> last file] [remove back tail out]
	reverse out
]

wildcard: func [
	"Return block of absolute path files filtered using wildcards."
	path  [file!]       "Source directory"
	value [any-string!] "Search value with possible * and ? wildcards"
	/local result
][
	result: make block! 8
	path: clean-path/dir path
	foreach file read path [
		if find/match/any file value [ append result path/:file ]
	]
	new-line/all result true
]

input: func [
	{Inputs a string from the console.}
	/hide "Turns off echoing inputs"
	/local line port
][
	port: system/ports/input
	if any [
		not port? port
		not open? port
	][
		system/ports/input: port: open [scheme: 'console]
	]
	if hide [ modify port 'echo false ]
	if line: read port [ line: to string! line ]
	if hide [ modify port 'echo true ]
	line
]

ask: func [
	"Ask the user for input."
	question [series!] "Prompt to user"
	/hide "Turns off echoing inputs"
	/char "Waits only on single key press and returns char as a result"
][
	prin question
	also apply :input [hide] prin LF
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
			][	join either dir? value [" ^[[32;1m"][" ^[[33;1m"][value "^[[m"] ]
			print ajoin [indent prefix "[^[[m" formed ]
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

list-dir: closure/with [
	"Print contents of a directory (ls)."
	'path [file! word! path! string! unset!] "Accepts %file, :variables, and just words (as dirs)"
	/f "Files only"
	/d "Dirs only"
	/r "Recursive"
	/i indent [string! char!]
	/l "Limit recursive output to given maximal depth"
		max-depth [integer!] 
][
	if f [r: l: false]
	recursive?: any [r max-depth]
	files-only?: f
	apply :dir-tree [
		:path d i indent
		true either recursive? [:max-depth][1]
		true :on-value
		i indent
	]
][
	recursive?: files-only?: none
	on-value: func[
		value depth
		/local info date time size
	][
		info: query/mode value [name size date]
		if depth = 0 [
			return ajoin ["^[[33;1mDIR: ^[[32;1m" to-local-file info/1 "^[[m"]
		]
		;@@ TODO: rewrite this date/time formating once it will be possible
		;@@       with some better method!
		date: info/3
		date/zone: 0
		time: date/time
		time: format/pad [2 #":" 2 ] reduce [time/hour time/minute] #"0"
		date: format/pad [-11] date/date #"0"
		date: ajoin [" ^[[32m" date "  " time "^[[m "]

		size: any [info/2 0]
		if size >= 100'000'000 [size: join to integer! round (size / 1'000'000) "M"]

		either dir? value [
			if files-only? [return none]
			ajoin [
				date "^[[32;1m"
				either recursive? [
					to-local-file info/1
				][  join "         " dirize second split-path info/1]
				"^[[m"
			]
		][
			format [date $33 -8 $0 #" "] reduce [
				size
				"^[[33;1m"
				second split-path info/1
				"^[[m"
			]
		]
	]
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
	also
		attempt [do block]
		change-dir old-dir
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
		file: any [find/match/tail file to-local-file what-dir  file]
		if as-rebol [file: to-rebol-file file  no-copy: true]
	] [
		file: any [find/match/tail file what-dir  file]
		if as-local [file: to-local-file file  no-copy: true]
	]
	unless no-copy [file: copy file]
	file
]
