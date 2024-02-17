REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Common make-* code"
]

do %map-conv.reb
;-- UTILITIES ----------------------------------------------------------

needs-map-syntax-swap?: map? transcode/one/error "#()"
map-conv-if-needed: func[data][
	if needs-map-syntax-swap? [map-conv data]
	data
]

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
	if needs-map-syntax-swap? [ file: map-conv read file]
	try/except [either header [load/header/all file][ load file ]][
		sys/log/error 'pre-make.r3 system/state/last-error
		quit/return 3
	]
]


write-generated: func[file data][
	write file data
	print-more ["Generated:^[[33m" to-local-file file]
	if all [
		needs-map-syntax-swap?
		find [%.reb %.r3] suffix? file
	][	map-conv file ]
]
save-generated: func[file data /header hdr][
	either header [
		save/header file data hdr
	][	save file data ]
	print-more ["Generated:^[[33m" to-local-file file]
	if needs-map-syntax-swap? [ map-conv file]
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

string-to-c: func[str /local out][
	out: copy ""
	foreach line split str lf [
		replace/all line #"^"" {\"}
		append out ajoin [{^/^-"} line {\n"\}] 
	]
	take/last out ;; removes the last slash
	out
]

get-libc-version: function[][
	tmp: copy ""
	num: system/catalog/bitsets/numeric
	try [
		;; we may not be sure, if the output will be in the stdout or stderr
		call/output/error/shell/wait "ldd --version" :tmp :tmp
		parse tmp [
			"musl " to end (ver: 'musl)
			|
			thru "GLIBC" some SP copy ver: [some num #"." some num] to end (
				ver: to word! ajoin ["glibc_" ver]
			)
		]
	]
	ver
]

get-os-info: function[
	"Tries to collect information about hosting operating system"
	;@@ Based on this research: https://www.tecmint.com/check-linux-os-version/
][
	tmp: copy  ""
	err: copy  ""
	out: copy #()
	key: charset [#"A"-#"Z" #"_"]
	enl: system/catalog/bitsets/crlf
	whs: system/catalog/bitsets/whitespace
	num: system/catalog/bitsets/numeric
	any [
		;- macOS    
		all [
			system/platform = 'macOS
			0 = call/shell/wait/output/error "sw_vers -productVersion" :tmp :err
			out/ID: 'macos
			out/VERSION_ID: attempt [transcode/one tmp]
		]
		;- Windows  
		all [
			system/platform = 'Windows
			0 = call/shell/wait/output/error "ver" :tmp :err
			parse tmp [
				to num copy v: [some num #"." some num] to end (
					out/ID: 'windows
					out/VERSION_ID: to decimal! v
				)
			]
		]
		;- Linux, OpenBSD, FreeBSD (but not tested on BSD yet)
		all [
			0 = call/shell/wait/output/error "cat /etc/*-release" :tmp :err
			parse tmp [
				any [
					copy k: some key #"=" copy v: to enl some whs (
						try [v: transcode/one v]
						try [parse v ["http" to end (v: as url! v)]]
						put out to word! k v
					)
					|  thru enl
				]
				to end
			]
		]
	]
	out
]

