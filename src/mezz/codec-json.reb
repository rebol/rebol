Rebol [
	Name:    json
	Type:    module
	Exports: [to-json load-json]
	Version: 0.1.2
	Title:   "Codec: JSON"
	Purpose: "Convert Rebol value into JSON format and back."
	File:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-json.reb
	Author: [
		"Gregg Irwin" {
			Ported from %json.r by Romano Paolo Tenca, Douglas Crockford, 
			and Gregg Irwin.
			Further research: json libs by Chris Ross-Gill, Kaj de Vos, and
			@WiseGenius.
		}
		"Gabriele Santilli" {
			See History.
		}
		"Oldes" {
			Slightly modified Red's version (0.0.4) for use in Rebol (Oldes' branch).
		}
	]
	History: [
		0.0.1 10-Sep-2016 "Gregg"    "First release. Based on %json.r"    
		0.0.2  9-Aug-2018 "Gabriele" "Refactoring and minor improvements"
		0.0.3 31-Aug-2018 "Gabriele" "Converted to non-recursive version"
		0.0.4  9-Oct-2018 "Gabriele" "Back to an easier to read recursive version"
		0.1.0 13-Feb-2020 "Oldes"    "Ported Red's version back to Rebol"
		0.1.1 22-Dec-2021 "Oldes"    "Handle '+1' and/or '-1' JSON keys"
		0.1.2  4-May-2023 "Oldes"    "Fixed decode-unicode-char"
	]

	Rights:  {
		Copyright (C) 2019 Red Foundation. All rights reserved.
		Copyright 2020-2023 Rebol Open Source Contributors
	}
	License: {
		Distributed under the Boost Software License, Version 1.0.
		See https://github.com/red/red/blob/master/BSL-License.txt
	}

	Repository: [
		https://github.com/red/red/blob/master/environment/codecs/json.red
		https://github.com/giesse/red-json
	]
	References: [
		http://www.json.org/
		https://www.ietf.org/rfc/rfc4627.txt
		http://www.rfc-editor.org/rfc/rfc7159.txt
		http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf
		https://github.com/rebolek/red-tools/blob/master/json.red
		https://github.com/rgchris/Scripts/blob/master/red/altjson.red
	]
]

;----------------------------------------------------------------
;@@ load-json                                                    


;-----------------------------------------------------------
;-- Generic support funcs


translit: func [
	"Transliterate sub-strings in a string"
	string [string!] "Input (modified)"
	rule   [block! bitset!] "What to change"
	xlat   [block! function!] "Translation table or function. MUST map a string! to a string!."
	/local val
][
	parse string [
		some [
			change copy val rule (either block? :xlat [xlat/:val][xlat val])
			| skip
		]
	]
	string
]

;-----------------------------------------------------------
;-- JSON backslash escaping

;TBD: I think this can be improved. --Gregg
	
json-to-rebol-escape-table: [
;   JSON Rebol
	{\"} "^""
	{\\} "\"
	{\/} "/"
	{\b} "^H"   ; #"^(back)"
	{\f} "^L"   ; #"^(page)"
	{\n} "^/"
	{\r} "^M"
	{\t} "^-"
]
rebol-to-json-escape-table: reverse copy json-to-rebol-escape-table

rebol-esc-ch: charset {^"^-\/^/^M^H^L} ; Rebol chars requiring JSON backslash escapes
json-esc-ch:  charset {"t\/nrbf}       ; Backslash escaped JSON chars
json-escaped: [#"\" json-esc-ch]       ; Backslash escape rule

decode-backslash-escapes: func [string [string!] "(modified)"][
	translit string json-escaped json-to-rebol-escape-table
]

encode-backslash-escapes: func [string [string!] "(modified)"][
	translit string rebol-esc-ch rebol-to-json-escape-table
]

;-----------------------------------------------------------
;-- JSON decoder                                            
;-----------------------------------------------------------

;# Basic rules
hex-char: system/catalog/bitsets/hex-digits
digit:    system/catalog/bitsets/numeric
ws:       system/catalog/bitsets/whitespace
ws*: [any ws]
ws+: [some ws]
sep: [ws* #"," ws*]							   ; JSON value separator
non-zero-digit: #[bitset! #{0000000000007FC0}] ;= charset "123456789"
; Unescaped chars (NOT creates a virtual bitset)
chars: #[bitset! [not bits #{FFFFFFFF2000000000000008}]] ;=charset [not {\"} #"^@"-#"^_"]

; chars allowed in Rebol word! values - note that we don't allow < and > at all even though they are somewhat valid in word!
not-word-char: #[bitset! #{00640000BCC9003A8000001E000000140000000080}] ;=charset {/\^^,[](){}"#%$@:;^/^(00A0) ^-^M<>}
word-1st:  complement append union not-word-char digit #"'"
word-char: complement not-word-char

;-----------------------------------------------------------
;-- JSON value rules                                        
;-----------------------------------------------------------

;-----------------------------------------------------------
;-- Number  
sign: [#"-"]
; Integers can't have leading zeros, but zero by itself is valid.
int:  [[non-zero-digit any digit] | digit]
frac: [#"." some digit]
exp:  [[#"e" | #"E"] opt [#"+" | #"-"] some digit]
number: [opt sign  int  opt frac  opt exp]
numeric-literal: :number

;-----------------------------------------------------------
;-- String  
string-literal: [
	#"^"" copy _str [
		any [some chars | #"\" [#"u" 4 hex-char | json-esc-ch]]
	] #"^"" (
		if not empty? _str: any [_str copy ""][
			;!! If we reverse the decode-backslash-escapes and replace-unicode-escapes
			;!! calls, the string gets munged (extra U+ chars). Need to investigate.
			decode-backslash-escapes _str			; _str is modified
			replace-unicode-escapes _str			; _str is modified
			;replace-unicode-escapes decode-backslash-escapes _str
		]
	)
]

decode-unicode-char: func [
	"Convert \uxxxx format (NOT simple JSON backslash escapes) to a Unicode char"
	ch [string!] "4 hex digits"
][
	try/with [to char! to integer! debase ch 16][ none ]
]

replace-unicode-escapes: func [
	s [string!] "(modified)"
	/local c
][
	parse s [
		any [
			some chars								; Pass over unescaped chars
			| json-escaped							; Pass over simple backslash escapes
			| change ["\u" copy c 4 hex-char] (decode-unicode-char c) ()
			;| "\u" followed by anything else is an invalid \uXXXX escape
		]
	]
	s
]
;str: {\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%&*()_+-=[]{}|;:',./<>?}
;mod-str: decode-backslash-escapes json-ctx/replace-unicode-escapes copy str
;mod-str: json-ctx/replace-unicode-escapes decode-backslash-escapes copy str

;-----------------------------------------------------------
;-- Object  
json-object: [
	; Emit a new block to our output target, and push it on our
	; working stack, to handle nested structures. Emit returns
	; the insertion point for another value to go out into '_res,
	; but we want the target to be the block we just added, so
	; we reset '_res to that after 'emit is done.
	#"{" (push emit _tmp: copy []  _res: _tmp)
	ws* opt property-list
	; This is a little confusing. We're at the tail of our current
	; output target, which is on our stack. We pop that, then need
	; to back up one, which puts us AT the block of values we 
	; collected for this object in our output target. i.e., the 
	; values are in a sub-block at the first position now. We use
	; that (FIRST) to make a map! and replace the block of values
	; with the map! we just made. Note that a map is treated as a
	; single value, like an object. Using a block as the new value
	; requires using `change/only`.
	#"}" (
		_res: back pop
		_res: change _res make map! first _res
	)
]

property-list: [property any [sep property]]
property: [
	json-name (
		try [_str: to word! _str]
		emit _str
	)
	json-value
]
json-name: [ws* string-literal ws* #":"]

;-----------------------------------------------------------
;-- List  
array-list: [json-value any [sep json-value]]
json-array: [
	; Emit a new block to our output target, and push it on our
	; working stack, to handle nested structures. Emit returns
	; the insertion point for another value to go out into '_res,
	; but we want the target to be the block we just added, so
	; we reset '_res to that after 'emit is done.
	#"[" (push emit _tmp: copy []  _res: _tmp)
	ws* opt array-list
	#"]" (_res: pop)
]

;-----------------------------------------------------------
;-- Any JSON Value (top level JSON parse rule)  
json-value: [
	ws*
	[
		"true"    (emit true)							; Literals must be lowercase
		| "false" (emit false)
		| "null"  (emit none)
		| json-object
		| json-array
		| string-literal (emit _str)
		| copy _str numeric-literal (emit load _str)	; Number
		mark:   										; Set mark for failure location
	]
	ws*
]

;-----------------------------------------------------------
;-- Decoder data structures  

; The stack is used to handle nested structures (objects and lists)
stack: copy []
push:  func [val][append/only stack val]
pop:   does [take/last stack]

_out: none	; Our overall output target/result                          
_res: none	; The current output position where new values are inserted
_tmp: none  ; Temporary
_str: none	; Where string value parse results go               
mark: none	; Current parse position

; Add a new value to our output target, and set the position for
; the next emit to the tail of the insertion.
;!! I really don't like how this updates _res as a side effect. --Gregg
emit: func [value][_res: insert/only _res value]

;-----------------------------------------------------------
;-- Main decoder func  

load-json: func [
	"Convert a JSON string to Rebol data"
	input [string!] "The JSON string"
][
	_out: _res: copy []		; These point to the same position to start with
	mark: input
	either parse/case input json-value [pick _out 1][
		make error! form reduce [
			"Invalid JSON string. Near:"
			either tail? mark ["<end of input>"][mold copy/part mark 40]
		]
	]
]


;----------------------------------------------------------------
;@@ to-json                                                      

indent: none
indent-level: 0
normal-chars: none
escapes: #[map! [
	#"^"" {\"}
	#"\"  "\\"
	#"^H" "\b"
	#"^L" "\f"
	#"^/" "\n"
	#"^M" "\r"
	#"^-" "\t"
]]

init-state: func [ind ascii?][
	indent: ind
	indent-level: 0
	; 34 is double quote "
	; 92 is backslash \
	normal-chars: either ascii? [
		#[bitset! #{00000000DFFFFFFFFFFFFFF7FFFFFFFF}]
		;= charset [32 33 35 - 91 93 - 127]
	][
		#[bitset! [not bits #{FFFFFFFF2000000000000008}]]
		;= complement charset [0 - 31 34 92]
	]
]

emit-indent: func [output level][
	indent-level: indent-level + level
	append/dup output indent indent-level
]

emit-key-value: function [output sep map key][
	value: select/case map :key
	if any-word?   :key [key: form key]
	unless string? :key [key: mold :key]
	rebol-to-json-value output :key
	append output :sep
	rebol-to-json-value output :value
]

rebol-to-json-value: function [output value][
	special-char: none
	switch/default type?/word :value [
		none!    [append output "null"]
		logic!   [append output pick ["true" "false"] value]
		integer!
		decimal! [append output value]
		percent! [append output to decimal! value]
		string!  [
			append output #"^""
			parse value [
				any [
					mark1: some normal-chars mark2: (append/part output mark1 mark2)
					|
					set special-char skip (
						either escape: select escapes special-char [
							append output escape
						][
							insert insert tail output "\u" to-hex/size to integer! special-char 4
						]
					)
				]
			]
			append output #"^""
		]
		block! [
			either empty? value [
				append output "[]"
			][
				either indent [
					append output "[^/"
					emit-indent output +1
					rebol-to-json-value output first value
					foreach v next value [
						append output ",^/"
						append/dup output indent indent-level
						rebol-to-json-value output :v
					]
					append output #"^/"
					emit-indent output -1
				][
					append output #"["
					rebol-to-json-value output first value
					foreach v next value [
						append output #","
						rebol-to-json-value output :v
					]
				]
				append output #"]"
			]
		]
		map!
		object! [
			keys: words-of value
			either empty? keys [
				append output "{}"
			][
				either indent [
					append output "{^/"
					emit-indent output +1
					emit-key-value output ": " value first keys
					foreach k next keys [
						append output ",^/"
						append/dup output indent indent-level
						emit-key-value output ": " value :k
					]
					append output #"^/"
					emit-indent output -1
				][
					append output #"{"
					emit-key-value output #":" value first keys
					foreach k next keys [
						append output #","
						emit-key-value output #":" value :k
					]
				]
				append output #"}"
			]
		]
	][
		rebol-to-json-value output either any-block? :value [
			to block! :value
		][
			either any-string? :value [form value][mold :value]
		]
	]
	output
]

to-json: function [
	"Convert Rebol data to a JSON string"
	data
	/pretty indent [string!] "Pretty format the output, using given indentation"
	/ascii "Force ASCII output (instead of UTF-8)"
][
	result: make string! 4000
	init-state indent ascii
	rebol-to-json-value result data
]



register-codec [
	name:  'json
	type:  'text
	title: "JavaScript Object Notation"
	suffixes: [%.json]

	encode: func [data [any-type!]][
		to-json data
	]
	decode: func [text [string! binary! file!]][
		if file?   text [text: read text]
		if binary? text [text: to string! text]
		load-json text
	]
]