REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Startup Banner"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2026 Rebol Open Source Developers
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
	append str format/pad [/reset #"╔" 74 "╗^/"] "" #"═"

	spc: format [#"║" /banner 74 /reset #"║"] ""
	sf: [#"║" /banner "  " /magenta 72 /reset #"║"]
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
					(s: either none? b [none][format [#"║" /banner "    " /green 11 /red 59 /reset #"║"] [a b]])
			  | '* (s: star)
			  | '- (s: spc)
			]
			(unless none? s [append append str s newline])
		]
	]
	append str format/pad [#"╚" 74 "╝^/"] "" #"═"
	str
]

if all [
    system/options/home
    #"/" <> first system/options/home
][
	;make sure that home directory is absolute on all platforms
	system/options/home: clean-path join what-dir system/options/home
]

sys/boot-banner: make-banner [
	-
	["REBOL/" system/product #" " system/version " (Oldes branch)"]
	-
	= Copyright: "2012 REBOL Technologies"
	= "" "2012-2026 Rebol Open Source Contributors"
	= "" "Apache 2.0 License, see LICENSE."
	= Website:  "https://github.com/Oldes/Rebol3"
	-
	= Platform: [
		ajoin [
			system/platform " | " system/build/target
			any [all [system/build/compiler join " | " system/build/compiler] ()]
		]
	]	
	= Build:    system/build/date
	-
	= Home: [to-local-file any [system/options/home %"_"]]
	= Data: [to-local-file any [system/options/data %"_"]]
	-
]

system/license: make-banner [
	-
	= Copyright: "2012 REBOL Technologies"
	= "" "2012-2026 Rebol Open Source Contributors"
	= "" "Licensed under the Apache License, Version 2.0."
	= "" "https://www.apache.org/licenses/LICENSE-2.0"
	-
	= Notice: "https://github.com/Oldes/Rebol3/blob/master/NOTICE"
	-
]

;sys/boot-banner: ajoin ["REBOL/" system/product #" " system/version " (Oldes branch)"]
;system/license: "Licensed under the Apache License, Version 2.0."

append sys/boot-banner format [
	LF /bright-yellow "Important notes" /reset {:

  * Use at your own risk.
  * } /bright-green "//" /reset { is now used as } /bright-red "integer-divide" /reset {, for } /bright-red "remainder" /reset { use } /bright-green "%" /reset { or } /bright-green "%%" /reset { (Euclidean division)!
  * For Python compatible } /bright-red "modulo" /reset " use " /bright-green "modulo/floor" /reset {.

} /bright-yellow {Special functions} /reset {:

  } /bright-green "Help" /reset { - show built-in help information
}] _

if system/options/no-color [
	sys/remove-ansi sys/boot-banner
	sys/remove-ansi system/license
]

;print make-banner boot-banner halt
;print boot-help
