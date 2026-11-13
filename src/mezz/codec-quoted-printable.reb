REBOL [
	Title:   "Codec: quoted-printable encoding"
	Name:    quoted-printable
	Type:    module
	Options: [delay]
	Version: 1.1.0
	Author:  "Oldes"
	Rights:  "Copyright (C) 2022-2025 Oldes. All rights reserved."
	License: MIT
	Test:    %tests/units/codec-test.r3
	Specification: https://en.wikipedia.org/wiki/Quoted-printable
]

register-codec [
	name: 'quoted-printable
	type: 'text
	title: "Quoted-Printable encoding"
	
	decode: function [
		"Decodes quoted-printable data"
		data [binary! any-string!]
		/uri
		/space "*** DEPRECATED *** Use /uri instead"
	][
		if space [uri: space log-error 'REBOL "/space is deprecated!"]

		output: either binary? data [ copy data ][ to binary! data ]
		; remove soft line breaks
		parse output [any [to #"=" remove [#"=" [LF | CR LF]] | skip] to end]
		to data dehex/escape/:uri output #"="
	]

	encode: function/with [
		"Encodes data using quoted-printable encoding"
		data [binary! any-string!]
		/uri "Q-encoding - space may not be represented directly"
		/no-space "*** DEPRECATED *** Use /uri instead"
	][
		assert [number? :max-line-length]

		if no-space [uri: no-space log-error 'REBOL "/no-space is deprecated!"]

		output: enhex/escape/except/:uri to binary! data #"=" :quoted-printable

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
