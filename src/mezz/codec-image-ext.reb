REBOL [
	title:  "REBOL 3 image codecs extensions"
	name:   'codec-image-ext
	author: "Oldes"
	version: 0.3.0
	date:    30-Aug-2021
	history: [
		0.1.0 10-Nov-2020 "Oldes" {Extend native PNG codec with `size?` function}
		0.2.0 08-Mar-2021 "Oldes" {Extend native PNG with `chunks` function}
		0.3.0 30-Aug-2021 "Oldes" {Extend native JPEG codec with `size?` function}
	]
]

if find codecs 'png [
	extend codecs/png 'size? func ["Return PNG image size or none" bin [binary!]][
		if all [
			parse bin [
				#{89504E470D0A1A0A} ;- PNG magic number
				thru #{49484452}    ;- IHDR chunk
				bin: to end
			]
			8 <= length? bin
		][
			to pair! binary/read bin [UI32 UI32]
		]
	]
	
	extend codecs/png 'chunks function [
		"Decode PNG into block of chunks (or encode back to binary from a block)"
		data [binary! file! url! block!] "Input data"
		/only tags [block!]
	][
		if block? data [
			;- Composing previously decoded chunks back to binary...
			if #{49484452} <> data/1 [
				sys/log/error 'PNG ["First chunk must be IHDR, but is:" as-red mold to string! tag]
				return none
			]
			out: binary 10000
			binary/write out #{89504E470D0A1A0A}
			foreach [tag dat] data [
				if tag = #{49454E44} [continue] ; IEND is added automatically
				unless all [binary? tag 4 = length? tag binary? dat][
					sys/log/error 'PNG ["Wrong chunk input!" as-red tag]
					return none
				]
				len: length? dat
				binary/write out [ui32be :len]
				tmp: out/buffer-write
				binary/write out [:tag :dat]
				crc: checksum tmp 'crc32
				binary/write out [ui32be :crc]
			]
			; add IEND
			binary/write out #{0000000049454E44AE426082}
			return copy out/buffer
		]

		;- Examine inner content of PNG files converting it to block of chunks...
		if only [
			; make sure that all tag ids are as binary
			forall tags [
				unless binary? tags/1 [change tags to binary! form tags/1]
			]
		]

		unless binary? data [ data: read data ]
		sys/log/info 'PNG ["^[[1;32mDecode PNG data^[[m (^[[1m" length? data "^[[mbytes )"]
		unless parse data [#{89504E470D0A1A0A} data: to end][ return none ]
		bin: binary data
		out: make block! 12
		; in cloud builds the console width is not resolved!
		num: try/except [-40 + query/mode console:// 'window-cols][40]
		while [8 < length? bin/buffer][
			len: binary/read bin 'ui32be
			tag: copy/part bin/buffer 4
			; check if we are interested in specific tags
			if all [tags none? find tags tag][
				; ignore this tag
				sys/log/more 'PNG rejoin [form tag #" " as-red to string! tag " ^[[33m" pad len 10 "^[[35mignored"]
				bin/buffer: skip bin/buffer len + 8
				continue
			]
			; count checksum of this chunk...
			crc: checksum/part bin/buffer 'crc32 len + 4
			; skip tag which we already know...
			bin/buffer: skip bin/buffer 4
			; get raw data...
			dat: binary/read/with bin 'BYTES len
			; read CRC and compare with computed value...
			if crc <> binary/read bin 'si32be [
				sys/log/error 'PNG "CRC check failed!"
				return none
			]
			; use some user friendly info output for specific chunks...
			info: switch/default tag [
				#{49484452} ;IHDR
							[binary/read dat [UI32 UI32 UI8 UI8 UI8 UI8 UI8]]
				#{70485973} ;pHYs
							[binary/read dat [UI32 UI32 UI8]]
				#{69545874} ;iTXt
				#{74455874} ;tEXt
							[to string! dat]
			][	dat	]
			info: mold/flat/part info num + 3
			; cropped to fit on single line...
			if num < length? info [ change skip tail info -3 "..." ]
			; output info...
			sys/log/more 'PNG rejoin [form tag #" " as-red to string! tag " ^[[33m" pad len 10 info]
			; store data...
			append/only append out tag dat
		]
		new-line/skip out true 2
		out
	]
]

if find codecs 'jpeg [
	extend codecs/jpeg 'size? function ["Return JPEG image size or none" img [file! url! binary!]][
		unless binary? img [img: read/binary img]
		unless img: find/tail img #{FFD8} [return none]
		while [2 <= length? img][
			if img/1 <> 255 [break] ;invalid chunk
			switch img/2 [
				192 ;baseline
				193 ;baseline extended
				194 ;progressive
				195 ;lossless
				[
					binary/read img [
						skip 5 ; tag, length, bpp
						h: UI16
						w: UI16
					]
					return as-pair w h
				]
				217 [break] ;end of image
				218 0 [
					unless img: find img 255 [return none] ; error
					continue
				]
			]
			img: skip img 2 ; skip chunk name
			img: skip img binary/read img 'ui16
		]
		none
	]
]


if find codecs 'gif [
	extend codecs/gif 'size? function ["Return GIF image size or none" img [file! url! binary!]][
		unless binary? img [img: read/binary/part img 16]
		parse img [
			["GIF87a" | "GIF89a"] img: (
				return to pair! binary/read img [UI16LE UI16LE]
			) 
		]
		none
	]
]

if find codecs 'bmp [
	extend codecs/bmp 'size? function ["Return BMP image size or none" img [file! url! binary!]][
		unless binary? img [img: read/binary/part img 32]
		unless find/match img #{424D} [return none]
		try [return to pair! binary/read img [SKIP 18 UI32LE UI32LE]]
		none
	]
]

if find codecs 'dds [
	extend codecs/dds 'size? function ["Return DDS image size or none" img [file! url! binary!]][
		unless binary? img [img: read/binary/part img 32]
		unless find/match img #{444453207C000000} [return none]
		try [return to pair! reverse binary/read img [SKIP 12 UI32LE UI32LE]]
		none
	]
]

if find codecs 'qoi [
	extend codecs/qoi 'size? function ["Return QOI image size or none" img [file! url! binary!]][
		unless binary? img [img: read/binary/part img 32]
		unless find/match img #{716F6966} [return none]
		try [return to pair! binary/read img [SKIP 4 UI32BE UI32BE]]
		none
	]
]
