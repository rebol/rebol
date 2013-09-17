REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Sys: Encoder and Decoder"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Context: sys
	Note: {
		The boot binding of this module is SYS then LIB deep.
		Any non-local words not found in those contexts WILL BE
		UNBOUND and will error out at runtime!
	}
]

decode: function [
 	{Decodes a series of bytes into the related datatype (e.g. image!).}
	type [word!] {Media type (jpeg, png, etc.)}
	data [binary!] {The data to decode}
][
	unless all [
		cod: select system/codecs type
		data: do-codec cod/entry 'decode data
	][
		cause-error 'access 'no-codec type
	]
	data
]

encode: function [
	{Encodes a datatype (e.g. image!) into a series of bytes.}
	type [word!] {Media type (jpeg, png, etc.)}
	data [image! binary! string!] {The data to encode}
	/options opts [block!] {Special encoding options}
][
	unless all [
		cod: select system/codecs type
		data: do-codec cod/entry 'encode data
	][
		cause-error 'access 'no-codec type
	]
	data
]

encoding?: function [
	"Returns the media codec name for given binary data. (identify)"
	data [binary!]
][
	foreach [name codec] system/codecs [
		if do-codec codec/entry 'identify data [
			return name
		]
	]
	none
]

export [decode encode encoding?]
