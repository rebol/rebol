REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Sys: Top Context Functions"
	Rights: {
		Copyright 2012 REBOL Technologies
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
;   WARNING: ORDER DEPENDENT part of context (accessed from C code)

native: none ; for boot only
action: none ; for boot only

do*: func [
	{SYS: Called by system for DO on datatypes that require special handling.}
	value [file! url! string! binary!]
	/args "If value is a script, this will set its system/script/args"
	arg   "Args passed to a script (normally a string)"
	/next "Do next expression only, return it, update block variable"
	var [word!] "Variable updated with new block position"
	/local data file spec dir hdr scr mod?
][
	; This code is only called for urls, files, and strings.
	; DO of functions, blocks, paths, and other do-able types is done in the
	; native, and this code is not called.
	; Note that DO of file path evaluates in the directory of the target file.
	; Files, urls and modules evaluate as scripts, other strings don't.
	; Note: LOAD/header returns a block with the header object in the first
	;       position, or will cause an error. No exceptions, not even for
	;       directories or media.
	;       Currently, load of URL has no special block forms.

	; Load the data, first so it will error before change-dir
	data: load/header/type value 'unbound ; unbound so DO-NEEDS runs before INTERN
	; Get the header and advance 'data to the code position
	hdr: first+ data  ; object or none
	; data is a block! here, with the header object in the first position back
	mod?: 'module = select hdr 'type

	either all [string? value  not mod?] [
		; Return result without script overhead
		do-needs hdr  ; Load the script requirements
		if empty? data [if var [set var data]  exit] ; Shortcut return empty
		intern data   ; Bind the user script
		catch/quit either var [[do/next data var]] [data]
	][ ; Otherwise we are in script mode

		; Do file in directory if necessary
		dir: none ; in case of /local hack
		if all [file? value  file: find/last/tail value slash] [
			dir: what-dir ; save the current directory for later restoration
			change-dir copy/part value file
		]

		; Make the new script object
		scr: system/script  ; and save old one
		system/script: make system/standard/script [
			title: select hdr 'title
			header: hdr
			parent: :scr
			path: what-dir
			args: :arg
		]

		; Print out the script info
		boot-print [
			pick ["Module:" "Script:"] mod?  mold select hdr 'title
			"Version:" select hdr 'version
			"Date:" select hdr 'date
		]

		also
			; Eval the block or make the module, returned
			either mod? [ ; Import the module and set the var
				spec: reduce [hdr data do-needs/no-user hdr]
				also import catch/quit [make module! spec]
					if var [set var tail data]
			][
				do-needs hdr  ; Load the script requirements
				intern data   ; Bind the user script
				catch/quit either var [[do/next data var]] [data]
			]
			; Restore system/script and the dir
			all [system/script: :scr  dir  change-dir dir]
	]
]

make-module*: func [
	"SYS: Called by system on MAKE of MODULE! datatype."
	spec [block!] "As [spec-block body-block opt-mixins-object]"
	/local body obj mixins hidden w
][
	set [spec body mixins] spec

	; Convert header block to standard header object:
	if block? :spec [
		spec: attempt [construct/with :spec system/standard/header]
	]

	; Validate the important fields of header:
	assert/type [
		spec object!
		body block!
		mixins [object! none!]
		spec/name [word! none!]
		spec/type [word! none!]
		spec/version [tuple! none!]
		spec/options [block! none!]
	]

	; Module is an object during its initialization:
	obj: make object! 7 ; arbitrary starting size

	if find spec/options 'extension [
		append obj 'lib-base ; specific runtime values MUST BE FIRST
	]

	unless spec/type [spec/type: 'module] ; in case not set earlier

	; Collect 'export keyword exports, removing the keywords
	if find body 'export [
		unless block? select spec 'exports [repend spec ['exports make block! 10]]
		; Note: 'export overrides 'hidden, silently for now
		parse body [while [to 'export remove skip opt remove 'hidden opt [
			set w any-word! (
				unless find spec/exports w: to word! w [append spec/exports w]
			) |
			set w block! (append spec/exports collect-words/ignore w spec/exports)
		]] to end]
	]

	; Add exported words at top of context (performance):
	if block? select spec 'exports [bind/new spec/exports obj]

	; Collect 'hidden keyword words, removing the keywords. Ignore exports.
	hidden: none
	if find body 'hidden [
		hidden: make block! 10
		; Note: Exports are not hidden, silently for now
		parse body [while [to 'hidden remove skip opt [
			set w any-word! (
				unless find select spec 'exports w: to word! w [append hidden w]
			) |
			set w block! (append hidden collect-words/ignore w select spec 'exports)
		]] to end]
	]

	; Add hidden words next to the context (performance):
	if block? hidden [bind/new hidden obj]

	either find spec/options 'isolate [
		; All words of the module body are module variables:
		bind/new body obj
		; The module keeps its own variables (not shared with system):
		if object? mixins [resolve obj mixins]
		;resolve obj sys -- no longer done -Carl
		resolve obj lib
	][
		; Only top level defined words are module variables.
		bind/only/set body obj
		; The module shares system exported variables:
		bind body lib
		;bind body sys -- no longer done -Carl
		if object? mixins [bind body mixins]
	]

	bind body obj
	if block? hidden [protect/hide/words hidden]
	obj: to module! reduce [spec obj]
	do body

	;print ["Module created" spec/name spec/version]
	obj
]

; MOVE some of these to SYSTEM?
boot-banner: ajoin ["REBOL 3.0 A" system/version/3 " " system/build newline]
boot-help: "Boot-sys level - no extra features."
boot-host: none ; any host add-ons to the lib (binary)
boot-mezz: none ; built-in mezz code (put here on boot)
boot-prot: none ; built-in boot protocols
boot-exts: none ; boot extension list

export: func [
	"Low level export of values (e.g. functions) to lib."
	words [block!] "Block of words (already defined in local context)"
][
	foreach word words [repend lib [word get word]]
]

assert-utf8: function [
	"If binary data is UTF-8, returns it, else throws an error."
	data [binary!]
][
	unless find [0 8] tmp: utf? data [ ; Not UTF-8
		cause-error 'script 'no-decode ajoin ["UTF-" abs tmp]
	]
	data
]
