REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Graphics"
	Author: ["Richard Smolak" "Carl Sassenrath"]
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
		
		Additional code modifications and improvements Copyright 2012 Saphirion AG
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Name: graphics
	Type: extension
	Exports: [] ; added by make-host-ext.reb
	Note: "Run make-host-ext.reb to convert"
]

words: [
]

;temp hack - will be removed later
init-words: command [
	words [block!]
]

init-words words

caret-to-offset: command [
	"Returns the xy offset (pair) for a specific string position in a graphics object."
	gob [gob!]
	element [integer! block!] "The position of the string in the richtext block"
	position [integer! string!] "The position within the string"
]


offset-to-caret: command [ ;returns pair! instead of the block..needs to be fixed
	"Returns the richtext block at the string position for an XY offset in the graphics object."
	gob [gob!]
	position [pair!]
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

;#not-yet-used [
;
;effect: command [
;	"Renders effect dialect to an image (returned)."
;	image [image! pair!] "Image or size of image"
;	commands [block!] "Effect commands"
;]
;
;]
