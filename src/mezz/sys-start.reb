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
	/local tmp script-path script-args code ver
] bind [ ; context is: system/options (must use full path sys/log/.. as there is options/log too!)

	;** Note ** We need to make this work for lower boot levels too!

	;-- DEBUG: enable these lines for debug or related testing
	sys/log/debug 'REBOL ["Starting... boot level:" boot-level]
	;trace 1
	;crash-here ; test error handling (undefined word)

	boot-level: any [boot-level 'full]
	start: 'done ; only once
	init-schemes ; only once

	ver: load/type lib/version 'unbound
	system/product:        ver/2
	system/version:        ver/3
	system/platform:       ver/4
	system/build/os:       ver/5
	system/build/arch:     ver/6
	system/build/vendor:   ver/7
	system/build/sys:      ver/8
	system/build/abi:      ver/9
	system/build/compiler: ver/10
	system/build/target:   ver/11
	system/build/date:     ver/12
	system/build/git:      ver/13

	if flags/verbose [system/options/log/rebol: 3] ;maximum log output for system messages

	;-- Print minimal identification banner if needed:
	if all [
		not quiet
		any [flags/verbose flags/usage flags/help]
	][
		; basic boot banner only
		prin "^/  "
		print boot-banner: form ver
	]
	if any [do-arg script] [quiet: true]

	;-- Set up option/paths for /path, /boot, /home, and script path (for SECURE):         
	;sys/log/more 'REBOL ["Initial path:" path] ; current dir
	;sys/log/more 'REBOL ["Initial boot:" boot] ; executable
	;sys/log/more 'REBOL ["Initial home:" home] ; always NONE at this state! 
	;-  1. /path - that is current directory (resolved from C as a part of args processing)
	;-  2. /boot - path to executable (resolved from C as well)                            
	;-  3. /home - preferably one of environment variables or current starting dir         
	home: dirize to-rebol-file any [
		get-env "REBOL_HOME"  ; User can set this environment variable with own location
		get-env "HOME"        ; Default user's home directory on Linux
		get-env "USERPROFILE" ; Default user's home directory on Windows
		path                  ; Directory where we started (O: not sure with this one)
	]

	if file? script [ ; Get the path (needed for SECURE setup)
		script: any [to-real-file script script]
		script-path: split-path script
	]

	;-- Convert command line arg strings as needed:
	script-args: args ; save for below
	foreach [opt act] [
		;args    [parse args ""]
		do-arg  string!
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

	;if flags/verbose [print self]

	;-- Boot up the rest of the run-time environment:
	;   NOTE: this can still be split up into more boot-levels !!!
	;   For example: mods, plus, host, and full
	if boot-level [
		load-boot-exts
		sys/log/debug 'REBOL "Init mezz plus..."

		do bind-lib boot-mezz
		boot-mezz: 'done

		;loud-print "Init protocols..."
		foreach [spec body] boot-prot [module spec body]
		;do bind-lib boot-prot
		boot-prot: 'done

		;-- User is requesting usage info:
		if flags/help [
			lib/usage
			unless flags/halt [quit/now]
			quiet: true
		]

		if boot-host [
			sys/log/debug 'REBOL "Init host code..."
			;probe load boot-host
			do load boot-host
			boot-host: none
		]
		;-- Print fancy banner (created by mezz plus):
		if any [
			flags/verbose
			not any [quiet script do-arg]
		][
			print boot-banner
		]
	]

	;-- Setup SECURE configuration
	if select lib 'secure [
		lib/secure (case [
			flags/secure [secure]
			flags/secure-min ['allow]
			flags/secure-max ['quit]
			file? script [
				compose [
					file throw
					(path) [allow read]
					(home) [allow read]
					(first script-path) allow
				]
			]
			'else ['none] ;compose [file throw (file) [allow read] %. allow]] ; default
		])
	]

	;-- Evaluate rebol.reb script:
	;@@ https://github.com/Oldes/Rebol-issues/issues/706
	tmp: first split-path boot
	sys/log/info 'REBOL ["Checking for rebol.reb file in" tmp]
	
	if all [
		#"/" = first tmp ; only if we know absolute path
		exists? tmp/rebol.reb
	][
		try/except [do tmp/rebol.reb][sys/log/error 'REBOL system/state/last-error]
	]

	;-- Make the user's global context:
	tmp: make object! 320
	append tmp reduce ['REBOL :system 'lib-local :tmp]
	system/contexts/user: tmp

	sys/log/info 'REBOL ["Checking for user.reb file in" home]
	if exists? home/user.reb [
		try/except [do home/user.reb][sys/log/error 'REBOL system/state/last-error]
	]


	;if :lib/secure [protect-system-object]

	; Import module?
	if import [lib/import import]

	;-- Evaluate: --do "some code" if found
	if do-arg [
		do intern load/all do-arg
		unless script [quit/now]
	]

	;-- Evaluate script argument?
	if file? script [
		; !!! Would be nice to use DO for this section. !!!
		; NOTE: We can't use DO here because it calls the code it does with CATCH/quit
		;   and we shouldn't catch QUIT in the top-level script, we should just quit.
		; script-path holds: [dir file] for script
		assert/type [script-path [block!] script-path/1 [file!] script-path/2 [file!]]
		; /path dir is where our script gets started.
		change-dir first script-path
		either exists? second script-path [
			sys/log/info 'REBOL ["Evaluating:" script]
			code: load/header/type second script-path 'unbound
			; update system/script (Make into a function?)
			system/script: make system/standard/script [
				title: select first code 'title
				header: first code
				parent: none
				path: what-dir
				; if there was used --args option, pass to script only this value
				; and ignore other optional arguments
				args: either system/options/flags/args [to block! first script-args][script-args]
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
	]

	exit

] system/options
