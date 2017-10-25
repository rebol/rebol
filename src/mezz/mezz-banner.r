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
	fmt /local str star spc a b s
][
	if string? fmt [return fmt] ; aleady built
	str: make string! 200
	star: append/dup make string! 74 #"*" 74
	spc: format ["**" 70 "**"] ""
	append str "^[[7m"
	parse fmt [
		some [
			[
				set a string! (s: format ["**  " 68 "**"] a)
			  | '= set a [string! | word! | set-word!] [
			  			b:
						  path! (b: get b/1)
						| word! (b: get b/1)
						| block! (b: reform b/1)
						| string! (b: b/1)
					]
					(s: either none? b [none][format ["**    " 11 55 "**"] reduce [a b]])
			  | '* (s: star)
			  | '- (s: spc)
			]
			(unless none? s [append append str s newline])
		]
	]
	append str "^[[0m"
	str
]

sys/boot-banner: make-banner [
	*
	-
	"REBOL 3.0 (Oldes branch)"
	-
	= Copyright: "2012 REBOL Technologies"
	= "" "2012-2017 Rebol Open Source Contributors"
	= "" "Apache 2.0 License, see LICENSE."
	= Website:  "https://github.com/oldes/r3"
	-
	= Version:  system/version
	= Platform: system/platform
	= Build:    system/build
	= Warning:  "For testing purposes only. Use at your own risk."
	-
	= Language: system/locale/language*
	= Locale:   system/locale/locale*
	= Home:     [to-local-file clean-path join what-dir system/options/home]
	-
	*
]

sys/boot-help:
{^[[1;33;49mImportant notes^[[0m:

  * Sandbox and security are not available.
  * Direct access to TCP HTTP required (no proxies).

^[[1;33;49mSpecial functions^[[0m:

  ^[[1;32;49mHelp^[[0m  - show built-in help information
}

;print make-banner boot-banner halt
;print boot-help
