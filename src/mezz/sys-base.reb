Rebol [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Sys: Top Context Functions"

	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2023 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}

	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}

	Context: sys

	Note: {
		Follows the BASE lib init that provides a basic set of functions
		to be able to evaluate this code.

		The boot binding of this module is SYS then LIB deep.
		Any non-local words not found in those contexts WILL BE
		UNBOUND and will error out at runtime!
	}
]

;-- SYS context definition begins here --
;	WARNING: ORDER DEPENDENT part of context (accessed from C code)

native: _
; for boot only

action: _
; for boot only

do*: func [
	"SYS: Called by system for DO on datatypes that require special handling."

	value [file! url! string! binary!]

	/args
	"If value is a script, this will set its system/script/args"

	arg
	"Args passed to a script (normally a string)"

	/next
	"Do next expression only, return it, update block variable"

	mark [word!]
	"Variable updated with new block position"

	/local body file spec current-path header saved-script is-module
][
	; This code is only called for urls, files, and strings.
	; DO of functions, blocks, paths, and other do-able types is done in the
	; native, and this code is not called.
	; Note that DO of file path evaluates in the directory of the target file.
	; Files, urls and modules evaluate as scripts, other strings don't.
	;
	; Note: LOAD/header returns a block with the header object in the first
	;		position, or will cause an error. No exceptions, not even for
	;		directories or media.
	;		Currently, load of URL has no special block forms.

	; Load the data, first so it will error before change-dir

	either string? value [
		body: load/all/as value 'unbound
		; does not evaluate Rebol header
	][
		body: load/header/as value 'unbound
		; unbound so DO-NEEDS runs before INTERN

		header: first+ body
		; Get the header and advance 'data to the code position
		; object or none

		is-module: 'module = select header 'type
		; data is a block! here, with the header object in the first position back
	]

	either all [
		string? value
		not is-module
	][
		; Return result without script overhead

		do-needs header
		; Load the script requirements

		if empty? body [
			if mark [
				set mark body
			]

			exit
			; Shortcut return empty
		]

		intern body
		; Bind the user script

		catch/quit either mark [
			[do/next body mark]
		][
			body
		]
	][
		; Otherwise we are in script mode
		; Do file in directory if necessary

		current-path: _
		; in case of /local hack

		if all [
			file? value
			file: find/last/tail value slash
		][
			current-path: what-dir
			; save the current directory for later restoration

			change-dir copy/part value file
		]

		; Make the new script object
		; and save old one

		saved-script: system/script

		system/script: make system/standard/script compose [
			title: (select header 'title)
			header: (header)
			parent: (saved-script)
			path: (what-dir)
			args: :arg
		]

		; Print out the script info
		;
		log/info 'REBOL [
			pick ["Module:" "Script:"] is-module
			mold select header 'title
			"Version:" select header 'version
			"Date:"	   select header 'date
		]

		set/any 'value try [
			; Eval the block or make the module, returned

			either is-module [
				; Import the module and set the var

				spec: reduce [
					header
					body
					do-needs/no-user header
				]

				also
				import (
					catch/quit [
						make module! spec
					]
				)
				if mark [
					set mark tail body
				]
			][
				do-needs header
				; Load the script requirements

				intern body
				; Bind the user script

				catch/quit either mark [
					[do/next body mark]
				][
					body
				]
			]
		]

		all [
			; Restore system/script and the dir
			system/script: :saved-script
			current-path
			change-dir current-path
		]

		if error? :value [
			do :value
		]

		:value
	]
]

make-module*: func [
	"SYS: Called by system on MAKE of MODULE! datatype."

	spec [block!]
	"As [spec-block body-block opt-mixins-object]"

	/local body context mixins hidden words
][
	set [spec body mixins] spec

	if block? :spec [
		; Convert header block to standard header object:
		;
		spec: attempt [
			construct/with :spec system/standard/header
		]
	]

	; Validate the important fields of header:
	;
	assert/type [
		spec object!
		body block!
		mixins [object! none!]
		spec/name [any-word! none!]
		spec/type [any-word! none!]
		spec/version [tuple! none!]
		spec/options [block! none!]
	]

	; Module is an object during its initialization:
	;
	context: make object! 7
	; arbitrary starting size

	either find spec/options 'extension [
		bind/new [
			lib-base
			; specific runtime values MUST BE FIRST

			lib-file
			lib-local
			words
		] context
	][
		append context 'lib-local
		; local import library for the module
	]

	if spec/name [
		spec/name: to word! spec/name
	]

	unless spec/type [
		spec/type: 'module
		; in case not set earlier
	]

	if find body 'export [
		; Collect 'export keyword exports, removing the keywords
		;
		unless block? select spec 'exports [
			repend spec [
				'exports make block! 10
			]
		]

		; Note: 'export overrides 'hidden, silently for now
		;
		parse body [
			while [
				to 'export
				remove skip
				opt remove 'hidden
				opt [
					set words any-word!
					(
						unless find spec/exports words: to word! words [
							append spec/exports words
						]
					)
					|
					set words block!
					(append spec/exports collect-words/ignore words spec/exports)
				]
			]

			to end
		]
	]

	if block? select spec 'exports [
		bind/new spec/exports context
		; Add exported words at top of context (performance)
	]

	; Collect 'hidden keyword words, removing the keywords. Ignore exports.
	;
	hidden: _

	if find body 'hidden [
		hidden: make block! 10

		; Note: Exports are not hidden, silently for now

		parse body [
			while [
				to 'hidden
				remove skip
				opt [
					set words any-word!
					(
						unless find select spec 'exports words: to word! words [
							append hidden words
						]
					)
					|
					set words block!
					(append hidden collect-words/ignore words select spec 'exports)
				]
			]

			to end
		]
	]

	; Add hidden words next to the context (performance)
	;
	if block? hidden [
		bind/new hidden context
	]

	either find spec/options 'isolate [
		bind/new body context
		; All words of the module body are module variables

		if object? mixins [
			resolve context mixins
			; The module keeps its own variables (not shared with system)
		]

		; resolve context sys -- no longer done -Carl
		resolve context lib
	][
		bind/only/set body context
		; Only top level defined words are module variables.

		bind body lib
		; The module shares system exported variables:

		; bind body sys -- no longer done -Carl
		if object? mixins [
			bind body mixins
		]
	]

	bind body context

	context/lib-local: any [
		; always set, always overrides
		mixins
		make object! 0
	]

	if block? hidden [
		protect/hide/words hidden
	]

	context: to module! reduce [
		spec context
	]

	do body

	;print ["Module created" spec/name spec/version]

	context
]

; MOVE some of these to SYSTEM?
;
boot-banner: _

boot-help: "Boot-sys level - no extra features."

boot-host: _
; any host add-ons to the lib (binary)

boot-mezz: _
; built-in mezz code (put here on boot)

boot-prot: _
; built-in boot protocols

boot-exts: _
; boot extension list

export: func [
	"Low level export of values (e.g. functions) to lib."

	words [block!]
	"Block of words (already defined in local context)"
][
	foreach word words [
		repend lib [
			word get word
		]
	]
]

assert-utf8: function [
	"If binary data is UTF-8, returns it, else throws an error."

	source [binary!]
][
	unless find [0 8] encoding: utf? source [
		; Not UTF-8
		cause-error 'script 'no-decode ajoin [
			"UTF-" abs encoding
		]
	]

	source
]

log: func [
	"Prints out debug message"

	'id [any-word!]
	"Source of the log message"

	message
	"Output message"

	/info
	/more
	/debug
	/error

	/local level options ansi
][
	options: system/options
	ansi: options/ansi

	if error [
		message: trim/head/tail form either block? message [
			reduce message
		][
			message
		]

		foreach line split-lines message [
			print ajoin [
				" " ansi/error "[" id "] " ansi/bold

				either line/1 = #"*" [] [
					"** Error: "
				]

				copy/part line 200 ;@@ I am not sure with this line length limit

				ansi/reset
			]
		]

		exit
	]

	if options/quiet [
		exit
	]

	level: any [
		select options/log id
		3
	]

	if level <= 0 [
		exit
	]

	if block? message [
		message: form reduce :message
	]

	case [
		info  [
			if level > 0 [
				print ajoin [
					#" " ansi/bright-yellow "[" id "] " ansi/cyan message ansi/reset
				]
			]
		]

		more  [
			if level > 1 [
				print ajoin [
					#" " ansi/yellow "[" id "] " ansi/cyan message ansi/reset
				]
			]
		]

		debug [
			if level > 2 [
				print ajoin [
					#" " ansi/yellow "[" id "] " ansi/green message ansi/reset
				]
			]
		]

		#(true)  [
			if level > 0 [
				print ajoin [
					#" " ansi/yellow "[" id "] " message ansi/reset
				]
			]
		]
	]
]

remove-ansi: function/with [
	"Removes ANSI color escape codes from a string or binary"

	string [any-string! binary!]
	"(modified)"
][
	parse string [ 
		any [
			to escape
			; Move to the next escape sequence

			remove [
				; Remove the sequence matching this pattern

				escape
				#"["

				any [
					1 3 numeric
					opt
					#";"
				]

				#"m"
				; End of ANSI code
			]
		]
	]

	string
] system/catalog/bitsets
; for the `numeric` bitset
