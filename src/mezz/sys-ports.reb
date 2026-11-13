REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Sys: Port and Scheme Functions"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2023 Rebol Open Source Contributors
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
			name: case [
				wildcard?  spec ['dir]
				dir?/check spec [spec: dirize spec 'dir]
				true            ['file]
			]
			spec: compose [ref: (spec)]
		]
		url? spec [
			spec: repend decode-url spec [to set-word! 'ref spec]
			name: select spec to set-word! 'scheme
			if lit-word? name [name: to word! name]
		]
		block? spec [
			name: select spec to set-word! 'scheme
			if lit-word? name [name: to word! name]
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

url-parser: make object! [
	;; Source of this url-parser is inspired by Gregg Irwin's code:
	;; https://gist.github.com/greggirwin/207149d46441cd48a1426e60926a7d25
	;; which is now used in Red:
	;; https://github.com/red/red/blob/f619641b573621ee4c0ca7e0a8b706053db53a36/environment/networking.red#L34-L209
	;; Output of this version is different than in Red!

	out: make block! 14
	value: none

	;-- Basic Character Sets
	digit:       system/catalog/bitsets/numeric
	alpha:       system/catalog/bitsets/alpha
	alpha-num:   system/catalog/bitsets/alpha-numeric
	hex-digit:   system/catalog/bitsets/hex-digits
	enhex-bits:  copy system/catalog/bitsets/uri
	enhex-bits/(#"%"): true ;; ignore percentage char (it may be used for already escaped content)

	;-- URL Character Sets
	;URIs include components and subcomponents that are delimited by characters in the "reserved" set.
	gen-delims:  #(bitset! #{000000001001002180000014})         ;= charset ":/?#[]@"
	sub-delims:  #(bitset! #{000000004BF80014})                 ;= charset "!$&'()*+,;="
	reserved:    #(bitset! #{000000005BF9003580000014})         ;= [gen-delims | sub-delims]
	;The purpose of reserved characters is to provide a set of delimiting
	;characters that are distinguishable from other data within a URI.

	;Characters that are allowed in a URI but do not have a reserved purpose are "unreserved"
	unreserved:  #(bitset! #{000000000006FFC07FFFFFE17FFFFFE2}) ;= compose [alpha | digit | (charset "-._~")]
	scheme-char: #(bitset! #{000000000016FFC07FFFFFE07FFFFFE0}) ;= union alpha-num "+-."
	
	;-- URL Grammar
	url-rules:   [
		scheme-part
		hier-part (
			if all [value not empty? value][
				case [
					out/scheme = 'mailto [
						emit target to string! dehex :value
					]

					all [out/scheme = 'urn parse value [
						; case like: urn:example:animal:ferret:nose (#":" is not a valid file char)
						; https://datatracker.ietf.org/doc/html/rfc2141
						copy value to #":" (
							emit path   to string! dehex value ;= Namespace Identifier
						)
						1 skip
						copy value to end (
							emit target to string! dehex value ;= Namespace Specific String
						)
					]] true

					'else [
						value: to file! dehex :value
						either dir? value [
							emit path value
						][
							value: split-path value
							if %./ <> value/1 [emit path value/1]
							emit target value/2
						]
					]
				]
			]
		)
		opt query
		opt fragment
	]
	scheme-part: [copy value [alpha any scheme-char] #":" (emit scheme to lit-word! lowercase to string! :value)]
	hier-part:   [#"/" #"/" authority path-abempty | path-absolute | path-rootless | path-empty]

	;   The authority component is preceded by a double slash ("//") and is
	;   terminated by the next slash ("/"), question mark ("?"), or number
	;   sign ("#") character, or by the end of the URI.
	authority:   [opt user  host  opt [#":" port]]
	user:	 [
		copy value [any [unreserved | pct-encoded | sub-delims | #":"] #"@"]
		(
			take/last value
			value: to string! dehex value
			parse value [
				copy value to #":" (emit user value)
				1 skip
				copy value to end ( emit pass value)
				|
				(emit user value)
			]
		)
	]
	host: [
		ip-literal (emit host to string! dehex :value) 
		|
		copy value any [unreserved | pct-encoded | sub-delims]
		(unless empty? value [emit host to string! dehex :value])
	]
	ip-literal:    [copy value [[#"[" thru #"]"] | ["%5B" thru "%5D"]]] ; simplified from [IPv6address | IPvFuture]
	port:          [copy value [1 5 digit] (emit port to integer! to string! :value)]
	pct-encoded:   [#"%" 2 hex-digit]
	pchar:         [unreserved | pct-encoded | sub-delims | #":" | #"@" | #"%"]	; path characters (allows also %)
	path-abempty:  [copy value any-segments | path-empty]
	path-absolute: [copy value [#"/" opt [segment-nz any-segments]]]
	path-rootless: [copy value [segment-nz any-segments]]
	path-empty:    [none]
	segment:       [any pchar]
	segment-nz:    [some pchar]
	segment-nz-nc: [some [unreserved | pct-encoded | sub-delims | #"@"]]	; non-zero-length segment with no colon
	any-segments:  [any [#"/" segment]]
	;; not using dehex for the query value, because there may be escaped important chars like & and =
	query:         [#"?" copy value any [pchar | slash | #"?"] (emit query    to string! :value)]
	fragment:      [#"#" copy value any [pchar | slash | #"?"] (emit fragment to string! dehex :value)]

	; Helper function
	emit: func ['w v] [reduce/into [to set-word! w :v] tail out]


	;-- Parse Function
	parse-url: function [
		"Return object with URL components, or cause an error if not a valid URL"
		url  [url! string!]
	][
		;; escape all not escaped chars of the url first
		;; and convert it to binary, so it is treated as UTF-8 before parsing               
		url: to binary! enhex/except url enhex-bits 
		result: either parse url url-rules [
			copy out
		][
			none
		]
		; cleanup (so there are no remains visible in the url-parser object)
		clear out
		set 'value none
		; done
		result
	]

	; Exported function (Rebol compatible name)
	set 'decode-url function [
		"Decode a URL into an object containing its constituent parts"
		url [url! string!]
	][
		parse-url url
	]
]

decode-url: none ; used by sys funcs, defined above, set below

;-- Native Schemes -----------------------------------------------------------

make-scheme: func [
	"INIT: Make a scheme from a specification and add it to the system."
	def [block!] "Scheme specification"
	/with 'scheme "Scheme name to use as base"
	/local actor name func* args body pos
][
	with: either with [get in system/schemes scheme][system/standard/scheme]
	unless with [cause-error 'access 'no-scheme scheme]

	def: make with def
	;print ["Scheme:" def/name]
	unless def/name [cause-error 'access 'no-scheme-name def]
	unless def/title [def/title: uppercase/part form def/name 1]
	set-scheme def

	; If actor is block build a non-contextual actor object:
	if block? :def/actor [
		actor: make object! (length? :def/actor) / 4
		parse :def/actor [any [
			copy name some set-word! [
				set func* any-function!
				|
				'func set args block! set body block! (func*: func args body)
				|
				'function set args block! set body block! (func*: function args body)
			] (
				forall name [put actor name/1 :func*]
			)
			| end
			| pos: (
				cause-error 'script 'invalid-arg pos
			)
		]]

		def/actor: actor
	]

	append system/schemes reduce [def/name def]
	def
]

init-schemes: func [
	"INIT: Init system native schemes and ports."
	/local schemes
][
	log/debug 'REBOL "Init schemes"

	sys/decode-url: lib/decode-url: :sys/url-parser/parse-url

	; optional schemes are collected into a block
	; and will be initialized after common schemes
	schemes: system/schemes
	; schemes are finally stored in the object
	system/schemes: make object! 20

	make-scheme [
		title: "System Port"
		name: 'system
		awake: func [
			sport "System port (State block holds events)"
			ports "Port list (Copy of block passed to WAIT)"
			/only
			/local event event-list n-event port waked
		][
			waked: sport/data ; The wake list (pending awakes)

			if only [
				unless block? ports [return none] ;short cut for a pause
			]

			; Process all events (even if no awake ports).
			n-event: 0
			event-list: sport/state
			while [not empty? event-list][
				if n-event > 8 [break] ; Do only 8 events at a time (to prevent polling lockout).
				event: first event-list
				port: event/port
				either any [
					none? only
					find ports port
				][
					remove event-list ;avoid event overflow caused by wake-up recursively calling into wait
					if wake-up port event [
						;@@ `wake-up` function is natively calling UPDATE action on current port
						;@@ this is processed in port's actor function,                         
						;@@ for example in `Console_Actor` in p-console.c file                  
						;@@ If result of this action is truethy, port is waked up here          
						;print ["==System-waked:" port/spec/ref]
						; Add port to wake list:
						unless find waked port [append waked port]
					]
					++ n-event
				][
					event-list: next event-list
				]
			]

			; No wake ports (just a timer), return now.
			unless block? ports [return none]

			; Are any of the requested ports awake?
			forall ports [
				if find waked first ports [return true]
			]

			either zero? n-event [
				none ;events are ignored
			][
				false ; keep waiting
			]
		]
		init: func [port] [
			;;print ["Init" title]
			port/data: copy [] ; The port wake list
		]
	]

	make-scheme [
		title: "Console Access"
		name: 'console
		;awake: func [event] [
		;	print ["Console event:" event/type mold event/key event/offset]
		;	;probe event
		;	false
		;]
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
		awake: func [event] [true]
	]

	make-scheme [
		title: "TCP Networking"
		name: 'tcp
		spec: system/standard/port-spec-net
		info: system/standard/net-info ; for C enums
		awake: func [event] [print ['TCP-event event/type] true]
	]

	make-scheme [
		title: "UDP Networking"
		name: 'udp
		spec: system/standard/port-spec-net
		info: system/standard/net-info ; for C enums
		awake: func [event] [print ['UDP-event event/type] true]
	]

	make-scheme [
		title: "Checksum port"
		info: "Possible methods are in `system/catalog/checksums`"
		spec: system/standard/port-spec-checksum
		name: 'checksum
		init: function [
			port [port!]
		][
			spec: port/spec
			method: any [
				select spec 'method
				select spec 'target ; if scheme was opened using url type (checksum:sha1)
				select spec 'host   ; when used as: checksum://sha1
				'md5                ; default method
			]
			if any [
				error? try [spec/method: to word! method] ; in case it was not
				not find system/catalog/checksums spec/method
			][
				cause-error 'access 'invalid-spec method
			] 
			; make port/spec to be only with checksum related keys
			set port/spec: copy system/standard/port-spec-checksum spec
			;protect/words port/spec ; protect spec object keys of modification
		]

	]

	make-scheme [
		title: "Crypt"
		spec: system/standard/port-spec-crypt
		name: 'crypt
		init: function [
			port [port!]
		][
			spec: port/spec
			algorithm: any [
				select spec 'algorithm
				select spec 'target ; if scheme was opened using url type: crypt:chacha20
				select spec 'host   ; or when used as: crypt://chacha20
			]
			direction: any [
				select spec 'fragment ; from: crypt://chacha20#decrypt
				select spec 'direction
			]
			if any [
				error? try [spec/algorithm: to word! :algorithm] ; in case it was not
				not find system/catalog/ciphers spec/algorithm
			][
				cause-error 'access 'invalid-spec :algorithm
			]
			if any [
				error? try [spec/direction: to word! :direction] ; in case it was not
				not find [encrypt decrypt] spec/direction
			][
				cause-error 'access 'invalid-spec :direction
			]
			; make port/spec to be only with crypt related keys
			set port/spec: copy system/standard/port-spec-crypt spec
			if block? port/spec/ref [
				port/spec/ref: as url! ajoin ["crypt://" :algorithm #"#" :direction]
			]
			;protect/words port/spec ; protect spec object keys of modification
		]
	]


	make-scheme [
		title: "Clipboard"
		name: 'clipboard
	]

	make-scheme [
		title: "Serial Port"
		name: 'serial
		spec: system/standard/port-spec-serial
		init: func [port /local path speed] [
			if url? port/spec/ref [
				parse port/spec/ref [
					thru #":" 0 2 slash
					copy path [to slash | end] skip
					copy speed to end
				]
				try/with [port/spec/path: to file! path][
					cause-error 'access 'invalid-spec :path
				]
				try/with [port/spec/speed: to integer! speed][
					cause-error 'access 'invalid-spec :speed
				]
			]
		]
	]

	;- init optional schemes (from own source files)...
	forall schemes [make-scheme schemes/1]


	system/ports/system:   open [scheme: 'system]
	system/ports/event:    open [scheme: 'event]
	system/ports/input:
	system/ports/output:   open [scheme: 'console]
	system/ports/callback: open [scheme: 'callback]

	init-schemes: 'done ; only once
]
