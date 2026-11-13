REBOL [
	title:  "Codec: image file formats"
	name:    image-codecs
	type:    module
	version: 0.2.0
	date:    10-Sep-2021
	author: "Oldes"
	history: [
		0.1.0 31-Jul-2019 "Oldes" {Initial version}
		0.2.0 10-Sep-2021 "Oldes" {Merged with macOS codecs}
	]
	note: {Add the most common codec at tail (identify is testing the last codec as first)}
]

;- Not cross-platform codecs                                                    
switch system/platform [
	Windows [
		;@@ Using codec-ico.reb instead
		;register-codec [
		;	name:  'ico
		;	type:  'image
		;	title: "Computer icons in MS Windows"
		;	suffixes: [%.ico %.cur]
		;
		;	decode:   func [data [binary!]][lib/image/load/as data 'ICO]
		;	;encode not available!
		;	identify: func [data [binary!]][
		;		parse data [#{0000} [#{0100} | #{0200}] to end] ;.ico or .cur
		;	]
		;]

		register-codec [
			name:  'dng
			type:  'image
			title: "Digital Negative"
			suffixes: [%.dng]
			comment: {Decodes only thumbnail, not RAW data!}

			decode:   func [data [binary!]][lib/image/load/as data 'DNG]
			;encode not available!
			identify: func [data [binary!]][none] ; same like TIFF
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
	]
	Macintosh [
		register-codec [
			name:  'heif
			type:  'image
			title: "High Efficiency Image File Format"
			suffixes: [%.heif %.heic]

			decode:   func [data [binary!]][lib/image/load/as data 'HEIF]
			encode:   func [data [image! ]][lib/image/save/as none data 'HEIF]
			identify: func [data [binary!]][parse data [4 skip #{6674797068656963} to end]]
		]
		register-codec [
			name:  'jpeg2000
			type:  'image
			title: "JPEG 2000"
			suffixes: [%.jp2 %.j2k %.jpf %.jpm %.jpg2 %.j2c %.jpc %.jpx %.mj2]

			decode:   func [data [binary!]][lib/image/load/as data 'JP2]
			encode:   func [data [image! ]][lib/image/save/as none data 'JP2]
			identify: func [data [binary!]][parse data [#{0000000C6A5020200D0A870A} to end]]
		]
	]
]

;- Common codecs                                                                
register-codec [
	name:  'dds
	type:  'image
	title: "DirectDraw Surface"
	suffixes: [%.dds]

	decode:   func [data [binary!]][lib/image/load/as data 'DDS]
	encode:   func [data [image! ]][lib/image/save/as none data 'DDS]
	identify: func [data [binary!]][parse data [#{444453} to end]]
]

register-codec [
	name:  'tiff
	type:  'image
	title: "Tagged Image File Format"
	suffixes: [%.tif %.tiff]

	decode:   func [data [binary!]][lib/image/load/as data 'TIFF]
	encode:   func [data [image! ]][lib/image/save/as none data 'TIFF]
	identify: func [data [binary!]][parse data [#{4949} to end]]
]

register-codec [
	name:  'gif
	type:  'image
	title: "Graphics Interchange Format"
	suffixes: [%.gif]

	decode:   func [data [binary!]][lib/image/load/as data 'GIF]
	encode:   func [data [image! ]][lib/image/save/as none data 'GIF]
	identify: func [data [binary!]][parse data [#{4749463839} to end]]
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
