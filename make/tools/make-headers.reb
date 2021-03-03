REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Generate auto headers"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2021 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: ["Carl Sassenrath" "Oldes"]
	Needs: 3.5.0
]

chk-dups?: true
dups: make block! 1000
dup-found: false

context [
	count: 0
	spec: none
	output:    make string! 20000
	natives:   make string! 20000
	base-code: make string! 100000
	insert base-code
{REBOL [
	title:    "Rebol base code collected from C sources"
	purpose:  "This is code which must be evaluated just after code from base-defs.reb file"
	commment: "AUTO-GENERATED FILE - Do not modify. (From: pre-make.r3 [make-headers])"
]}

	emit:    func [d] [append repend output  d newline]
	emit-rl: func [d] [append repend rlib    d newline]
	emit-n:  func [d] [append repend natives d newline]

	emit-header: func [t f] [emit form-header/gen t f %pre-make]
	rlib: form-header/gen "REBOL Interface Library" %reb-lib.h %pre-make.r3
	append rlib newline

	emit-n
{REBOL [
	Title:   "REBOL automatically collected natives."
	Purpose: {Data in format: [c-name {rebol-specification}]}
	Note:    "AUTO-GENERATED FILE - Do not modify. (From: pre-make.r3 [make-headers])"
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
				chk-dups?
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

	ch_func-chars: charset [#"a" - #"z" #"A" - #"Z" "_!?-" #"0" - #"9"] ;just limited set!
	spec-reb: make string! 1024
	name: s: e: none

	emit-native-spec: func[
		spec-c
		spec-rebol
		/local spec name
	][
		parse spec-c [
			thru "REBNATIVE(" copy name to ")"
		]

		;trim leading chars from the rebol code block
		parse spec-rebol [
			["**" | " *" | "//"] (
				replace/all spec-rebol join "^/" (take/part spec-rebol 2) #"^/"
			) 
		]

		if any [
			error? try [spec: load spec-rebol]
			3 <> length? spec
			error? try [name: load name]
			not word? name
			(form name) <> (to-c-name to word! first spec)
		][
			print ["^[[1;32;49m** In file: ^[[0m" the-file]
			print "^[[1;32;49m** Invalid NATIVE spec definition found: ^[[0m"
			print spec-rebol
			prin  "^[[1;32;49m** For C spec:^[[0m^/^-"
			print spec-c
			ask "^/Press ENTER to continue."
			exit
		]

		if c-file <> the-file [
			emit-n ["^/;-- " the-file]
			c-file: the-file
		]
		emit-n #"^/"
		emit-n trim/head/tail detab spec-rebol
	]

	func-header: [
		;-- Scan for function header box:
		"^/**" to newline
		"^/*/" any [#" " | #"^-"]
		copy spec to newline (append-spec spec)
		newline
		[
			"/*" ; must be in func header section, not file banner
			[
				thru newline s:
				opt  ["**" | " *" | "//"]
				some [#" " | #"^-"] some ch_func-chars #":" some [#" " | #"^-"] "native" any [#" " | #"^-"] #"[" thru newline
				to "^/****" e: (
					emit-native-spec spec copy/part s e
				)
			]
			|
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


	;@@ Remove this rule later... let's use just the new style
	native-header: [
		;-- Scan for native header box:
		"^///" to newline (clear spec-reb)
		any [ "^///" copy line to newline (append append spec-reb line newline)]
		any [#"^/" | #" " | #"^-"]
		"REBNATIVE(" copy name to ")" (probe name
			print ["^[[1;32;49m** In file: ^[[0m" the-file]
			print "^[[1;32;49m** Found deprecated NATIVE spec definition: ^[[0m"
			print spec-reb
			ask "^/Press ENTER to continue."
		)
	]
	;@@-------------------------------------------------------

	sym-chars: charset [#"A" - #"Z" #"_" #"0" - #"9"]
	sym-check: charset "/S"
	symbols: make block! 256

	process: func [file /local sym p comm spec commented?] [
		try/except [data: read-file file][
			print-info ["File not found:" file]
			exit
		]

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
						if not find sym-chars p/-1 [
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
				thru "/*" copy comm to "*/" (
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

	foreach file c-core-files [ process file ] 

	symbols: sort unique symbols ;contains all symbols (like: SYM_CALL) used in above processed C files (without the SYM_ part)
	assert [not empty? symbols]
	symbols: new-line/skip symbols true 1
	if verbose [? symbols]
	save-generated/header gen-dir/gen-symbols.reb symbols [
		title:    "C Symbols"
		purpose:  "Automaticly collected symbols from C files"
		commment: "AUTO-GENERATED FILE - Do not modify. (From: pre-make.r3 [make-headers])"
	]
	write-generated root-dir/src/mezz/base-collected.reb base-code
	write-generated gen-dir/gen-natives.reb natives
	write-generated root-dir/src/include/gen-funcs.h output

	print-info ["Function prototypes found:" as-yellow count]

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

	acts: load root-dir/src/boot/actions.reb

	foreach word [
		copy
		find
		put
		select
		insert
		trim
		open
		read
		write
		query
		take
		remove
	] [make-arg-enums word]

	acts: load root-dir/src/boot/natives.reb
	append acts load gen-dir/gen-natives.reb

	foreach word [
		checksum
		request-file
		request-dir
	] [make-arg-enums word]

;?? output
	write-generated root-dir/src/include/gen-funcargs.h output


	;-------------------------------------------------------------------------

	clear output

	emit-header "REBOL Constants Strings" %str-consts.h

	data: read/string root-dir/src/core/a-constants.c

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

	write-generated root-dir/src/include/gen-strings.h output

] ; tmp context end

if dup-found [
	print "***** NOTE ABOVE PROBLEM! (duplicate found)"
	wait 5
]

;print "[DONE headers]^/"
