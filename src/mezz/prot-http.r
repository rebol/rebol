REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 HTTP protocol scheme"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Name: 'http
	Type: 'module
	Version: 0.1.6
	File: %prot-http.r
	Purpose: {
		This program defines the HTTP protocol scheme for REBOL 3.
	}
	Author: ["Gabriele Santilli" "Richard Smolak" "Oldes"]
	Date: 21-May-2018
	History: [
		0.1.1 22-Jun-2007 "Gabriele Santilli" "Version used in R3-Alpha"
		0.1.4 26-Nov-2012 "Richard Smolak"    "Version from Atronix's fork"
		0.1.5 10-May-2018 "Oldes" "FIX: Query on URL was returning just none"
		0.1.6 21-May-2018 "Oldes" "FEAT: Added support for basic redirection"
	]
]

;@@ idate-to-date should be moved to other location!
digit: charset [#"0" - #"9"]
alpha: charset [#"a" - #"z" #"A" - #"Z"]
idate-to-date: function [date [string!]] [
	either parse date [
		5 skip
		copy day: 2 digit
		space
		copy month: 3 alpha
		space
		copy year: 4 digit
		space
		copy time: to space
		space
		copy zone: to end
	][
		if zone = "GMT" [zone: copy "+0"]
		to date! rejoin [day "-" month "-" year "/" time zone]
	][
		none
	]
]
;@@==================================================

;@@ just simple trace function
;@@net-log: :print

sync-op: func [port body /local state] [
	unless port/state [open port port/state/close?: yes]
	state: port/state
	state/awake: :read-sync-awake
	do body
	if state/state = 'ready [do-request port]
	;NOTE: We'll wait in a WHILE loop so the timeout cannot occur during 'reading-data state.
	;The timeout should be triggered only when the response from other side exceeds the timeout value.
	;--Richard
	while [not find [ready close] state/state][
		;@@net-log ["########### sync-op.." state/state]
		unless port? wait [state/connection port/spec/timeout] [http-error "HTTP(s) Timeout"]
		;@@net-log ["########### sync-op wakeup" state/state]
		switch state/state [
			inited [
				;@@net-log ["state/connection open? ->" open? state/connection]
				if not open? state/connection [
					state/error: make-http-error rejoin ["Internal " state/connection/spec/ref " connection closed"]
					break
				]
			]
			reading-data [
				;? state/connection
				read state/connection
			]
			redirect [
				do-redirect port port/state/info/headers/location
				state: port/state
				state/awake: :read-sync-awake
			]
		]
	]
	if state/error [do state/error]
	body: copy port
	if state/close? [close port]
	body
]
read-sync-awake: func [event [event!] /local error] [
	;@@net-log ["[HTTP read-sync-awake]" event/type]
	switch/default event/type [
		connect ready [
			do-request event/port
			false
		]
		done [
			true
		]
		close [
			true
		]
		custom [
			if all [event/offset event/offset/x = 300] [
				event/port/state/state: 'redirect
				return true
			]
			false
		]
		error [
			error: event/port/state/error
			event/port/state/error: none
			do error
		]
	] [
		false
	]
]
http-awake: func [event /local port http-port state awake res] [
	port: event/port
	http-port: port/locals
	state: http-port/state
	if any-function? :http-port/awake [state/awake: :http-port/awake]
	awake: :state/awake

	;@@net-log ["[HTTP http-awake]"  event/type]

	switch/default event/type [
		read [
			awake make event! [type: 'read port: http-port]
			check-response http-port
		]
		wrote [
			awake make event! [type: 'wrote port: http-port]
			state/state: 'reading-headers
			read port
			false
		]
		lookup [open port false]
		connect [
			state/state: 'ready
			awake make event! [type: 'connect port: http-port]
		]
		close [
			;?? state/state
			res: switch state/state [
				ready [
					awake make event! [type: 'close port: http-port]
				]
				doing-request reading-headers [
					state/error: make-http-error "Server closed connection"
					awake make event! [type: 'error port: http-port]
				]
				reading-data [
					either any [integer? state/info/headers/content-length state/info/headers/transfer-encoding = "chunked"] [
						state/error: make-http-error "Server closed connection"
						awake make event! [type: 'error port: http-port]
					] [
						;set state to CLOSE so the WAIT loop in 'sync-op can be interrupted --Richard
						state/state: 'close
						any [
							awake make event! [type: 'done port: http-port]
							awake make event! [type: 'close port: http-port]
						]
					]
				]
			]
			close http-port
			res
		]
	] [true]
]
make-http-error: func [
	"Make an error for the HTTP protocol"
	message [string! block!]
] [
	if block? message [message: ajoin message]
	make error! [
		type: 'Access
		id: 'Protocol
		arg1: message
	]
]
http-error: func [
	"Throw an error for the HTTP protocol"
	message [string! block!]
] [
	do make-http-error message
]
make-http-request: func [
	"Create an HTTP request (returns string!)"
	method [word! string!] "E.g. GET, HEAD, POST etc."
	target [file! string!] {In case of string!, no escaping is performed (eg. useful to override escaping etc.). Careful!}
	headers [block!] "Request headers (set-word! string! pairs)"
	content [any-string! binary! none!] {Request contents (Content-Length is created automatically). Empty string not exactly like none.}
	/local result
] [
	result: rejoin [
		uppercase form method #" "
		either file? target [next mold target] [target]
		" HTTP/1.0" CRLF
	]
	foreach [word string] headers [
		repend result [mold word #" " string CRLF]
	]
	if content [
		content: to binary! content
		repend result ["Content-Length: " length? content CRLF]
	]
	append result CRLF
	result: to binary! result
	if content [append result content]
	result
]
do-request: func [
	"Perform an HTTP request"
	port [port!]
	/local spec info
] [
	spec: port/spec
	info: port/state/info
	spec/headers: body-of make make object! [
		Accept: "*/*"
		Accept-Charset: "utf-8"
		Host: either not find [80 443] spec/port-id [
			rejoin [form spec/host #":" spec/port-id]
		] [
			form spec/host
		]
		User-Agent: "REBOL"
	] spec/headers
	port/state/state: 'doing-request
	info/headers: info/response-line: info/response-parsed: port/data:
	info/size: info/date: info/name: none

	;@@net-log ["[HTTP do-request]" spec/method spec/host spec/path]

	write port/state/connection make-http-request spec/method to file! any [spec/path %/] spec/headers spec/content
]
parse-write-dialect: func [port block /local spec] [
	spec: port/spec
	parse block [[set block word! (spec/method: block) | (spec/method: 'post)]
		opt [set block [file! | url!] (spec/path: block)] [set block block! (spec/headers: block) | (spec/headers: [])] [set block [any-string! | binary!] (spec/content: block) | (spec/content: none)]
	]
]
check-response: func [port /local conn res headers d1 d2 line info state awake spec] [
	state: port/state
	conn: state/connection
	info: state/info
	headers: info/headers
	line: info/response-line
	awake: :state/awake
	spec: port/spec
	
	;@@net-log ["[HTTP check-response]" info/response-parsed]

	if all [
		not headers
		any [
			all [
				d1: find conn/data crlfbin
				d2: find/tail d1 crlf2bin
				;@@net-log "server using standard content separator of #{0D0A0D0A}"
			]
			all [
				d1: find conn/data #{0A}
				d2: find/tail d1 #{0A0A}
				;@@net-log "server using malformed line separator of #{0A0A}"
			]
		]
	] [
		info/response-line: line: to string! copy/part conn/data d1
		;?? line
		info/headers: headers: construct/with d1 http-response-headers
		info/name: spec/ref
		if headers/content-length [info/size: headers/content-length: to integer! headers/content-length]
		if headers/last-modified [info/date: attempt [idate-to-date headers/last-modified]]
		remove/part conn/data d2
		state/state: 'reading-data
	]
	;? state
	;?? headers
	unless headers [
		read conn
		return false
	]
	res: false
	unless info/response-parsed [
		;?? line
		parse/all line [
			"HTTP/1." [#"0" | #"1"] some #" " [
				#"1" (info/response-parsed: 'info)
				|
				#"2" [["04" | "05"] (info/response-parsed: 'no-content)
					| (info/response-parsed: 'ok)
				]
				|
				#"3" [
					"03" (info/response-parsed: 'see-other)
					|
					"04" (info/response-parsed: 'not-modified)
					|
					"05" (info/response-parsed: 'use-proxy)
					| (info/response-parsed: 'redirect)
				]
				|
				#"4" [
					"01" (info/response-parsed: 'unauthorized)
					|
					"07" (info/response-parsed: 'proxy-auth)
					| (info/response-parsed: 'client-error)
				]
				|
				#"5" (info/response-parsed: 'server-error)
			]
			| (info/response-parsed: 'version-not-supported)
		]
	]
	;?? info/response-parsed
	;?? spec/method

	switch/all info/response-parsed [
		ok [
			either spec/method = 'head [
				state/state: 'ready
				res: awake make event! [type: 'done port: port]
				unless res [res: awake make event! [type: 'ready port: port]]
			] [
				res: check-data port
				;?? res
				;?? state/state
				if all [not res state/state = 'ready] [
					res: awake make event! [type: 'done port: port]
					unless res [res: awake make event! [type: 'ready port: port]]
				]
			]
		]
		redirect see-other [
			either spec/method = 'head [
				state/state: 'ready
				res: awake make event! [type: 'custom port: port code: 0]
			] [
				res: check-data port
				unless open? port [
					;NOTE some servers(e.g. yahoo.com) don't supply content-data in the redirect header so the state/state can be left in 'reading-data after check-data call
					;I think it is better to check if port has been closed here and set the state so redirect sequence can happen. --Richard
					state/state: 'ready
				]
			]
			;?? res
			;?? headers
			;?? state/state

			if all [not res state/state = 'ready] [
				either all [
					any [
						find [get head] spec/method
						all [
							info/response-parsed = 'see-other
							spec/method: 'get
						]
					]
					in headers 'Location
				] [
					return awake make event! [type: 'custom port: port code: 300]
				] [
					state/error: make-http-error "Redirect requires manual intervention"
					res: awake make event! [type: 'error port: port]
				]
			]
		]
		unauthorized client-error server-error proxy-auth [
			either spec/method = 'head [
				state/state: 'ready
			] [
				check-data port
			]
		]
		unauthorized [
			state/error: make-http-error "Authentication not supported yet"
			res: awake make event! [type: 'error port: port]
		]
		client-error server-error [
			state/error: make-http-error ["Server error: " line]
			res: awake make event! [type: 'error port: port]
		]
		not-modified [state/state: 'ready
			res: awake make event! [type: 'done port: port]
			unless res [res: awake make event! [type: 'ready port: port]]
		]
		use-proxy [
			state/state: 'ready
			state/error: make-http-error "Proxies not supported yet"
			res: awake make event! [type: 'error port: port]
		]
		proxy-auth [
			state/error: make-http-error "Authentication and proxies not supported yet"
			res: awake make event! [type: 'error port: port]
		]
		no-content [
			state/state: 'ready
			res: awake make event! [type: 'done port: port]
			unless res [res: awake make event! [type: 'ready port: port]]
		]
		info [
			info/headers: info/response-line: info/response-parsed: port/data: none
			state/state: 'reading-headers
			read conn
		]
		version-not-supported [
			state/error: make-http-error "HTTP response version not supported"
			res: awake make event! [type: 'error port: port]
			close port
		]
	]
	res
]
crlfbin: #{0D0A}
crlf2bin: #{0D0A0D0A}
crlf2: to string! crlf2bin
http-response-headers: context [
	Content-Length:
	Transfer-Encoding:
	Last-Modified: none
]
do-redirect: func [port [port!] new-uri [url! string! file!] /local spec state] [
	spec: port/spec
	state: port/state
	if #"/" = first new-uri [
		new-uri: to url! ajoin [spec/scheme "://" spec/host new-uri]
	]
	new-uri: decode-url new-uri

	unless select new-uri 'port-id [
		switch new-uri/scheme [
			'https [append new-uri [port-id: 443]]
			'http [append new-uri [port-id: 80]]
		]
	]
	new-uri: construct/with new-uri port/scheme/spec
	new-uri/method: spec/method
	new-uri/ref: to url! ajoin either find [#[none] 80 443] new-uri/port-id [
		[new-uri/scheme "://" new-uri/host new-uri/path]
	][	[new-uri/scheme "://" new-uri/host #":" new-uri/port-id new-uri/path]]

	;@@net-log ["[HTTP do-redirect] new-uri:" mold new-uri]
	;?? port

	unless find [http https] new-uri/scheme [
		state/error: make-http-error {Redirect to a protocol different from HTTP or HTTPS not supported}
		return state/awake make event! [type: 'error port: port]
	]

	;we need to reset tcp connection here before doing a redirect
	clear spec/headers
	port/data: none
	close port/state/connection
		
	port/spec: spec: new-uri
	port/state: none
	open port
]
check-data: func [port /local headers res data out chunk-size mk1 mk2 trailer state conn] [
	state: port/state
	headers: state/info/headers
	conn: state/connection
	res: false

	;@@net-log ["[HTTP check-data] bytes:" length? conn/data]
	;? conn

	case [
		headers/transfer-encoding = "chunked" [
			data: conn/data
			;clear the port data only at the beginning of the request --Richard
			unless port/data [port/data: make binary! length? data]
			out: port/data
			until [
				either parse/all data [
					copy chunk-size some hex-digits thru crlfbin mk1: to end
				] [
					chunk-size: to integer! to issue! to string! chunk-size
					either chunk-size = 0 [
						if parse/all mk1 [
							crlfbin (trailer: "") to end | copy trailer to crlf2bin to end
						] [
							trailer: construct trailer
							append headers body-of trailer
							state/state: 'ready
							res: state/awake make event! [type: 'custom port: port code: 0]
							clear data
						]
						true
					] [
						either parse/all mk1 [
							chunk-size skip mk2: crlfbin to end
						] [
							insert/part tail out mk1 mk2
							remove/part data skip mk2 2
							empty? data
						] [
							true
						]
					]
				] [
					true
				]
			]
			unless state/state = 'ready [
				;Awake from the WAIT loop to prevent timeout when reading big data. --Richard
				res: true
			]
		]
		integer? headers/content-length [
			port/data: conn/data
			either headers/content-length <= length? port/data [
				state/state: 'ready
				conn/data: make binary! 32000 ;@@ Oldes: why not just none?
				res: state/awake make event! [type: 'custom port: port code: 0]
			] [
				;Awake from the WAIT loop to prevent timeout when reading big data. --Richard
				res: true
			]
		]
		true [
			port/data: conn/data
			either state/info/response-parsed = 'ok [
				;Awake from the WAIT loop to prevent timeout when reading big data. --Richard
				res: true
			][
				;On other response than OK read all data asynchronously (assuming the data are small). --Richard
				read conn
			]
		]
	]
	res
]
hex-digits: charset "1234567890abcdefABCDEF"
sys/make-scheme [
	name: 'http
	title: "HyperText Transport Protocol v1.1"
	spec: make system/standard/port-spec-net [
		path: %/
		method: 'get
		headers: []
		content: none
		timeout: 15
	]
	info: make system/standard/file-info [
		response-line:
		response-parsed:
		headers: none
	]
	actor: [
		read: func [
			port [port!]
		] [
			;@@net-log "[HTTP read]"
			either any-function? :port/awake [
				unless open? port [cause-error 'Access 'not-open port/spec/ref]
				if port/state/state <> 'ready [http-error "Port not ready"]
				port/state/awake: :port/awake
				do-request port
				port
			] [
				sync-op port []
			]
		]
		write: func [
			port [port!]
			value
		] [
			;@@net-log "[HTTP write]"
			;?? port
			unless any [block? :value binary? :value any-string? :value] [value: form :value]
			unless block? value [value: reduce [[Content-Type: "application/x-www-form-urlencoded; charset=utf-8"] value]]

			either any-function? :port/awake [
				unless open? port [cause-error 'Access 'not-open port/spec/ref]
				if port/state/state <> 'ready [http-error "Port not ready"]
				port/state/awake: :port/awake
				parse-write-dialect port value
				do-request port
				port
			] [
				sync-op port [parse-write-dialect port value]
			]
		]
		open: func [
			port [port!]
			/local conn
		] [
			;@@net-log ["[HTTP open] state:" port/state]
			if port/state [return port]
			if none? port/spec/host [http-error "Missing host address"]
			port/state: context [
				state: 'inited
				connection:
				error: none
				close?: no
				info: make port/scheme/info [type: 'url]
				awake: :port/awake
			]
			;? port/state/info
			port/state/connection: conn: make port! compose [
				scheme: (to lit-word! either port/spec/scheme = 'http ['tcp]['tls])
				host: port/spec/host
				port-id: port/spec/port-id
				ref: to url! ajoin [scheme "://" host #":" port-id]
			]
			;?? conn 
			conn/awake: :http-awake
			conn/locals: port
			;@@net-log ["[HTTP open] scheme:" conn/spec/scheme conn/spec/host]
			;?? conn
			open conn
			port
		]
		open?: func [
			port [port!]
		] [
			found? all [port/state open? port/state/connection]
		]
		close: func [
			port [port!]
		] [
			;@@net-log "[HTTP close]"
			if port/state [
				close port/state/connection
				port/state/connection/awake: none
				port/state: none
			]
			port
		]
		copy: func [
			port [port!]
		] [
			either all [port/spec/method = 'head port/state] [
				reduce bind [name size date] port/state/info
			] [
				if port/data [copy port/data]
			]
		]
		query: func [
			port [port!]
			/mode
			field [word! none!]
			/local error state
		] [
			if all [mode none? field] [ return words-of system/standard/file-info]
			if none? state: port/state [
				open port ;there is port opening in sync-op, but it would also close the port later and so clear the state
				attempt [sync-op port [parse-write-dialect port [HEAD]]]
				state: port/state
				close port
			]
			;?? state
			either all [
				state
				state/info/response-parsed
			][
				either field [
					select state/info field
				][	state/info ]
			][	none ]
		]
		length?: func [
			port [port!]
		] [
			either port/data [length? port/data] [0]
		]
	]
]

sys/make-scheme/with [
	name: 'https
	title: "Secure HyperText Transport Protocol v1.1"
	spec: make spec [
		port-id: 443
	]
] 'http
