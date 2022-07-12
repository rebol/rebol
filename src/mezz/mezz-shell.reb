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
				not error? try [set 'val get/any path]
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

user's: func[
	"Resolves user's data value"
	'key /local data
][
	all [
		data: select system/user 'data
		select :data :key
	]
]

su: set-user: func[
	"Initialize user's persistent data under system/user"
	'name [word! ref! string! unset!] "User's name"
	/p "Password used to encrypt the data"
	 password [string! binary!]
	/f "Use custom persistent data file location"
	 file [file!]
	/n "Setup a user if does not exists"
	/local su
][
	su: system/user
	if unset? :name [su/name: none su/data: make map! 1 exit]

	sys/log/info 'REBOL ["Initialize user:" as-green :name]
	file: to-real-file any [file rejoin [system/options/home #"." :name %.safe]]
	sys/log/more 'REBOL ["Checking if exists: " as-green file]
	unless exists? file [
		unless n [
			sys/log/error 'REBOL "User's persistent storage file not found!"
			exit
		]
		sys/log/info 'REBOL ["Creating a new persistent storage file:" as-green file]
	]
	password: any [
		password
		get-env "REBOL_SAFE_PASS"
		ask/hide "Enter password: "
	]
	
	if port? su/data [ close su/data ]
	su/name: to ref! :name
	file: split-path :file
	su/data: open [
		scheme: 'safe
		pass:   password
		path:   file/1
		target: file/2
	]
]
