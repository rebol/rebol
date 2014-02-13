REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Save"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Issues: {
		Is MOLD Missing a terminating newline? -CS
		Add MOLD/options -CS
	}
]

mold64: function [
	"Temporary function to mold binary base 64." ; fix the need for this! -CS
	data
][
	base: system/options/binary-base
	system/options/binary-base: 64
	data: mold :data
	system/options/binary-base: :base
	data
]

save: function [
	{Saves a value, block, or other data to a file, URL, binary, or string.}
	where [file! url! binary! string! none!] {Where to save (suffix determines encoding)}
	value {Value(s) to save}
	/header {Provide a REBOL header block (or output non-code datatypes)}
	header-data [block! object! logic!] {Header block, object, or TRUE (header is in value)}
	/all {Save in serialized format}
	/length {Save the length of the script content in the header}
	/compress {Save in a compressed format or not}
	method [logic! word!] "true = compressed, false = not, 'script = encoded string"
][
	;-- Special datatypes use codecs directly (e.g. PNG image file):
	if lib/all [
		not header ; User wants to save value as script, not data file
		any [file? where url? where]
		type: file-type? where
	][ ; We have a codec:
		return write where encode type :value ; will check for valid type
	]

	;-- Compressed scripts and script lengths require a header:
	if any [length method] [
		header: true
		header-data: any [header-data []]
	]

	;-- Handle the header object:
	if header-data [
		; TRUE indicates the header is the first value in the block:
		if header-data = true [
			header-data: any [
				lib/all [
					block? :value
					first+ value ; the header (do not use TAKE)
				]
				[] ; empty header
			]
		]

		; Make it an object if it's not already (ok to ignore overhead):
		header-data: either object? :header-data [
			trim :header-data ; clean out the words set to none
		][
			construct :header-data ; standard/header intentionally not used
		]

		if compress [ ; Make the header option match
			case [
				not method [remove find select header-data 'options 'compress]
				not block? select header-data 'options [
					repend header-data ['options copy [compress]]
				]
				not find header-data/options 'compress [
					append header-data/options 'compress
				]
			]
		]

		if length [
			append header-data [length: #[true]] ; any true? value will work
		]

		unless compress: true? find select header-data 'options 'compress [method: none]
		length: true? select header-data 'length
		header-data: body-of header-data
	]

	; (Maybe /all should be the default? See CureCode.)
	data: either all [mold/all/only :value] [mold/only :value]
	append data newline ; mold does not append a newline? Nope.

	case/all [
		; Checksum uncompressed data, if requested
		tmp: find header-data 'checksum [change next tmp checksum/secure data: to-binary data]
		; Compress the data if necessary
		compress [data: lib/compress data]
		; File content is encoded as base-64:
		method = 'script [data: mold64 data]
		not binary? data [data: to-binary data]
		length [change find/tail header-data 'length length? data]
		header-data [insert data ajoin ['REBOL #" " mold header-data newline]]
	]
	case [
		file? where [write where data] ; WRITE converts to UTF-8, saves overhead
		url? where [write where data]  ; But some schemes don't support it
		none? where [data] ; just return the UTF-8 binary
		'else [insert tail where data] ; string! or binary!, insert data
	]
]

#test [
	data: [1 1.2 10:20 "test" user@example.com [sub block]]
	probe to string! save none []
	probe to string! save none data
	probe to string! save/header none data [title: "my code"]
	probe to string! save/compress none [] true
	probe to string! save/compress none data true
	probe to string! save/compress none data 'script
	probe to string! save/header/compress none data [title: "my code"] true
	probe to string! save/header/compress none data [title: "my code"] 'script
	probe to string! save/header none data [title: "my code" options: [compress]]
	probe to string! save/header/compress none data [title: "my code" options: [compress]] none
	probe to string! save/header none data [title: "my code" checksum: true]
	halt
	; more needed
]
