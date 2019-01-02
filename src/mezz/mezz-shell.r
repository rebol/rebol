REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Shell-like Command Functions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

ls:		:list-dir
pwd:	:what-dir
rm:		:delete
mkdir:	:make-dir

cd: func [
	"Change directory (shell shortcut function)."
	'path [file! word! path! unset! string!] "Accepts %file, :variables and just words (as dirs)"
	/local val
][
	switch type?/word :path [
		;unset!  [print what-dir] ;-- looks like now need to print it as what-dir is now as a result
		file!   [change-dir get :path]
		string! [change-dir to-rebol-file path]
		word! path! [
			change-dir either all [
				not error? set/any 'val try [get :path]
				not function? :val
				val
			][  val ][ to-file path ]
		]
	]
	what-dir
]

more: func [
	"Print file (shell shortcut function)."
	'file [file! word! path! string!] "Accepts %file and also just words (as file names)"
][
	print deline to-string read switch type?/word :file [
		file! [file]
		string! [to-rebol-file file]
		word! path! [to-file file]
	]
]
