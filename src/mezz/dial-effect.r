REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Internal Dialect: Graphic Effects"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Note: "Modification requires recompiling affected source files."
]

system/dialects/effect: context [

	type-spec:	[]

	add:		[image! image!]
	alphamul:	[image! integer!]
	aspect: 	[image! word! word! decimal!]
	blur:		[image!]
	colorify:	[image! tuple! integer!]
	colorize:	[image! tuple!]
	convolve:	[image! block! decimal! integer! logic!]
	contrast:	[image! integer!]
	crop:		[image! pair! pair!]
	difference:	[image! image! tuple!]
	emboss:		[image!]
	extend:		[image! pair! pair!]
	fit: 		[image! word! word! decimal!]
	flip:		[image! pair!]
	gradcol:	[image! pair! tuple! tuple!]
	gradient:	[image! pair! tuple! tuple!]
	gradmul:	[image! pair! tuple! tuple!]
	grayscale:	[image!]
	hsv:		[image! tuple!]
	invert:		[image!]
	key:		[image! tuple!]
	luma:		[image! integer!]
	mix:		[image! image!]
	multiply:	[image! image! tuple! integer!]
	reflect:	[image! pair!]
	rotate:		[image! integer!]
	shadow:		[image! pair! pair! tuple! decimal! word!]
	sharpen:	[image!]
	tile:		[image! pair!]
	tile-view:	[image!]
	tint:		[image! integer!]

;not yet
comment {
	clip:		[]
}
	;-- EFFECTS Options:

	;SHADOW option
	only:

	;FIT options
	nearest:
	bilinear:
	bicubic:
	gaussian:
	resample: none
]
