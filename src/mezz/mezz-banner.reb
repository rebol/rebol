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
					(s: format ["**    " 11 55 "**"] reduce [a b])
			  | '* (s: star)
			  | '- (s: spc)
			]
			(append append str s newline)
		]
	]
	str
]

sys/boot-banner: make-banner [
	*
	-
	"REBOL 3.0 [Alpha Test]"
	-
	= Copyright: [system/build/year "REBOL Technologies"]
	= "" "All rights reserved."
	= Website:  "www.REBOL.com"
	-
	= Version:  system/version
	= Platform: system/platform
	= Build:    system/build
	= Warning:  "For testing purposes only. Use at your own risk."
	-
	= Language: system/locale/language*
	= Locale:   system/locale/locale*
	= Home:     [to-local-file system/options/home]
	-
	*
]

sys/boot-help:
{Important notes:

  * Sandbox and security are not available.
  * Direct access to TCP HTTP required (no proxies).
  * Default web browser must be available.

Special functions:

  Chat - open DevBase developer forum/BBS
  Docs - open DocBase document wiki (web)
  Bugs - open CureCode bug database (web)
  Demo - run demo launcher (from rebol.com)
  Help - show built-in help information
  Upgrade - check for newer releases
  Changes - what's new about this version (web)
}

;print make-banner boot-banner halt
;print boot-help
