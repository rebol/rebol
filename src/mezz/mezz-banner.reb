REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Startup Banner"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

make-banner: func [
	"Build startup banner."
	fmt /local str star spc a b s sf
][
	if string? fmt [return fmt] ; aleady built
	str: make string! 2000
	star: format/pad [$30.107 74 $0] "" #"*"

	spc: format [$30.107 "**" 70 "**" $0] ""
	sf: [$30.107 "**  " $35 68 $30.107 "**" $0]
	parse fmt [
		some [
			[
				set a string! (s: format sf a)
			  | set a block!  (s: format sf ajoin a)
			  | '= set a [string! | word! | set-word!] [
			  			b:
						  path! (b: get b/1)
						| word! (b: get b/1)
						| block! (b: reform b/1)
						| string! (b: b/1)
					]
					(s: either none? b [none][format [$30.107 "**    " $32 11 $31 55 $30 "**" $0] reduce [a b]])
			  | '* (s: star)
			  | '- (s: spc)
			]
			(unless none? s [append append str s newline])
		]
	]
	str
]

if #"/" <> first system/options/home [
	;make sure that home directory is absolute on all platforms
	system/options/home: clean-path join what-dir system/options/home
]

sys/boot-banner: make-banner [
	*
	-
	["REBOL 3." system/version/2 #"." system/version/3 " (Oldes branch)"]
	-
	= Copyright: "2012 REBOL Technologies"
	= "" "2012-2021 Rebol Open Source Contributors"
	= "" "Apache 2.0 License, see LICENSE."
	= Website:  "https://github.com/Oldes/Rebol3"
	-
	= Platform: [ajoin [system/platform " (" system/build/os ")"]]
	= Build:    system/build/date
	-
	= Home:     [to-local-file system/options/home]
	-
	*
]

system/license: make-banner [
	*
	-
	= Copyright: "2012 REBOL Technologies"
	= "" "2012-2021 Rebol Open Source Contributors"
	= "" "Licensed under the Apache License, Version 2.0."
	= "" "https://www.apache.org/licenses/LICENSE-2.0"
	-
	= Notice: "https://github.com/Oldes/Rebol3/blob/master/NOTICE"
	-
	*
]


sys/boot-help:
{^[[1;33mImportant notes^[[0m:

  * Sandbox and security are not fully available.
  * Direct access to TCP HTTP required (no proxies).
  * Use at your own risk.

^[[1;33mSpecial functions^[[0m:

  ^[[1;32mHelp^[[0m  - show built-in help information
}

;print make-banner boot-banner halt
;print boot-help
