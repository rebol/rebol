REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Rebol3 HTTP protocol scheme"
	Name: http
	Type: module
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2026 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Version: 0.8.4
	Needs: 3.18.5 ;; because using the new log-* functions
	Date: 27-May-2026
	File: %prot-http.r3
	Purpose: {
		This program defines the HTTP protocol scheme for REBOL 3.
	}
	Author: ["Gabriele Santilli" "Richard Smolak" "Oldes"]
	;;History: [
	;;	0.1.1 22-Jun-2007 "Gabriele Santilli" "Version used in R3-Alpha"
	;;	0.1.4 26-Nov-2012 "Richard Smolak"    "Version from Atronix's fork"
	;;  ...
	;;  https://github.com/Oldes/Rebol3/commits/master/src/mezz/prot-http.reb
	;;]
	exports: [set-cookies get-cookies]
]

sync-op: func [port body /local header state][
	unless port/state [open port port/state/close?: yes]
	state: port/state
	state/awake: :read-sync-awake
	;print ["sync-op" mold/flat body]
	do body
	;? state/state
	if state/state = 'ready [do-request port]
	;NOTE: We'll wait in a WHILE loop so the timeout cannot occur during 'reading-data state.
	;The timeout should be triggered only when the response from other side exceeds the timeout value.
	;--Richard
	while [not find [ready close] state/state][
		;print ["HTTP sync-op loop.. state:" state/state "open?" open? state/connection]
		if all [state/state = 'closing not open? state/connection][
			; server already closed connection
			state/state: 'ready
			break
		]
		unless port? wait [state/connection port/spec/timeout][
			throw-http-error port make error! [
				type: 'Access
				id:   'no-connect
				arg1:  port/spec/ref
				arg2: 'timeout
			]
			exit
		]
		
		if all [
			state/info/status-code >= 300
			state/info/status-code < 400
			find port/state/info/headers 'location
		][
			state/state: 'redirect
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
				either port/spec/redirect? [
					do-redirect port port/state/info/headers/location
					state: port/state
					state/awake: :read-sync-awake
				][	state/state: 'ready ]
			]
		]
	]
	if state/error [
		throw-http-error port state/error
		exit
	]

	body: copy port

	log-info 'HTTP ["Done reading:^[[22m" length? body "bytes"]

	header: copy port/state/info/headers

	if all [state/close? open? port][
		log-debug 'HTTP ["Closing port for:^[[m" port/spec/ref]
		close port
	]

	reduce [port/state/info/status-code header body]
]

read-sync-awake: func [event [event!] /local error state][
	log-trace 'HTTP ["Read-sync-awake:" event/type]
	state: event/port/state
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
				state/state: 'redirect
				return true
			]
			false
		]
		error [
			if all [
				state
				state/state <> 'closing
			][
				log-trace 'HTTP ["Closing (sync-awake):^[[1m" event/port/spec/ref]
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
	http-port: port/parent

	state: http-port/state
	if any-function? :http-port/awake [state/awake: :http-port/awake]
	awake: :state/awake

	;? awake

	log-trace 'HTTP ["Awake:^[[1m" event/type "^[[22mstate:^[[1m" state/state]

	res: switch/default event/type [
		read [
			awake make event! [type: 'read port: http-port]
			check-response http-port ;@@ really check response on every read event?!
		]
		wrote [
			awake make event! [type: 'wrote port: http-port]
			state/state: 'reading-headers
			read port
			false
		]
		lookup [
			open port false
		]
		connect [
			state/state: 'ready
			awake make event! [type: 'connect port: http-port]
		]
		close
		error [
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
						state/state: 'ready
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
			log-trace 'HTTP ["Closing:^[[1m" http-port/spec/ref]
			close http-port
			if error? state [ do state ]
			res
		]
	][true]
	;print ["http-awake res:" mold res]
	res
]

throw-http-error: func [
	http-port  [port!]
	error [error! string! block!]
][
	log-trace 'HTTP ["Throwing error:^[[m" error]
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

escape-query: function/with [
;;	"Escapes all chars which are not allowed in the HTTP query part (if not yet escaped)"
	query [any-string!]
][
	parse query [some [
		some allowed
		| #"%" 2 hex-digits ;; already escaped
		| change #" " #"+" 
		| change set c: skip (
			c: enbase to binary! c 16
			while [not tail? c][c: skip insert c #"%" 2]
			head c
		)
	]]
	query
][
	allowed: charset [#"a"-#"z" #"A"-#"Z" #"0"-#"9" "-~!@*/|\;,._()[]{}+=?~&"]
]

make-http-request: func [
	"Create an HTTP request (returns binary!)"
	spec [block! object!] "Request specification from an opened port"
	/local method path target query headers content request cookies-to-send
][
	method:  any [select spec 'method 'GET]
	path:    any [select spec 'path    %/]
	target:       select spec 'target
	query:        select spec 'query
	headers: any [select spec 'headers []]
	content:      select spec 'content

	request: ajoin [
		uppercase form :method SP
		enhex/uri :path
	]
	if :target [append request enhex/uri :target]
	if :query  [append append request #"?" escape-query :query]

	if cookies-to-send: get-cookies spec/host path [
		put headers 'Cookie cookies-to-send
	]

	append request " HTTP/1.1^M^/"
	
	foreach [word string] :headers [
		append request ajoin [form :word #":" SP :string CRLF]
	]

	if :content [
		if map? :content [
			content: to-json content
			unless find headers 'Content-Type [
				append request "Content-Type: application/json^M^/" 
			]
		]
		content: to binary! :content
		append request ajoin [
			"Content-Length: " length? content CRLF
		]
	]
	log-info 'HTTP ["Request:^[[22m" anonymize mold request]

	append request CRLF
	request: to binary! request
	if content [append request content]
	request
]

do-request: func [
	"Perform an HTTP request"
	port [port!]
	/local spec info
][
	spec: port/spec
	info: port/state/info
	spec/headers: make system/schemes/http/headers to block! spec/headers

	;; Use 'identity' encoding in HEAD request (otherwise, the content-length may be none).
	if spec/method == 'HEAD [spec/headers/Accept-Encoding: "identity"]

	unless spec/headers/host [
		spec/headers/host: either find [80 443] spec/port [
			; default http/https scheme port ids
			form spec/host
		][	; custom port id
			ajoin [spec/host #":" spec/port]
		]
	]
	port/state/state: 'doing-request
	info/headers: info/response-line: info/status-code: port/data:
	info/size: info/modified: info/name: none

	write port/state/connection make-http-request :spec
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

put system/catalog 'http-status-codes http-status-codes: #[
	200 "OK"
	201 "Created"
	202 "Accepted"
	203 "Non-Authoritative Information"
	204 "No Connect"
	205 "Reset"
	206 "Partial"

	300 "Multiple Choices"
	301 "Moved Permanently"
	302 "Found"
	303 "See Other"
	304 "Not Modified"
	305 "Use Proxy"
	308 "Moved Permanently"

	400 "Bad Request"
	401 "Unauthorized"
	402 "Payment Required"
	403 "Forbidden"
	404 "Not Found"
	405 "Not Allowed"
	406 "Not Acceptable"
	407 "Proxy Authentication Required"
	408 "Request Timeout"
	409 "Conflict"
	410 "Gone"
	411 "Length Required"
	412 "Precondition Failed"
	413 "Payload Too Large"
	414 "URI Too Long"
	415 "Unsupported Media Type"
	416 "Range Not Satisfiable"
	417 "Expectation Failed"
	418 "I am a teapot"
	426 "Upgrade Required"
	428 "Precondition Required"
	429 "Too Many Requests"
	431 "Request Header Fields Too Large"
	451 "Unavailable For Legal Reasons"

	500 "Internal Server Error"
	501 "Not Implemented"
	502 "Bad Gateway"
	503 "Service Unavailable"
	504 "Gateway Timeout"
	505 "HTTP Version Not Supported"
	506 "Variant Also Negotiates"
	507 "Insufficient Storage"
	508 "Loop Detected "
	510 "Not Extended"
	511 "Network Authentication Required"
]
check-response: func [port /local conn res headers d1 d2 line info state awake spec date code cookies][
	state:   port/state
	spec:    port/spec
	conn:    state/connection
	info:    state/info
	headers: info/headers
	line:    info/response-line
	awake:  :state/awake
	
	if all [
		not headers
		any [
			all [
				d1: find conn/data crlfbin
				d2: find/tail d1 crlf2bin
				;log-trace 'HTML "server using standard content separator of #{0D0A0D0A}"
			]
			all [
				d1: find conn/data #{0A}
				d2: find/tail d1 #{0A0A}
				log-trace 'HTML "Server using malformed line separator of #{0A0A}"
			]
		]
	][
		info/response-line: line: to string! copy/part conn/data d1
		log-info 'HTTP line
		;probe to-string copy/part d1 d2
		info/headers: headers: construct/with d1 http-response-headers
		log-info 'HTTP ["Headers:^[[22m" mold body-of headers]
		info/name: spec/ref
		if state/error: try [
			; make sure that values bellow are valid
			if headers/content-length [info/size: headers/content-length: to integer! headers/content-length]
			none ; no error
		][
			awake make event! [type: 'error port: port]
		]
		if cookies: select headers 'set-cookie [
			set-cookies port/spec/host cookies
		] 
		if date: any [
			;@@ https://github.com/Oldes/Rebol-issues/issues/2496
			select headers 'last-modified
			select headers 'date
		][
			; allow invalid date, but ignore it on error
			try [info/modified: to-date/utc date]
		]
		remove/part conn/data d2
		state/state: 'reading-data
	]
	unless headers [
		read conn
		return false
	]
	res: false
	unless info/status-code [
		if parse line [
			"HTTP/1." [#"0" | #"1"] some #" " copy code: 3 digits to end
		][
			info/status-code: to integer! code
		]
	]
	code: info/status-code

	log-trace 'HTTP ["Check-response code:" code "means:" select http-status-codes code]

	case [
		code < 200 [ ;= Information responses
			info/headers: info/response-line: info/status-code: port/data: none
			state/state: 'reading-headers
			read conn
		]
		find [204 304] code [ ;= No Content | Not Modified
			state/state: 'ready
			res: awake make event! [type: 'done port: port]
			unless res [res: awake make event! [type: 'ready port: port]]
		]
		'else [
			
			if code = 404 [info/type: none] ; not exists!

			either spec/method = 'HEAD [
				state/state: 'ready
				res: awake make event! [type: 'done port: port]
				unless res [res: awake make event! [type: 'ready port: port]]
			][
				res: check-data port
				if all [not res state/state = 'ready][
					res: awake make event! [type: 'done port: port]
					unless res [res: awake make event! [type: 'ready port: port]]
				]
			]
		]

	]
	res
]

check-result: func[ result [block!] all [logic! none!]][
	either all [
		result ;= [status-code headers content]
	][
		if any [result/1 >= 400 result/1 == 300] [
			do make error! [
				type: 'Access
				id:   'Protocol
				arg1: any [select http-status-codes result/1 "Unknown error response"]
			]
		]
		result/3
	]
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

do-redirect: func [port [port!] new-uri [url! string! file!] /local spec state headers temp][
	spec: port/spec
	state: port/state
	port/data: none

	;new-uri: as url! new-uri

	log-info 'HTTP ["Redirect to:^[[m" mold new-uri]

	state/redirects: state/redirects + 1
	if state/redirects > system/options/http-redirects [
		res: throw-http-error port {Too many redirections}
	]

	spec/query: spec/target: none ; old parts not used in redirection!

	;; If decoding of the new uri fails, then it must be just change of the path
	either temp: decode-url new-uri [
		new-uri: temp
	][
		;; Some servers may have invalid location (missing slash) - Rebol-issues/issues/2604
		if new-uri/1 != #"/" [insert new-uri #"/"]
		spec/path: new-uri: as file! new-uri
		;; If it's redirection under same url, we can reuse the opened connection
		if "keep-alive" = select state/info/headers 'Connection [
			do-request port
			return true
		]
		new-uri: decode-url as url! ajoin [spec/scheme "://" spec/host #":" spec/port new-uri]
	]

	spec/headers/host: new-uri/host

	unless select new-uri 'port [
		switch new-uri/scheme [
			'https [append new-uri [port: 443]]
			'http  [append new-uri [port: 80 ]]
		]
	]
	new-uri: construct/with new-uri port/scheme/spec
	new-uri/method: spec/method
	new-uri/ref: as url! ajoin either find [#(none) 80 443] new-uri/port [
		[new-uri/scheme "://" new-uri/host new-uri/path]
	][	[new-uri/scheme "://" new-uri/host #":" new-uri/port new-uri/path]]

	unless find [http https] new-uri/scheme [
		return throw-http-error port {Redirect to a protocol different from HTTP or HTTPS not supported}
	]

	; store original request headers
	headers: spec/headers
	; we need to reset tcp connection here before doing a redirect
	close port/state/connection
	port/spec: new-uri
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

	log-debug 'HTTP ["Check-data; bytes:^[[m" length? conn/data]

	case [
		headers/transfer-encoding = "chunked" [
			data: conn/data ;- data from lower layer (TLS or TCP)
			available: length? data

			log-debug 'HTTP ["Chunked data: " state/chunk-size "av:" available]

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
					either parse data [
						copy chunk-size some hex-digits
						crlfbin pos: to end
					][
						;- ugly conversion of the chunk size from hexadecimal string to integer
						chunk-size: to integer! to issue! to string! :chunk-size
						remove/part data pos
						available: length? data
						log-debug 'HTTP ["Chunk-size:^[[m" chunk-size " ^[[36mavailable:^[[m " available]
						either chunk-size = 0 [
							if parse data [
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
			if headers/content-length <= length? conn/data [
				state/state: 'ready
				port/data: copy/part conn/data headers/content-length
				if state/info/status-code >= 300 [ return false ]
			;	if state/info/status-code < 300 [
			;		return state/awake make event! [type: 'custom port: port code: 0]
			;	]
			]
			;Awake from the WAIT loop to prevent timeout when reading big data. --Richard
			res: true
		]
		true [
			port/data: conn/data
			either state/info/status-code < 300 [
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
	if encoding: attempt [to word! result/2/Content-Encoding] [
		either find system/catalog/compressions encoding [
			try/with [
				result/3: decompress result/3 encoding
			][
				log-info 'HTTP ["Failed to decode data using:^[[22m" encoding]
				return result
			]
			log-info 'HTTP ["Extracted using:^[[22m" encoding "^[[1mto:^[[22m" length? result/3 "bytes"]
		][
			log-info 'HTTP ["Unknown Content-Encoding:^[[m" encoding]
		]
	]
	if all [
		content-type: select result/2 'Content-Type
		any [
			; consider content to be a text if charset specification is included
			parse content-type [
				to #";" thru "charset=" [
					  #"^"" copy code-page to #"^"" to end ; Facebook is using this!
					| copy code-page to end
				]
			]
			; or when it is without charset, but of type text/*
			parse content-type [["text/" | "application/json"] to end]
		]
	][
		either all [code-page code-page != "utf-8"] [
			log-info 'HTTP ["Trying to decode from code-page:^[[m" code-page]
			; using also deline to normalize possible CRLF to LF
			try [result/3: deline iconv result/3 code-page]
		][
			result/3: to string! result/3
		]
	]
	result
]

anonymize: func[
	;; remove identifying information from data
	data [string!]
] bind [
	parse data [
		any [
			thru LF [
				  "Authorization:" some SP some uri
				| ["X-Token:" | "X-Auth-Token:" | "X-goog-api-key:"]
			] some SP 0 4 uri change to LF "****"
			| skip
		]
	]
	data
] system/catalog/bitsets

hex-digits: system/catalog/bitsets/hex-digits
    digits: system/catalog/bitsets/numeric

sys/make-scheme [
	name: 'http
	title: "HyperText Transport Protocol v1.1"
	spec: make system/standard/port-spec-net [
		path: %/
		method: 'GET
		headers: []
		content: none
		timeout: 15
		redirect?: on
	]
	info: make system/standard/file-info [
		response-line:
		status-code:
		headers: none
	]
	actor: [
		read: func [
			port [port!]
			/part {Partial read a given number of units (source relative)}
				length [integer!]
			/seek {Read from a specific position (source relative)}
				index  [integer!] "zero-based!"
			/string {Convert UTF and line terminators to standard text string}
			/binary {Preserves contents exactly}
			/lines  {Convert to block of strings (implies /string)}
			/all    {Read may include additional information}
			/local result
		][
			log-trace 'HTTP "READ"
			if lines [
				if binary [cause-error 'Script 'bad-refine /binary ]
				seek: part: none
			]
			if lib/all [string binary] [cause-error 'Script 'bad-refines none]

			unless port/state [open port port/state/close?: yes]

			if lib/all [any [part seek] not string] [
				either seek [ binary: true assert [index >= 0]][ index: 0 ]
				length: either part [ assert [length > 0] length + index][ none ]
				put port/spec/headers quote Range: ajoin ["bytes=" index #"-" any [lib/all [length length - 1] ""]]
			]
			either any-function? :port/awake [
				unless open? port [cause-error 'Access 'not-open port/spec/ref]
				if port/state/state <> 'ready [throw-http-error port "Port not ready"]
				port/state/awake: :port/awake
				do-request port
			][
				result: sync-op port []
				either binary [
					if result/1 <> 206 [ ;= Partial
						case/all [
							seek  [result/3: at result/3 index  if part [length: length - index]]
							part  [clear skip result/3 length]
						]
					]
				][
					decode-result result
					case/all [
						string [ try [result/3: to string! result/3] ]
						lines  [ try [result/3: split-lines to string! result/3] ]
						index  [ result/3: skip result/3 index ]
						length [ clear skip result/3 length]
					]
				]
				check-result result :all
			]
		]
		write: func [
			port [port!]
			value {Data to write (non-binary converts to UTF-8)}
		;	/part {Partial write a given number of units}
		;		length [number!]
		;	/seek {Write at a specific position}
		;		index [number!]
		;	/append {Write data at end of file}
		;	/allow  {Specifies protection attributes}
		;		access [block!]
		;	/lines  {Write each value in a block as a separate line}
			/binary {Preserves contents exactly}
			/all    {Response may include additional information (source relative)}
			/local result
		][
			;@@ NOTE: `all` is redefined!
			log-trace 'HTTP "WRITE"
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
				if port/state/state <> 'ready [throw-http-error port "Port not ready"]
				port/state/awake: :port/awake
				parse-write-dialect port value
				do-request port
			][
				result: sync-op port [parse-write-dialect port value]
				if lib/all [
					not binary
					find [GET POST PATCH] port/spec/method
				] [decode-result result]
				check-result result :all
			]
		]
		update: func[
			port [port!]
		][
			? port
			? port/state
			read port/state/connection

		]
		open: func [
			port [port!]
			/local conn spec
		][
			log-trace 'HTTP ["OPEN, state:" port/state]
			if port/state [return port]
			if none? port/spec/host [throw-http-error port "Missing host address"]
			unless integer? system/options/http-redirects [system/options/http-redirects: 0]
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
			spec: port/spec
			port/state/connection: conn: make port! compose [
				scheme: (to lit-word! either spec/scheme = 'http ['tcp]['tls])
				host: spec/host
				port: spec/port
				ref: as url! ajoin [scheme "://" host #":" port]
			]
			
			conn/awake: :http-awake
			conn/parent: port
			log-info 'HTTP ["Opening connection:^[[22m" conn/spec/ref]
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
			log-trace 'HTTP "CLOSE"
			if object? port/state [
				port/state/state: 'closing
				close port/state/connection
				port/state/connection/awake: none
				; release state and if there was error, keep it there
				if error? port/state/error [
					port/state: port/state/error
				]
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
			field [word! block! none! datatype!]
			/mode ;@@ deprecated!
			/local error state result
		][
			if none? field [ return words-of system/schemes/http/info]
			if none? state: port/state [
				open port ;there is port opening in sync-op, but it would also close the port later and so clear the state
				try/with [sync-op port [parse-write-dialect port [HEAD]]][
					log-error 'HTTP system/state/last-error
				]
				state: port/state
				close port
			]
			either all [
				state
				state/info/status-code
			][
				case [
					word? field [
						select state/info field
					]
					block? field [
						result: make block! length? field
						foreach word field [
							if any-word? word [
								if set-word? word [ append result word ]
								append result state/info/(to word! word)
							]
						]
						result
					]
					field = #(object!) [ state/info ]
					field = #(map!)   [to map! state/info ]
					field = #(block!) [to block! state/info ]
				]
			][	none ]
		]
		length?: func [
			port [port!]
		][
			either port/data [length? port/data][0]
		]
	]
	; default request header values...
	headers: context [
		Host: none
		Accept: "*/*"
		Accept-charset: "utf-8"
		Accept-Encoding: "gzip,deflate"
		User-Agent: ajoin ["rebol/" system/version " (" system/platform "; " system/build/arch #")"]
		;@@ One can set above value for example to: "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.103 Safari/537.36"
		;@@ And so pretend that request is coming from Chrome on Windows10
	]
	if find system/catalog/compressions 'br [
		append headers/Accept-Encoding ",br"
	]
]

sys/make-scheme/with [
	name: 'https
	title: "Secure HyperText Transport Protocol v1.1"
	spec: make spec [
		port: 443
	]
] 'http


;-- Cookies support --

with cookies-rules: context [
	; US-ASCII characters excluding CTLs, whitespace DQUOTE, comma, semicolon, and backslash:
	cookie-octet: make bitset! #{000000005FF7FFEFFFFFFFF7FFFFFFFE}
	cookie-octet-sp: make bitset! #{00400000DFF7FFEFFFFFFFF7FFFFFFFE} ; includes SPACE
	; any CHAR except CTLs or separators:
	token-char: complement charset [{()<>@,;:\"/[]?={} ^-} 0 - 31 127]
	domain-char: charset [#"a"-#"z" #"A"-#"Z" ".-"]
	path-char: complement charset [0 - 31 #";"]
	digit: system/catalog/bitsets/numeric

	=cookie-value: [#"^"" any cookie-octet-sp #"^"" | any cookie-octet]
	=cookie-name:  [some token-char]

	;https://datatracker.ietf.org/doc/html/rfc2616#section-3.3.1
	=month: ["Jan"|"Feb"|"Mar"|"Apr"|"May"|"Jun"|"Jul"|"Aug"|"Sep"|"Oct"|"Nov"|"Dec"]
	=wkday: ["Mon"|"Tue"|"Wed"|"Thu"|"Fri"|"Sat"|"Sun"]
	=itime: [2 digit #":" 2 digit #":" 2 digit]
	;; The rule automatically fixes common non-compliant dates:
	=date:  [2 digit [SP | change #"-" #" "]  =month [SP | change #"-" #" "] 4 digit]
	=rfc1123-date: [=wkday ", " =date SP =itime SP "GMT"]
	unless cookies-data: select system/state 'cookies [
		cookies-data: make map! []
		extend system/state 'cookies :cookies-data
	]
][
	set-cookies: function [
		"Processes `Set-Cookie` headers and stores cookies according to RFC 6265."
		"Cookies are stored in `system/state/cookies` map."
		host [string!] "Current host (used when the cookie doesn't set its domain)"
		data [string! block!] "Either a string or block containing `Set-Cookie` lines."
	][
		timestamp: to integer! now/utc
		foreach line to block! data [
			Expires: domain: path: max-age: none
			attr: parse line [
				copy c-name:  =cookie-name #"="
				[
					#"^"" copy c-value: any cookie-octet-sp #"^""
					| copy c-value: any cookie-octet
				]
				collect any ["; " [
					"Expires=" copy Expires: =rfc1123-date |
					"Domain="  copy Domain: any domain-char |
					"Path="    copy Path:   any path-char |
					"Max-Age=" copy Max-Age: some digit |
					"Secure"   keep ('Secure) |
					"HttpOnly" keep ('HttpOnly) |
					keep some path-char
				]]
			]
			;print [Domain Path Expires mold head attr]
			;; The Expires attribute indicates the maximum lifetime of the cookie
			try [Expires: to integer! to-date expires]
			;; the Max-Age attribute has precedence and controls the expiration date of the cookie!
			if Max-Age [Expires: timestamp + to integer! Max-Age]
			unless Expires [Expires: timestamp + 3600 append attr 'not-persistent]

			;; https://datatracker.ietf.org/doc/html/rfc6265#section-4.1.2.3
			either Domain [
				;; If the value of the Domain attribute is "example.com", the user
				;; agent will include the cookie in the Cookie header to example.com,
				;; www.example.com, and www.corp.example.com
				if #"." != Domain/1 [insert Domain #"."]
			][
				;; When domain is not specified, use host name without the leading dot!
				;; So when host was: "example.com", cookie must be available only to this
				;; exact domain. Not for "www.example.com"!
				Domain: copy host
			]
			if empty? attr [attr: none]
			
			set?: false
			dcooks: head cookies-data/:Domain: any [cookies-data/:Domain copy []]
			while [not tail? dcooks] [
				if all [
					dcooks/3 == c-name
					dcooks/2 == Path
				][	;; cookie with this name and path already exists
					either Expires <= timestamp [
						;; cookie is expired, so remove it
						remove/part dcooks 5
						log-info 'COOKIES ["DEL" domain path c-name]
					][
						;; update the cookie
						dcooks/1: Expires
						dcooks/4: c-value
						dcooks/5: attr
						;; and skip to the next one
						log-info 'COOKIES ["UPD" Expires Path c-name "=>" c-value]
					]
					set?: true
					break
				]
				either dcooks/1 <= timestamp [
					log-info 'COOKIES ["DEL" domain dcooks/2 dcooks/3]
					remove/part dcooks 5
				][	dcooks: skip dcooks 5 ]
			]
			
			unless set? [
				log-info 'COOKIES ["SET" domain Path c-name "=>" c-value]
				repend dcooks [Expires Path c-name c-value attr]
				new-line skip dcooks -5 true
			]
			dcooks: head dcooks
		]
		() ;= no return
	]
	get-cookies: function/with [
		"Retrieves and formats valid cookies for HTTP requests."
		host [string!] "The host for which cookies are being retrieved."
		path [string! file!] "The path for which cookies are being retrieved."
		return: [string! "Cookies formated into a single string suitable for HTTP requests."]
	][
		clear values
		timestamp: to integer! now/utc
		domain: ajoin ["." host]
		;; collect values for subdomains
		while [ all [domain temp: find next domain #"."] ][
			get-host-cookies domain
			domain: temp
		]
		;; collect "host-only" cookies
		get-host-cookies host
		if empty? values [return none]
		foreach [key value] values [
			append str ajoin [key #"=" value "; "]
		]
		clear skip tail str -2
		also copy str  clear str
	][
		values: make map! []
		str: make string! 500
		timestamp: domain: none
		get-host-cookies: func[domain /local data Expires Path Name Value attr][
			;print ["get-host-cookies for" domain]
			if block? data: cookies-data/:domain [
				while [not tail? data][
					set [Expires: Path: Name: Value: attr:] data
					either Expires >= timestamp [
						values/:Name: Value
						data: skip data 5
					][
						log-info 'COOKIES ["DEL" domain path name]
						remove/part data 5
					]
				]
			]
			values
		]
	]
]

