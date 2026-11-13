REBOL [
	Title:  "Codec: ICO (Windows icon or cursor file)"
	Name:    ico
	Type:    module
	Version: 0.0.1
	Options: [delay]
	Author: "Oldes"
	History: [6-Mar-2021 "Oldes" {Initial version}]
]

system/options/log/ico: 2
register-codec [
	name:  'ico
	type:  'image
	title: "Windows icon or cursor file"
	suffixes: [%.ico %.cur]

	decode: function [
		{Extract content of the ICO file}
		data  [binary! file! url!]
	][
		unless binary? data [ data: read data ]
		log-info 'ICO ["^[[1;32mDecode ICO data^[[m (^[[1m" length? data "^[[mbytes )"]
		bin: binary data
		;- ICONDIR:
		binary/read bin [ tmp: UI16LE type: UI16LE num: UI16LE ]
		unless all [tmp = 0 any [type = 1 type = 2] ] [return none]
		icons: copy []
		repeat n num [
			binary/read bin [
				width:  UI8
				height: UI8
				colors: UI8
						UI8
				planes: UI16LE
				bpp:    UI16LE
				size:   UI32LE
				ofs:    UI32LE
			]
			binary/read bin [
				pos:    INDEX
						ATz   :ofs
				data:   BYTES :size
						AT    :pos
			]
			if width  = 0 [width:  256]
			if height = 0 [height: 256]
			log-debug 'ICO ["Image^[[1;33m" n "^[[0;36mbpp:^[[33m" bpp "^[[36mcolors:^[[33m" colors "^[[36msize:^[[33m" as-pair width height]
			append/only icons reduce [width bpp data]
		]
		icons
	]
	encode: function [
		data [block!]
	][
		out: binary 30000
		images: copy []
		parse data [
			some [
				set file: file! (
					bin: read/binary file
					if size: codecs/png/size? bin [
						append/only images reduce ['png to integer! size/1 32 bin]
					]
				)
			]
		]
		imgs: length? images
		offset: 6 + (imgs * 16)
		img-data: clear #{}
		binary/write out [UI16LE 0 UI16LE 1 UI16LE :imgs]

		forall images [
			set [type: size: bpp: bin:] images/1
			bytes: length? bin
			if size = 256 [size: 0]
			binary/write out [
				UI8    :size
				UI8    :size
				UI16LE 0 ;colors
				UI16LE 0 ;planes
				UI16LE :bpp
				UI32LE :bytes
				UI32LE :offset
			]
			append img-data bin
			offset: offset + length? bin
		]
		binary/write out img-data
		copy out/buffer
	]

	identify: function [data [binary!]][
		parse data [#{0000} [#{0100} | #{0200}] to end] ;.ico or .cur
	]
]