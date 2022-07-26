REBOL [
	Name:    mime-field
	Type:    module
	Options: [delay]
	Version: 1.0.0
	Title:   "Codec: MIME header field encoding"
	Author:  "Oldes"
	Rights:  "Copyright (C) 2022 Oldes. All rights reserved."
	License: "BSD-3"
	Test:    %tests/units/codec-test.r3
	Specification: https://datatracker.ietf.org/doc/html/rfc2045
]

import 'quoted-printable

register-codec [
	name: 'mime-field
	type: 'text
	title: "MIME header's field (rfc2045)"
	
	decode: function/with [
		"Decodes MIME header's field (rfc2045)"
		data [binary! any-string!]
	][
		output: either binary? data [ copy data ][ to binary! data ]
		parse output [
			no-case ; binary input is by default case-sensitive
			any [
				to ch-crlf= [
					s: [#"^M" #"^/" | #"^/" #"^M" | #"^/"] some ch-space e: [
						ahead "=?" (e: remove/part s e)
						| (e: change/part s #" " e)
					] :e
					|
					s: "=?" [
						copy chr: to #"?" skip
						; q and b bellow are handled as case-insensitive!
						[#"q" (enc: #"q") | #"b" (enc: #"b")] #"?"
						copy txt: to "?=" 2 skip
						;Any amount of linear-space-white between 'encoded-word's,
						;even if it includes a CRLF followed by one or more SPACEs,
						;is ignored for the purposes of display.
						[some whitespace e: "=?" | e:]
						(
							txt: either enc = #"q" [
								qp-decode/space txt
							][	debase txt 64 ]
							if chr <> "utf-8" [
								txt: iconv txt to string! chr
							]
							e: change/part s txt e
						) :e
					]
					| skip
				]	
			]
		]
		to data output
	][
		ch-crlf:  system/catalog/bitsets/crlf
		ch-crlf=: #[bitset! #{0024000000000004}] ;charset "^/^M="
		ch-space: system/catalog/bitsets/space
		qp-decode: :codecs/quoted-printable/decode
	]

	encode: function/with [data][
		; First try if there is needed any encoding...
		if parse data [
			opt [some key-chars #":" some space]
			any safe-chars
		][	return data	]
		; Encode using plain BASE64 encoding divided into multiple 
		; parts not to exceed line length limit 76 bytes...
		; It is stupid, but the most simplest solution, which is
		; good for Facebook, so may be good for us too.
		out: make binary! 1.5 * length? data
		; The split cannot be in the middle of the unicode encoding,
		; so the input should be string (so the length bellow is in codepoints)
		parse data [
			s: opt [some key-chars #":" some space e: (
				append out copy/part s e
			)]
			any [
				; taking maximum input of 17 codepoints should be enough to fit
				; into the MIME line length limit.
				s: 1 17 skip e: (
					append out #{3D3F5554462D383F423F} ;= "=?UTF-8?B?"
					append out enbase/part s 64 e
					append out #{3F3D0D0A20}           ;="?=^M^/ "
				)
			]
		]
		take/part/last out 3 ; removed the last CRLFSP chars
		to data out
	][
		safe-chars: #[bitset! #{004000008000FFC07FFFFFE07FFFFFE0}] ;= alpha-numeric + space
		key-chars:  #[bitset! #{000000000004FFC07FFFFFE17FFFFFE0}]
	]
]