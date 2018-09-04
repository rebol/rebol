REBOL [
	System:  "REBOL [R3] Language Interpreter and Run-time Environment"
	Title:   "REBOL 3 Mezzanine: Crypt"
	Author:  "Oldes"
	Rights:  "Copyright (C) 2018 Oldes. All rights reserved."
	License: "BSD-3"
	Test:    %tests/units/mezz-crypt-test.r3
]

import module [
	Title: "Cryptography related mezzanine functions"
	Name:  Crypt-utils
	Version: 0.0.1
	Exports: [load-PKIX]
][

;local helpers

	ch_space:   charset " ^-^/^M"
	ch_tag:     exclude charset [#" " - #"~"] charset #":"
	ch_val:     complement charset "\^/"
	ch_pretext: complement charset #"-"
	ch_base64:  charset [#"a" - #"z" #"A" - #"Z" #"0" - #"9" #"/" #"+" #"="]
	ch_label:   charset [#"^(21)" - #"^(2C)" #"^(2E)" - #"^(7E)" #" "]

	load-PKIX: function[
		"Loads PKIX Textual Encoded data (RFC 7468). Returns block! or binary!"
		input [string! binary!] "Data to load"
		/binary "Returns only debased binary"
		/local tag val base64-data label pre-text post-text
	][
		if binary? input [input: to-string input]

		header: copy []

		rl_label: [
			e: [
				"---- BEGIN " copy label any ch_label "----" |
				"-----BEGIN " copy label any ch_label "-----"
			] opt cr lf (trim/tail copy label)
			|
			some ch_pretext rl_label
		]

		unless parse/all input [
			s: rl_label ( pre-text: copy/part s e )
			any [
				copy tag some ch_tag #":"
				s: [
					some ch_val "^/"
					|
					any [some ch_val "\^/"] some ch_val "^/"
				] e: (
					val: trim/head/tail copy/part s e
					replace/all val "\^/" ""
					if all [#"^"" = val/1 #"^"" = last val][
						remove back tail remove val
					] 
					repend header reduce [tag val]
				)
			]
			copy base64-data some [ch_base64 | ch_space] 
			[
				"---- END " label "----" |
				"-----END " label "-----"
			] any [cr | lf]
			copy post-text to end
		][	return none ]
		
		either binary [
			try [debase base64-data]
		][
			compose/only [
				label:     (trim/tail label)
				binary:    (try [debase base64-data])
				header:    (new-line/skip header true 2)
				pre-text:  (trim/head/tail pre-text)
				post-text: (trim/head/tail post-text)
			]
		]
	]
] ;- end of module