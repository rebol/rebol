REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Common make-* code"
]


;-- UTILITIES ----------------------------------------------------------


print-title: func[msg][if block? msg [msg: reform msg] print rejoin ["^/ [pre-make.r3] ^[[1;35m" msg "^[[m"]]
print-info:  func[msg][if block? msg [msg: reform msg] print rejoin [  " [pre-make.r3] ^[[0;32m" msg "^[[m"]]
print-more:  func[msg][if block? msg [msg: reform msg] print rejoin [  " [pre-make.r3] ^[[0;36m" msg "^[[m"]]

read-file: func[file [file!]][
	the-file: find/tail file root-dir ; shortened version
	print rejoin [" [pre-make.r3] ^[[0;36mProcessing: ^[[0;31m" the-file "^[[m"]
	read/string file
]
load-file: func[file [file!] /header][
	the-file: find/tail file root-dir ; shortened version
	print rejoin [" [pre-make.r3] ^[[0;36mProcessing: ^[[0;31m" the-file "^[[m"]
	try/except [either header [load/header/all file][ load file ]][
		sys/log/error 'pre-make.r3 system/state/last-error
		quit/return 3
	]
]


write-generated: func[file data][
	write file data
	print-more ["Generated:^[[33m" to-local-file file]
]
save-generated: func[file data /header hdr][
	either header [
		save/header file data hdr
	][	save file data ]
	print-more ["Generated:^[[33m" to-local-file file]
]



; using checksum mezzanines to be able use build scripts also with old Rebol versions
needs-new-checksum-version?: error? try [checksum #{}]
crc24: func[data [binary!]] either needs-new-checksum-version? [
	[checksum data 'crc24]
][	; old R2 compatible:
	[checksum data]
]
; for some reason extensions are tested agains IP checksum :-/
ipc: func[data [binary!]] either needs-new-checksum-version? [
	[checksum data 'tcp]
][	; old R2 compatible:
	[checksum/tcp data]
]


error: func[msg [string! block!]][
	if block? msg [msg: reform msg]
	make error! msg
]

up-word: func [w] [
	w: uppercase form w
	foreach [f t] [
		#"-" #"_"
	][replace/all w f t]
	w
]

to-c-name: func [word] [
	word: form word
	foreach [f t] [
		#"-" #"_"
		#"." #"_"
		#"?" #"q"
		#"!" #"x"
		#"~" ""
		#"*" "_p"
		#"+" "_add"
		#"|" "or_bar"
	][replace/all word f t]
	word
]

write-if: func [file data] [
	if data <> attempt [read file][
		print ["UPDATE:" file]
		write file data
	]
]

enbase-16: func[data][
	either system/version >= 3.1.0 [
		enbase data 16
	][	enbase/base data 16]
]

binary-to-c: func [comp-data /local out data] [
	out: make string! 4 * (length? comp-data)
	forall comp-data [
		data: copy/part comp-data 16
		comp-data: skip comp-data 15
		data: enbase-16 data
		forall data [
			insert data ",0x"
			data: skip data 4
		]
		data: tail data
		insert data {,^/^-}
		;append out {"}
		append out next head data
	]
	head out
]
