REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "View - windowing system basic API"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

; The View system handles windowing, events, popups, requestors, and modal
; operations.  Normally VID provides the contents for these, but users are
; also allowed to build and display their own windows directly.

system/standard/font: construct [
	name: "arial"
	style: none
	size: 12
	color: 0.0.0
	offset: 2x2
	space: 0x0
	shadow: none
]

system/standard/para: construct [
	origin: 2x2
	margin: 2x2
	indent: 0x0
	tabs: 40
	wrap?: true
	scroll: 0x0
	align: 'left
	valign: 'top
]

view: func [
	"Displays a window view."
	window [gob! block! object! image!] "Window gob, VID face, or VID layout block"
	/options opts [block!] "Window options spec block"
	/no-wait "Return immediately. Do not wait and process events."
	/as-is "Leave window as is. Do not add a parent gob."
	/local screen tmp xy image
][
	if any [
		not screen: system/view/screen-gob
		window = screen
	][	return none]

	; Convert option block to a map:
	opts: make map! any [reduce/no-set opts []]
	case/all [
		no-wait [opts/no-wait: true]
		as-is   [opts/as-is: true]
;		options [append opts reduce/no-set opts]
	]

	if image? window [
		image: window
		window: make gob! [
			size: image/size
			image: image
		]
		opts/flags: [resize]
		opts/offset: 'center
	]

	; GOB based view:
	if gob? window [
		; Build the window:
		unless opts/as-is [
			tmp: window
			tmp/offset: 0x0
			window: make gob! [size: tmp/size]
			window/offset: system/view/metrics/title-size
			append window tmp
		]
		; Set optional background:
		if any [opts/color opts/draw] [
			insert window make gob! append copy [
				size: window/size
				offset: 0x0
			] pick [
				[draw:  opts/draw]
				[color: opts/color]
			] block? opts/draw
		]
		; Set up default handler...
		if all [
			empty? system/ports/event/extra/handlers  ; ...if there is no other handler
			not opts/handler                              ; ...and user did not provide one
		][
			handle-events [
				name: 'view-default
				priority: 50
				handler: func [event] [
					;print ["view-event:" event/type event/offset]
					if switch event/type [
						close [true]
						key [event/key = escape]
					][
						unhandle-events self
						unview event/window
						;quit
					]
					;show event/window
					none ; we handled it
				]
			]
		]
	]

	; VID-layout based view:
	if block? window [
		window: layout/background window any [opts/draw opts/color]
	]

	; VID-face based view:
	if object? window [
		; Build the window:
		window: append make gob! [
			data: window
			size: window/size
		] window/gob
		window/flags: [resize] ; may be overwritten below
	]

	; Window title:
	window/text: any [opts/title window/text "REBOL: untitled"]

	;!!! Add later: use script title - once modules provide that

	; Other options:
	if opts/offset [
		; 'Center is allowed:
		if word? opts/offset [
			opts/offset: either opts/offset = 'center [screen/size - window/size / 2][100x100]
		]
		window/offset: opts/offset
	]
	if opts/owner [window/owner: opts/owner]
	if opts/flags [window/flags: opts/flags]
	if opts/handler [handle-events opts/handler]

	; Add the window to the screen. If it is already there, this action
	; will move it to the top:
	unless window = screen [append screen window]

	; Open or refresh the window:
	show window

	; Wait for the event port to tell us we can return:
	if all [
		not opts/no-wait
		1 = length? screen
	][
		do-events
	]

	; Return window (which may have been created here):
	window
]

unview: func [
	"Closes a window view."
	window [object! gob! word! none!] "Window face or GOB. 'all for all. none for last"
	/local screen
][
	screen: system/view/screen-gob
	case [
		object? window [window: window/gob/parent]
		window = 'all [show clear screen exit]
		not window [window: last screen] ; NONE is ok
	]
	remove find screen window ; none ok
	show window ; closes it, none ok
]

base-handler: construct [
	name: 'no-name
	priority: 0
]

handle-events: func [
	"Adds a handler to the view event system."
	handler [block!]
	/local sys-hand
][
	handler: make base-handler handler
	sys-hand: system/ports/event/extra/handlers
	; First check if there is not any handler with such a name...
	forall sys-hand [
		if handler/name = sys-hand/1/name [
			; ...if so, replace it with the new one
			change sys-hand handler
			return handler
		]
	]
	; Insert by priority:
	unless foreach [here: hand] sys-hand [
		if handler/priority > hand/priority [
			insert here handler
			break/return true
		]
	][
		append sys-hand handler
	]
	handler
]

unhandle-events: func [
	"Removes a handler from the view event system."
	handler [object!]
][
	remove find system/ports/event/extra/handlers handler
	exit
]

handled-events?: func [
	"Returns event handler object matching a given name."
	name
][
	foreach hand system/ports/event/extra/handlers [
		if hand/name = name [return hand]
	]
	none
]

do-events: func [
	"Waits for window events. Returns when all windows are closed."
][
	wait system/ports/event
]

init-view-system: func [
	"Initialize the View subsystem."
	/local ep
][
	; The init function called here resides in this module
	init-top-window system/view/screen-gob: make gob! [text: "Top Gob"]

	;update the metrics object (temp - will become mezz later)
	foreach w words-of system/view/metrics [
		set in system/view/metrics w gui-metric w
	]

	ep: system/ports/event

	unless ep/extra [
		; Create block of event handlers:
		ep/extra: object [handlers: copy []]

		; Global event handler for view system:
		ep/awake: func [event /local h] [
			h: event/port/extra/handlers
			while [ ; (no binding needed)
				all [event not tail? h]
			][
				; Handlers should return event in order to continue.
				event: h/1/handler event
				h: next h
			]
			tail? system/view/screen-gob
		]
	]

	init-top-window:
	init-view-system: 'done
]

init-view-system
