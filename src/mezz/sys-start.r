REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Sys: Startup"
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
		The boot binding of this module is SYS then LIB deep.
		Any non-local words not found in those contexts WILL BE
		UNBOUND and will error out at runtime!
	}
]

start: func [
	"INIT: Completes the boot sequence. Loads extras, handles args, security, scripts."
	/local file tmp script-path script-args code
] bind [ ; context is: system/options

	;** Note ** We need to make this work for lower boot levels too!

	;-- DEBUG: enable these lines for debug or related testing
	loud-print ["Starting... boot level:" boot-level]
	;trace 1
	;crash-here ; test error handling (undefined word)

	boot-level: any [boot-level 'full]
	start: 'done ; only once
	init-schemes ; only once

	;-- Print minimal identification banner if needed:
	if all [
		not quiet
		any [flags/verbose flags/usage flags/help]
	][
		boot-print boot-banner ; basic boot banner only
	]
	if any [do-arg script] [quiet: true]

	;-- Set up option/paths for /path, /boot, /home, and script path (for SECURE):
	path: dirize any [path home]
	home: dirize home
	;if slash <> first boot [boot: clean-path boot] ;;;;; HAVE C CODE DO IT PROPERLY !!!!
	home: file: first split-path boot
	if file? script [ ; Get the path (needed for SECURE setup)
		script-path: split-path script
		case [
			slash = first first script-path []		; absolute
			%./ = first script-path [script-path/1: path]	; curr dir
			'else [insert first script-path path]	; relative
		]
	]

	;-- Convert command line arg strings as needed:
	script-args: args ; save for below
	foreach [opt act] [
		args    [parse args ""]
		do-arg  block!
		debug   block!
		secure  word!
		import  [to-rebol-file import]
		version tuple!
	][
		set opt attempt either block? act [act][
			[all [get opt to get act get opt]]
		]
	]
	; version, import, secure are all of valid type or none

	if flags/verbose [print self]

	;-- Boot up the rest of the run-time environment:
	;   NOTE: this can still be split up into more boot-levels !!!
	;   For example: mods, plus, host, and full
	if boot-level [
		load-boot-exts
		loud-print "Init mezz plus..."

		do bind-lib boot-mezz
		boot-mezz: 'done

		foreach [spec body] boot-prot [module spec body]
		;do bind-lib boot-prot
		;boot-prot: 'done

		;-- User is requesting usage info:
		if flags/help [lib/usage quiet: true]

		;-- Print fancy banner (created by mezz plus):
		if any [
			flags/verbose
			not any [quiet script do-arg]
		][
			boot-print boot-banner
		]
		if boot-host [
			loud-print "Init host code..."
			do load boot-host
			boot-host: none
		]
	]

	;-- Setup SECURE configuration (a NO-OP for min boot)
	lib/secure (case [
		flags/secure [secure]
		flags/secure-min ['allow]
		flags/secure-max ['quit]
		file? script [compose [file throw (file) [allow read] (first script-path) allow]]
		'else [compose [file throw (file) [allow read] %. allow]] ; default
	])

	;-- Evaluate rebol.r script:
	loud-print ["Checking for rebol.r file in" file]
	if exists? file/rebol.r [do file/rebol.r] ; bug#706

	;-- Make the user's global context:
	tmp: make object! 320
	append tmp reduce ['system :system]
	system/contexts/user: tmp

	;boot-print ["Checking for user.r file in" file]
	;if exists? file/user.r [do file/user.r]

	boot-print ""

	;if :lib/secure [protect-system-object]

	; Import module?
	if import [lib/import import]

	;-- Evaluate: --do "some code" if found
	if do-arg [
		do intern do-arg
		unless script [quit/now]
	]

	;-- Evaluate script argument?
	either file? script [
		; !!! Would be nice to use DO for this section. !!!
		; NOTE: We can't use DO here because it calls the code it does with CATCH/quit
		;   and we shouldn't catch QUIT in the top-level script, we should just quit.
		; script-path holds: [dir file] for script
		assert/type [script-path [block!] script-path/1 [file!] script-path/2 [file!]]
		; /path dir is where our script gets started.
		change-dir first script-path
		either exists? second script-path [
			boot-print ["Evaluating:" script]
			code: load/header/type second script-path 'unbound
			; update system/script (Make into a function?)
			system/script: make system/standard/script [
				title: select first code 'title
				header: first code
				parent: none
				path: what-dir
				args: script-args
			]
			either 'module = select first code 'type [
				code: reduce [first+ code code]
				if object? tmp: do-needs/no-user first code [append code tmp]
				import make module! code
			][
				do-needs first+ code
				do intern code
			]
			if flags/halt [lib/halt]
		] [
			cause-error 'access 'no-script script
		]
	][
		boot-print boot-help
	]

	exit

] system/options
