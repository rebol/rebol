REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Rebol3 HTTP protocol scheme"
	Name: http
	Type: module
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2022 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Version: 0.5.2
	Date: 22-Jul-2023
	File: %prot-http.r3
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
		0.4.0 04-Feb-2022 "Oldes" "FIX: situation when server does not provide Content-Length and just closes connection"
		0.4.1 13-Jun-2022 "Oldes" "FIX: Using `query` on URL sometimes reports `date: none`"
		0.5.0 18-Jul-2022 "Oldes" "FEAT: `read/seek` and `read/all` implementation"
		0.5.1 12-Jun-2023 "Oldes" "FEAT: anonymize authentication tokens in log"
		0.5.2 22-Jul-2023 "Oldes" "FEAT: support for optional Brotli encoding"
	]
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

	if all [state/close? open? port][
		sys/log/more 'HTTP ["Closing port for:^[[m" port/spec/ref]
		close port
	]

	reduce [port/state/info/status-code header body]
]

read-sync-awake: func [event [event!] /local error state][
	sys/log/debug 'HTTP ["Read-sync-awake:" event/type]
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
	http-port: port/parent

	state: http-port/state
	if any-function? :http-port/awake [state/awake: :http-port/awake]
	awake: :state/awake

	;? awake

	sys/log/debug 'HTTP ["Awake:^[[1m" event/type "^[[22mstate:^[[1m" state/state]

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
			sys/log/debug 'HTTP ["Closing:^[[1m" http-port/spec/ref]
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
	"Create an HTTP request (returns binary!)"
	spec [block! object!] "Request specification from an opened port"
	/local method path target query headers content request 
][
	method:  any [select spec 'method 'GET]
	path:    any [select spec 'path    %/]
	target:       select spec 'target
	query:        select spec 'query
	headers: any [select spec 'headers []]
	content:      select spec 'content

	request: ajoin [
		uppercase form :method SP
		mold as url! :path        ;; `mold as url!` is used because it produces correct escaping
	]
	if :target [append request mold as url! :target]
	if :query  [append append request #"?"  :query]

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
	sys/log/info 'HTTP ["Request:^[[22m" anonymize mold request]

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
	info/size: info/date: info/name: none

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

put system/catalog 'http-status-codes http-status-codes: #(
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
)
check-response: func [port /local conn res headers d1 d2 line info state awake spec date code][
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
		sys/log/info 'HTTP line
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
		if date: any [
			;@@ https://github.com/Oldes/Rebol-issues/issues/2496
			select headers 'last-modified
			select headers 'date
		][
			; allow invalid date, but ignore it on error
			try [info/date: to-date/utc date]
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

	sys/log/debug 'HTTP ["Check-response code:" code "means:" select http-status-codes code]

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

	spec/query: spec/target: none ; old parts not used in redirection!

	if #"/" = first new-uri [
		; if it's redirection under same url, we can reuse the opened connection
		if "keep-alive" = select state/info/headers 'Connection [
			spec/path: new-uri
			do-request port
			return true
		]
		new-uri: as url! ajoin [spec/scheme "://" spec/host #":" spec/port new-uri]
	]
	new-uri: decode-url new-uri
	spec/headers/host: new-uri/host

	unless select new-uri 'port [
		switch new-uri/scheme [
			'https [append new-uri [port: 443]]
			'http  [append new-uri [port: 80 ]]
		]
	]
	new-uri: construct/with new-uri port/scheme/spec
	new-uri/method: spec/method
	new-uri/ref: as url! ajoin either find [#[none] 80 443] new-uri/port [
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
					either parse data [
						copy chunk-size some hex-digits
						crlfbin pos: to end
					][
						;- ugly conversion of the chunk size from hexadecimal string to integer
						chunk-size: to integer! to issue! to string! :chunk-size
						remove/part data pos
						available: length? data
						sys/log/more 'HTTP ["Chunk-size:^[[m" chunk-size " ^[[36mavailable:^[[m " available]
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
	if encoding: select result/2 'Content-Encoding [
		either find ["gzip" "deflate" "br"] encoding [
			if encoding == "br" [encoding: 'brotli]
			try/with [
				result/3: decompress result/3 to word! encoding
			][
				sys/log/info 'HTTP ["Failed to decode data using:^[[22m" encoding]
				return result
			]
			sys/log/info 'HTTP ["Extracted using:^[[22m" encoding "^[[1mto:^[[22m" length? result/3 "bytes"]
		][
			sys/log/info 'HTTP ["Unknown Content-Encoding:^[[m" encoding]
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
		code-page: any [code-page "utf-8"]
		sys/log/info 'HTTP ["Trying to decode from code-page:^[[m" code-page]
		; using also deline to normalize possible CRLF to LF
		try [result/3: deline iconv result/3 code-page]
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
			sys/log/debug 'HTTP "READ"
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
				if port/state/state <> 'ready [throw-http-error "Port not ready"]
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
						lines  [ result/3: split-lines result/3 ]
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
			spec: port/spec
			port/state/connection: conn: make port! compose [
				scheme: (to lit-word! either spec/scheme = 'http ['tcp]['tls])
				host: spec/host
				port: spec/port
				ref: as url! ajoin [scheme "://" host #":" port]
			]
			
			conn/awake: :http-awake
			conn/parent: port
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
				state/info/status-code
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
	if find system/catalog/compressions 'brotli [
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
