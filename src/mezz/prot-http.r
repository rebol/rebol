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
	Version: 0.1.0
	File: %prot-http.r
	Purpose: {
		This program defines the HTTP protocol scheme for REBOL 3.
	}
	Author: "Gabriele Santilli"
	Date: 22-Jun-2007
]

sync-op: func [port body /local state] [
	unless port/state [open port port/state/close?: yes]
	state: port/state
	state/awake: :read-sync-awake
	do body
	if state/state = 'ready [do-request port]
	unless port? wait [state/connection port/spec/timeout] [http-error "Timeout"]
	body: copy port
	if state/close? [close port]
	body
]
read-sync-awake: func [event [event!] /local error] [
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
		Host: either spec/port-id <> 80 [
			rejoin [form spec/host #":" spec/port-id]
		] [
			form spec/host
		]
		User-Agent: "REBOL"
	] spec/headers
	port/state/state: 'doing-request
	info/headers: info/response-line: info/response-parsed: port/data:
	info/size: info/date: info/name: none
	write port/state/connection
	make-http-request spec/method to file! any [spec/path %/]
	spec/headers spec/content
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
	if all [
		not headers
		d1: find conn/data crlfbin
		d2: find/tail d1 crlf2bin
	] [
		info/response-line: line: to string! copy/part conn/data d1
		info/headers: headers: construct/with d1 http-response-headers
		info/name: to file! any [spec/path %/]
		if headers/content-length [info/size: headers/content-length: to integer! headers/content-length]
		if headers/last-modified [info/date: attempt [to date! headers/last-modified]]
		remove/part conn/data d2
		state/state: 'reading-data
	]
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
	switch/all info/response-parsed [
		ok [
			either spec/method = 'head [
				state/state: 'ready
				res: awake make event! [type: 'done port: port]
				unless res [res: awake make event! [type: 'ready port: port]]
			] [
				res: check-data port
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
			]
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
					res: do-redirect port headers/location
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
	new-uri: construct/with decode-url new-uri port/scheme/spec
	if new-uri/scheme <> 'http [
		state/error: make-http-error {Redirect to a protocol different from HTTP not supported}
		return state/awake make event! [type: 'error port: port]
	]
	either all [
		new-uri/host = spec/host
		new-uri/port-id = spec/port-id
	] [
		spec/path: new-uri/path
		do-request port
		false
	] [
		state/error: make-http-error "Redirect to other host - requires custom handling"
		state/awake make event! [type: 'error port: port]
	]
]
check-data: func [port /local headers res data out chunk-size mk1 mk2 trailer state conn] [
	state: port/state
	headers: state/info/headers
	conn: state/connection
	res: false
	case [
		headers/transfer-encoding = "chunked" [
			data: conn/data
			out: port/data: make binary! length? data
			until [
				either parse/all data [
					copy chunk-size some hex-digits thru crlfbin mk1: to end
				] [
					chunk-size: to integer! to issue! chunk-size
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
			unless state/state = 'ready [read conn]
		]
		integer? headers/content-length [
			port/data: conn/data
			either headers/content-length <= length? port/data [
				state/state: 'ready
				conn/data: make binary! 32000
				res: state/awake make event! [type: 'custom port: port code: 0]
			] [
				read conn
			]
		]
		true [
			port/data: conn/data
			read conn
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
			if port/state [return port]
			if none? port/spec/host [http-error "Missing host address"]
			port/state: context [
				state: 'inited
				connection:
				error: none
				close?: no
				info: make port/scheme/info [type: 'file]
				awake: :port/awake
			]
			port/state/connection: conn: make port! [
				scheme: 'tcp
				host: port/spec/host
				port-id: port/spec/port-id
				ref: rejoin [tcp:// host ":" port-id]
			]
			conn/awake: :http-awake
			conn/locals: port
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
			/local error state
		] [
			if state: port/state [
				either error? error: state/error [
					state/error: none
					error
				] [
					state/info
				]
			]
		]
		length?: func [
			port [port!]
		] [
			either port/data [length? port/data] [0]
		]
	]
]
