REBOL [
	title:    "Rebol base code collected from C sources"
	purpose:  "This is code which must be evaluated just after code from base-defs.r file"
	commment: "AUTO-GENERATED FILE - Do not modify. (From: make-headers.reb)"
]

;- code from: %u-bincode.c


system/standard/bincode: make object! [
	type: 'bincode
	buffer:
	buffer-write: none
	r-mask:
	w-mask: 0
]

;- code from: %u-bmp.c


	if find system/codecs 'bmp [
		system/codecs/bmp/suffixes: [%.bmp]
		append append system/options/file-types system/codecs/bmp/suffixes 'bmp
	]

;- code from: %u-gif.c


	if find system/codecs 'gif [
		system/codecs/gif/suffixes: [%.gif]
		append append system/options/file-types system/codecs/gif/suffixes 'gif
	]

;- code from: %u-image-resize.c


	append system/catalog [
		filters [
		  Point
		  Box
		  Triangle
		  Hermite
		  Hanning
		  Hamming
		  Blackman
		  Gaussian
		  Quadratic
		  Cubic
		  Catrom
		  Mitchell
		  Lanczos
		  Bessel
		  Sinc
		]
	]

;- code from: %u-jpg.c


  if find system/codecs 'jpeg [
    system/codecs/jpeg/suffixes: [%.jpg %.jpeg]
    append append system/options/file-types system/codecs/jpeg/suffixes 'jpeg
  ]

;- code from: %u-png.c


	if find system/codecs 'png [
		system/codecs/png/suffixes: [%.png]
		append append system/options/file-types system/codecs/png/suffixes 'png
	]

;- code from: %u-wav.c


	if find system/codecs 'wav [
		system/codecs/wav/suffixes: [%.wav %.wave]
		system/codecs/wav/type: 'binary!
		append append system/options/file-types system/codecs/wav/suffixes 'wav
	] 
