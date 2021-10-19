REBOL [
	title: "REBOL 3 coded lossless compressed data format compatible with GZIP utility."
	name: 'codec-gzip
	author: rights: "Oldes"
	version: 0.0.1
	specification: https://tools.ietf.org/html/rfc1952
]

register-codec [
	name:  'gzip
	type:  'compression
	title: "Lossless compressed data format compatible with GZIP utility."
	suffixes: [%.gz]
	decode: function[data [binary!]] [
		if verbose > 0 [ identify data ]
		decompress data 'gzip
	]

	encode: function [data [binary!]][
		compress/level data 'gzip level
	]

	identify: function [data [binary!]][
		if 10 > length? data [return none]
		if verbose > 0 [
			print ["^[[1;32mDecode GZIP data^[[m (^[[1m" length? data "^[[mbytes )"]
		]
		bin: binary data
		binary/read bin [
			id:    UI16   ; Magick: #{1F8B}
			cm:    UI8    ; Compression Method
			flg:   UI8    ; FLaGs
			mtime: UI32LE ; Modification TIME
			xfl:   UI8    ; eXtra FLags
			os:    UI8    ; Operating System
		]
		if any [
			id <> 8075 ;- not GZIP magick number
			cm <> 8    ;- unsupported compression
		][ return none ] 

		mtime: either mtime > 0 [ 1-1-1970 + to time! mtime ][ none ]

		if 4 = (4 and flg) [ ;FEXTRA
			extra: binary/read bin 'UI16LEBYTES
		]
		if 8 = (8 and flg) [ ;FNAME
			name: to file! binary/read bin 'STRING ; zero terminated
		]
		if 16 = (16 and flg) [ ;FCOMMENT
			comm: to file! binary/read bin 'STRING ; zero terminated
		]
		if 2 = (2 and flg) [ ;FHCRC
			;the two least significant bytes of the CRC32 for all bytes of the gzip header up to and not including the CRC16
			;checksum/part bin/buffer-write 'CRC32 index? bin/buffer
			crc16: binary/read bin 'UI16LE
		]
		if verbose > 0 [	
			print ["^[[32mModified:         ^[[0;1m" mtime "^[[m"]
			print ["^[[32mExtra flags:      ^[[0;1m" xfl "^[[m"]
			print ["^[[32mOperating system: ^[[0;1m" os "^[[m"]
			print ["^[[32mExtra field:      ^[[0;1m" extra "^[[m"]
			print ["^[[32mFile name:        ^[[0;1m" name "^[[m"]
			print ["^[[32mHeader CRC:       ^[[0;1m" crc16 "^[[m"]
		]
		compose [
			file:        (all [name  to file! name])
			modified:    (mtime)
			os:          (os)
			extra-flags: (xfl)
			extra-data:  (extra)
		]
	]

	verbose: 1
	level: 9
]