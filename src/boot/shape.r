REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Graphics - SHAPE commands"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Name: shape
	Type: extension
	Exports: none
	Note: "Run make-host-ext.r to convert"
]

;don't change order of already defined words unless you know what you are doing

words: [
	;arc
	negative
	positive
	small
	large
]

;temp hack - will be removed later
init-words: command [
	words [block!]
]

init-words words

;please alphabetize the order of commands so it easier to lookup things

arc: command [
	"Draws an elliptical arc from the current point."
	end-point [pair!]
	radius [pair!]
	angle [number!] 
	'sweep-flag [word!] "The arc will be drawn in POSITIVE or NEGATIVE angle direction"
	'arc-flag [word!] "User SMALL or LARGE arc sweep"
]

arc': command [
	"Draws an elliptical arc from the current point.(uses relative coordinates)"
	end-point [pair!]
	radius [pair!]
	angle [number!] 
	'sweep-flag [word!] "The arc will be drawn in POSITIVE or NEGATIVE angle direction"
	'arc-flag [word!] "User SMALL or LARGE arc sweep"
]

close: command [
	"Closes previously defined set of lines in the SHAPE block."
]

curv: command [
	"Draws a cubic Bezier curve or polybezier using two points."
	points [block!] "Block of point pairs (2nd control point, end point)"
]

curv': command [
	"Draws a cubic Bezier curve or polybezier using two points.(uses relative coordinates)"
	points [block!] "Block of point pairs (2nd control point, end point)"
]

curve: command [
	"Draws a cubic Bezier curve or polybezier using three points."
	points [block!] "Block of point triplets (1st control point, 2nd control point, end point)"
]

curve': command [
	"Draws a cubic Bezier curve or polybezier using three points.(uses relative coordinates)"
	points [block!] "Block of point triplets (1st control point, 2nd control point, end point)"
]

hline: command [
	"Draws a horizontal line from the current point."
	end-x [number!]
]

hline': command [
	"Draws a horizontal line from the current point.(uses relative coordinates)"
	end-x [number!]
]

line: command [
	"Draws a line from the current point through the given points."
	points [pair! block!]
]

line': command [
	"Draws a line from the current point through the given points.(uses relative coordinates)"
	points [pair! block!]
]

move: command [
	"Set's the starting point for a new path without drawing anything."
	point [pair!]
]

move': command [
	"Set's the starting point for a new path without drawing anything.(uses relative coordinates)"
	point [pair!]
]

qcurv: command [
	"Draws a quadratic Bezier curve from the current point to end point."
	end-point [pair!]
]

qcurv': command [
	"Draws a quadratic Bezier curve from the current point to end point.(uses relative coordinates)"
	end-point [pair!]
]

qcurve: command [
	"Draws a quadratic Bezier curve using two points."
	points [block!] "Block of point pairs (control point, end point)"
]

qcurve': command [
	"Draws a quadratic Bezier curve using two points.(uses relative coordinates)"
	points [block!] "Block of point pairs (control point, end point)"
]

vline: command [
	"Draws a vertical line from the current point."
	end-y [number!]
]

vline': command [
	"Draws a vertical line from the current point.(uses relative coordinates)"
	end-y [number!]
]
