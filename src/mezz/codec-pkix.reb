REBOL [
	Title:   "Codec: PKIX codec for public-Key Infrastructure (X.509)"
	Author:  "Oldes"
	Rights:  "Copyright (C) 2018 Oldes. All rights reserved."
	License: "BSD-3"
	Test:    %tests/units/crypt-test.r3
]

wrap [
	;local helpers
	ch_space:   charset " ^-^/^M"
	ch_tag:     exclude charset [#" " - #"~"] charset #":"
	ch_val:     complement charset "\^/"
	ch_pretext: complement charset #"-"
	ch_base64:  charset [#"a" - #"z" #"A" - #"Z" #"0" - #"9" #"/" #"+" #"="]
	ch_label:   charset [#"^(21)" - #"^(2C)" #"^(2E)" - #"^(7E)" #" "]

	register-codec [
		name:  'pkix
		type:  'cryptography
		title: "Public-Key Infrastructure (X.509)"
		suffixes: [%.pem %.ssh %.certSigningRequest]
		decode: function[
			"Loads PKIX Textual Encoded data (RFC 7468). Returns block! or binary!"
			input [string! binary!] "Data to load"
			/binary "Returns only debased binary"
			/local tag val base64-data label pre-text post-text
		][
			header: copy []

			rl_label: [
				e: [
					"---- BEGIN " copy label any ch_label "----" |
					"-----BEGIN " copy label any ch_label "-----"
				] opt cr lf
				|
				some ch_pretext rl_label
			]

			unless parse input [
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
						repend header reduce [to string! tag to string! val]
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
				try [debase base64-data 64]
			][
				compose/only [
					label:     (trim/tail to string! label)
					binary:    (try [debase base64-data 64])
					header:    (new-line/skip header true 2)
					pre-text:  (trim/head/tail to string! pre-text)
					post-text: (trim/head/tail to string! post-text)
				]
			]
		]
		identify: function[data [string! binary!]][
			rl_label: [
				[
					"---- BEGIN " any ch_label "----" |
					"-----BEGIN " any ch_label "-----"
				] opt cr lf
				|
				some ch_pretext rl_label
			]
			parse data [rl_label to end]
    	]
    ]
]