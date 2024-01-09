Rebol [
	Title:  "HTTPd Scheme"
	Type:    module
	Name:    httpd
	Date:    10-Jan-2024
	Version: 0.9.1
	Author: ["Andreas Bolka" "Christopher Ross-Gill" "Oldes"]
	Exports: [serve-http http-server decode-target to-CLF-idate]
	Home:    https://github.com/Oldes/Rebol-HTTPd
	Rights:  http://opensource.org/licenses/Apache-2.0
	Purpose: {
		A Tiny Webserver Scheme for Rebol 3 (Oldes' branch)
		Features:
		* handle basic POST, GET and HEAD methods
		* send large files in chunks (using a file port)
		* using _actors_ for main actions which may be customized
		* implemented `keep-alive` behaviour
		* sends `Not modified` response if file was not modified in given time
		* client can stop the server
	}
	TODO: {
		* support for multidomain serving using `Host` header field
		* add support for other methods - PUT, DELETE, TRACE, CONNECT, OPTIONS?
		* better error handling
	}
	Usage: {Check %server-test.r3 script how to start a simple server}
	History: [
		04-Nov-2009 "Andreas Bolka" {A Tiny HTTP Server
		https://github.com/earl/rebol3/blob/master/scripts/shttpd.r}
		04-Jan-2017 "Christopher Ross-Gill" {Adaptation to Scheme
		https://gist.github.com/rgchris/73510e7d643eb0a6b9fa69b849cd9880}
		01-Apr-2019 "Oldes" {Rewritten to be usable in real life situations.}
		10-May-2020 "Oldes" {Implemented directory listing, logging and multipart POST processing}
		02-Jul-2020 "Oldes" {Added possibility to stop the server from a client and return data to it (useful for OAuth2)}
		06-Dec-2022 "Oldes" {Added minimal support for WebSocket connections}
		09-Jan-2023 "Oldes" {New home: https://github.com/Oldes/Rebol-HTTPd}
		09-May-2023 "Oldes" {Root-less configuration possibility (default)}
		14-Dec-2023 "Oldes" {Deprecated the `http-server` function in favor of `serve-http` with a different configuration input}
	]
	Needs: [3.11.0 mime-types]
]

append system/options/log [httpd: 1]

;------------------------------------------------------------------------
;-- Net utils:                                                         --
;------------------------------------------------------------------------
;@@ move elsewhere and with different name?

decode-target: wrap [
	control: charset [#"?" #"&" #"=" #"#"]
	chars: complement control
	func [
		"Splits target into file part and key/pair query values if any"
		target [file! string! binary! none!]
		/local key val
	][
		
		result: object [file: none values: make block! 8 fragment: none original: target]
		unless target [return result]
		parse to binary! target [
			opt [
				copy val any chars (result/file: val) [
					  #"?" 
					| #"#" copy val to end (result/fragment: dehex val)
					| end
				] 
			]
			any [
				copy key some chars #"=" copy val any chars (
					unless empty? val [
						append result/values to set-word! to string! dehex key
						append result/values to string! dehex val
					]
				) opt #"&"
				| #"&"
				| #"#" copy val to end (result/fragment: dehex val)
			]
		]
		result/file: to file! dehex either all [
			none?  result/file
			empty? result/values
			none?  result/fragment
		][
			target
		][	any [result/file %""] ]
		new-line/skip result/values true 2 
		result
	]
]

;- Multipart data related code:
; token:       https://tools.ietf.org/html/rfc7230#section-3.2.6
;              ["!#$%&'*+-.^^_`|~" #"0"-#"9" #"a"-#"z" #"A"-#"Z"]
; whitespace:  https://tools.ietf.org/html/rfc7230#section-3.2.3
ch_token: make bitset! #{000000005F36FFC07FFFFFE3FFFFFFEA}
ch_ws:    make bitset! #{0040000080}

rl_q-string: [#"^"" any [ #"\" 1 skip | some ch_token ] #"^""]

trim-q-string: func[str][
	parse str [
		remove #"^""
		any [ some ch_token
			| remove #"\" 1 skip
		]
		remove #"^""
	]
]
decode-content-params: func[
	str
	/local params type subtype key val
][
	if none? str [ return reduce [none none] ]
	str: to string! str
	params: clear []
	either parse str [
		any ch_ws
		copy type some ch_token
		opt [#"/" copy subtype some ch_token]
		any [
			any ch_ws
			#";"
			any ch_ws
			  copy key some ch_token #"="
			[ copy val some ch_token
			| copy val rl_q-string (trim-q-string val)]
			( put params key val )
		]
	][
		reduce [type subtype copy params]
	][	str ] ;- returns original in case of error
]

decode-multipart-data: func[
	{Decodes multipart encoded data (used with POST method)}
	type [binary! string! block!] "Content-type header value"
	data [binary!] "Raw data after HTTP header"
	/local boundary header body key val result boundary-end
][	;@@NOTE: this may be used with emails too! Where it should live?

	unless block? type [ type: decode-content-params type ]

	if any [
		not block? type
		not block? type/3
		type/1 <> "multipart"
		not boundary: select type/3 "boundary"
	][ return data ]  ;- returns original in case of error

	boundary-end: join "^M^/--" boundary
	result: copy []
	parse data [
		any [
			"--" boundary CRLF
			(header: copy [])
			some [
				copy key some ch_token #":"
				copy val to CRLF 2 skip
				( put header to string! key decode-content-params val )
			]
			( append/only result header )
			CRLF
			copy body to boundary-end
			( append result either empty? body [none][body] )
			CRLF
		]
		"--" boundary "--" CRLF end
	]
	new-line/all result true
	result
]

to-CLF-idate: func [
	"Returns a standard Common Log Format date string."
	date [date!]
	/local zone
][
	zone: form date/zone
	remove find zone ":"
	if #"-" <> first zone [insert zone #"+"]
	if 4 >= length? zone [insert next zone #"0"]
	ajoin [
		pad/with date/day -2 #"0"
		#"/"
		pick ["Jan" "Feb" "Mar" "Apr" "May" "Jun" "Jul" "Aug" "Sep" "Oct" "Nov" "Dec"] date/month
		#"/"
		date/year
		#":"
		to-itime any [date/time 0:00]
		#" "
		zone
	]
]

;------------------------------------------------------------------------
;-- Scheme definition:                                                 --
;------------------------------------------------------------------------
sys/make-scheme [
	Title: "HTTP Server"
	Name:  'httpd

	Actor: [
		Open: func [port [port!] /local spec][
			spec: port/spec
			log-more ["Opening server at port:^[[22m" spec/port]
			port/extra: make object! [
				subport: open compose [
					scheme: 'tcp
					port: (spec/port)
				]
				subport/awake: :port/scheme/awake-server
				subport/extra: make object! [
					parent: port
					config: none
					clients: make block! 16
				]
				subport/extra/config:
				config: make map! [
					root:       #[none]
					index:       [%index.html %index.htm]
					keep-alive: #[true]
					list-dir?:  #[true]
					server-name: "Rebol3-HTTPd"
				]
			]
			port/state: port/extra/subport/extra/clients
			port/awake: :awake-client
			port
		]

		Close: func [port [port!]][
			log-more ["Closing server at port:^[[22m" port/spec/port]
			close port/extra/subport
		]

		On-Accept: func [ctx [object!]][ true ]
		On-Header: func [ctx [object!]][] ;= placeholder; user can use it for early request processing

		On-Get: func [
			ctx [object!]
			/local target path info index modified If-Modified-Since
		][
			target: ctx/inp/target
			unless ctx/config/root [
				Actor/On-Not-Found ctx target
				exit
			]
			target/file: path: join ctx/config/root next clean-path/only target/file
			ctx/out/header/Date: to-idate/gmt now
			ctx/out/status: 200
			either exists? path [
				if dir? path [
					foreach file ctx/config/index [
						if exists? index: path/:file [
							path: index
							log-debug ["using index file:" index]
							break
						]
					]
					if path <> index [
						; client is requesting directory, which don't have index file
						return Actor/On-List-Dir ctx target
					]
				]
				info: query path
				; prepare modified date of the target
				modified: info/date
				modified/timezone: 0 ; converted to UTC
				modified/second: to integer! modified/second ; remove miliseconds from time

				ctx/state: 'send-data
				ctx/out/status: 200
				ctx/out/target: path
				ctx/out/header/Content-Length: info/size
				ctx/out/header/Last-Modified:  to-idate/gmt info/date
				if ctx/inp/method = "GET" [
					;? ctx/inp/header
					either all [
						date? If-Modified-Since: try [to-date ctx/inp/header/If-Modified-Since]
						If-Modified-Since >= modified
					][
						ctx/out/status: 304 ;= not modified
					][
						ctx/out/content: either info/size <= 32000 [read path][open/read path]
					]
				]
			][
				Actor/On-Not-Found ctx target
			]
		]

		On-Post: func [ctx [object!]][
			;@@ this is just a placeholder!
			true
		]

		On-Read-Post: func[ctx [object!] /local content header length type temp][
			;@@ TODO: handle `Expect` header: https://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.20
			header: ctx/inp/header
			length: select header 'Content-Length
			unless find header 'Content-Type [
				;make sure there is any Content-Type
				extend header 'Content-Type "application/octet-stream"
			]
			either all [
				integer? length
				length > length? content: ctx/inp/content
			][
				ctx/state: 'read-data
			][
				ctx/state: 'data-received
				ctx/out/status: 200
				type: header/Content-Type: decode-content-params header/Content-Type
				;@@ TODO: handle charset if not utf-8!
				case [
					type/2 = "x-www-form-urlencoded" [
						; using full target decoder, although only values are needed
						temp: decode-target content
						ctx/inp/content: reduce [
							select temp 'values   ; parsed [key value ...]
							select temp 'original ; raw data
						]
					]
					type/2 = "json" [
						ctx/inp/content: decode 'json content
					]
					type/1 = "multipart" [
						ctx/inp/content: decode-multipart-data type content
					]
					type/1 = "text" [
						ctx/inp/content: to string! content
					]
				]
				Actor/On-Post ctx
			]
		]

		On-Read: func[
			"Process READ action on client's port"
			ctx [object!]
		][
			switch/default ctx/inp/method [
				"HEAD" ; same like GET, but without sending any content
				"GET"  [ Actor/on-get  ctx ]
				"POST" [ Actor/on-read-post ctx ]
			][
				ctx/state: 'data-received
				ctx/out/status: 405 ; Method Not Allowed
			]
		]

		On-Read-Websocket: func[
			"Process READ action on client's port using websocket"
			ctx [object!]
			final? [logic!]   "Indicates that this is the final fragment in a message."
			opcode [integer!] "Defines the interpretation of the 'Payload data'."
		][
			;@@ this is just a placeholder!
		]

		On-Close-Websocket: func[
			"Process CLOSE action on client's port using websocket"
			ctx [object!] code [integer!]
			/local reason
		][
			reason: any [
				select [
					1000 "the purpose for which the connection was established has been fulfilled."
					1001 "a browser navigated away from a page."
					1002 "a protocol error."
					1003 "it has received a type of data it cannot accept."
					1007 "it has received data within a message that was not consistent with the type of the message."
					1008 "it has received a message that violates its policy."
					1009 "it has received a message that is too big for it to process."
					1010 "it has expected the server to negotiate one or more extension, but the server didn't return them in the response message of the WebSocket handshake."
					1011 "it encountered an unexpected condition that prevented it from fulfilling the request."
				] code
				ajoin ["an unknown reason (" code ")"]
			]
			log-info ["WS connection is closing because" reason]
			unless empty? reason: ctx/inp/content [
				;; optional client's reason
				log-info ["Client's reason:" as-red to string! reason]
			]
		]

		On-List-Dir: func[
			ctx [object!] target [object!]
			/local path dir out size date files dirs
		][
			unless ctx/config/list-dir? [
				log-more ["Listing dir not allowed:^[[1m" mold target/file]
				ctx/out/status: 404 ; using not-found response!
				return false
			]
			dir: target/file
			path: join "/" find/match/tail dir ctx/config/root
		
			try/with [
				out: make string! 2000
				append out ajoin [
					{<html><head><title>Index of } path
					{</title></head><body bgcolor="white"><h1>Index of } path
					{</h1><hr><pre>^/}
				]
				unless empty? path [
					append out {<a href="../">../</a>^/}
				]

				files: copy []
				dirs:  copy []
				foreach file read dir [
					; reversed naming bellow intended!
					append either dir? file [files][dirs] file
				]
				append files dirs

				foreach file files [
					set [size date] query/mode dir/:file [size date]
					append out ajoin [
						{<a href="} file {">} file {</a> }
						pad copy "" 50 - length? file
						pad/with date/date  -11 #"0" #" "
						pad/with date/hour   -2 #"0" #":"
						pad/with date/minute -2 #"0" #" "
						pad  any [size "-"] -19
						lf
					]
				]
				append out {</pre><hr></body></html>}
				ctx/out/content: out
				return true
			][
				ctx/out/status: 404 ; using not-found response!
				return false
			]
		]

		On-Not-Found: func[ctx [object!] target [object!]][
			log-more ["Target not found:^[[1m" mold target/file]
			ctx/out/status: 404
		]

		WS-handshake: func[ctx /local key][
			if all [
				"websocket" = select ctx/inp/header 'Upgrade
				key: select ctx/inp/header 'Sec-WebSocket-Key
			][
				ctx/out/status: 101
				ctx/out/header/Upgrade: "websocket"
				ctx/out/header/Connection: "Upgrade"
				ctx/out/header/Sec-WebSocket-Accept: enbase checksum join key "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" 'sha1 64
			]
		]
	]

	Status-Codes: make map! [
		100 "Continue"
		101 "Switching Protocols"
		200 "OK"
		201 "Created"
		202 "Accepted"
		203 "Non-Authoritative Information"
		204 "No Content"
		205 "Reset Content"
		206 "Partial Content"
		;207 "Multi-Status"
		;208 "Already Reported"
		;226 "IM Used"
		300 "Multiple Choices"
		301 "Moved Permanently"
		302 "Found"              ;Previously "Moved temporarily"
		303 "See Other"          ;since HTTP/1.1
		304 "Not Modified"       ;RFC 7232
		305 "Use Proxy"          ;since HTTP/1.1
		306 "Switch Proxy"
		307 "Temporary Redirect" ;since HTTP/1.1
		308 "Permanent Redirect" ;RFC 7538
		400 "Bad Request"
		401 "Unauthorized"
		402 "Payment Required"
		403 "Forbidden"
		404 "Not Found"
		405 "Method Not Allowed"
		406 "Not Acceptable"
		407 "Proxy Authentication Required"
		408 "Request Timeout"
		409 "Conflict"
		410 "Gone"
		;411 "Length Required"
		;412 "Precondition Failed"
		;413 "Payload Too Large"
		;414 "URI Too Long"
		;415 "Unsupported Media Type"
		;416 "Range Not Satisfiable"
		;417 "Expectation Failed"
		;418 "I'm a teapot"
		;421 "Misdirected Request"
		;422 "Unprocessable Entity"
		;423 "Locked"
		;424 "Failed Dependency"
		;426 "Upgrade Required"
		;428 "Precondition Required"
		;429 "Too Many Requests"
		;431 "Request Header Fields Too Large"
		;451 "Unavailable For Legal Reasons" ;@@ https://tools.ietf.org/html/rfc7725
		500 "Internal Server Error"
		501 "Not Implemented"
		502 "Bad Gateway"
		503 "Service Unavailable"
		504 "Gateway Timeout"
		505 "HTTP Version Not Supported"
		;506 "Variant Also Negotiates"
		;507 "Insufficient Storage"
		;508 "Loop Detected"
		;510 "Not Extended"
		;511 "Network Authentication Required"
	]

	Respond: function [port [port!]][
		ctx: port/extra
		out: ctx/out
		unless out/status [out/status: 200] ;; expect OK response if not set
		log-more ["Respond:^[[22m" out/status status-codes/(out/status) length? out/content]
		; send the response header
		buffer: make binary! 1024
		append buffer ajoin ["HTTP/" ctx/inp/version #" " out/status #" " status-codes/(out/status) CRLF]

		either "websocket" = out/header/upgrade [
			ctx/inp/method: "websocket"
			try [ctx/inp/version: to integer! ctx/inp/header/Sec-WebSocket-Version]
			port/awake: :Awake-Websocket
		][
			unless out/header/Content-Type [
				if out/target [
					out/header/Content-Type: mime-type? out/target
				]
				if all [
					none? out/header/Content-Type ; no mime found above
					string? out/content
				][
					out/header/Content-Type: "text/html; charset=UTF-8"
				]
			]		

			out/header/Content-Length: either out/content [
				if string? out/content [
					; must be converted to binary to have proper length if not ascii
					out/content: to binary! out/content
				]
				length? out/content
			][
				0
			]

			if keep-alive: ctx/config/keep-alive [
				if logic? keep-alive  [
					; using defaults
					ctx/config/keep-alive:
					keep-alive: [15 100] ; [timeout max-requests]
				]
				ctx/out/header/Connection: "keep-alive"
				ctx/out/header/Keep-Alive: ajoin ["timeout=" keep-alive/1 ", max=" keep-alive/2]
			]
			out/header/Server: ctx/config/server-name
		]
		
		;probe out/header
		foreach [name value] out/header [
			if value [
				append buffer ajoin [mold to set-word! name #" " value CRLF]
			]
		]
		;print to-string buffer
		append buffer CRLF

		if all [out/content not port? out/content] [
			append buffer out/content
			out/content: none
		]

		try/with [
			write port buffer
		][
			log-error "Write failed!"
			;probe copy/part buffer 100
			Awake-Client make event! [type: 'close port: port ]
		]
	]

	Write-log: function [ctx][
		try/with [
			msg: ajoin [
				ctx/remote-ip
				{ - - [} to-CLF-idate now {] "}
				ctx/inp/method #" "
				to string! ctx/inp/target/original
				" HTTP/" ctx/inp/version {" }
				ctx/out/status #" "
				any [ctx/out/header/Content-Length #"-"]
				#"^/"
			]
			prin msg
			if file? file: ctx/config/log-access [
				write/append file msg
			]
			if all [
				ctx/out/status >= 400
				file? file: ctx/config/log-errors
			][
				write/append file msg
			]
		][
			log-error "Failed to write a log"
			log-error system/state/last-error
		]
	]

	Awake-Client: wrap [
		chars-method: #[bitset! #{00000000000000007FFFFFE0}] ; #"A" - #"Z"
		;from-method: ["GET" | "POST" | "HEAD" | "PUT" | "DELETE" | "TRACE" | "CONNECT" | "OPTIONS"]
		chars: complement union space: charset " " charset [#"^@" - #"^_"]
		CRLF2BIN: #{0D0A0D0A}

		function [
			event [event!]
		][
			port: event/port
			ctx: port/extra
			inp: ctx/inp
			out: ctx/out

			log-more ["Awake:^[[1m" ctx/remote "^[[22m" event/type]

			ctx/timeout: now + 0:0:15

			switch event/type [
				READ [
					log-more ["bytes:^[[1m" length? port/data]
					either header-end: find/tail port/data CRLF2BIN [
						try/with [
							if none? ctx/state [
								with inp [
									parse copy/part port/data header-end [
										copy method some chars-method some space
										copy target some chars        some space
										"HTTP/" copy version some chars thru CRLF
										copy header to end
										(
											method:  to string! method
											target:  decode-target target
											version: to string! version
											header:  construct header
											try [header/Content-Length: to integer! header/Content-Length]
										)
									]
									content: header-end
								]
								log-more ["Request header:^[[22m" ctx/inp/method mold ctx/inp/header]
								; on-header actor may be used for rewrite rules (redirection)
								actor/on-header ctx
								if ctx/out/status [
								; if status is defined (probably redirection), than we can send a response now
									respond port
									break
								]
							]
							actor/on-read port/extra
						][
							log-error system/state/last-error
							ctx/state: 'error
							ctx/out/status: 500 ; Internal Server Error
						]
						log-debug ["State:^[[1m" ctx/state "^[[22mstatus:^[[1m" out/status]
						either ctx/state = 'read-data [
							; posted data not fully read
							read port
						][	respond port ]
					][
						; request header not yet fully received
						read port
					]
				]
				WROTE [
					event/type: 'CLOSE 
					either all [
						out
						out/content
					][
						case [
							port? out/content [
								; streaming output from port
								buffer: copy/part out/content 32000 ;4096
								either empty? buffer [
									; end of stream
									close out/content ; closing source port
									End-Client port
								][
									try/with [
										write port buffer
									][
										log-error  "Write failed (2)!"
										;probe buffer
										End-Client port
									]
								]
							]
							true [
								; for now just remove the content which is written
								; no need to chunk data manually, these are handled internaly
								; just make sure you don't use any other `write` before receiving `wrote`
								write port out/content
								out/content: none
							]
						]
					][
						; there is no other content to write, so close connection
						End-Client port
					]
					port
				]
				CLOSE [
					log-more ["Closing:^[[22m" ctx/remote]
					if pos: find ctx/parent/extra/clients port [ remove pos ]
					close port
				]
			]
		]
	]

	Awake-Server: func [event [event!] /local ctx client config] [
		log-debug ["Awake (server):^[[22m" event/type]
		switch event/type [
			ACCEPT [ New-Client event/port ]
			CLOSE  [
				close event/port
				close event/port/extra/parent
			]
		]
		true
	]

	Awake-Websocket: function [
		event [event!]
	][
		port: event/port
		ctx: port/extra

		log-more ["Awake Websocket:^[[1m" ctx/remote "^[[22m" event/type]

		ctx/timeout: now + 0:0:30

		switch event/type [
			READ [
				ready?: false
				data: head port/data
				log-more ["bytes:^[[1m" length? data]
				try/with [
					while [2 < length? data][
						final?: data/1 & 2#10000000 = 2#10000000
						opcode: data/1 & 2#00001111
						mask?:  data/2 & 2#10000000 = 2#10000000
						len:    data/2 & 2#01111111
						data: skip data 2
						;? final? ? opcode ? len

						;@@ Not removing bytes until we make sure, that there is enough data!
						case [
							len = 126 [
								if 2 >= length? data [break]
								len: binary/read data 'UI16
								data: skip data 2
							]
							len = 127 [
								if 8 >= length? data [break]
								len: binary/read data 'UI64
								data: skip data 8
							]
						]
						if (4 + length? data) < len [break]
						data: truncate data ;; removes already processed bytes from the head
						either mask? [
							masks:   take/part data 4
							request-data: masks xor take/part data len
						][
							request-data: take/part data len
						]
						ready?: true
						ctx/inp/content: truncate/part request-data len
						if opcode = 8 [
							log-more "WS Connection Close Frame!"
							code: 0
							if all [
								2 <= len
								2 <= length? request-data
							][
								code: to integer! take/part request-data 2
								log-more ["WS Close reason:" as-red code]
							]
							actor/On-Close-Websocket ctx code
							event/type: 'CLOSE
							Awake-Websocket event
							exit
						]
						actor/On-Read-Websocket ctx final? opcode
					]
				][
					log-error system/state/last-error
				]
				either ready? [
					;; there was complete input...
					write port either all [
						series? content: ctx/out/content
						not empty? content
					][
						content: to binary! content
						clear ctx/out/content
						len: length? content
						;print len
						;prin "out: " ? content
						bin: binary len
						binary/write bin case [
							len < 127                     [ [UI8 129 UI8          :len :content] ]
							all [ len > 126 len <= 65535 ][ [UI8 129 UI8 126 UI16 :len :content] ]
							len > 65535                   [ [UI8 129 UI8 127 UI64 :len :content] ]	  
						]
						head bin/buffer
					][	"" ]
				][
					;; needs more data!
					read port
				]
			]
			WROTE [
				read port
			]
			CLOSE [
				log-more ["Closing:^[[22m" ctx/remote]
				if pos: find ctx/parent/extra/clients port [ remove pos ]
				close port
			]
		]
	]


	New-Client: func[port [port!] /local client info err][
		client: first port
		info: query client
		unless Actor/On-Accept info [
			; connection not allowed
			log-more ["Client not accepted:^[[22m" info/remote-ip]
			close client
			return false
		]
		client/awake: :Awake-Client
		client/extra: make object! [
			state: none
			parent: port
			remote-ip: info/remote-ip
			remote: rejoin [tcp:// info/remote-ip #":" info/remote-port]
			inp: object [
				method:
				version:
				target:
				header:
				content: none
				started: stats/timer
			]
			out: object [
				Status: none
				Header: make map! 12
				Target: none
				Content: none
			]
			config: none
			timeout: none
			done?: none
			requests: 0   ; number of already served requests per connection
		]
		;? port
		client/extra/config: port/extra/config
		append port/extra/clients client

		log-more ["New client:^[[1;31m" client/extra/remote]
		try/with [read client][
			log-error ["Failed to read new client:" client/extra/remote]
			log-error system/state/last-error
		]
	]

	End-Client: function [port [port!]][
		ctx: port/extra
		Write-log ctx
		clients: ctx/parent/extra/clients
		keep-alive: ctx/config/keep-alive
		
		either all [
			keep-alive
			open? port
			ctx/requests <= keep-alive/2 ; limit to max requests
			"close" <> select port/extra/inp/Header 'Connection ; client don't want or cannot handle persistent connection
		][
			ctx/requests: ctx/requests + 1
			log-more ["Keep-alive:^[[22m" ctx/remote "requests:" ctx/requests]
			; reset client state
			foreach v ctx/inp [ctx/inp/:v: none]
			foreach v ctx/out [ctx/out/:v: none]
			ctx/out/Header: make map! 12
			ctx/state: none
			; set new timeout
			ctx/timeout: now + to time! ctx/config/keep-alive/1 
			read port
		][
			; close client connection
			Awake-Client make event! [type: 'CLOSE port: port]
			;try [remove find clients port]
		]
		log-debug ["Ports open:" length? clients]
		if ctx/done? [
			log-more "Server's job done, closing initiated"
			ctx/parent/data: ctx/done?
			Awake-Server make event! [type: 'CLOSE port: ctx/parent]
		]
	]

	Check-Clients: func[
		"Checks if some client connection isn't open too long time"
		port [port!] /local tm tmc
	][
		tm: now
		;log-debug ["Check-Clients:" length? port/state #"-" now]
		if block? port/state [
			foreach client reverse copy port/state [
				;log-debug ["Checking:" client/extra/remote client/extra/timeout]
				try [
					if all [
						date? tmc: client/extra/timeout
						tm >= tmc
					][
						Awake-Client make event! [type: 'CLOSE port: client]
					]
				]
			]
		]
	]

	anti-hacking-rules: [
		some [
			;; common scripts, which we don't use
			  #"." [
			  	  %php
			  	| %aspx
			  	| %cgi
			][end | #"?" | #"#"] reject
			; common hacking attempts to root folders...
			| #"/" [
				  %ecp/      ; we are not an exchange server
				| %mifs/     ; either not MobileIron (https://stackoverflow.com/questions/67901776/what-does-the-line-mifs-services-logservice-mean)
				| %GponForm/ ; nor Gpon router (https://www.vpnmentor.com/blog/critical-vulnerability-gpon-router/)
				| %.env end  ; https://stackoverflow.com/questions/64109005/do-these-env-get-requests-from-localhost-indicate-an-attack
			] reject
			| 1 skip
		]
	]

	;=====================================================================
	log-error: log-info: log-more: log-debug: none
	set-verbose: func[verbose [integer!]][
		log-error: log-info: log-more: log-debug: none
		case/all [
			verbose >= 0 [log-error: func[msg][sys/log/error 'HTTPD :msg]]
			verbose >= 1 [log-info:  func[msg][sys/log/info  'HTTPD :msg]]
			verbose >= 2 [log-more:  func[msg][sys/log/more  'HTTPD :msg]]
			verbose >= 3 [log-debug: func[msg][sys/log/debug 'HTTPD :msg]]
		]
		system/options/log/httpd: verbose
	]
	set-verbose 1
]

http-server: function [
	"Initialize simple HTTP server (DEPRECATED)"
	port  [integer!]        "Port to listen"
	/config                 "Possibility to change default settings"
	spec  [block! object!]  "Can hold: root, index, keep-alive, server-name"
	/actor  "Redefine default actor actions"
	actions [block! object!] "Functions like: On-Get On-Post On-Post-Received On-Read On-List-Dir On-Not-Found"
	/no-wait "Will not enter wait loop"
][
	sys/log/error 'HTTPD "`http-server` function is deprecated, use `start-http` instead!"
	spec: either config [[]][to block! spec]
	if actor [extend spec 'actor actions]
	extend spec 'port port
	start-http/:no-wait spec
]

serve-http: function [
	"Initiate a HTTP server and handle HTTP requests"
	spec [integer! file! block! object! map!] "Can hold: port, root, index, keep-alive, server-name, actor callbacks"
	/no-wait "Will not enter wait loop"
][
	case [
		integer? port: spec [
			spec: reduce/no-set [port: spec root: what-dir]
		]
		file? spec [
			root: dirize to-real-file clean-path spec
			port: 8000
			spec: reduce/no-set [port: port root: root]
		]
		'else [
			unless block? spec [spec: body-of spec]
			spec: reduce/no-set spec
			port: any [select spec 'port 8000] ;; default port
			root: select spec 'root
			if string? root [root: to-rebol-file root]
			if file?   root [
				;; to-real-file returns none when file does not exists on Posix
				;; that should be changed... also on Linux there is no trailing slash
				;; even when the source is a directory :-/ 
				spec/root: either exists? root [
					dirize to-real-file root
				][
					sys/log/error 'HTTPD ["Specified root not found:" as-red root]
					none
				]
			]
		]
	]

	server: open join httpd://: :port
	sys/log/info 'HTTPD ["Listening on port:" :port "with root directory:" as-green spec/root]
	
	if actions: select spec 'actor [
		append server/actor either block? actions [
			bind actions server/scheme
			reduce/no-set actions
		][	bind body-of actions server/scheme ]
		remove/part find spec 'actor 2 ;; not including actor in the config
	]
	append server/extra/config spec
	unless no-wait [
		forever [
			p: wait [server server/extra/subport 15]
			if all [port? p not open? p] [
				return p/data
			]
			server/scheme/Check-Clients server
		]
	]
	server
]
