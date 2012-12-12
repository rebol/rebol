REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Sys: Port and Scheme Functions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Context: sys
	Note: {
		The boot binding of this module is SYS then LIB deep.
		Any non-local words not found in those contexts WILL BE
		UNBOUND and will error out at runtime!
	}
]

make-port*: func [
	"SYS: Called by system on MAKE of PORT! port from a scheme."
	spec [file! url! block! object! word! port!] "port specification"
	/local name scheme port
][
	; The first job is to identify the scheme specified:
	case [
		file? spec  [
			name: pick [dir file] dir? spec
			spec: join [ref:] spec
		]
		url? spec [
			spec: repend decode-url spec [to set-word! 'ref spec]
			name: select spec to set-word! 'scheme
		]
		block? spec [
			name: select spec to set-word! 'scheme
		]
		object? spec [
			name: get in spec 'scheme
		]
		word? spec [
			name: spec
			spec: []
		]
		port? spec [
			name: port/scheme/name
			spec: port/spec
		]
		true [
			return none
		]
	]

	; Get the scheme definition:
	unless all [
		word? name
		scheme: get in system/schemes name
	][cause-error 'access 'no-scheme name]

	; Create the port with the correct scheme spec:
	port: make system/standard/port []
	port/spec: make any [scheme/spec system/standard/port-spec-head] spec
	port/spec/scheme: name
	port/scheme: scheme

	; Defaults:
	port/actor: get in scheme 'actor ; avoid evaluation
	port/awake: any [get in port/spec 'awake :scheme/awake]
	unless port/spec/ref [port/spec/ref: spec]
	unless port/spec/title [port/spec/title: scheme/title]
	port: to port! port

	; Call the scheme-specific port init. Note that if the
	; scheme has not yet been initialized, it can be done
	; at this time.
	if in scheme 'init [scheme/init port]
	port
]

*parse-url: make object! [
	digit:       make bitset! "0123456789"
	digits:      [1 5 digit]
	alpha-num:   make bitset! [#"a" - #"z" #"A" - #"Z" #"0" - #"9"]
	scheme-char: insert copy alpha-num "+-."
	path-char:   insert copy alpha-num "/=+-_.;:&$@%*',~?| []()^"" ; !!! note: space allowed
	user-char:   insert copy alpha-num "=+-_.;&$%*,'#|"
	pass-char:   complement make bitset! "^/ ^-@"
	s1: s2: none ; in R3, input datatype is preserved - these are now URL strings!
	out: []
	emit: func ['w v] [reduce/into [to set-word! w if :v [to string! :v]] tail out]

	rules: [
		; Scheme://user-host-part
		[
			; scheme name: [//]
			copy s1 some scheme-char ":" opt "//" ; we allow it
			(reduce/into [to set-word! 'scheme to lit-word! to string! s1] tail out)

			; optional user [:pass]
			opt [
				copy s1 some user-char
				opt [#":" copy s2 to #"@" (emit pass s2)]
				#"@" (emit user s1)
			]

			; optional host [:port]
			opt [
				copy s1 any user-char
				opt [#":" copy s2 digits (compose/into [port-id: (to integer! s2)] tail out)]
				(unless empty? s1 [attempt [s1: to tuple! s1] emit host s1])
			]
		]

		; optional path
		opt [copy s1 some path-char (emit path s1)]

		; optional bookmark
		opt [#"#" copy s1 some path-char (emit tag s1)]
	]

	decode-url: func ["Decode a URL according to rules of sys/*parse-url." url] [
		--- "This function is bound in the context of sys/*parse-url."
		out: make block! 8
		parse/all url rules
		out
	]
]

decode-url: none ; used by sys funcs, defined above, set below

;-- Native Schemes -----------------------------------------------------------

make-scheme: func [
	"INIT: Make a scheme from a specification and add it to the system."
	def [block!] "Scheme specification"
	/with 'scheme "Scheme name to use as base"
	/local actor
][
	with: either with [get in system/schemes scheme][system/standard/scheme]
	unless with [cause-error 'access 'no-scheme scheme]

	def: make with def
	;print ["Scheme:" def/name]
	unless def/name [cause-error 'access 'no-scheme-name def]
	set-scheme def

	; If actor is block build a non-contextual actor object:
	if block? :def/actor [
		actor: make object! (length? def/actor) / 4
		foreach [name func* args body] def/actor [ ; (maybe PARSE is better here)
			name: to word! name ; bug!!! (should not be necessary?)
			repend actor [name func args body]
		]
		def/actor: actor
	]

	append system/schemes reduce [def/name def]
]

init-schemes: func [
	"INIT: Init system native schemes and ports."
][
	loud-print "Init schemes"

	sys/decode-url: lib/decode-url: :sys/*parse-url/decode-url

	system/schemes: make object! 10

	make-scheme [
		title: "System Port"
		name: 'system
		awake: func [
			sport "System port (State block holds events)"
			ports "Port list (Copy of block passed to WAIT)"
			/local event port waked
		][
			waked: sport/data ; The wake list (pending awakes)

			; Process all events (even if no awake ports).
			; Do only 8 events at a time (to prevent polling lockout).
			loop 8 [
				unless event: take sport/state [break]
				port: event/port
				if wake-up port event [
					; Add port to wake list:
					;print ["==System-waked:" port/spec/ref]
					unless find waked port [append waked port]
				]
			]

			; No wake ports (just a timer), return now.
			unless block? ports [return none]

			; Are any of the requested ports awake?
			forall ports [
				if find waked first ports [return true]
			]

			false ; keep waiting
		]
		init: func [port] [
			;;print ["Init" title]
			port/data: copy [] ; The port wake list
		]
	]

	make-scheme [
		title: "Console Access"
		name: 'console
	]

	make-scheme [
		title: "Callback Event Functions"
		name: 'callback
		awake: func [event] [
			do-callback event
			true
		]
	]

	make-scheme [
		title: "File Access"
		name: 'file
		info: system/standard/file-info ; for C enums
		init: func [port /local path] [
			if url? port/spec/ref [
				parse port/spec/ref [thru #":" 0 2 slash path:]
				append port/spec compose [path: (to file! path)]
			]
		]
	]

	make-scheme/with [
		title: "File Directory Access"
		name: 'dir
	] 'file

	make-scheme [
		title: "GUI Events"
		name: 'event
		awake: func [event] [
			print ["Default GUI event/awake:" event/type]
			true
		]
	]

	make-scheme [
		title: "DNS Lookup"
		name: 'dns
		spec: system/standard/port-spec-net
		awake: func [event] [print event/type true]
	]

	make-scheme [
		title: "TCP Networking"
		name: 'tcp
		spec: system/standard/port-spec-net
		info: system/standard/net-info ; for C enums
		awake: func [event] [print ['TCP-event event/type] true]
	]

	make-scheme [
		title: "Clipboard"
		name: 'clipboard
	]

	system/ports/system:   open [scheme: 'system]
	system/ports/input:    open [scheme: 'console]
	system/ports/callback: open [scheme: 'callback]

	init-schemes: 'done ; only once
]
