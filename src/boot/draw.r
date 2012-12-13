REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Graphics - DRAW commands"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Name: draw
	Type: extension
	Exports: none
	Note: "Run make-host-ext.r to convert"
]

;don't change order of already defined words unless you know what you are doing

words: [
	;fill-rule
	even-odd
	non-zero

	;grad-pen
	conic
	cubic
	diagonal
	diamond
	linear
	radial
	normal
	repeat
	reflect
	
	;line-cap
	butt
	square
	rounded
	
	;line-join
	miter
	miter-bevel
	round
	bevel
	
	;line-width
	variable
	fixed
	
	;arc
	opened
	closed
	
	;image
	no-border
	border
	
	;image-filter
	nearest
	bilinear
	bicubic
	gaussian
	resize
	resample
	
	;text
	raster
	vectorial	
]

;temp hack - will be removed later
init-words: command [
	words [block!]
]

init-words words

;please alphabetize the order of commands so it easier to lookup things

anti-alias: command [
	"Turns anti-aliasing on or off."
	state [logic!]
]

arc: command [
	"Draws a partial section of an ellipse or circle."
	center [pair!] "The center of the circle"
	radius [pair!] "The radius of the circle"
	angle-begin [number!] "The angle where the arc begins, in degrees"
	angle-length [number!] "The length of the arc in degrees"
	'arc-ending [word!] "Leave the arc: OPENED or CLOSED"
]

arrow: command [
	"Sets the arrow mode."
	mode [pair!] "Possible numbers for combination. 0 for none, 1 for head, 2 for tail"
	color [tuple! none!] "Color of the head/tail of the arrow. NONE means use PEN color"
]

box: command [
	"Draws a rectangular box."
	origin [pair!] "Corner of box"
	end [pair!] "End of box"
	corner-radius [number!] "Rounds corners"
]

circle: command [
	"Draws a circle or ellipse."
	center [pair!]
	radius [pair!]
]

curve: command [
	"Draws a smooth Bezier curve.(using 3 or 4 points)"
	point-1 [pair!] "End point A"
	point-2 [pair!] "Control point A"
	point-3 [pair!] "End point B, or control point B"
	point-4 [pair! none!] "End point B"
]

clip: command [
	"Specifies a clipping region."
	origin [pair!] "Corner of box"
	end [pair!] "End of box"
]

ellipse: command [
	"Draws an ellipse."
	origin [pair!] "The upper-left-point of the ellipse bounding box"
	diameter [pair!]
]

fill-pen: command [
	"Sets the area fill pen color."
	color [tuple! image! logic!] "Set to OFF to disable fill pen"
]

fill-rule: command [
	"Determines the algorithm used to determine what area to fill."
	'mode [word!] "Rule type: EVEN-ODD or NON-ZERO"
]

gamma: command [
	"Sets the gamma correction value."
	gamma-value [number!]
]

grad-pen: command [
	"Sets the color gradient for area filling. To disable it set the color block to NONE."
	'type [word!] "The gradient type: RADIAL CONIC DIAMOND LINEAR DIAGONAL CUBIC"
	'mode [word!] "The gradient rendering mode: NORMAL REPEAT REFLECT"
	offset [pair!] "offset from where should the gradient be rendered"
	range [pair!] "begin and end of the gradient range"
	angle [number!] "rotation of the gradient in degrees"
	scale [pair!] "X and Y scale factor"
	colors [block! none!] "block containing up to 256 gradient colors (optionally with color offsets)"
]

image: command [
	"Draws an image, with optional scaling, borders, and color keying."
	image [image!]
	offset-points [pair! block!]
]

image-filter: command [
	"Specifies type of algorithm used when an image is scaled."
	'filter-type [word!] "supported filters: NEAREST, BILINEAR, BICUBIC, GAUSSIAN"
	'filter-mode [word!] "Output quality: RESIZE(low, faster) or RESAMPLE(high, slower)"
	blur [number! none!] "Used only in RESAMPLE mode"
]

image-options: command [
	"Sets options related to image rendering."
	key-color [tuple! none!] "Color to be rendered as transparent or NONE to disable it"
	'border-flag [word!] "can be BORDER or NO-BORDER"
]

image-pattern: command [
	"Configure the image pattern fill settings."
	'pattern-mode [word!] "can be NORMAL, REPEAT or REFLECT"
	pattern-offset [pair!]
	pattern-size [pair!] "set to 0x0 for auto-size"
	
]

line: command [
	"Draws (poly)line from a number of points."
	lines [block!] "Block of pairs"
]

line-cap: command [
	"Sets the style that will be used when drawing the ends of lines."
	'type [word!] "Cap type: BUTT, SQUARE or ROUNDED"
]

line-join: command [
	"Sets the style that will be used where lines are joined."
	'type [word!] "Join type: MITER, MITER-BEVEL, ROUND, or BEVEL"
]

line-pattern: command [
	"Sets the line pattern. To disable it set the pattern block to NONE."
	color [tuple!] "Dash color"
	pattern [block! none!] "Block of dash-size/stroke-size number pairs"
]

line-width: command [
	"Sets the line width."
	width [number!] "Zero, or negative values, produce a line-width of 1."
	'mode [word!] "Line width mode during scaling: FIXED or VARIABLE"
]

invert-matrix: command [
	"Applies an algebraic matrix inversion operation on the current transformation matrix."
]

matrix: command [
	"Premultiplies the current transformation matrix with the given block."
	matrix-setup [block!] "content must be 6 numbers"
]

pen: command [
	"Sets the line pen color."
	color [tuple! image! logic!] "Set to OFF to disable pen"
]

polygon: command [
	"Draws a closed area of line segments. First and last points are connected."
	vertices [block!] "Block of pairs"
]

push: command [
	"Stores the current attribute setup in stack."
	draw-block [block!]
]

reset-matrix: command [
	"Resets the current transformation matrix to its default values."
]

rotate: command [
	"Sets the clockwise rotation in current transformation matrix."
	angle [number!] "in degrees"
]

scale: command [
	"Sets the scaling factor in current transformation matrix."
	factor [pair!]
]

shape: command [
	"Draws shapes using the SHAPE sub-dialect."
	commands [block!] "Block of SHAPE sub-commands"
]

skew: command [
	"Sets a coordinate system skewed from the original by the given number of degrees in specified axis."
	angle [pair!] "Positive numbers skew to the right; negative numbers skew to the left."
]

spline: command [
	"Draws a curve through any number of points. The smoothness of the curve will be determined by the segment factor."
	points [block!] "Block of pairs"
	segmentation [integer!]
	'spline-ending [word!] "Leave the spline: OPENED or CLOSED"
]

text: command [
	"Draws a string of text."
	offset [pair!] "offset from where should the text be rendered"
	size [pair!] "size of the text area"
	'render-mode [word!] "RASTER or VECTORIAL"
	rich-text-block [block!]
]

transform: command [
	"Applies transformation such as translation, scaling, and rotation."
	angle [number!]
	center [pair!]
	scale [pair!]
	translation [pair!]
]

translate: command [
	"Sets the translation in current transformation matrix."
	offset [pair!]
]

triangle: command [
	"Draws triangular polygon with shading parameters (Gouraud shading). Set colors to NONE to turn of shading."
	vertex-1 [pair!]
	vertex-2 [pair!]
	vertex-3 [pair!]
	color-1 [tuple! none!]
	color-2 [tuple! none!]
	color-3 [tuple! none!]
	dilation [number!] "Useful for eliminating anitaliased edges"
]
