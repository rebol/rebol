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
			str: form-pad word 15
			append str #" "
			append str form-pad type 10 - ((length? str) - 15)
			append out reform [
				"  " str
				if type <> 'unset! [form-val :val]
				newline
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
			Use HELP or ? to see built-in info:

				help insert
				? insert

			To search within the system, use quotes:

				? "insert"

			To browse online web documents:

				help/doc insert

			To view words and values of a context or object:

				? lib    - the runtime library
				? self   - your user context
				? system - the system object
				? system/options - special settings

			To see all words of a specific datatype:

				? native!
				? function!
				? datatype!

			Other debug functions:

				docs - open browser to web documentation
				?? - display a variable and its value
				probe - print a value (molded)
				source func - show source code of func
				trace - trace evaluation steps
				what - show a list of known functions
				why? - explain more about last error (via web)

			Other information:

				chat - open DevBase developer forum/BBS
				docs - open DocBase document wiki website
				bugs - open CureCore bug database website
				demo - run demo launcher (from rebol.com)
				about - see general product info
				upgrade - check for newer versions
				changes - show changes for recent version
				install - install (when applicable)
				license - show user license
				usage - program cmd line options
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
		item: form :word
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
			print [
				mold :word "is a datatype" newline
				"It is defined as" either find "aeiou" first value/title ["an"] ["a"] value/title newline
				"It is of the general type" value/type newline
			]
		]
		if any [:word = 'unset! not value? :word] [exit]
		types: dump-obj/match lib :word
		sort types
		if not empty? types [
			print ["Found these related words:" newline types]
			exit
		]
		if all [word? :word datatype? get :word] [
			print ["No values defined for" word]
			exit
		]
		print ["No information on" word]
		exit
	]

	; Print type name with proper singular article:
	type-name: func [value] [
		value: mold type? :value
		clear back tail value
		join either find "aeiou" first value ["an "]["a "] value
	]

	; Print literal values:
	if not any [word? :word path? :word][
		print [mold :word "is" type-name :word]
		exit
	]

	; Get value (may be a function, so handle with ":")
	either path? :word [
		if any [
			error? set/any 'value try [get :word] ;try reduce [to-get-path word]
			not value? 'value
		][
			print ["No information on" word "(path has no value)"]
			exit
		]
	][
		value: get :word
	]
	unless any-function? :value [
		prin [uppercase mold word "is" type-name :value "of value: "]
		print either any [object? value port? value]  [print "" dump-obj value][mold :value]
		exit
	]

	; Must be a function...
	; If it has refinements, strip them:
	;if path? :word [word: first :word]

	;-- Print info about function:
	prin "USAGE:^/^-"

	args: words-of :value
	clear find args /local
	either op? :value [
		print [args/1 word args/2]
	][
		print [uppercase mold word args]
	]

	print ajoin [
		newline "DESCRIPTION:" newline
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
			str: ajoin [tab arg/1]
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

		print-args "^/ARGUMENTS:" argl
		print-args/extra "^/REFINEMENTS:" refl
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
	print trim/auto {
	Command line usage:

		REBOL |options| |script| |arguments|

	Standard options:

		--args data      Explicit arguments to script (quoted)
		--do expr        Evaluate expression (quoted)
		--help (-?)      Display this usage information
		--script file    Explicit script filename
		--version tuple  Script must be this version or greater

	Special options:

		--boot level     Valid levels: base sys mods
		--debug flags    For user scripts (system/options/debug)
		--halt (-h)      Leave console open when script is done
		--import file    Import a module prior to script
		--quiet (-q)     No startup banners or information
		--secure policy  Can be: none allow ask throw quit
		--trace (-t)     Enable trace mode during boot
		--verbose        Show detailed startup information

	Other quick options:

		-s               No security
		+s               Full security
		-v               Display version only (then quit)

	Examples:

		REBOL script.r
		REBOL -s script.r
		REBOL script.r 10:30 test@example.com
		REBOL --do "watch: on" script.r
	}
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
	size: 0

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

	vals: make string! size
	foreach [word arg] sort/skip list 2 [
		append/dup clear vals #" " size
		print [head change vals word any [arg ""]]
	]
	exit
]

pending: does [
	comment "temp function"
	print "Pending implementation."
]

say-browser: does [
	comment "temp function"
	print "Opening web browser..."
]

upgrade: function [
	"Check for newer versions (update REBOL)."
][
	print "Fetching upgrade check ..."
	if error? err: try [do http://www.rebol.com/r3/upgrade.r none][
		either err/id = 'protocol [print "Cannot upgrade from web."][do err]
	]
	exit
]

chat: function [
	"Open REBOL DevBase forum/BBS."
][
	print "Fetching chat..."
	if error? err: try [do http://www.rebol.com/r3/chat.r none][
		either err/id = 'protocol [print "Cannot load chat from web."][do err]
	]
	exit
]

docs: func [
	"Browse on-line documentation."
][
	say-browser
	browse http://www.rebol.com/r3/docs
	exit
]

bugs: func [
	"View bug database."
][
	say-browser
	browse http://curecode.org/rebol3/
	exit
]

changes: func [
	"What's new about this version."
][
	say-browser
	browse http://www.rebol.com/r3/changes.html
	exit
]

why?: func [
	"Explain the last error in more detail."
	'err [word! path! error! none! unset!] "Optional error value"
][
	case [
		unset? :err [err: none]
		word? err [err: get err]
		path? err [err: get err]
	]

	either all [
		error? err: any [:err system/state/last-error]
		err/type ; avoids lower level error types (like halt)
	][
		say-browser
		err: lowercase ajoin [err/type #"-" err/id]
		browse join http://www.rebol.com/r3/docs/errors/ [err ".html"]
	][
		print "No information is available."
	]
	exit
]

demo: function [
	"Run R3 demo."
][
	print "Fetching demo..."
	if error? err: try [do http://www.rebol.com/r3/demo.r none][
		either err/id = 'protocol [print "Cannot load demo from web."][do err]
	]
	exit
]

load-gui: function [
	"Download current GUI module from web. (Temporary)"
][
	print "Fetching GUI..."
	either error? data: try [load http://www.rebol.com/r3/gui.r][
		either data/id = 'protocol [print "Cannot load GUI from web."][do err]
	][
		do data
	]
	exit
]
