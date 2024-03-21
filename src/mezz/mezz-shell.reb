REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Shell-like Command Functions"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2023 Rebol Open Source Contributors
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
	'name [word! ref! string! email! unset! none!] "User's name"
	/p "Password used to encrypt the data"
	 password [string! binary!]
	/f "Use custom persistent data file location"
	 file [file!]
	/n "Setup a user if does not exists"
	/local su
][
	su: system/user
	if any [none? :name unset? :name] [
		try [update su/data] ;; save changes if there are any
		su/name: none
		su/data: make map! 1
		exit
	]

	sys/log/info 'REBOL ["Initialize user:" as-green :name]

	file: any [
		all [file to-real-file file] ;@@ could to-real-file accept none?
		rejoin [system/options/home #"." :name %.safe]
	]
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

	file: split-path :file
	su/name: either email? :name [:name][to ref! :name]
	su/data: open [
		scheme: 'safe
		pass:   password
		path:   file/1
		target: file/2
	]
]

file-checksum: function [
	"Computes a checksum of a given file's content"
	file   [file!] "Using 256kB chunks"
	method [word!] "One of system/catalog/checksums"
][
	;; it is ok to throw an error on invalid input args
	port: open join checksum:// method 
	file: open/read/seek file
	;; but catch an error when computing the sum,
	;; so we could close the file later
	try [
		while [not empty? bin: read/part file 262144][ write port bin ]
	]
	;; not using try to get none as a result in case of errors
	attempt [
		close file
		read port
	]
]

wait-for-key: func[
	"Wait for single key press and return char (or word for control keys) as a result"
	/only limit [bitset! string! block!] "Limit input to specified chars or control words"
	/local port old-awake
][
	;; using existing input port
	port: system/ports/input
	;; store awake actor and turn off read-line mode
	old-awake: :port/awake
	modify port 'line false ;@@ what if it is already off?
	;; clear old data (in case user cancel's waiting)
	port/data: none
	;; define new awake, which checks single key
	port/awake: func[event][
		all [
			event/key
			any [
				none? limit
				try [ find limit event/key ]
			]
			event/port/data: event/key 
			true
		]
	]
	;; wait for user input
	wait/only port
	;; put back original awake actor and read-line mode
	port/awake: :old-awake
	modify port 'line true
	;; return result and clear port's data
	also port/data port/data: none
]

