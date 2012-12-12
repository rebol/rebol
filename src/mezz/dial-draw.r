REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Internal Dialect: Draw Commands (SVG)"
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

system/dialects/draw: context [

	type-spec:		[block!]

	;-- DRAW Commands:

	anti-alias:		[logic!]
	arc:			[;shared with SHAPE command
						pair! pair! decimal! decimal! word!
						decimal! word!
					]
	arrow:			[tuple! pair!]
	box:			[pair! pair! decimal!]
	circle:			[pair! decimal! decimal!]
	clip:			[pair! pair! logic!]
	curve:			[* pair!] ;shared with SHAPE command
	effect:			[pair! pair! block!]
	ellipse:		[pair! pair!]
	fill-pen:		[tuple! image! logic!]
	fill-rule:		[word!]
	gamma:			[decimal!]
	grad-pen:		[word! word! pair! logic! decimal! decimal! decimal! decimal! decimal! block!]
	invert-matrix:	[]
	image:			[image! tuple! word! word! integer! integer! integer! integer! * pair!]
	image-filter:	[word! word! decimal!]
	line:			[* pair!] ;shared with SHAPE command
	line-cap:		[word!]
	line-join:		[word!]
	line-pattern:	[logic! tuple! * decimal!]
	line-width:		[decimal! word!]
	matrix:			[block!]
	pen:			[tuple! image! logic!]
	polygon:		[* pair!]
	push:			[block!]
	reset-matrix:	[]
	rotate:			[decimal!]
	scale:			[decimal! decimal!]
	shape:			[block!]
	skew:			[decimal!]
	spline:			[integer! word! * pair!]
	text:			[word! pair! pair! block!]
	transform:		[decimal! pair! decimal! decimal! pair!]
	translate:		[pair!]
	triangle:		[pair! pair! pair! tuple! tuple! tuple! decimal!]

	;-- SHAPE Commands
	;arc is shared
	close: []
	curv: [* pair!]
	;curve is shared
	hline: [decimal!]
	;line is shared
	move: [* pair!]
	qcurv: [pair!]
	qcurve: [* pair!]
	vline:	[decimal!]

	;-- DRAW Options:

	; FILL-PEN
	radial:
	conic:
	diamond:
	linear:
	diagonal:
	cubic:

	; FILL-RULE
	non-zero:
	even-odd:

	; IMAGE
	border:

	; IMAGE-FILTER
	nearest:
	bilinear:
	bicubic:
	gaussian:
	resample:

	; LINE-CAP
	butt:
	square:
	rounded:

	; LINE-JOIN
	miter:
	miter-bevel:
	round:
	bevel:

	;LINE-WIDTH
	fixed:

	; SPLINE & ARC & TEXT
	closed:

	; GRADIENT
	normal:
	repeat:
	reflect:

	;SHAPE ARC
	large:
	sweep:

	;TEXT
	vectorial:
		none
]
