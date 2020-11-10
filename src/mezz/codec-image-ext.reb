REBOL [
	title:  "REBOL 3 image codecs extensions"
	name:   'codec-image-ext
	author: "Oldes"
	version: 0.1.0
	date:    10-Nov-2020
	history: [
		0.1.0 10-Nov-2020 "Oldes" {Extend native PNG codec with `size?` function}
	]
]

if find codecs 'png [
	extend codecs/png 'size2? func ["Return PNG image size or none" bin [binary!]][
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
]
















