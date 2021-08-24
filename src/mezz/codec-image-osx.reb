REBOL [
	title:  "REBOL 3 codec for image file formats (OSX version)"
	name:   'codec-image
	author: "Oldes"
	version: 0.1.0
	date:    13-Aug-2021
	history: [
		0.1.0 13-Aug-2021 "Oldes" {Initial version}
	]
	note: {Add the most common codec at tail (identify is testing the last codec as first)}
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
	name:  'jpeg2000
	title: "JPEG 2000"
	suffixes: [%.jp2 %.j2k %.jpf %.jpm %.jpg2 %.j2c %.jpc %.jpx %.mj2]

	decode:   func [data [binary!]][lib/image/load/as data 'JP2]
	encode:   func [data [image! ]][lib/image/save/as none data 'JP2]
	identify: func [data [binary!]][parse data [#{0000000C6A5020200D0A870A} to end]]
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
















