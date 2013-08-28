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
][
	switch type?/word :path [
		unset! [print what-dir]
		file! [change-dir path]
		string! [change-dir to-rebol-file path]
		word! path! [change-dir to-file path]
	]
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
