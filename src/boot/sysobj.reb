REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "System object"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		Defines the system object. This is a special block that is evaluted
		such that its words do not get put into the current context. 
	}
	Note: "Remove older/unused fields before beta release"
]

product:  'core

; Next three fields are updated during build:
version:  0.0.0
build:    1
platform: none

license: {Copyright 2012 REBOL Technologies
REBOL is a trademark of REBOL Technologies
Licensed under the Apache License, Version 2.0.
See: http://www.apache.org/licenses/LICENSE-2.0
}

catalog: context [
	; Static (non-changing) values, blocks, objects
	datatypes: []
	actions: none
	natives: none
	errors: none
	reflectors: [spec body words values types title]
	; Official list of system/options/flags that can appear.
	; Must match host reb-args.h enum!
	boot-flags: [
		script args do import version debug secure
		help vers quiet verbose
		secure-min secure-max trace halt cgi boot-level no-window
	]
]

contexts: context [
	root:
	sys:
	lib:
	user:
		none
]

state: context [
	; Mutable system state variables
	note: "contains protected hidden fields"
	policies: context [ ; Security policies
		file:    ; file access
		net:     ; network access
		eval:    ; evaluation limit
		memory:  ; memory limit
		secure:  ; secure changes
		protect: ; protect function
		debug:   ; debugging features
		envr:    ; read/write
		call:    ; execute only
		browse:  ; execute only
			0.0.0
		extension: 2.2.2 ; execute only
	]
	last-error: none ; used by WHY?
]

modules: []

codecs: context []

dialects: context [
	secure:
	draw:
	effect:
	text:
	rebcode:
		none
]

schemes: context []

ports: context [
	wait-list: []	; List of ports to add to 'wait
	input:          ; Port for user input.
	output:         ; Port for user output
	echo:           ; Port for echoing output
	system:         ; Port for system events
	callback: none	; Port for callback events
;	serial: none	; serial device name block
]

locale: context [
	language: 	; Human language locale
	language*:
	locale:
	locale*: none
	months: [
		"January" "February" "March" "April" "May" "June"
		"July" "August" "September" "October" "November" "December"
	]
	days: [
		"Monday" "Tuesday" "Wednesday" "Thursday" "Friday" "Saturday" "Sunday"
	]
]

options: context [  ; Options supplied to REBOL during startup
	boot:           ; The path to the executable
	home:           ; Path of home directory
	path:           ; Where script was started or the startup dir
		none

	flags:          ; Boot flag bits (see system/catalog/boot-flags)
	script:         ; Filename of script to evaluate
	args:           ; Command line arguments passed to script
	do-arg:         ; Set to a block if --do was specified
	import:         ; imported modules
	debug:          ; debug flags
	secure:         ; security policy
	version:        ; script version needed
	boot-level:     ; how far to boot up
		none

	quiet: false    ; do not show startup info (compatibility)

	binary-base: 16    ; Default base for FORMed binary values (64, 16, 2)
	decimal-digits: 15 ; Max number of decimal digits to print.
	module-paths: [%./]
	default-suffix: %.reb ; Used by IMPORT if no suffix is provided
	file-types: []
	result-types: none
]

script: context [
	title:          ; Title string of script
	header:         ; Script header as evaluated
	parent:         ; Script that loaded the current one
	path:           ; Location of the script being evaluated
	args:           ; args passed to script
		none
]

standard: context [

	error: context [ ; Template used for all errors:
		code: 0
		type: 'user
		id:   'message
		arg1:
		arg2:
		arg3:
		near:
		where:
			none
	]
 
	script: context [
		title:
		header:
		parent: 
		path:
		args:
			none
	]

	header: context [
		title: {Untitled}
		name:
		type:
		version:
		date:
		file:
		author:
		needs:
		options:
		checksum:
;		compress:
;		exports:
;		content:
			none
	]

	scheme: context [
		name:		; word of http, ftp, sound, etc.
		title:		; user-friendly title for the scheme
		spec:		; custom spec for scheme (if needed)
		info:		; prototype info object returned from query
;		kind:		; network, file, driver
;		type:		; bytes, integers, objects, values, block
		actor:		; standard action handler for scheme port functions
		awake:		; standard awake handler for this scheme's ports
			none
	]

	port: context [ ; Port specification object
		spec:		; published specification of the port
		scheme:		; scheme object used for this port
		actor:		; port action handler (script driven)
		awake:		; port awake function (event driven)
		state:		; internal state values (private)
		data:		; data buffer (usually binary or block)
		locals:		; user-defined storage of local data
;		stats:		; stats on operation (optional)
			none
	]

	port-spec-head: context [
		title:		; user-friendly title for port
		scheme:		; reference to scheme that defines this port
		ref:		; reference path or url (for errors)
		path:		; used for files
		   none 	; (extended here)
	]

	port-spec-net: make port-spec-head [
		host: none
		port-id: 80
			none
	]
	
	file-info: context [
		name:
		size:
		date:
		type:
			none
	]

	net-info: context [
		local-ip:
		local-port:
		remote-ip:
		remote-port:
			none
	]

	extension: context [
		lib-base:	; handle to DLL
		lib-file:	; file name loaded
		lib-boot:	; module header and body
		command:	; command function
		cmd-index:	; command index counter
		words:		; symbol references
			none
	]

	stats: context [ ; port stats
		timer:		; timer (nanos)
		evals:		; evaluations
		eval-natives:
		eval-functions:
		series-made:
		series-freed:
		series-expanded:
		series-bytes:
		series-recycled:
		made-blocks:
		made-objects:
		recycles:
			none
	]

	type-spec: context [
		title:
		type:
			none
	]

	utype: none
	font: none	; mezz-graphics.h
	para: none	; mezz-graphics.h
]

view: context [
	screen-gob: none
	handler: none
	event-port: none
	metrics: context [
		screen-size: 0x0
		border-size: 0x0
		border-fixed: 0x0
		title-size: 0x0
		work-origin: 0x0
		work-size: 0x0
	]
	event-types: [
		; Event types. Order dependent for C and REBOL.
		; Due to fixed C constants, this list cannot be reordered after release!
		ignore			; ignore event (0)
		interrupt		; user interrupt
		device			; misc device request
		callback		; callback event
		custom			; custom events
		error
		init

		open
		close
		connect
		accept
		read
		write
		wrote
		lookup

		ready
		done
		time

		show
		hide
		offset
		resize
		active
		inactive 
		minimize
		maximize
		restore

		move
		down
		up
		alt-down 
		alt-up 
		aux-down 
		aux-up 
		key
		key-up ; Move above when version changes!!!

		scroll-line
		scroll-page

		drop-file
	]
	event-keys: [
		; Event types. Order dependent for C and REBOL.
		; Due to fixed C constants, this list cannot be reordered after release!
		page-up
		page-down
		end
		home
		left
		up
		right
		down
		insert
		delete
		f1
		f2
		f3
		f4
		f5
		f6
		f7
		f8
		f9
		f10
		f11
		f12
	]
]

;;stats: none

;user-license: context [
;	name:
;	email:
;	id:
;	message:
;		none
;]



; (returns value)

;		model:		; Network, File, Driver
;		type:		; bytes, integers, values
;		user:		; User data

;		host:
;		port-id:
;		user:
;		pass:
;		target:
;		path:
;		proxy:
;		access:
;		allow:
;		buffer-size:
;		limit:
;		handler:
;		status:
;		size:
;		date:
;		sub-port:
;		locals:
;		state:
;		timeout:
;		local-ip:
;		local-service:
;		remote-service:
;		last-remote-service:
;		direction:
;		key:
;		strength:
;		algorithm:
;		block-chaining:
;		init-vector:
;		padding:
;		async-modes:
;		remote-ip:
;		local-port:
;		remote-port:
;		backlog:
;		device:
;		speed:
;		data-bits:
;		parity:
;		stop-bits:
;			none
;		rts-cts: true
;		user-data:
;		awake:

;	port-flags: context [
;		direct:
;		pass-thru:
;		open-append:
;		open-new:
;			none
;	]

;	email: context [ ; Email header object
;		To:
;		CC:
;		BCC:
;		From:
;		Reply-To:
;		Date:
;		Subject:
;		Return-Path:
;		Organization:
;		Message-Id:
;		Comment:
;		X-REBOL:
;		MIME-Version:
;		Content-Type:
;		Content:
;			none
;	]

;user: context [
;	name:           ; User's name
;	email:          ; User's default email address
;	home:			; The HOME environment variable
;	words: none
;]

;network: context [
;	host: ""        ; Host name of the user's computer
;	host-address: 0.0.0.0 ; Host computer's TCP-IP address
;	trace: none
;]

;console: context [
;	hide-types: none ; types not to print
;	history:         ; Log of user inputs
;	keys: none       ; Keymap for special key
;	prompt:  {>> }   ; Specifies the prompt
;	result:  {== }   ; Specifies result
;	escape:  {(escape)} ; Indicates an escape
;	busy:    {|/-\}  ; Spinner for network progress
;	tab-size: 4      ; default tab size
;	break: true		 ; whether escape breaks or not
;]

;			decimal: #"."	; The character used as the decimal point in decimal and money vals
;			sig-digits: none	; Significant digits to use for decimals ; none for normal printing
;			date-sep: #"-"	; The character used as the date separator
;			date-month-num: false	; True if months are displayed as numbers; False for names
;			time-sep: #":"	; The character used as the time separator
;	cgi: context [ ; CGI environment variables
;		server-software:
;		server-name:
;		gateway-interface:
;		server-protocol:
;		server-port:
;		request-method:
;		path-info:
;		path-translated:
;		script-name:
;		query-string:
;		remote-host:
;		remote-addr:
;		auth-type:
;		remote-user:
;		remote-ident:
;		Content-Type:			; cap'd for email header
;		content-length: none
;		other-headers: []
;	]
;	browser-type: 0

;	trace:			; True if the --trace flag was specified
;	help: none      ; True if the --help flags was specified
;	halt: none		; halt after script

