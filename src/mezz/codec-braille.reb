REBOL [
	Title:  "Codec: Braille text"
	Name:    braille
	Type:    module
	Options: [delay]
	Version: 0.0.1
	Date:    21-5-2025
	Author: "Oldes"
	Usage: [
		print bra: encode 'braille "Hello Rebol!"
		print txt: decode 'braille bra
	]
]

encode-braille: function [
	"Process string and returns Braille string"
	text [any-string!]
][  
	out: clear copy text
	foreach char text [
		if char < 255 [char: char + 10240]
		append out char
	]
	out
]
decode-braille: function [
	"Process string while decoding Braille's characters"
	text [any-string!]
][
	out: clear copy text
	foreach char text [
		if all [char >= 10240 char <= 10495] [ char: char - 10240 ]
		append out char
	]
	out
]

register-codec [
	name: 'braille
	type: 'text
	title: "Braille"

	encode: func [data [any-string!]][
		encode-braille data
	]
	decode: func [text [any-string! binary! file!]][
		if file?   text [text: read text]
		if binary? text [text: to string! text]
		decode-braille text
	]
]
