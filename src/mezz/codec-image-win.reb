REBOL [
	title:  "REBOL 3 codec for image file formats (Windows)"
	name:   'codec-image
	author: "Oldes"
	version: 0.2.0
	date:    13-Aug-2021
	history: [
		0.2.0 13-Aug-2021 "Oldes" {Removed ICO codec registration as there is platform independent implementation now}
		0.1.0 31-Jul-2019 "Oldes" {Initial version}
	]
	note: {Add the most common codec at tail (identify is testing the last codec as first)}
]

register-codec [
	name:  'dng
	title: "Digital Negative"
	type:  'image
	suffixes: [%.dng]
	comment: {Decodes only thumbnail, not RAW data!}

	decode:   func [data [binary!]][lib/image/load/as data 'DNG]
	;encode not available!
	identify: func [data [binary!]][none] ; same like TIFF
]

register-codec [
	name:  'dds
	title: "DirectDraw Surface"
	type:  'image
	suffixes: [%.dds]

	decode:   func [data [binary!]][lib/image/load/as data 'DDS]
	encode:   func [data [image! ]][lib/image/save/as none data 'DDS]
	identify: func [data [binary!]][parse data [#{444453207C000000} to end]]
]

register-codec [
	name:  'tiff
	title: "Tagged Image File Format"
	type:  'image
	suffixes: [%.tif %.tiff]

	decode:   func [data [binary!]][lib/image/load/as data 'TIFF]
	encode:   func [data [image! ]][lib/image/save/as none data 'TIFF]
	identify: func [data [binary!]][parse data [#{4949} to end]]
]

register-codec [
	name:  'gif
	title: "Graphics Interchange Format"
	type:  'image
	suffixes: [%.gif]

	decode:   func [data [binary!]][lib/image/load/as data 'GIF]
	encode:   func [data [image! ]][lib/image/save/as none data 'GIF]
	identify: func [data [binary!]][parse data [["GIF89a" | "GIF87a"] to end]]
]

register-codec [
	name:  'bmp
	type:  'image
	title: "Portable Bitmap"
	suffixes: [%.bmp]

	decode:   func [data [binary!]][lib/image/load/as data 'BMP]
	encode:   func [data [image! ]][lib/image/save/as none data 'BMP]
	identify: func [data [binary!]][parse data [#{4249} to end]]
]

register-codec [
	name:  'jpegxr
	type:  'image
	title: "JPEG extended range"
	suffixes: [%.jxr %.hdp %.wdp]

	decode:   func [data [binary!]][lib/image/load/as data 'JPEGXR]
	encode:   func [data [image! ]][lib/image/save/as none data 'JPEGXR]
	identify: func [data [binary!]][parse data [#{4949} to end]]
]

register-codec [
	name:  'jpeg
	type:  'image
	title: "Joint Photographic Experts Group"
	suffixes: [%.jpg %.jpeg]

	decode:   func [data [binary!]][lib/image/load/as data 'JPEG]
	encode:   func [data [image! ]][lib/image/save/as none data 'JPEG]
	identify: func [data [binary!]][parse data [#{FFD8} to end]]
]

register-codec [
	name:  'png
	type:  'image
	title: "Portable Network Graphics"
	suffixes: [%.png]

	decode:   func [data [binary!]][lib/image/load/as data 'PNG]
	encode:   func [data [image! ]][lib/image/save/as none data 'PNG]
	identify: func [data [binary!]][parse data [#{89504E47} to end]]
]
