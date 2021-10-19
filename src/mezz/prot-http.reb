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
	Version: 0.3.5
	Date: 26-Oct-2020
	File: %prot-http.r
	Purpose: {
		This program defines the HTTP protocol scheme for REBOL 3.
	}
	Author: ["Gabriele Santilli" "Richard Smolak" "Oldes"]
	History: [
		0.1.1 22-Jun-2007 "Gabriele Santilli" "Version used in R3-Alpha"
		0.1.4 26-Nov-2012 "Richard Smolak"    "Version from Atronix's fork"
		0.1.5 10-May-2018 "Oldes" "FIX: Query on URL was returning just none"
		0.1.6 21-May-2018 "Oldes" "FEAT: Added support for basic redirection"
		0.1.7 03-Dec-2018 "Oldes" "FEAT: Added support for QUERY/MODE action"
		0.1.8 21-Mar-2019 "Oldes" "FEAT: Using system trace outputs"
		0.1.9 21-Mar-2019 "Oldes" "FEAT: Added support for transfer compression"
		0.2.0 28-Mar-2019 "Oldes" "FIX: close connection in case of errors"
		0.2.1 02-Apr-2019 "Oldes" "FEAT: Reusing connection in redirect when possible"
		0.3.0 06-Jul-2019 "Oldes" "FIX: Error handling revisited and improved dealing with chunked data"
		0.3.1 13-Feb-2020 "Oldes" "FEAT: Possible auto conversion to text if found charset specification in content-type"
		0.3.2 25-Feb-2020 "Oldes" "FIX: Properly handling chunked data"
		0.3.3 25-Feb-2020 "Oldes" "FEAT: support for read/binary and write/binary to force raw data result"
		0.3.4 26-Feb-2020 "Oldes" "FIX: limit input data according Content-Length (#issues/2386)"
		0.3.5 26-Oct-2020 "Oldes" "FEAT: support for read/part (using Range request with read/part/binary)"
	]
]

sync-op: func [port body /local header state][
	unless port/state [open port port/state/close?: yes]
	state: port/state
	state/awake: :read-sync-awake
	do body
	if state/state = 'ready [do-request port]
	;NOTE: We'll wait in a WHILE loop so the timeout cannot occur during 'reading-data state.
	;The timeout should be triggered only when the response from other side exceeds the timeout value.
	;--Richard
	while [not find [ready close] state/state][
		;print "HTTP sync-op loop"
		unless port? wait [state/connection port/spec/timeout][
			throw-http-error port make error! [
				type: 'Access
				id:   'no-connect
				arg1:  port/spec/ref
				arg2: 'timeout
			]
			exit
		]
		switch state/state [
			inited [
				if not open? state/connection [
					throw-http-error port ["Internal " state/connection/spec/ref " connection closed"]
					exit
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
	if state/error [
		throw-http-error port state/error
		exit
	]

	body: copy port

	sys/log/info 'HTTP ["Done reading:^[[22m" length? body "bytes"]

	header: copy port/state/info/headers

	if state/close? [
		sys/log/more 'HTTP ["Closing port for:^[[m" port/spec/ref]
		close port
	]

	reduce [header body]
]

read-sync-awake: func [event [event!] /local error][
	sys/log/debug 'HTTP ["Read-sync-awake:" event/type]
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
			if event/code = 300 [
				event/port/state/state: 'redirect
				return true
			]
			false
		]
		error [
			if all [
				event/port/state
				event/port/state/state <> 'closing
			][
				sys/log/debug 'HTTP ["Closing (sync-awake):^[[1m" event/port/spec/ref]
				close event/port
			]
			if error? event/port/state [do event/port/state]
			true
		]
	][
		false
	]
]
http-awake: func [event /local port http-port state awake res][
	port: event/port
	http-port: port/locals
	state: http-port/state
	if any-function? :http-port/awake [state/awake: :http-port/awake]
	awake: :state/awake

	sys/log/debug 'HTTP ["Awake:^[[1m" event/type "^[[22mstate:^[[1m" state/state]

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
		lookup [
			open port false]
		connect [
			state/state: 'ready
			awake make event! [type: 'connect port: http-port]
		]
		close
		error [
			;?? state/state
			res: switch state/state [
				ready [
					awake make event! [type: 'close port: http-port]
				]
				inited [
					throw-http-error http-port any [
						http-port/state/error
						all [object? state/connection/state state/connection/state/error]
						make error! [
							type: 'Access
							id:   'no-connect
							arg1: http-port/spec/ref
						]
					]
				]
				doing-request reading-headers [
					throw-http-error http-port any [
						all [object? state/connection/state state/connection/state/error]
						"Server closed connection"
					]
				]
				reading-data [
					either any [
						integer? state/info/headers/content-length
						state/info/headers/transfer-encoding = "chunked"
					][
						throw-http-error http-port "Server closed connection"
					][
						;set state to CLOSE so the WAIT loop in 'sync-op can be interrupted --Richard
						state/state: 'close
						any [
							awake make event! [type: 'done  port: http-port]
							awake make event! [type: 'close port: http-port]
						]
					]
				]
			]
			try [
				; check if there is some error from inner (connection) layer
				state/error: state/connection/state/error
			]
			sys/log/debug 'HTTP ["Closing:^[[1m" http-port/spec/ref]
			close http-port
			if error? state [ do state ]
			res
		]
	][true]
]

throw-http-error: func [
	http-port  [port!]
	error [error! string! block!]
][
	sys/log/debug 'HTTP ["Throwing error:^[[m" error]
	unless error? error [
		error: make error! [
			type: 'Access
			id:   'Protocol
			arg1: either block? error [ajoin error][error]
		]
	]
	either object? http-port/state [
		http-port/state/error: error
		http-port/state/awake make event! [type: 'error port: http-port]
	][  do error ]
]

make-http-request: func [
	"Create an HTTP request (returns string!)"
	method [word! string!] "E.g. GET, HEAD, POST etc."
	target [file! string!] {In case of string!, no escaping is performed (eg. useful to override escaping etc.). Careful!}
	headers [block! map!] "Request headers (set-word! string! pairs)"
	content [any-string! binary! map! none!] {Request contents (Content-Length is created automatically). Empty string not exactly like none.}
	/local result
][
	result: rejoin [
		uppercase form method #" "
		either file? target [next mold target][target]
		" HTTP/1.1" CRLF
	]
	foreach [word string] headers [
		repend result [mold word #" " string CRLF]
	]
	if content [
		if map? content [content: to-json content]
		content: to binary! content
		repend result ["Content-Length: " length? content CRLF]
	]
	sys/log/info 'HTTP ["Request:^[[22m" mold result]

	append result CRLF
	result: to binary! result
	if content [append result content]
	result
]
do-request: func [
	"Perform an HTTP request"
	port [port!]
	/local spec info
][
	spec: port/spec
	info: port/state/info
	spec/headers: body-of make make object! [
		Accept: "*/*"
		Accept-charset: "utf-8"
		Accept-Encoding: "gzip,deflate"
		Host: either not find [80 443] spec/port-id [
			ajoin [spec/host #":" spec/port-id]
		][
			form spec/host
		]
		User-Agent: any [system/schemes/http/User-Agent "REBOL"]
	] to block! spec/headers
	port/state/state: 'doing-request
	info/headers: info/response-line: info/response-parsed: port/data:
	info/size: info/date: info/name: none

	;sys/log/info 'HTTP ["Request:^[[22m" spec/method spec/host mold spec/path]

	;write port/state/connection make-http-request spec/method enhex as file! any [spec/path %/] spec/headers spec/content
	write port/state/connection make-http-request spec/method any [spec/path %/] spec/headers spec/content
]
parse-write-dialect: func [port block /local spec][
	spec: port/spec
	parse block [
		[set block word! (spec/method: block) | (spec/method: 'POST)]
		opt [set block [file! | url!] (spec/path: block)]
		[set block [block! | map!] (spec/headers: block) | (spec/headers: [])]
		[set block [any-string! | binary! | map!] (spec/content: block) | (spec/content: none)]
	]
]
check-response: func [port /local conn res headers d1 d2 line info state awake spec][
	state: port/state
	conn: state/connection
	info: state/info
	headers: info/headers
	line: info/response-line
	awake: :state/awake
	spec: port/spec
	
	if all [
		not headers
		any [
			all [
				d1: find conn/data crlfbin
				d2: find/tail d1 crlf2bin
				;sys/log/debug 'HTML "server using standard content separator of #{0D0A0D0A}"
			]
			all [
				d1: find conn/data #{0A}
				d2: find/tail d1 #{0A0A}
				sys/log/debug 'HTML "Server using malformed line separator of #{0A0A}"
			]
		]
	][
		info/response-line: line: to string! copy/part conn/data d1
		sys/log/more 'HTTP line
		;probe to-string copy/part d1 d2
		info/headers: headers: construct/with d1 http-response-headers
		sys/log/info 'HTTP ["Headers:^[[22m" mold body-of headers]
		info/name: spec/ref
		if state/error: try [
			; make sure that values bellow are valid
			if headers/content-length [info/size: headers/content-length: to integer! headers/content-length]
			none ; no error
		][
			awake make event! [type: 'error port: port]
		]
		; allow invalid date, but ignore it on error
		try [if headers/last-modified  [info/date: to-date/utc headers/last-modified]]
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

	sys/log/debug 'HTTP ["Check-response:" info/response-parsed]

	;?? info/response-parsed
	;?? spec/method

	switch/all info/response-parsed [
		ok [
			either spec/method = 'HEAD [
				state/state: 'ready
				res: awake make event! [type: 'done port: port]
				unless res [res: awake make event! [type: 'ready port: port]]
			][
				res: check-data port
				;?? res
				;?? state/state
				if all [not res state/state = 'ready][
					res: awake make event! [type: 'done port: port]
					unless res [res: awake make event! [type: 'ready port: port]]
				]
			]
		]
		redirect see-other [
			either spec/method = 'HEAD [
				state/state: 'ready
				res: awake make event! [type: 'custom port: port code: 0]
			][
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

			if all [not res state/state = 'ready][
				either all [
					any [
						find [get head] spec/method
						all [
							info/response-parsed = 'see-other
							spec/method: 'GET
						]
					]
					in headers 'Location
				][
					return awake make event! [type: 'custom port: port code: 300]
				][
					res: throw-http-error port "Redirect requires manual intervention"
				]
			]
		]
		unauthorized client-error server-error proxy-auth [
			either spec/method = 'HEAD [
				state/state: 'ready
			][
				check-data port
			]
		]
		unauthorized [
			res: throw-http-error port "Authentication not supported yet"
		]
		client-error server-error [
			res: throw-http-error port reduce ["Server error: " line]
		]
		not-modified [state/state: 'ready
			res: awake make event! [type: 'done port: port]
			unless res [res: awake make event! [type: 'ready port: port]]
		]
		use-proxy [
			state/state: 'ready
			res: throw-http-error port "Proxies not supported yet"
		]
		proxy-auth [
			res: throw-http-error port "Authentication and proxies not supported yet"
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
			res: throw-http-error port "HTTP response version not supported"
			close port
		]
	]
	res
]
crlfbin:  #{0D0A}
crlf2bin: #{0D0A0D0A}
crlf2: to string! crlf2bin
http-response-headers: construct [
	Content-Length:
	Content-Encoding:
	Transfer-Encoding:
	Last-Modified:
]

do-redirect: func [port [port!] new-uri [url! string! file!] /local spec state headers][
	spec: port/spec
	state: port/state
	port/data: none

	;new-uri: as url! new-uri

	sys/log/info 'HTTP ["Redirect to:^[[m" mold new-uri]

	state/redirects: state/redirects + 1
	if state/redirects > 10 [
		res: throw-http-error port {Too many redirections}
	]

	if #"/" = first new-uri [
		; if it's redirection under same url, we can reuse the opened connection
		if "keep-alive" = select state/info/headers 'Connection [
			spec/path: new-uri
			do-request port
			return true
		]
		new-uri: as url! ajoin [spec/scheme "://" spec/host #":" spec/port-id new-uri]
	]
	new-uri: decode-url new-uri
	spec/headers/host: new-uri/host

	unless select new-uri 'port-id [
		switch new-uri/scheme [
			'https [append new-uri [port-id: 443]]
			'http  [append new-uri [port-id: 80 ]]
		]
	]
	new-uri: construct/with new-uri port/scheme/spec
	new-uri/method: spec/method
	new-uri/ref: as url! ajoin either find [#[none] 80 443] new-uri/port-id [
		[new-uri/scheme "://" new-uri/host new-uri/path]
	][	[new-uri/scheme "://" new-uri/host #":" new-uri/port-id new-uri/path]]

	unless find [http https] new-uri/scheme [
		return throw-http-error port {Redirect to a protocol different from HTTP or HTTPS not supported}
	]

	; store original request headers
	headers: spec/headers
	; we need to reset tcp connection here before doing a redirect
	close port/state/connection
	port/spec: spec: new-uri
	port/state: none
	open port
	; restore original request headers
	port/spec/headers: headers
	port
]

check-data: func [port /local headers res data available out chunk-size pos trailer state conn][
	state: port/state
	headers: state/info/headers
	conn: state/connection
	res: false

	sys/log/more 'HTTP ["Check-data; bytes:^[[m" length? conn/data]

	case [
		headers/transfer-encoding = "chunked" [
			data: conn/data ;- data from lower layer (TLS or TCP)
			available: length? data

			sys/log/more 'HTTP ["Chunked data: " state/chunk-size "av:" available]

			unless port/data [ port/data: make binary! 32000 ]
			out: port/data 

			if state/chunk-size [
				;- rests from previous unfinished chunk
				either state/chunk-size <= available [
					; we have enough data to end the chunk
					append out take/part data state/chunk-size
					state/chunk-size: none
					if crlfbin <> take/part data 2 [
						throw-http-error port "Missing CRLF after chunk end!"
					]
				][
					append out take/part data available
					state/chunk-size: state/chunk-size - available
				]
			]
			if not empty? data [
				until [
					either parse/all data [
						copy chunk-size some hex-digits
						crlfbin pos: to end
					][
						;- ugly conversion of the chunk size from hexadecimal string to integer
						chunk-size: to integer! to issue! to string! :chunk-size
						remove/part data pos
						available: length? data
						sys/log/more 'HTTP ["Chunk-size:^[[m" chunk-size " ^[[36mavailable:^[[m " available]
						either chunk-size = 0 [
							if parse/all data [
								crlfbin (trailer: "") to end | copy trailer to crlf2bin to end
							][
								trailer: construct trailer
								append headers body-of trailer
								state/state: 'ready
								res: state/awake make event! [type: 'custom port: port code: 0]
								clear head conn/data
							]
							true ; end of loop
						][
							available: length? data
							either chunk-size <= available [
								append out take/part data :chunk-size
								if crlfbin <> take/part data 2 [
									throw-http-error port "Missing CRLF after chunk end!???"
								]
								empty? data
							][
								;print "unfinished chunk.."
								state/chunk-size: chunk-size - available
								append out take/part data available
								true ; end of loop
							]
						]
					][
						throw-http-error port "Invalid chunk data!"
						true ; end of loop
					] 
				]
			]
			unless state/state = 'ready [
				;Awake from the WAIT loop to prevent timeout when reading big data. --Richard
				res: true
			]
		]
		integer? headers/content-length [
			either headers/content-length <= length? conn/data [
				state/state: 'ready
				port/data: copy/part conn/data headers/content-length
				conn/data: none
				res: state/awake make event! [type: 'custom port: port code: 0]
			][
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

decode-result: func[
	result [block!] {[header body]}
	/local body content-type code-page encoding
][
	if encoding: select result/1 'Content-Encoding [
		either find ["gzip" "deflate"] encoding [
			try/except [
				result/2: switch encoding [
					"gzip"    [ decompress result/2 'gzip]
					"deflate" [ decompress result/2 'deflate]
				]
			][
				sys/log/info 'HTTP ["Failed to decode data using:^[[22m" encoding]
				return result
			]
			sys/log/info 'HTTP ["Extracted using:^[[22m" encoding "^[[1mto:^[[22m" length? result/2 "bytes"]
		][
			sys/log/info 'HTTP ["Unknown Content-Encoding:^[[m" encoding]
		]
	]
	if all [
		content-type: select result/1 'Content-Type
		any [
			; consider content to be a text if charset specification is included
			parse content-type [to #";" thru "charset=" copy code-page to end]
			; or when it is without charset, but of type text/*
			parse content-type [["text/" | "application/json"] to end]
		]
	][
		unless code-page [code-page: "utf-8"]
		sys/log/info 'HTTP ["Trying to decode from code-page:^[[m" code-page]
		; using also deline to normalize possible CRLF to LF
		try [result/2: deline iconv result/2 code-page]
	]
	result
]

hex-digits: system/catalog/bitsets/hex-digits
sys/make-scheme [
	name: 'http
	title: "HyperText Transport Protocol v1.1"
	spec: make system/standard/port-spec-net [
		path: %/
		method: 'GET
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
			/binary
			/part length [number!]
			/local result
		][
			sys/log/debug 'HTTP "READ"
			unless port/state [open port port/state/close?: yes]
			if all [part binary length > 0] [
				append port/spec/headers compose [Range: (join "bytes=0-" (to integer! length) - 1)]
			]
			either any-function? :port/awake [
				unless open? port [cause-error 'Access 'not-open port/spec/ref]
				if port/state/state <> 'ready [throw-http-error "Port not ready"]
				port/state/awake: :port/awake
				do-request port
			][
				result: sync-op port []
				unless binary [decode-result result]
				if all [part result/2] [ clear skip result/2 length ]
				result/2
			]
		]
		write: func [
			port [port!]
			value
			/binary
			/local result
		][
			sys/log/debug 'HTTP "WRITE"
			;?? port
			case [
				binary? value [
					value: reduce [[Content-Type: "application/octet-stream"] value]
				]
				map? value [
					value: reduce [[Content-Type: "application/json; charset=utf-8"] encode 'JSON value]
				]
				not block? value [
					value: reduce [[Content-Type: "application/x-www-form-urlencoded; charset=utf-8"] form value]
				]
			]

			either any-function? :port/awake [
				unless open? port [cause-error 'Access 'not-open port/spec/ref]
				if port/state/state <> 'ready [throw-http-error "Port not ready"]
				port/state/awake: :port/awake
				parse-write-dialect port value
				do-request port
			][
				result: sync-op port [parse-write-dialect port value]
				unless binary [decode-result result]
				result/2
			]
		]
		open: func [
			port [port!]
			/local conn
		][
			sys/log/debug 'HTTP ["OPEN, state:" port/state]
			if port/state [return port]
			if none? port/spec/host [throw-http-error port "Missing host address"]
			port/state: object [
				state: 'inited
				connection:
				error: none
				close?: no
				binary?: no
				info: make port/scheme/info [type: 'url]
				awake: :port/awake
				redirects: 0
				chunk: none
				chunk-size: none
			]
			;? port/state/info
			port/state/connection: conn: make port! compose [
				scheme: (to lit-word! either port/spec/scheme = 'http ['tcp]['tls])
				host: port/spec/host
				port-id: port/spec/port-id
				ref: as url! ajoin [scheme "://" host #":" port-id]
			]
			
			conn/awake: :http-awake
			conn/locals: port
			sys/log/info 'HTTP ["Opening connection:^[[22m" conn/spec/ref]
			open conn

			port
		]
		open?: func [
			port [port!]
		][
			all [object? port/state  open? port/state/connection  true]
		]
		close: func [
			port [port!]
		][
			sys/log/debug 'HTTP "CLOSE"
			if object? port/state [
				port/state/state: 'closing
				close port/state/connection
				port/state/connection/awake: none
				; release state and if there was error, keep it there
				port/state: port/state/error
			]
			if error? port/state [do port/state]
			port
		]
		copy: func [
			port [port!]
		][
			either all [port/spec/method = 'HEAD port/state][
				reduce bind [name size date] port/state/info
			][
				if port/data [copy port/data]
			]
		]
		query: func [
			port [port!]
			/mode
			field [word! block! none!]
			/local error state result
		][
			if all [mode none? field][ return words-of system/schemes/http/info]
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
					either word? field [
						select state/info field
					][
						result: make block! length? field
						foreach word field [
							if any-word? word [
								if set-word? word [ append result word ]
								append result state/info/(to word! word)
							]
						]
						result
					]
				][	state/info ]
			][	none ]
		]
		length?: func [
			port [port!]
		][
			either port/data [length? port/data][0]
		]
	]
	User-Agent: none
	;@@ One can set above value for example to: "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.103 Safari/537.36"
	;@@ And so pretend that request is coming from Chrome on Windows10
]

sys/make-scheme/with [
	name: 'https
	title: "Secure HyperText Transport Protocol v1.1"
	spec: make spec [
		port-id: 443
	]
] 'http
