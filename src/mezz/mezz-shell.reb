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

ls:
dir:    :list-dir
pwd:	:what-dir
rm:		:delete
mkdir:	:make-dir

cd: func [
	"Change directory (shell shortcut function)."
	'path "Accepts %file, :variables and just words (as dirs)"
	/local val
][
	change-dir to-rebol-file switch/default type?/word :path [
		unset!  [return what-dir]
		file!   [get :path]
		string! [path]
		word! path! [
			form either all [
				not error? try [set/any 'val get/any path]
				not any-function? :val
				probe val
			][  val ][ path ]
		]
	][ form path ]
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
