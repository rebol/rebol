Rebol [
	title: "Unicode utils"
	name:   unicode-utils
	type:   module
	version: 0.1.0
	exports: [decode-utf8]
	author: @Oldes
	file: %unicode-utils.reb
	home: https://src.rebol.tech/modules/unicode-utils.reb
	note: {
		Based on Bjoern Hoehrmann's C code: 
		Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
		See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
	}
]

decode-utf8: closure/with [
	"Converts UTF8 encoded binary to Rebol string ignoring chars outside the Basic Multilingual Plane (BMP)."
	bin [binary! file! url!] "Source data in the UTF-8 encoding"
	/html "Converts chars over BMP to HTML entities instead of ignoring these"
][
	unless binary? bin [bin: read/binary bin]
	state: UTF8_ACCEPT
	codep: 0
	str: make string! length? bin
	foreach byte bin [
		if byte < 128 [ append str to char! byte continue]
		type: pickz utf8d byte
		codep: either state = UTF8_ACCEPT [
			(0#ff >> type) & byte
		][
			(byte & 0#3f) | (codep << 6)
		]
		state: pickz utf8d (256 + state + type)
		if state = UTF8_ACCEPT [
			case [
				codep <= 0#FFFF [ append str to char! codep ]
				html [ append str ajoin ["&#" codep #";"]   ]
			]
			codep: 0
		]
	]
	str
][
	utf8d: #[u8! [
		;; The first part of the table maps bytes to character classes that
		;; to reduce the size of the transition table and create bitmasks.
		 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
		 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
		 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
		 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
		 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1   9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 
		 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7   7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 
		 8 8 2 2 2 2 2 2 2 2 2 2 2 2 2 2   2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 
		10 3 3 3 3 3 3 3 3 3 3 3 3 4 3 3  11 6 6 6 5 8 8 8 8 8 8 8 8 8 8 8 

		;; The second part is a transition table that maps a combination
		;; of a state of the automaton and a character class to a state.
		 0 12 24 36 60 96 84 12 12 12 48 72  12 12 12 12 12 12 12 12 12 12 12 12 
		12  0 12 12 12 12 12  0 12  0 12 12  12 24 12 12 12 12 12 24 12 24 12 12 
		12 12 12 12 12 12 12 24 12 12 12 12  12 24 12 12 12 12 12 12 12 24 12 12 
		12 12 12 12 12 12 12 36 12 36 12 12  12 36 12 12 12 12 12 36 12 36 12 12 
		12 36 12 12 12 12 12 12 12 12 12 12  
	]]

	UTF8_ACCEPT: 0
	UTF8_REJECT: 12
]