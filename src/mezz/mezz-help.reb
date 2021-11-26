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

import module [
	Title:  "Help related functions"
	Name:    help
	Version: 3.0.0
	Exports: [? help about usage what license source dump-obj]
][
	buffer: none
	cols:   80 ; default terminal width
	max-desc-width: 45

	help-text: {
  ^[[4;1;36mUse ^[[1;32mHELP^[[1;36m or ^[[1;32m?^[[1;36m to see built-in info^[[m:
  ^[[1;32m
      help insert
      ? insert
  ^[[m
  ^[[4;1;36mTo search within the system, use quotes^[[m:
  ^[[1;32m
      ? "insert"
  ^[[m
  ^[[4;1;36mTo browse online web documents^[[m:
  ^[[1;32m
      help/doc insert
  ^[[m
  ^[[4;1;36mTo view words and values of a context or object^[[m:
  
      ^[[1;32m? lib^[[m            - the runtime library
      ^[[1;32m? self^[[m           - your user context
      ^[[1;32m? system^[[m         - the system object
      ^[[1;32m? system/options^[[m - special settings
  
  ^[[4;1;36mTo see all words of a specific datatype^[[m:
  ^[[1;32m
      ? native!
      ? function!
      ? datatype!
  ^[[m
  ^[[4;1;36mTo see all available codecs^[[m:
  ^[[1;32m
      ? codecs
  ^[[m
  ^[[4;1;36mOther debug functions^[[m:
  
      ^[[1;32m??^[[m      - display a variable and its value
      ^[[1;32mprobe^[[m   - print a value (molded)
      ^[[1;32msource^[[m  - show source code of func
      ^[[1;32mtrace^[[m   - trace evaluation steps
      ^[[1;32mwhat^[[m    - show a list of known functions
  
  ^[[4;1;36mOther information^[[m:
  
      ^[[1;32mabout^[[m   - see general product info
      ^[[1;32mlicense^[[m - show user license
      ^[[1;32musage^[[m   - program cmd line options
}

	output: func[value][
		buffer: insert buffer form reduce value
	]

	interpunction: charset ";.?!"
	dot: func[value [string!]][
		unless find interpunction last value [append value #"."]
		value
	]

	pad: func [val [string!] size] [head insert/dup tail val #" " size - length? val]

	a-an: func [
		"Prepends the appropriate variant of a or an into a string"
		s [string!]
	][
		form reduce [pick ["an" "a"] make logic! find "aeiou" s/1 s]
	]

	form-type: func [value] [
		a-an head clear back tail mold type? :value
	]

	form-val: func [val /local limit hdr tmp] [
		; Form a limited string from the value provided.
		val: case [
			string?       :val [ mold/part/flat val max-desc-width]
			any-block?    :val [ reform ["length:" length? val mold/part/flat val max-desc-width] ]
			object?       :val [ words-of val ]
			module?       :val [
				hdr: spec-of :val
				either val: select hdr 'title [ if #"." <> last val [append val #"."] ][ val: copy "" ]
				if tmp: select hdr 'exports [	append append val #" " mold/flat tmp ]
				val
			]
			any-function? :val [ any [title-of :val spec-of :val] ]
			datatype?     :val [ get in spec-of val 'title ]
			typeset?      :val [ to block! val]
			port?         :val [ reduce [val/spec/title val/spec/ref] ]
			image?        :val [ mold/part/all/flat val max-desc-width]
			gob?          :val [ return reform ["offset:" val/offset "size:" val/size] ]
			vector?       :val [ mold/part/all/flat val max-desc-width]
			;none?         :val [ mold/all val]
			true [:val]
		]
		unless string? val [val: mold/part/flat val max-desc-width]
		ellipsize/one-line val max-desc-width - 1
	]

	form-pad: func [val size] [
		; Form a value with fixed size (space padding follows).
		val: form val
		insert/dup tail val #" " size - length? val
		val
	]

	dump-obj: func [
		"Returns a string with information about an object value"
		obj [any-object!]
		/weak "Provides sorting and does not displays unset values"
		/match "Include only those that match a string or datatype"
			pattern
		/local start wild type str result user?
	][
		result: clear ""
		user?: same? obj system/contexts/user
		; Search for matching strings:
		wild: all [string? pattern  find pattern "*"]
		foreach [word val] obj [
			type: type?/word :val
			if all [weak type = 'unset!][ continue ]
			str: either find [function! closure! native! action! op! object!] type [
				reform [word mold spec-of :val words-of :val]
			][
				form word
			]
			if any [
				not match
				either string? :pattern [
					either wild [
						tail? any [find/any/match/tail str pattern pattern]
					][
						find str pattern
					]
				][
					type = :pattern
				]
			][
				if all [
					user?   ; if we are using user's context (system/contexts/user)
					match   ; with a pattern or a datatype
					any [   ; don't show results
						word = 'lib-local ; for internal `lib-local` value (as it would always match)
						strict-equal? :val select system/contexts/lib word ; or if the same value is in the library context (already reported)
					]
				][ continue ]

				str: join "^[[1;32m" form-pad word 15
				append str "^[[m "
				append str form-pad type 11 - min 0 ((length? str) - 15)
				append result rejoin [
					"  " str
					either unset? :val [#"^/"][
						ajoin ["^[[32m" form-val :val "^[[m^/"]
					]
				]
			]
		]
		copy result
	]

	out-description: func [des [block!]][
		foreach line des [
			uppercase/part trim/lines line 1
			dot line
		]
		buffer: insert insert buffer #" " form des 
	]

	?: help: func [
		"Prints information about words and values"
		'word [any-type!]
		/into "Help text will be inserted into provided string instead of printed"
			string [string!] "Returned series will be past the insertion"
		/local value spec args refs rets type ret desc arg def des ref str cols tmp
	][
		;@@ quering buffer width in CI under Windows now throws error: `Access error: protocol error: 6`
		;@@ it should return `none` like under Posix systems!
		cols: any [ attempt [ query/mode system/ports/input 'buffer-cols ] 120]
		max-desc-width: cols - 35
		buffer: any [string  clear ""]
		catch [
			case/all [
				unset? :word [
					output help-text
					throw true
				]
				word? :word [
					either value? :word [
						value: get :word    ;lookup for word's value if any
						if :word = 'codecs [
							list-codecs :word
							if same? :value system/codecs [throw true]
							output lf
							if any-function? :value [
								; don't display help in case that user redefined `codecs` with a function
								output ajoin ["^[[1;32m" uppercase mold word "^[[m is " form-type :value ".^[[m"]
								throw true
							]
						]
					][	word: mold :word ]  ;or use it as a string input
				]
				string? :word  [
					tmp: false
					case/all [
						not empty? value: dump-obj/weak/match system/contexts/lib :word [
							output ajoin ["Found these related matches:^/" value]
							tmp: true
						]
						not empty? value: dump-obj/weak/match system/contexts/user :word [
							output ajoin ["Found these related matches in the user context:^/" value]
							tmp: true
						]
						not tmp [
							output ajoin ["No information on: ^[[32m" :word "^[[m^/"]
						]
					]
					throw true
				]
				datatype? :value [
					spec: spec-of :value
					either :word <> to word! :value [
						; for example: value: string! help value 
						output ajoin [
						 "^[[1;32m" uppercase mold :word "^[[m is a datatype of value: ^[[32m" mold :value "^[[m^/"
						]
					][
						; for example: help string! 
						output ajoin [
						 "^[[1;32m" uppercase mold :word "^[[m is a datatype.^[[m^/"
						 "It is defined as" either find "aeiou" first spec/title [" an "] [" a "] spec/title ".^/"
						 "It is of the general type ^[[1;32m" spec/type "^[[m.^/^/"
						]
						unless empty? value: dump-obj/match system/contexts/lib :word [
							output ajoin ["Found these related words:^/" value]
						]
						unless empty? value: dump-obj/match system/contexts/user :word [
							output ajoin ["Found these related words in the user context:^/" value]
						]
					]
					throw true
				]
				refinement? :word [
					output [mold :word "is" form-type :word "used in these functions:^/^/"]
					str: copy ""
					foreach [name val] system/contexts/lib [
						if all [
							any-function? :val
							spec: spec-of :val
							desc: find/case/tail spec :word
						][
							str: join "^[[1;32m" form-pad name 15
							append str "^[[m "
							append str form-pad type? :val 11 - min 0 ((length? str) - 15)
							append str join "^[[1;32m" mold :word
							if string? desc/1 [
								append str " ^[[0;32m"
								append str desc/1
							]
							output ajoin ["  " str "^[[m^/"]
						]
					]
					throw true
				]
				not any [word? :word path? :word] [
					output ajoin ["^[[1;32m" uppercase mold :word "^[[m is " form-type :word]
					throw true
				]
				path? :word [
					if error? set/any 'value try [get :word][
						;check if value is error or if it was really an invalid or path without value
						if all [
							value/id   = 'invalid-path
							value/arg1 = :word
						][
							output ajoin ["There is no ^[[1;32m" value/arg2 "^[[m in path ^[[1;32m" value/arg1 "^[[m"]
							throw true
						]
						if all [
							value/id = 'no-value
							value/arg1 = first :word
						][
							output ["No information on^[[1;32m" :word "^[[m(path has no value)"]
							throw true
						]
					]
				]
				any-function? :value [
					spec: copy/deep spec-of :value
					args: copy []
					refs: none
					rets: none
					type: type? :value

					if path? word [word: first word]
					
					clear find spec /local
					parse spec [
						any block!
						copy desc any string!
						any [
							set arg [word! | lit-word! | get-word!] 
							set def opt block!
							copy des any string! (
								repend args [arg def des]
							)
							|
							quote return: set rets block!
						]
						opt [refinement! refs:]
						to end
					]
					output "^[[4;1;36mUSAGE^[[m:^/     "
					either op? :value [
						output [args/1 word args/4]
					] [
						output ajoin ["^[[1;32m" uppercase mold word]
						foreach [arg def des] args [
							buffer: insert insert buffer #" " mold arg
						]
						output "^[[m"
					]

					output "^/^/^[[4;1;36mDESCRIPTION^[[m:^/"
					unless empty? desc [
						foreach line desc [
							trim/head/tail line
							unless empty? line [
								output ["    " dot uppercase/part line 1 #"^/"]
							]
						]
					]
					output ["    " uppercase form word "is" a-an mold type "value."]

					unless empty? args [
						output "^/^/^[[4;1;36mARGUMENTS^[[m:"
						foreach [arg def des] args [
							output ajoin [
								"^/     ^[[1;32m" pad mold arg 14 "^[[m"
								"^[[32m" pad either def [mold def]["[any-type!]"] 10 "^[[m"
							]
							out-description des
						]
					]

					if refs [
						output "^/^/^[[4;1;36mREFINEMENTS^[[m:"
						parse back refs [
							any [
								set ref refinement! (output ajoin ["^/     ^[[1;32m" pad mold ref 14 "^[[m"])
								opt [set des string! (output des)]
								any [
									set arg [word! | lit-word! | get-word!] 
									set def opt block! 
									copy des any string! (
										output ajoin [
											"^/      "
											"^[[1;33m" pad form arg 13  
											"^[[0;32m" either def [mold def]["[any-type!]"] "^[[m"
										]
										out-description des
									)
								]
							]
						]
					]
					if rets [
						output  "^/^/^[[4;1;36mRETURNS^[[m:"
						output ["^/    " mold rets ]
					]
					output newline
					throw true
				]
				'else [
					word: uppercase mold word
					type: form-type :value
					output ajoin ["^[[1;32m" word "^[[m is " type " of value: ^[[32m"]
					output either any [any-object? value] [
						output lf dump-obj :value
					][
						max-desc-width: cols - (length? word) - (length? type) - 21
						form-val :value
					]
					output "^[[m"
				]
			]
		]
		either into [buffer][print head buffer]
	]

	list-codecs: function [][
		names: sort keys-of codecs: system/codecs
		foreach type common-types: [
			time
			text			
			cryptography
			compression
			sound
			image
			other
		][
			tmp: clear []
			foreach name names [
				codec: codecs/:name
				if any [
					type = codec/type
					all [type = 'other not find common-types codec/type]
				][
					append tmp codec
				]
			]
			if empty? tmp [continue]

			output ajoin [{^[[4;1;36m} uppercase form type { CODECS^[[m:}]
			foreach codec tmp [
				output ajoin ["^/    ^[[4;1;33m" uppercase form codec/name "^[[m^/    ^[[1;32m" codec/title]
				if all [tmp: select codec 'suffixes not empty? tmp] [
					output ajoin ["^[[m^/    Suffixes: ^[[31m" codec/suffixes]
				]
				tmp: exclude keys-of codec [name type title entry suffixes]
				unless empty? tmp [
					output ajoin ["^[[m^/    Includes: ^[[35m" tmp]
				]
				output lf
			]
			output "^[[m^/^/"
		]
		output ajoin [
			"^[[1mTIP:^[[m use for example ^[[1;32mhelp system/codecs/" codec/name "^[[m to see more info.^/"
		]
	]

	about: func [
		"Information about REBOL"
	][
		print make-banner sys/boot-banner
	]

	usage: func [
		"Prints command-line arguments"
	][
		print {
  ^[[4;1;36mCommand line usage^[[m:
  
      ^[[1;32mREBOL |options| |script| |arguments|^[[m
  
  ^[[4;1;36mStandard options^[[m:
  
      ^[[1;32m--args data^[[m      Explicit arguments to script (quoted)
      ^[[1;32m--do expr^[[m        Evaluate expression (quoted)
      ^[[1;32m--help (-?)^[[m      Display this usage information (then quit)
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

      ; --cgi (-c)       Load CGI utiliy module and modes
	]


	license: func [
		"Prints the REBOL/core license agreement"
	][
		print system/license
	]

	source: func [
		"Prints the source code for a word"
		'word [word! path!]
	][
		if not value? word [print [word "undefined"] exit]
		print head insert mold get word reduce [word ": "]
		exit
	]

	what: func [
		{Prints a list of known functions}
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
]

;-- old alpha functions:
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
