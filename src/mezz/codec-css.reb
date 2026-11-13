Rebol [
	title:   "CSS utilities"
	name:    css
	type:    module
	version: 0.1.1
	date:    14-Mar-2025
	author:  @Oldes
	home:    https://github.com/Oldes/Rebol-CSS
	rights:  MIT
	purpose: {Tokenize CSS content and minify it}
	exports: [css-tokenize css-minify]
]

rules: context [
	;; bitsets
	*nonascii:  complement charset [0 - 177]
	*hexa:      charset [#"a"-#"f" #"A"-#"F" #"0"-#"9"]
	*notesc:    complement charset [#"a"-#"f" #"A"-#"F" #"0"-#"9" "^-^/^M^L"]
	*nmstart:   charset [#"_" #"a"-#"z" #"A"-#"Z"]
	*nmchar:    charset [#"_" #"a"-#"z" #"A"-#"Z" #"0"-#"9" #"-"]
	*num:       charset [#"0"-#"9"]
	*str:       complement charset "^/^M^L\"
	*n: charset "nN"
	*o: charset "oO"
	*t: charset "tT"
	*combinator: charset "+>~"
	*token-char: charset "{}:@;(),"
	*whitespace: charset " ^-^/^M^L"
	;; rules
	=escape:    [#"\" [*notesc | 1 6 *hexa]]
	=nmchar:    [some [*nmchar | *nonascii | =escape]]
	=number:    [opt [#"+" | #"-"] any *num #"." some *num | some *num]
	=newline:   [lf | crlf | cr | 12] ;= 12 = form feed char
	=str:       [*str | *nonascii | #"\" =newline | =escape]
	=string1:   [#"^"" some [#"^"" break | =str]]
	=string2:   [#"'"  some [#"'"  break | =str]]
	=string:    [=string1 | =string2]
	=invalid1:  [#"^"" any str]
	=invalid2:  [#"'" any str]
	=invalid:   [=invalid1 | =invalid2]
	=ws:        [any WHITESPACE]
	=nmstart:   [*nmstart | *nonascii | =escape]
	=name:      [some *nmchar]
	=ident:     [opt #"-" =nmstart any =nmchar]
	=hash: [#"#" =name] 
	=namespace_prefix: [opt [=ident | #"*"] #"|"]            ;; e.g. svg| in: svg|circle {...} 
	=type_selector:    [opt =namespace_prefix =ident]
	=universal:        [opt =namespace_prefix #"*"]
	=class:            [#"." =ident]
	=attrib: [
		#"[" =ws opt =namespace_prefix =ident =ws
		opt [
			[
				"^=" | ;; PREFIXMATCH
				"$=" | ;; SUFFIXMATCH
				"*=" | ;; SUBSTRINGMATCH
				#"=" |
				"~=" | ;; INCLUDES
				"|=" | ;; DASHMATCH
			] =ws [=ident | =string] =ws
		]
		#"]"
	]
	=pseudo: [#":" opt #":" [ =ident | functional_pseudo ]] ;; e.g. :hover or ::before
]

css-tokenize: function/with [
	;@@ https://www.w3.org/TR/css-syntax-3/#tokenizer-algorithms
	css [string! binary! url! file!]
][
	case [
		any [file? css url? css] [css: read/string css]
		binary? css [css: to string! css]
	]
	parse/case css [
		any *whitespace
		collect any [
			  "<!--" thru "-->" any *whitespace
			| "/*" thru "*/" any *whitespace
			| keep [
				  *token-char
				| *combinator
				| =string
				| =type_selector
				| =universal
				| =hash
				| =class
				| =attrib
				| =pseudo
				| =ident
			]
			| copy tmp: =number keep (transcode/one tmp) opt [keep ["%" | =name] ]
			| some *whitespace keep (SP)
			| keep skip
		]
	]
] :rules

css-minify: function [tokens][
	unless block? tokens [tokens: css-tokenize tokens]
	ajoin parse tokens [collect any [
		  #";" opt #" " ahead #"}" ;== removes ; in front of }
		| #"{" any [#" " | #";"] keep (#"{")
		| #"}" #" " keep (#"}")
		| #":" #" " keep (#":")
		| #";" any [#" " | #";"] [ahead #"}" | keep (#";")]
		| #"(" (expr?: on ) #" " keep (#"(")
		| #")" (expr?: off) #" " keep (#")")
		| #"," #" " keep (#",")
		| #" " [
			  ahead [#"{" | #"}" | #"(" | #")" | #":" | #";" | #","]
			| keep [#">" | #"~"] opt #" "
			| if (not expr?) keep #"+" opt #" "
			| end
		]
		| quote 0 [
			"ms" keep ("0s") |        ;== 0ms -> 0s
			[#"%" | string!] not #"," keep (0) ;== zero percent/dimension
		]
		| #"+" ahead number!
		| #"-" ahead quote 0
		;= and in a media query must be separated with a space
		| "and" #" " ahead #"(" keep ("and ")
		| if (expr?) [
			  #" " #"/" #" " keep (#"/")
			| #" " #"*" #" " keep (#"*")
		]
		| "black" keep ("#000")
		| "white" keep ("#fff")
		| keep skip
	]]
]

register-codec [
	name:  'css
	type:  'text
	title: "Cascading Style Sheets"
	suffixes: [%.css]

	decode: function [
		data [binary! file! url!]
	][
		css-tokenize data
	]
	encode: function[data [block!]][
		css-minify data
	]
]

