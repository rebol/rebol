REBOL [
	title:  "REBOL 3 codec for image file formats"
	name:   'codec-image
	author: "Oldes"
	version: 0.1.0
	date:    31-Jul-2019
	history: [
		0.1.0 31-Jul-2019 "Oldes" {Initial version}
	]
	note: {Add the most common codec at tail (identify is testing the last codec as first)}
]

register-codec [
	name:  'ico
	title: "Computer icons in MS Windows"
	suffixes: [%.ico]

	decode:   func [data [binary!]][lib/image/load/as data 'ICO]
	;encode not available!
	identify: func [data [binary!]][parse data [#{00000100} to end]]
]

register-codec [
	name:  'dng
	title: "Digital Negative"
	suffixes: [%.dng]
	comment: {Decodes only thumbnail, not RAW data!}

	decode:   func [data [binary!]][lib/image/load/as data 'DNG]
	;encode not available!
	identify: func [data [binary!]][none] ; same like TIFF
]

register-codec [
	name:  'dds
	title: "DirectDraw Surface"
	suffixes: [%.dds]

	decode:   func [data [binary!]][lib/image/load/as data 'DDS]
	encode:   func [data [image! ]][lib/image/save/as none data 'DDS]
	identify: func [data [binary!]][parse data [#{444453} to end]]
]

register-codec [
	name:  'tiff
	title: "Tagged Image File Format"
	suffixes: [%.tif %.tiff]

	decode:   func [data [binary!]][lib/image/load/as data 'TIFF]
	encode:   func [data [image! ]][lib/image/save/as none data 'TIFF]
	identify: func [data [binary!]][parse data [#{4949} to end]]
]

register-codec [
	name:  'gif
	title: "Graphics Interchange Format"
	suffixes: [%.gif]

	decode:   func [data [binary!]][lib/image/load/as data 'GIF]
	encode:   func [data [image! ]][lib/image/save/as none data 'GIF]
	identify: func [data [binary!]][parse data [#{4749463839} to end]]
]

register-codec [
	name:  'bmp
	title: "Portable Bitmap"
	suffixes: [%.bmp]

	decode:   func [data [binary!]][lib/image/load/as data 'BMP]
	encode:   func [data [image! ]][lib/image/save/as none data 'BMP]
	identify: func [data [binary!]][parse data [#{4249} to end]]
]

register-codec [
	name:  'jpegxr
	title: "JPEG extended range"
	suffixes: [%.jxr %.hdp %.wdp]

	decode:   func [data [binary!]][lib/image/load/as data 'JPEGXR]
	encode:   func [data [image! ]][lib/image/save/as none data 'JPEGXR]
	identify: func [data [binary!]][parse data [#{4949} to end]]
]

register-codec [
	name:  'jpeg
	title: "Joint Photographic Experts Group"
	suffixes: [%.jpg %.jpeg]

	decode:   func [data [binary!]][lib/image/load/as data 'JPEG]
	encode:   func [data [image! ]][lib/image/save/as none data 'JPEG]
	identify: func [data [binary!]][parse data [#{FFD8} to end]]
]

register-codec [
	name:  'png
	title: "Portable Network Graphics"
	suffixes: [%.png]

	decode:   func [data [binary!]][lib/image/load/as data 'PNG]
	encode:   func [data [image! ]][lib/image/save/as none data 'PNG]
	identify: func [data [binary!]][parse data [#{89504E47} to end]]
]
















