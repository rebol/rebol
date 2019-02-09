REBOL [
	title:  "REBOL 3 codec for SWF file format"
	name:   'codec-SWF
	author: "Oldes"
	version: 0.1.0
	date:    11-Oct-2018
	history: [
		0.1.0 11-Oct-2018 "Oldes" {
			Initial version with DECODE and IDENTIFY functions.
			Not all chunks are parsed.
		}
	]
]

import module [
	Title: "SWF file format related functions"
	Name:  SWF
	Version: 0.1.0
	;Exports: []
][

	readRECT: func[bin /local n rect][
		binary/read bin [n: UB 5]
		rect: make object! [xMin: xMax: yMin: yMax: 0]
		set rect binary/read bin [SB :n SB :n SB :n SB :n]
		rect 
	]

	register-codec [
		name:  'SWF
		title: "ShockWave Flash"
		suffixes: [%.swf]

		decode: function [
			data [binary!]
		][
			if verbose > 0 [
				print ["^[[1;32mDecode SWF data^[[m (^[[1m" length? data "^[[mbytes )"]
			]

			swf: make object! [
				header: none
				tags:   none
			]

			bin: binary data ; initializes Bincode streams
			binary/read bin [
				compression: UI8
				signature:   UI16
				version:     UI8
			]
			unless all [
				signature = 22355           ; "WS"
				find [67 70 90] compression ; C F Z
			][
				; not a SWF file
				return none
			]
			fileSize: binary/read bin 'UI32LE
			if verbose > 0 [
				print [
					"SWF file version:" version
					select [
						67 "compressed using deflate"
						70 "uncompressed"
						90 "compressed using LZMA"
					] compression
					lf
					"Data size:" fileSize - 8 "bytes"
				]
			]
			
			switch compression [
				67 [
					binary/init bin decompress/zlib/size bin/buffer fileSize - 8
				]
				90 [
					packed: binary/read bin 'UI32LE
					binary/init bin decompress/lzma/size bin/buffer fileSize - 8
				]
			]

			binary/read bin [n: UB 5]
			frame-size: binary/read bin [SB :n SB :n SB :n SB :n ALIGN]
			;? bin
			binary/read bin [frame-rate: UI16 frame-count: UI16LE]
			if verbose > 0 [	
				print ["^[[32mframe-size:  ^[[0;1m" frame-size "^[[m"]
				print ["^[[32mframe-rate:  ^[[0;1m" frame-rate "^[[m"]
				print ["^[[32mframe-count: ^[[0;1m" frame-count "^[[m"]
			]
			swf/header: object compose [
				version: (version)
				bounds:  (frame-size)
				rate:    (frame-rate)
				frames:  (frame-count)
			]
			swf/tags: make block! (2 * frame-count) + 100
			while [not tail? bin/buffer][
				tag: binary/read bin 'UI16LE
				tagId: (65472 and tag) >> 6
				tagLength:  tag and 63
				if tagLength = 63 [tagLength: binary/read bin 'UI32LE]
				either tagLength > 0 [
					binary/read bin [tagData: BYTES :tagLength]
				][	tagData: none ]
				repend swf/tags [tagId tagData]
			]
			
			new-line/all/skip swf/tags true 2 
			return swf
		]

		decode-tag: function[id [integer!] data [binary!]][

		]

		identify: func [
			"Returns TRUE if binary looks like SWF data"
			data [binary!]
		][
			parse/case data [[#"C" | #"F" | #"Z"] "WS" to end]
		]

		verbose: 0
	]
]