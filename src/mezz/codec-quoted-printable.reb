REBOL [
	Name:    quoted-printable
	Type:    module
	;Options: [delay]
	Version: 1.0.0
	Title:   "Codec: quoted-printable encoding"
	Author:  "Oldes"
	Rights:  "Copyright (C) 2022 Oldes. All rights reserved."
	License: "BSD-3"
	Test:    %tests/units/codec-test.r3
	Specification: https://en.wikipedia.org/wiki/Quoted-printable
]

register-codec [
	name: 'quoted-printable
	type: 'text
	title: "quoted-printable encoding"
	
	decode: function [
		"Decodes quoted-printable data"
		data [binary! any-string!]
	][
		output: either binary? data [ copy data ][ to binary! data ]
		; remove soft line breaks
		parse output [any [to #"=" remove [#"=" [LF | CR LF]] | skip] to end]
		to data dehex/escape output #"="
	]

	encode: function/with [
		"Encodes data using quoted-printable encoding"
		data [binary! any-string!]
	][
		output: enhex/escape/except to binary! data #"=" :quoted-printable
		assert [number? :max-line-length]

		if 0 < length: to integer! max-line-length - 1 [
			; limit line length to 76 chars
			parse output [any [
				; skip max-line-length - 1 chars
				length skip
				; insert =CRLF if there is not end yet
				[end | 1 skip end | insert #{3D0D0A}]
			]]
		]
		to data output
	] system/catalog/bitsets
	
	max-line-length: 76
]
