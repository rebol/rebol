REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Help"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2026 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

import (module [
	Title:  "Help related functions"
	Name:    help
	Version: 4.0.0
	Exports: [? help about usage what license source dump-obj bugs changes]
][
	buffer: none
	cols:   80 ; default terminal width
	max-desc-width: 45
	ansi: system/options/ansi

	help-text: {
  _Use `HELP` or `?` to see built-in info_:

	  `help insert`
	  `? insert`

  _To search within the system, use quotes_:

	  `? "insert"`

  _To browse online web documents_:

	  `help/doc insert`

  _To view words and values of a context or object_:

	  `? lib`            - the runtime library
	  `? self`           - your user context
	  `? system`         - the system object
	  `? system/options` - special settings
  
  _To see all words of a specific datatype_:

	  `? native!`
	  `? function!`
	  `? datatype!`

  _To see all available codecs_:

	  `? codecs`

  _Other debug functions_:
  
	  `??`      - display a variable and its value
	  `probe`   - print a value (molded)
	  `source`  - show source code of func
	  `trace`   - trace evaluation steps
	  `what`    - show a list of known functions
  
  _Other information_:
  
	  `about`   - see general product info
	  `license` - show user license
	  `usage`   - program cmd line options
}

	help-usage: {
  _Command line usage_:
  
	  `REBOL |options| |script| |arguments|`
  
  _Standard options_:
  
	  `--args data`      Explicit arguments to script (quoted)
	  `--do expr`        Evaluate expression (quoted)
	  `--help (-?)`      Display this usage information (then quit)
	  `--script file`    Explicit script filename
	  `--version tuple`  Script must be this version or greater
  
  _Special options_:
  
	  `--boot level`     Valid levels: base sys mods
	  `--cgi (-c)`       Starts in CGI mode
	  `--debug flags`    For user scripts (system/options/debug)
	  `--halt (-h)`      Keep console open after script completion
	  `--import file`    Import a module prior to script
	  `--legacy-repl`    Run the interactive console in legacy (pre-modern) mode
	  `--no-color`       Reduce the use of ANSI color escape sequences
	  `--quiet (-q)`     No startup banners or information
	  `--secure policy`  Can be: none allow ask throw quit
	  `--trace (-t)`     Enable trace mode during boot
	  `--verbose`        Show detailed startup information

  _Other quick options_:
  
	  `-s`               No security
	  `+s`               Full security
	  `-v`               Display version only (then quit)
  
  _Examples_:
  
	  REBOL script.r
	  REBOL -s script.r
	  REBOL script.r 10:30 test@example.com
	  REBOL --do "watch: on" script.r}

	output: func[value][
		buffer: insert buffer either block? :value [ajoin value][:value]
	]

	interpunction: charset ";.?!,:"
	dot: func[value [string!]][
		unless find interpunction last value [append value #"."]
		value
	]

	pad: func [val [string!] size] [head insert/dup tail val #" " size - length? val]

	a-an: func [
		"Prepends the appropriate variant of a or an into a string"
		s [string!]
	][
		form reduce [pick ["an" "a"] make logic! find "aeiou" s/1 as-yellow s]
	]

	form-type: func [value] [
		a-an head clear back tail form type? :value
	]

	form-val: func [val /local limit hdr tmp] [
		; Form a limited string from the value provided.
		val: case [
			string?       :val [ mold/part/flat val max-desc-width]
			any-block?    :val [ reform ["length:" length? val mold/part/flat val max-desc-width] ]
			object?       :val [ words-of val ]
			module?       :val [
				hdr: spec-of :val
				val: copy any [select hdr 'title ""]
				if all [tmp: last val tmp <> #"."] [append val #"."]
				if tmp: select hdr 'version [ append val ajoin [SP "Version: " tmp] ]
				if tmp: select hdr 'exports [ append append val SP mold/flat tmp ]
				val
			]
			any-function? :val [ any [title-of :val spec-of :val] ]
			datatype?     :val [ get in spec-of val 'title ]
			typeset?      :val [ ajoin [#"[" val #"]"] ]
			port?         :val [ reduce [val/spec/title val/spec/ref] ]
			image?        :val [ mold/part/all/flat val max-desc-width]
			gob?          :val [ return reform ["offset:" val/offset "size:" val/size] ]
			vector?       :val [ reform ["length:" length? val mold/part/flat val max-desc-width] ]
			any [logic? :val none? :val unset? :val] [ form val ]
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
		obj [any-object! map!]
		/match "Provides sorting; include only those that match a string or datatype"
			pattern
		/ignore "Ignore specified value types"
			ignored [datatype! typeset!] "Used to hide unset or none values."
		/local start wild type str result user? sorted
	][
		result: append clear "" LF
		user?: same? obj system/contexts/user
		; Search for matching strings:
		wild: all [string? pattern  find pattern "*"]
		ignored: to block! any [ignored []]

		if match [
			sorted: make block! 2 * length? obj
			foreach [word val] obj [
				type: type?/word :val
				str: either find [function! closure! native! action! op! object!] type [
					reform [word mold spec-of :val words-of :val]
				][	form word ]
				if any [
					all [

						string? :pattern
						either wild [
							tail? any [find/any/match/tail str pattern pattern]
						][
							find str pattern
						]
					]
					type = :pattern
				][	repend sorted [word :val] ]
			]
			;; sort according name
			sort/skip sorted 2
			;; sort according type
			sort/skip/all/compare sorted 2 func[a b][(type? :a/2) < (type? :b/2)]
		]
		foreach [word val] any [sorted obj] [
			if find/only ignored type: type? :val [ continue ]
			if all [
				user?   ;; if we are using user's context (system/contexts/user)
				match   ;; with a pattern or a datatype
				any [   ;; don't show results
					word = 'lib-local ;; for internal `lib-local` value (as it would always match)
					strict-equal? :val select system/contexts/lib word ;; or if the same value is in the library context (already reported)
				]
			][ continue ]

			;; construct in multiple steps to compensate padding with long names
			str: ajoin [ansi/bright-green form-pad either map? :obj [mold/flat :word][word] 17 "^[[m "]
			append str ajoin [ansi/bright-yellow form-pad type 11 - min 0 ((length? str) - 17)]
			append result rejoin [
				"^[[m  " str
				either unset? :val [#"^/"][
					ajoin [ansi/green form-val :val "^[[m^/"]
				]
			]
		]
		if system/options/no-color [sys/remove-ansi result]
		copy result
	]

	out-description: func [des [block!] /local pos len][
		if empty? des [exit]
		des: trim/auto ajoin/with des LF
		des: split-lines ansi-colorize des
		;; determine if the first string fits the width of the terminal
		if all [
			pos: find/reverse/tail buffer LF
			((length? sys/remove-ansi copy pos) + des/1/width) < cols
		][
			buffer: insert insert buffer SP dot uppercase/part des/1 1
			++ des
		]	
		foreach line des [
			buffer: insert insert buffer "^/                   " line
		]
	]
	out-title: func[title /line][
		output [
			if line ["^/^/"]
			ansi/bright-cyan "^[[4m" title "^[[m:"
		]
	]

	?: help: func [
		"Prints information about words and values"
		'word [any-type!]
		/doc "Open web browser to related documentation"
		/into "Help text will be inserted into provided string instead of printed"
			string [string!] "Returned series will be past the insertion"
		/local value spec args refs rets type ret desc desc-ext arg def des ref str tmp ret-desc
	][
		if all [
			doc
			word? :word
			any-function? get :word
		][
			browse join https://rebol.tech/docs/functions.html# word
		]
		cols: query system/ports/output 'window-cols
		max-desc-width: cols - 35
		buffer: any [string  clear ""]
		catch [
			case/all [
				unset? :word [
					output ansi-colorize help-text
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
								output [ansi/bright-green uppercase mold word "^[[m is " form-type :value ".^[[m"]
								throw true
							]
						]
					][	word: mold :word ]  ;or use it as a string input
				]
				string? :word  [
					tmp: false
					case/all [
						not single? value: dump-obj/match/ignore system/contexts/lib :word #(unset!) [
							output ["Found these related matches:" value]
							tmp: true
						]
						not single? value: dump-obj/match/ignore system/contexts/user :word #(unset!) [
							output ["Found these related matches in the user context:" value]
							tmp: true
						]
						not tmp [
							output ["No information on: " ansi/green :word "^[[m^/"]
						]
					]
					throw true
				]
				datatype? :value [
					spec: spec-of :value
					either :word <> to word! :value [
						; for example: value: string! help value 
						output [
							ansi/bright-green uppercase mold :word
							"^[[m is a datatype of value: "
							ansi/green mold :value "^[[m^/"
						]
					][
						; for example: help string! 
						output [
						 ansi/bright-green uppercase mold :word "^[[m is a datatype.^[[m^/"
						 "It is defined as" either find "aeiou" first spec/title [" an "] [" a "] spec/title ".^/"
						 "It is of the general type " ansi/bright-green spec/type "^[[m.^/^/"
						]
						unless single? value: dump-obj/match system/contexts/lib :word [
							output ["Found these related words:" value]
						]
						unless single? value: dump-obj/match system/contexts/user :word [
							output ["Found these related words in the user context:" value]
						]
					]
					throw true
				]
				refinement? :word [
					output [mold :word " is " form-type :word " used in these functions:^/^/"]
					str: copy ""
					foreach [name val] system/contexts/lib [
						if all [
							any-function? :val
							spec: spec-of :val
							desc: find/case/tail spec :word
						][
							str: ajoin [ansi/bright-green form-pad name 15 "^[[m "]
							append str form-pad type? :val 11 - min 0 ((length? str) - 15)
							append str ajoin [ansi/bright-green mold :word]
							if string? desc/1 [
								append str ajoin [SP ansi/green desc/1]
							]
							output ["  " str "^[[m^/"]
						]
					]
					throw true
				]
				not any [word? :word path? :word] [
					output [ansi/bright-green :word "^[[m is " form-type :word]
					throw true
				]
				path? :word [
					if error? set/any 'value try [get :word][
						;check if value is error or if it was really an invalid or path without value
						if all [
							value/id   = 'invalid-path
							value/arg1 = :word
						][
							output [
								"There is no " ansi/bright-green value/arg2
								"^[[m in path " ansi/bright-green value/arg1 "^[[m"
							]
							throw true
						]
						if all [
							value/id = 'no-value
							any [
								value/arg1 = first :word
								all [path? value/arg1 value/arg1/1 = first :word]
							]
						][
							output ["No information on " ansi/bright-green :word " ^[[m(path has no value)"]
							throw true
						]
					]
				]
				port? :value [
					output [
						ansi/bright-green uppercase mold :word
						"^[[m is " a-an value/spec/title SP
						ansi/bright-green value/spec/ref "^[[m^/"
					]
				]
				any-function? :value [
					spec: copy/deep spec-of :value
					args: copy []
					refs: none
					rets: ret-desc: none
					type: type? :value
					
					clear find spec /local
					parse spec [
						any block!
						set desc string! copy desc-ext any string!
						any [
							set arg [word! | lit-word! | get-word!] 
							set def opt block!
							copy des any string! (
								repend args [arg def des]
							)
							|
							quote return: set rets block! opt [set ret-desc string!]
						]
						opt [refinement! refs:]
						to end
					]
					out-title "USAGE"
					output ["^/     "]
					either op? :value [
						output [args/1 SP word SP args/4]
					][
						output [ansi/bright-green uppercase mold word]
						foreach [arg def des] args [
							buffer: insert insert buffer #" " mold arg
						]
						output "^[[m"
					]
					out-title/line "DESCRIPTION"
					if desc [output ["^/     " dot trim/head/tail desc]]
					unless empty? desc-ext [
						desc: split-lines ansi-colorize trim/auto ajoin/with desc-ext LF
						foreach line desc [output ["^/     " line]]
					]
					output ["^/     " uppercase form word " is " a-an form :type " value."]

					unless empty? args [
						out-title/line "ARGUMENTS"
						foreach [arg def des] args [
							output [
								"^/     " ansi/bright-green pad mold arg 14 "^[[m"
								ansi/green pad either def [mold def]["[any-type!]"] 10 "^[[m"
							]
							out-description des
						]
					]

					if refs [
						out-title/line "REFINEMENTS"
						parse back refs [
							any [
								set ref refinement! (output ["^/     " ansi/bright-green pad mold ref 13 "^[[m"])
								opt [copy des any string! (out-description des)]
								any [
									set arg [word! | lit-word! | get-word!] 
									set def opt block! 
									copy des any string! (
										output [
											"^/      "
											ansi/bright-yellow pad form arg 13  
											ansi/green either def [mold def]["[any-type!]"] "^[[m"
										]
										out-description des
									)
								]
							]
						]
					]
					if rets [
						out-title/line "RETURNS"
						if ret-desc [output ["^/     " ret-desc]]
						if block? rets [
							parse rets [
								any [
									set arg word! (output ["^/     " ansi/bright-yellow pad mold arg 14 "^[[m"])
									opt [set des string! (output des)]
									| skip
								]
							]
						]
					]
					output newline
					throw true
				]
				module? :value [
					output [
						ansi/bright-green uppercase mold :word "^[[m is " a-an "module with:^/"
						out-title "SPEC"
						dump-obj/ignore spec-of :value #(none!)
						out-title "BODY"
						dump-obj :value
					]
					throw true
				]
				'else [
					word: uppercase mold word
					type: form-type :value
					output [ansi/bright-green word "^[[m is " type " of value: " ansi/green]
					output either any [any-object? value map? value] [
						dump-obj :value
					][
						max-desc-width: cols - (length? word) - (length? type) - 21
						form-val :value
					]
					output "^[[m"
				]
			]
		]
		if system/options/no-color [sys/remove-ansi head buffer]
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

			out-title ajoin [uppercase form type " CODECS"]
			foreach codec tmp [
				output [
					"^/    ^[[4m" ansi/bright-yellow uppercase form codec/name
					"^[[m^/    " ansi/bright-green codec/title
				]
				if all [tmp: select codec 'suffixes not empty? tmp] [
					output ajoin ["^[[m^/    Suffixes: " ansi/red codec/suffixes]
				]
				tmp: exclude keys-of codec [name type title entry suffixes]
				unless empty? tmp [
					output ajoin ["^[[m^/    Includes: " ansi/magenta tmp]
				]
				output lf
			]
			output "^[[m^/^/"
		]
		output ajoin [
			"^[[1mTIP:^[[m use for example " ansi/bright-green "help system/codecs/" codec/name "^[[m to see more info.^/"
		]
		if system/options/no-color [sys/remove-ansi head buffer]
	]

	about: func [
		"Information about REBOL"
	][
		print make-banner sys/boot-banner
	]

	usage: func [
		"Prints command-line arguments"
	][
		print ansi-colorize help-usage
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
		/local ctx vals arg list size a
	][
		list: make block! 400
		size: 10 ; defines minimal function name padding

		ctx: any [select system/modules :name lib]
		a: system/options/ansi

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
			print rejoin [a/green head change vals word a/reset SP any [arg ""]]
		]
		exit
	]
;-- old alpha functions:
;pending: does [
;	comment "temp function"
;	print "Pending implementation."
;]
;
browse: func[url [url!]] [
	log-info 'REBOL ["Opening web browser:" as-green url]
	lib/browse url
]
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
;	browse http://www.rebol.com/r3/docs
;	exit
;]

bugs: func [
	"View bug database."
][
	browse https://github.com/Oldes/Rebol-issues/issues
	exit
]

changes: func [
	"What's new about this version."
][
	browse https://github.com/Oldes/Rebol3/blob/master/CHANGES.md
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
		err: lowercase ajoin [err/type #"-" err/id]
		browse join http://www.rebol.com/r3/docs/errors/ [err ".html"]
	][
		print "No information is available."
	]
	exit
]
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
])

