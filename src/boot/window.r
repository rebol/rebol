REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Window"
	Author: ["Richard Smolak" "Carl Sassenrath" "Oldes"]
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
		
		Additional code modifications and improvements Copyright 2012 Saphirion AG
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Name: window
	Type: extension
	Exports: [] ; added by make-host-ext.r
	Note: "Run make-host-ext.r to convert"
]

words: [
	;- widgets   
	button
	check
	radio
	field
	area
	text
	text-list
	progress
	slider
	date-time
	group-box

	;- gui-metric
	border-fixed
	border-size
	screen-size
	virtual-screen-size
	log-size
	phys-size
	screen-dpi
	screen-num
    screen-origin
	title-size
	window-min-size
	work-origin
	work-size
	restore
	minimize
	maximize
	activate
]

;temp hack - will be removed later
init-words: command [
	words [block!]
]

init-words words

init-top-window: command [
	"Initialize window subsystem."
	gob [gob!] "The screen gob (root gob)"
]

cursor: command [
	"Changes the mouse cursor image."
	image [integer! image! none!]
]

show: command [
	"Display or update a graphical object or block of them."
	gob [gob! none!]
]

gui-metric: command [
	"Returns specific gui related metric setting."
	keyword [word!] "Available keywords: BORDER-FIXED, BORDER-SIZE, SCREEN-DPI, LOG-SIZE, PHYS-SIZE, SCREEN-SIZE, VIRTUAL-SCREEN-SIZE, TITLE-SIZE, WINDOW-MIN-SIZE, WORK-ORIGIN and WORK-SIZE."
	/set
		val "Value used to set specific setting(works only on 'writable' keywords)."
    /display
        idx [integer!] "Display index, starting with 0"
]

show-soft-keyboard: command [
	"Display on-screen keyboard for user input."
	/attach
		gob [gob!] "GOB which should be visible during the input operation"
]

