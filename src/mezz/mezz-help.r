REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Help"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

; MOVE THIS INTERNAL FUNC:
dump-obj: function [
	"Returns a block of information about an object or port."
	obj [object! port!]
	/match "Include only those that match a string or datatype" pat
][
	clip-str: func [str] [
		; Keep string to one line.
		trim/lines str
		if (length? str) > 45 [str: append copy/part str 45 "..."]
		str
	]

	form-val: func [val] [
		; Form a limited string from the value provided.
		if any-block? :val [return reform ["length:" length? val]]
		if image? :val [return reform ["size:" val/size]]
		if datatype? :val [return get in spec-of val 'title]
		if any-function? :val [
			return clip-str any [title-of :val mold spec-of :val]
		]
		if object? :val [val: words-of val]
		if typeset? :val [val: to-block val]
		if port? :val [val: reduce [val/spec/title val/spec/ref]]
		if gob? :val [return reform ["offset:" val/offset "size:" val/size]]
		clip-str mold :val
	]

	form-pad: func [val size] [
		; Form a value with fixed size (space padding follows).
		val: form val
		insert/dup tail val #" " size - length? val
		val
	]

	; Search for matching strings:
	out: copy []
	wild: all [string? pat  find pat "*"]

	foreach [word val] obj [
		type: type?/word :val
		str: either find [function! closure! native! action! op! object!] type [
			reform [word mold spec-of :val words-of :val]
		][
			form word
		]
		if any [
			not match
			all [
				not unset? :val
				either string? :pat [
					either wild [
						tail? any [find/any/match str pat pat]
					][
						find str pat
					]
				][
					all [
						datatype? get :pat
						type = :pat
					]
				]
			]
		][
			str: join "^[[1;32m" form-pad word 15
			append str "^[[m "
			append str form-pad type 24 - ((length? str) - 15)
			append out reform [
				"  " str "^[[32m"
				if type <> 'unset! [form-val :val]
				"^[[m^/"
			]
		]
	]
	out
]

?: help: func [
	"Prints information about words and values."
	'word [any-type!]
	/doc "Open web browser to related documentation."
	/local value args item type-name types tmp print-args
][
	if unset? get/any 'word [
		print trim/auto {
			Use ^[[1;32mHELP^[[m or ^[[1;32m?^[[m to see built-in info:
			^[[1;32m
				help insert
				? insert
			^[[m
			To search within the system, use quotes:
			^[[1;32m
				? "insert"
			^[[m
			To browse online web documents:
			^[[1;32m
				help/doc insert
			^[[m
			To view words and values of a context or object:

				^[[1;32m? lib^[[m    - the runtime library
				^[[1;32m? self^[[m   - your user context
				^[[1;32m? system^[[m - the system object
				^[[1;32m? system/options^[[m - special settings

			To see all words of a specific datatype:
			^[[1;32m
				? native!
				? function!
				? datatype!
			^[[m
			Other debug functions:

				^[[1;32m??^[[m      - display a variable and its value
				^[[1;32mprobe^[[m   - print a value (molded)
				^[[1;32msource^[[m  - show source code of func
				^[[1;32mtrace^[[m   - trace evaluation steps
				^[[1;32mwhat^[[m    - show a list of known functions

			Other information:

				^[[1;32mabout^[[m   - see general product info
				^[[1;32mlicense^[[m - show user license
				^[[1;32musage^[[m   - program cmd line options
		}
		exit
	]

;			Word completion:
;
;				The command line can perform word
;				completion. Type a few chars and press TAB
;				to complete the word. If nothing happens,
;				there may be more than one word that
;				matches. Press TAB again to see choices.
;
;				Local filenames can also be completed.
;				Begin the filename with a %.
;
;			Other useful functions:
;
;				about - see general product info
;				usage - view program options
;				license - show terms of user license
;				source func - view source of a function
;				upgrade - updates your copy of REBOL
;
;			More information: http://www.rebol.com/docs.html

	; If arg is an undefined word, just make it into a string:
	if all [word? :word not value? :word] [word: mold :word]

	; Open the web page for it?
	if all [
		doc
		word? :word
		any [any-function? get :word datatype? get :word]
	][
		item: mold :word
		either any-function? get :word [
			foreach [a b] [ ; need a better method !
				"!" "-ex"
				"?" "-q"
				"*" "-mul"
				"+" "-plu"
				"/" "-div"
				"=" "-eq"
				"<" "-lt"
				">" "-gt"
			][replace/all item a b]
			tmp: http://www.rebol.com/r3/docs/functions/
		][
			tmp: http://www.rebol.com/r3/docs/datatypes/
			remove back tail item ; the !
		]
		browse join tmp [item ".html"]
	]

	; If arg is a string or datatype! word, search the system:
	if any [string? :word all [word? :word datatype? get :word]] [
		if all [word? :word datatype? get :word] [
			value: spec-of get :word
			print ajoin [
				"^[[1;32m" mold :word "^[[m is a datatype" newline
				"It is defined as " either find "aeiou" first value/title ["an "] ["a "] value/title newline
				"It is of the general type ^[[1;32m" value/type "^[[m^/"
			]
		]
		if any [:word = 'unset! not value? :word] [exit]
		types: dump-obj/match lib :word
		sort types
		if not empty? types [
			print ["Found these related words:^[[32m" newline types "^[[m"]
			exit
		]
		if all [word? :word datatype? get :word] [
			print ["No values defined for^[[1;32m" word "^[[m"]
			exit
		]
		print ["No information on^[[1;32m" word "^[[m"]
		exit
	]

	; Print type name with proper singular article:
	type-name: func [value] [
		value: mold type? :value
		clear back tail value ;removes the ! char from datatype name
		join either find "aeiou" first value ["an "]["a "] value
	]

	; Print literal values:
	if not any [word? :word path? :word][
		print ajoin ["^[[1;32m" mold :word "^[[m is " type-name :word]
		exit
	]

	; Get value (may be a function, so handle with ":")
	either path? :word [
		if error? set/any 'value try [get :word][
			;check if value is error or if it was really an invalid or path without value
			if all [
				value/id   = 'invalid-path
				value/arg1 = :word
			][
				print ["There is no" value/arg2 "in path" value/arg1]
				exit
			]
			if all [
				value/id = 'no-value
				value/arg1 = first :word
			][
				print ["No information on" word "(path has no value)"]
				exit
			]
		]
	][
		value: get :word
	]
	unless any-function? :value [
		prin ajoin ["^[[1;32m" uppercase mold word "^[[m is " type-name :value " of value: ^[[32m"]
		prin either any [object? value port? value]  [print "" dump-obj value][mold/all :value]
		print "^[[m"
		exit
	]

	; Must be a function...
	; If it has refinements, strip them:
	;if path? :word [word: first :word]

	;-- Print info about function:
	prin "^[[4;1;36mUSAGE^[[m:^/^-"

	args: words-of :value
	clear find args /local
	either op? :value [
		print [args/1 word args/2]
	][
		print [uppercase mold word mold/only args]
	]

	print ajoin [
		newline "^[[4;1;36mDESCRIPTION^[[m:" newline
		tab any [title-of :value "(undocumented)"] newline
		tab uppercase mold word " is " type-name :value " value."
	]

	unless args: find spec-of :value any-word! [exit]
	clear find args /local

	;-- Print arg lists:
	print-args: func [label list /extra /local str] [
		if empty? list [exit]
		print label
		foreach arg list [
			str: ajoin [tab mold arg/1]
			if all [extra word? arg/1] [insert str tab]
			if arg/2 [append append str " -- " arg/2]
			if all [arg/3 not refinement? arg/1] [
				repend str [" (" arg/3 ")"]
			]
			print str
		]
	]

	use [argl refl ref b v] [
		argl: copy []
		refl: copy []
		ref: b: v: none

		parse args [
			any [string! | block!]
			any [
				set word [refinement! (ref: true) | any-word!]
				(append/only either ref [refl][argl] b: reduce [word none none])
				any [set v block! (b/3: v) | set v string! (b/2: v)]
			]
		]

		print-args "^/^[[4;1;36mARGUMENTS^[[m:" argl
		print-args/extra "^/^[[4;1;36mREFINEMENTS^[[m:" refl
	]

	exit ; return unset
]

about: func [
	"Information about REBOL"
][
	print make-banner sys/boot-banner
]

;		--cgi (-c)       Load CGI utiliy module and modes

usage: func [
	"Prints command-line arguments."
][
	print {
    ^[[4;1;36mCommand line usage^[[m:
    
        ^[[1;32mREBOL |options| |script| |arguments|^[[m
    
    ^[[4;1;36mStandard options^[[m:
    
        ^[[1;32m--args data^[[m      Explicit arguments to script (quoted)
        ^[[1;32m--do expr^[[m        Evaluate expression (quoted)
        ^[[1;32m--help (-?)^[[m      Display this usage information
        ^[[1;32m--script file^[[m    Explicit script filename
        ^[[1;32m--version tuple^[[m  Script must be this version or greater
    
    ^[[4;1;36mSpecial options^[[m:
    
        ^[[1;32m--boot level^[[m     Valid levels: base sys mods
        ^[[1;32m--debug flags^[[m    For user scripts (system/options/debug)
        ^[[1;32m--halt (-h)^[[m      Leave console open when script is done
        ^[[1;32m--import file^[[m    Import a module prior to script
        ^[[1;32m--quiet (-q)^[[m     No startup banners or information
        ^[[1;32m--secure policy^[[m  Can be: none allow ask throw quit
        ^[[1;32m--trace (-t)^[[m     Enable trace mode during boot
        ^[[1;32m--verbose^[[m        Show detailed startup information
    
    ^[[4;1;36mOther quick options^[[m:
    
        ^[[1;32m-s^[[m               No security
        ^[[1;32m+s^[[m               Full security
        ^[[1;32m-v^[[m               Display version only (then quit)
    
    ^[[4;1;36mExamples^[[m:
    
        REBOL script.r
        REBOL -s script.r
        REBOL script.r 10:30 test@example.com
        REBOL --do "watch: on" script.r}
]

license: func [
	"Prints the REBOL/core license agreement."
][
	print system/license
]

source: func [
	"Prints the source code for a word."
	'word [word! path!]
][
	if not value? word [print [word "undefined"] exit]
	print head insert mold get word reduce [word ": "]
	exit
]

what: func [
	{Prints a list of known functions.}
	'name [word! lit-word! unset!] "Optional module name"
	/args "Show arguments not titles"
	/local ctx vals arg list size
][
	list: make block! 400
	size: 10 ; defines minimal function name padding

	ctx: any [select system/modules :name lib]

	foreach [word val] ctx [
		if any-function? :val [
			arg: either args [
				arg: words-of :val
				clear find arg /local
				mold arg
			][
				title-of :val
			]
			append list reduce [word arg]
			size: max size length? word
		]
	]
	size: min size 18 ; limits function name padding to be max 18 chars
	vals: make string! size
	foreach [word arg] sort/skip list 2 [
		append/dup clear vals #" " size
		print rejoin ["^[[1;32m" head change vals word "^[[0m " any [arg ""]]
	]
	exit
]

;pending: does [
;	comment "temp function"
;	print "Pending implementation."
;]
;
;say-browser: does [
;	comment "temp function"
;	print "Opening web browser..."
;]
;
;upgrade: function [
;	"Check for newer versions (update REBOL)."
;][
;	print "Fetching upgrade check ..."
;	if error? err: try [do http://www.rebol.com/r3/upgrade.r none][
;		either err/id = 'protocol [print "Cannot upgrade from web."][do err]
;	]
;	exit
;]
;
;chat: function [
;	"Open REBOL DevBase forum/BBS."
;][
;	print "Fetching chat..."
;	if error? err: try [do http://www.rebol.com/r3/chat.r none][
;		either err/id = 'protocol [print "Cannot load chat from web."][do err]
;	]
;	exit
;]
;
;docs: func [
;	"Browse on-line documentation."
;][
;	say-browser
;	browse http://www.rebol.com/r3/docs
;	exit
;]
;
;bugs: func [
;	"View bug database."
;][
;	say-browser
;	browse http://curecode.org/rebol3/
;	exit
;]
;
;changes: func [
;	"What's new about this version."
;][
;	say-browser
;	browse http://www.rebol.com/r3/changes.html
;	exit
;]
;
;why?: func [
;	"Explain the last error in more detail."
;	'err [word! path! error! none! unset!] "Optional error value"
;][
;	case [
;		unset? :err [err: none]
;		word? err [err: get err]
;		path? err [err: get err]
;	]
;
;	either all [
;		error? err: any [:err system/state/last-error]
;		err/type ; avoids lower level error types (like halt)
;	][
;		say-browser
;		err: lowercase ajoin [err/type #"-" err/id]
;		browse join http://www.rebol.com/r3/docs/errors/ [err ".html"]
;	][
;		print "No information is available."
;	]
;	exit
;]
;
;demo: function [
;	"Run R3 demo."
;][
;	print "Fetching demo..."
;	if error? err: try [do http://www.rebol.com/r3/demo.r none][
;		either err/id = 'protocol [print "Cannot load demo from web."][do err]
;	]
;	exit
;]
;
;load-gui: function [
;	"Download current GUI module from web. (Temporary)"
;][
;	print "Fetching GUI..."
;	either error? data: try [load http://www.rebol.com/r3/gui.r][
;		either data/id = 'protocol [print "Cannot load GUI from web."][do err]
;	][
;		do data
;	]
;	exit
;]
