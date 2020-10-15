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

register-codec: function [
	{Registers non-native codec to system/codecs and it's suffixes into system/options/file-types}
	codec [block! object!] "Codec to register (should be based on system/standard/codec template)"
	/local name suffixes
][
	if block? codec [codec: make object! codec]
	if not word? name: try [codec/name][
		cause-error 'Script 'wrong-type 'codec/name
	]
	
	append system/codecs reduce [to set-word! name codec]

	if block? suffixes: try [codec/suffixes][
		append append system/options/file-types suffixes name
	]
	codec
]

decode: function [
 	{Decodes a series of bytes into the related datatype (e.g. image!).}
	type [word!] {Media type (jpeg, png, etc.)}
	data {The data to decode}
][
	unless all [
		cod: select system/codecs type
		data: either handle? try [cod/entry] [
			; original codecs were only natives
			do-codec cod/entry 'decode data
		][
			either function? try [:cod/decode][
				cod/decode data
			][
				cause-error 'internal 'not-done type
			]
		]
	][
		cause-error 'access 'no-codec type
	]
	data
]

encode: function [
	{Encodes a datatype (e.g. image!) into a series of bytes.}
	type [word!] {Media type (jpeg, png, etc.)}
	data {The data to encode}
	/as {Special encoding options}
	 options {Value specific to type of codec}
][
	unless all [
		cod: select system/codecs type
		data: either handle? try [cod/entry] [
			; original codecs were only natives
			if type = 'text [
				return either binary? data [to string! data][mold/only data]
			]
			do-codec cod/entry 'encode data
		][
			either function? try [:cod/encode][
				either as [
					apply :cod/encode [data 'as options]
				][	cod/encode data ]
			][
				cause-error 'internal 'not-done type
			]
		]
	][
		cause-error 'access 'no-codec type
	]
	data
]

encoding?: function [
	"Returns the media codec name for given binary data. (identify)"
	data [binary!]
][
	if empty? data [return 'text] ;- optimization for an empty data case
	; using reversed order - the last added codec is compared first
	; without it the `text` codec takes everything
	foreach name reverse words-of system/codecs [
		codec: system/codecs/:name
		either handle? try [codec/entry] [
			if do-codec codec/entry 'identify data [
				return name
			]
		][
			if all [
				function? try [:codec/identify]
				codec/identify data
			][
				return name
			]
		]
	]
	none
]

log: func [
	"Prints out debug message"
	id [word!] "Source of the log message"
	msg        "Output message"
	/info
	/more
	/debug
	/local level
][
	level: select system/options/log id
	if any [none? level level <= 0] [exit]
	if block? msg [msg: form reduce :msg]
	case [
		info  [ if level > 0 [print ajoin [" ^[[1;33m[" id "] ^[[36m" msg "^[[0m"]]]
		more  [ if level > 1 [print ajoin [" ^[[33m[" id "] ^[[0;36m" msg "^[[0m"]]]
		debug [ if level > 2 [print ajoin [" ^[[33m[" id "] ^[[0;32m" msg "^[[0m"]]]
		true  [ if level > 0 [print ajoin [" ^[[33m[" id "] " msg "^[[0m"]]]
	]
]

export [register-codec decode encode encoding?]
