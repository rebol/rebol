REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Generate auto headers"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Carl Sassenrath"
	Needs: 2.100.100
]

print "------ Building headers"

r3: system/version > 2.100.0

zero-index?: not none? pick next system/platform 0 ;@@ used to be able compile using old R3 versions which were using path/0 instead of path/-1

verbose: false
chk-dups: true
dups: make block! 10000 ; get pick [map! hash!] r3 1000
dup-found: false

do %form-header.r
file-base: load %file-base.r

tmp: context [

change-dir %../core/

count: 0
output:  make string! 20000
natives: make string! 20000
base-code: make string! 2000
insert base-code {REBOL [
	title:    "Rebol base code collected from C sources"
	purpose:  "This is code which must be evaluated just after code from base-defs.r file"
	commment: "AUTO-GENERATED FILE - Do not modify. (From: make-headers.r)"
]
}


emit:    func [d] [append repend output  d newline]
emit-rl: func [d] [append repend rlib    d newline]
emit-n:  func [d] [append repend natives d newline]

emit-header: func [t f] [emit form-header/gen t f %make-headers]
rlib: form-header/gen "REBOL Interface Library" %reb-lib.h %make-headers.r
append rlib newline

emit-n {REBOL [
	Title:   "REBOL automatically collected natives."
	Purpose: {Data in format: [c-name {rebol-specification}]}
	Note:    "AUTO-GENERATED FILE - Do not modify. (From: make-headers.r)"
]}

c-file: none

append-spec: func [spec] [
	;?? spec
	if all [
		spec
		not find spec "static"
		not find spec "scan_state"
		not find spec "REBNATIVE"
		find spec #"("
	][
		spec: trim spec
		either all [
			chk-dups
			find dups spec
		][
			print ["Duplicate:" the-file ":" spec]
			dup-found: true
		][
			append dups spec
		]
		either find spec "RL_API" [
			emit-rl ["extern " spec "; // " the-file]
		][
			emit ["extern " spec "; // " the-file]
		]
		count: count + 1
	]
]

func-header: [
	;-- Scan for function header box:
	"^/**" to newline
	"^/*/" any [#" " | #"^-"]
	copy spec to newline (append-spec spec)
	newline
	[
		"/*" ; must be in func header section, not file banner
		any [
			thru "**"
			[#" " | #"^-"]
			copy  line thru newline
		]
		thru "*/"
		| 
		none
	]
]

ch_func-chars: charset [#"a" - #"z" #"A"]

spec-reb: make string! 1024
name: none

native-header: [
	;-- Scan for native header box:
	"^///" to newline (clear spec-reb)
	any [ "^///" copy line to newline (append append spec-reb line newline)]
	any [#"^/" | #" " | #"^-"]
	"REBNATIVE(" copy name to ")" (probe name
		either any [
			error? try [spec: load spec-reb]
			3 <> length? spec
			error? try [name: load name]
			not word? name
		][
			print "^[[1;32;49m** Invalid NATIVE spec definition found: ^[[0m"
			print spec-reb
			ask "Press ENTER to continue."
		][
			if c-file <> the-file [
				emit-n ["^/;-- " the-file]
				c-file: the-file
			]
			emit-n ["^/" name " {"]
			emit-n trim/head/tail detab spec-reb
			emit-n #"}"
		]
	)
]

sym-chars: charset [#"A" - #"Z" #"_" #"0" - #"9"]
sym-check: charset "/S"
symbols: make block! 256

process: func [file /local sym p comm spec commented?] [
	if verbose [?? file]
	data: read the-file: file
	if r3 [data: deline to-string data]
	parse/all data [
		any [
			thru "/******" to newline
			[
				func-header | native-header | thru newline
			]
		]
	]
	;collect all SYM_* uses
	parse/all/case data [
		any [
			to sym-check p: [
				  "/*" thru "*/"
				| "//" to newline
				| "SYM_" copy sym some sym-chars (
					if not find sym-chars either zero-index? [p/0][p/-1] [
						append symbols sym
					]
				)
				| 1 skip
			]
		]
	]

	;collect Rebol code which may be evaluated on startup
	parse data [
		any [
			;Search only in /*...*/ comments
			thru "^//*" copy comm to "*/" (
				parse/all comm [any [
					thru {^/**} [
						any [#" " | #"^-"]
						"Base-code:"
						any [#" " | #"^-"] newline
						copy spec to "^/*" (
							if not commented? [
								append base-code rejoin [{^/;- code from: } mold file lf lf]
								commented?: true
							]
							append base-code spec
						)
						| to newline
					]
				]]
			)
		]
	]
]

emit-header "Function Prototypes" %funcs.h

;in original source all files in core folder were parsed
;now only the files specified in file-base/core are processed
files: file-base/core

;do
[
	remove find files %a-lib2.c
	print "Non-extended reb-lib version"
	wait 5
]

foreach file files [
	file: to file! file
	if all [
		exists? file
		%.c = suffix? file
		not find/match file "host-"
		not find/match file "os-"
	][process file]
]

symbols: sort unique symbols ;contains all symbols (like: SYM_CALL) used in above processed C files (without the SYM_ part)
symbols: new-line/skip symbols true 1

save/header %../boot/tmp-symbols.r symbols [
	title:    "C Symbols"
	purpose:  "Automaticly collected symbols from C files"
	commment: "AUTO-GENERATED FILE - Do not modify. (From: make-headers.r)"

]
write %../mezz/base-collected.r base-code
write %../boot/tmp-natives.r natives
write %../include/tmp-funcs.h output

print [count "function prototypes"]
;wait 1

;-------------------------------------------------------------------------

clear output

emit-header "Function Argument Enums" %func-args.h

make-arg-enums: func [word] [
	; Search file for definition:
	def: find acts to-set-word word
	def: skip def 2
	args: copy []
	refs: copy []
	; Gather arg words:
	foreach w first def [
		if any-word? w [
			append args uw: uppercase replace/all form to word! w #"-" #"_" ; R3
			if refinement? w [append refs uw  w: to word! w] ; R3
	 	]
	]

	uword: uppercase form word
	replace/all uword #"-" #"_"
	word: lowercase copy uword

	; Argument numbers:
	emit ["enum act_" word "_arg {"]
	emit [tab "ARG_" uword "_0,"]
	foreach w args [emit [tab "ARG_" uword "_" w ","]]
	emit "};^/"

	; Argument bitmask:
	n: 0
	emit ["enum act_" word "_mask {"]
	foreach w args [
		emit [tab "AM_" uword "_" w " = 1 << " n ","]
		n: n + 1
	]
	emit "};^/"

	repend output ["#define ALL_" uword "_REFS ("]
	foreach w refs [
		repend output ["AM_" uword "_" w "|"]
	]
	remove back tail output
	append output ")^/^/"
	
	;?? output halt
]

acts: load %../boot/actions.r

foreach word [
	copy
	find
	select
	insert
	trim
	open
	read
	write
] [make-arg-enums word]

acts: load %../boot/natives.r

foreach word [
	checksum
	request-file
] [make-arg-enums word]

;?? output
write %../include/tmp-funcargs.h output


;-------------------------------------------------------------------------

clear output

emit-header "REBOL Constants Strings" %str-consts.h

data: to string! read %a-constants.c ;R3

parse/all data [
	some [
		to "^/const"
		copy d to "="
		(
			remove d
			;replace d "const" "extern"
			insert d "extern "
			append trim/tail d #";"
			emit d
		)
	]
]

write %../include/tmp-strings.h output

]

if any [dup-found verbose] [
	print "** NOTE ABOVE PROBLEM!"
	wait 5
]

print "   "
