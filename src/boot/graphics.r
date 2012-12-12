REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Graphics"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Name: graphics
	Type: extension
	Exports: [] ; added by make-host-ext.r
	Note: "Run make-host-ext.r to convert"
]

words: [
	;gui-metric
    screen-size
    border-size
    border-fixed
    title-size
    work-origin
    work-size
]

;temp hack - will be removed later
init-words: command [
	words [block!]
]

init-words words

init: command [
	"Initialize graphics subsystem."
	gob [gob!] "The screen gob (root gob)"
]

caret-to-offset: command [
	"Returns the xy offset (pair) for a specific string position in a graphics object."
	gob [gob!]
	element [integer! block!] "The position of the string in the richtext block"
	position [integer! string!] "The position within the string"
]

cursor: command [
	"Changes the mouse cursor image."
	image [integer! image! none!]
]

offset-to-caret: command [ ;returns pair! instead of the block..needs to be fixed
	"Returns the richtext block at the string position for an XY offset in the graphics object."
	gob [gob!]
	position [pair!]
]

show: command [
	"Display or update a graphical object or block of them."
	gob [gob! none!]
]

size-text: command [
	"Returns the size of text rendered by a graphics object."
	gob [gob!]
]

draw: command [
	"Renders draw dialect (scalable vector graphics) to an image (returned)."
	image [image! pair!] "Image or size of image"
	commands [block!] "Draw commands"
]

gui-metric: command [
	"Returns specific gui related metric setting."
	keyword [word!] "Available keywords: SCREEN-SIZE, BORDER-SIZE, BORDER-FIXED, TITLE-SIZE, WORK-ORIGIN and WORK-SIZE."
]

;#not-yet-used [
;
;effect: command [
;	"Renders effect dialect to an image (returned)."
;	image [image! pair!] "Image or size of image"
;	commands [block!] "Effect commands"
;]
;
;]
