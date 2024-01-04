REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "System object"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2023 Rebol Open Source Contributors
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

; Next four fields are updated during build:
platform: none
version:  0.0.0
build:    object [os: os-version: abi: sys: arch: libc: vendor: target: compiler: date: git: none]

user: construct [
	name: none
	data: #()
]

options: object [  ; Options supplied to REBOL during startup
	boot:           ; The path to the executable
	path:           ; Where script was started or the startup dir
	home:           ; Path of home directory
	data:           ; Path of application data directory
	modules:        ; Path of extension modules
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
	probe-limit: 16000 ; Max probed output size
	module-paths: none ;@@ DEPRECATED!
	default-suffix: %.reb ; Used by IMPORT if no suffix is provided
	file-types: []
	mime-types: none
	result-types: none

	; verbosity of logs per service (codecs, schemes)
	; 0 = nothing; 1 = info; 2 = more; 3 = debug
	log: #[map! [
		rebol: 1
		http: 1
		tls:  1
		zip:  1
		tar:  1
	]]
	domain-name: none ; Specifies system's domain name (used in SMTP scheme so far)
]

catalog: object [
	; Static (non-changing) values, blocks, objects
	datatypes: none
	actions: none
	natives: none
	handles: none
	errors:  none
	; Reflectors are used on boot to create *-of functions
	reflectors: [
		spec   [any-function! any-object! vector! datatype! struct!]
		body   [any-function! any-object! map! struct!]
		words  [any-function! any-object! map! date! handle! struct!]
		values [any-object! map! struct!]
		types  [any-function!]
		title  [any-function! datatype! module!]
	]
	; Official list of system/options/flags that can appear.
	; Must match host reb-args.h enum!
	boot-flags: [
		script args do import version debug secure
		help vers quiet verbose
		secure-min secure-max trace halt cgi boot-level no-window
	]
	bitsets: object [
		crlf:          #[bitset! #{0024}]                             ;charset "^/^M"
		space:         #[bitset! #{0040000080}]                       ;charset " ^-"
		whitespace:    #[bitset! #{0064000080}]                       ;charset "^/^M^- "
		numeric:       #[bitset! #{000000000000FFC0}]                 ;0-9
		alpha:         #[bitset! #{00000000000000007FFFFFE07FFFFFE0}] ;A-Z a-z
		alpha-numeric: #[bitset! #{000000000000FFC07FFFFFE07FFFFFE0}] ;A-Z a-z 0-9
		hex-digits:    #[bitset! #{000000000000FFC07E0000007E}]       ;A-F a-f 0-9
		plus-minus:    #[bitset! #{000000000014}]                     ;charset "+-"
		; chars which does not have to be url-encoded:
		uri:           #[bitset! #{000000005BFFFFF5FFFFFFE17FFFFFE2}] ;A-Z a-z 0-9 !#$&'()*+,-./:;=?@_~
		uri-component: #[bitset! #{0000000041E6FFC07FFFFFE17FFFFFE2}] ;A-Z a-z 0-9 !'()*-._~
		quoted-printable: #[bitset! #{FFFFFFFFFFFFFFFBFFFFFFFFFFFFFFFF}]
	]
	checksums: [
		; will be filled on boot from `Init_Crypt` in `n-crypt.c
	]
	compressions: [deflate zlib gzip] ;; these are always available
	elliptic-curves: [
		; will be filled on boot from `Init_Crypt` in `n-crypt.c`
	]
	ciphers: [
		; will be filled on boot from `Init_Crypt` in `n-crypt.c`
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

		click
		change
		focus
		unfocus
		scroll

		control    ;; used to pass control key events to a console port
		control-up ;; only on Windows?
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
		paste-start  ;; Bracketed paste turned on - https://cirw.in/blog/bracketed-paste
		paste-end    ;; Bracketed paste turned off
		escape       ;; Escape key
		shift
		control
		alt
		pause
		capital	
	]
]

contexts: construct [
	root:
	sys:
	lib:
	user:
]

state: object [
	; Mutable system state variables
	note: "contains protected hidden fields"
	policies: construct [ ; Security policies
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
	last-error:  none ; used by WHY?
	last-result: none ; used to store last console result
	wait-list: []     ; List of ports to add to 'wait
]

modules: object [
	help:    none
	;; external native extensions
	blend2d:       https://github.com/Siskin-framework/Rebol-Blend2D/releases/download/0.0.18.1/
	blurhash:      https://github.com/Siskin-framework/Rebol-BlurHash/releases/download/1.0.0/
	easing:        https://github.com/Siskin-framework/Rebol-Easing/releases/download/1.0.0/
	mathpresso:    https://github.com/Siskin-framework/Rebol-MathPresso/releases/download/0.1.0/
	miniaudio:     https://github.com/Oldes/Rebol-MiniAudio/releases/download/0.11.18.0/
	sqlite:        https://github.com/Siskin-framework/Rebol-SQLite/releases/download/3.42.0.0/
	triangulate:   https://github.com/Siskin-framework/Rebol-Triangulate/releases/download/1.6.0.0/
	;; optional modules, protocol and codecs
	httpd:            https://src.rebol.tech/modules/httpd.reb
	prebol:           https://src.rebol.tech/modules/prebol.reb
	spotify:          https://src.rebol.tech/modules/spotify.reb
	unicode-utils:    https://src.rebol.tech/modules/unicode-utils.reb
	daytime:          https://src.rebol.tech/mezz/prot-daytime.reb
	mail:             https://src.rebol.tech/mezz/prot-mail.reb
	mysql:            https://src.rebol.tech/mezz/prot-mysql.reb
	csv:              https://src.rebol.tech/mezz/codec-csv.reb
	ico:              https://src.rebol.tech/mezz/codec-ico.reb
	pdf:              https://src.rebol.tech/mezz/codec-pdf.reb
	swf:              https://src.rebol.tech/mezz/codec-swf.reb
	xml:              https://src.rebol.tech/mezz/codec-xml.reb
	json:             https://src.rebol.tech/mezz/codec-json.reb
	plist:            https://src.rebol.tech/mezz/codec-plist.reb
	bbcode:           https://src.rebol.tech/mezz/codec-bbcode.reb
	html-entities:    https://src.rebol.tech/mezz/codec-html-entities.reb
	mime-field:       https://src.rebol.tech/mezz/codec-mime-field.reb
	mime-types:       https://src.rebol.tech/mezz/codec-mime-types.reb
	quoted-printable: https://src.rebol.tech/mezz/codec-quoted-printable.reb
	webdriver:        https://src.rebol.tech/modules/webdriver.reb
	websocket:        https://src.rebol.tech/modules/websocket.reb
	;; and..
	window: none ;- internal extension for gui (on Windows so far!)
]

codecs: object []

dialects: construct [
	secure:
	draw:
	effect:
	text:
	rebcode:
]

schemes: make block! 20 ; Block only before init-scheme! Than it is an object.

ports: object [
	system:         ; Port for system events
	event:          ; Port for GUI
	input:          ; Port for user input.
	output:         ; Port for user output
	echo:           ; Port for echoing output
	mail:           ; Port for sending and receiving emails
	callback: none	; Port for callback events
;	serial: none	; serial device name block
]

locale: object [
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

script: construct [
	title:          ; Title string of script
	header:         ; Script header as evaluated
	parent:         ; Script that loaded the current one
	path:           ; Location of the script being evaluated
	args:           ; args passed to script
]

standard: object [

	codec: construct [
		name:       ;word!
		type:       ;word!
		title:      ;string!
		suffixes:   ;block!
		decode:     ;[any-function! none!]
		encode:     ;[any-function! none!]
		identify:   ;[any-function! none!]
	]

	enum: none ; is defined later in %mezz-func.r file

	error: construct [ ; Template used for all errors:
		code: 0
		type: user
		id:   message
		arg1:
		arg2:
		arg3:
		near:
		where:
	]
 
	script: construct [
		title:
		header:
		parent: 
		path:
		args:
	]

	header: construct [
		version: 0.0.0
		title: {Untitled}
		name:
		type:
		date:
		file:
		author:
		needs:
		options:
		checksum:
;		compress:
;		exports:
;		content:
	]

	scheme: construct [
		name:		; word of http, ftp, sound, etc.
		title:		; user-friendly title for the scheme
		spec:		; custom spec for scheme (if needed)
		info:		; prototype info object returned from query
;		kind:		; network, file, driver
;		type:		; bytes, integers, objects, values, block
		actor:		; standard action handler for scheme port functions
		awake:		; standard awake handler for this scheme's ports
	]

	port: construct [ ; Port specification object
		spec:		; published specification of the port
		scheme:		; scheme object used for this port
		parent:     ; port's parent (for example HTTPS port in TLS)
		actor:		; port action handler (script driven)
		awake:		; port awake function (event driven)
		state:		; internal state values (private)
		extra:		; user-defined storage of local data
		data:		; data buffer (usually binary or block)
;		stats:		; stats on operation (optional)
	]

	port-spec-head: construct [
		title:		; user-friendly title for port
		scheme:		; reference to scheme that defines this port
		ref:		; reference path or url (for errors)
		            ; (extended here)
	]

	port-spec-file: make port-spec-head [
		path:  none
	]

	port-spec-net: make port-spec-head [
		host:   none
		port:   80
		path:   
		target: 
		query:  
		fragment: none
	]

	port-spec-checksum: make port-spec-head [
		scheme: 'checksum
		method: none
	]

	port-spec-crypt: make port-spec-head [
		scheme:    'crypt
		direction: 'encrypt
		algorithm: 
		init-vector:
		key: none
	]

	port-spec-midi: make port-spec-head [
		scheme:    'midi
		device-in:  
		device-out: none
	]

	port-spec-serial: make port-spec-head [
		path: none
		speed: 115200
		data-size: 8
		parity: none
		stop-bits: 1
		flow-control: none ;not supported on all systems
	]
	
	port-spec-audio: make port-spec-head [
		scheme: 'audio
		source: none
		channels: 2
		rate: 44100
		bits: 16
		sample-type: 1
		loop-count: 0
	]

	file-info: construct [
		name:
		size:
		date:
		type:
	]

	net-info: construct [
		local-ip:
		local-port:
		remote-ip:
		remote-port:
	]

	console-info: construct [
		buffer-cols:
		buffer-rows:
		window-cols:
		window-rows:
	]

	vector-info: construct [
		signed:     ; false if unsigned (always true for decimals)
		type:       ; integer! or decimal! so far
		size:       ; size per value in bits
		length:     ; number of values
	]

	date-info: construct [
		year:
		month:
		day:
		time:
		date:
		zone:
		hour:
		minute:
		second:
		weekday:
		yearday:
		timezone:
		;week:
		;isoweek:
		utc:
		julian:
	]
	handle-info: construct [
		type:
	]

	midi-info: construct [
		devices-in:
		devices-out:
	]

	extension: construct [
		lib-base:	; handle to DLL
		lib-file:	; file name loaded
		lib-boot:	; module header and body
		command:	; command function
		cmd-index:	; command index counter
		words:		; symbol references
	]

	stats: construct [ ; port stats
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
	]

	type-spec: construct [
		title:
		type:
	]

	bincode: none
	utype: none
	font: none	; mezz-graphics.h
	para: none	; mezz-graphics.h
]

view: object [
	screen-gob: none
	handler: none
	metrics: construct [
		screen-size:
		border-size:
		border-fixed:
		title-size:
		work-origin:
		work-size: 0x0
	]
]

license: none

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
;		port:
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

