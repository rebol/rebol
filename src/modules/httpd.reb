Rebol [
	Title: "HTTPD Scheme"
	Date: 02-Jul-2020
	Author: ["Andreas Bolka" "Christopher Ross-Gill" "Oldes"]
	File: %httpd.reb
	Name: 'httpd
	Type: 'module
	Version: 0.6.0
	Exports: [http-server decode-target to-CLF-idate]
	Rights: http://opensource.org/licenses/Apache-2.0
	Purpose: {
		A Tiny Webserver Scheme for Rebol 3 (Oldes' branch)
		Features:
		* handle basic POST, GET and HEAD methods
		* send large files in chunks (using a file port)
		* using _actors_ for main actions which may be customized
		* implemented `keep-alive` behaviour
		* sends `Not modified` response if file was not modified in given time
		* client can stop server
	}
	TODO: {
		* support for multidomain serving using `Host` header field
		* add support for other methods - PUT, DELETE, TRACE, CONNECT, OPTIONS?
		* better error handling
		* test in real life
	}
	Usage: {Check %tests/test-httpd.r3 script how to start a simple server}
	History: [
		04-Nov-2009 "Andreas Bolka" {A Tiny HTTP Server
		https://github.com/earl/rebol3/blob/master/scripts/shttpd.r}
		04-Jan-2017 "Christopher Ross-Gill" {Adaptation to Scheme
		https://gist.github.com/rgchris/73510e7d643eb0a6b9fa69b849cd9880}
		01-Apr-2019 "Oldes" {Rewritten to be usable in real life situations.}
		10-May-2020 "Oldes" {Implemented directory listing, logging and multipart POST processing}
		02-Jul-2020 "Oldes" {Added possibility to stop server and return data from client (useful for OAuth2)}
	]
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
		parse/all to binary! target [
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
	probe parse data [
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
	Name: 'httpd

	Actor: [
		Open: func [port [port!]][
			; probe port/spec
			sys/log/info 'HTTPD ["Opening server at port:^[[22m" port/spec/port-id]
			port/locals: make object! [
				subport: open compose [
					scheme: 'tcp
					port-id: (port/spec/port-id)
				]
				subport/awake: :port/scheme/awake-server
				subport/locals: make object! [
					parent: port
					config: none
					clients: make block! 16
				]
				subport/locals/config:
				config: make object! [
					root: system/options/home
					index: [%index.html %index.htm]
					keep-alive: true
					server-name: "Rebol3-HTTPD"
				]
			]
			port/state: port/locals/subport/locals/clients
			port/awake: :awake-client
			port
		]

		Close: func [port [port!]][
			sys/log/info 'HTTPD ["Closing server at port:^[[22m" port/spec/port-id]
			close port/locals/subport
		]

		On-Accept: func [ctx [object!]][ true ]
		On-Header: func [ctx [object!]][] ;= placeholder; user can use it for early request processing

		On-Get: func [
			ctx [object!]
			/local target path info index modified If-Modified-Since
		][
			target: ctx/inp/target
			target/file: path: join dirize ctx/config/root next clean-path/only target/file
			ctx/out/header/Date: to-idate/gmt now
			ctx/out/status: 200
			either exists? path [
				if dir? path [
					foreach file ctx/config/index [
						if exists? index: path/:file [
							path: index
							sys/log/debug 'HTTPD ["using index file:" index]
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

		On-Post: func[ctx [object!] /local content header length type temp][
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
				Actor/On-Post-Received ctx
			]
		]

		On-Post-Received: func [ctx [object!]][
			;@@ this is just a placeholder!
			true
		]

		On-Read: func[
			"Process READ action on client's port"
			ctx [object!]
		][
			switch/default ctx/inp/method [
				"HEAD" ; same like GET, but without sending content
				"GET"  [ Actor/on-get  ctx ]
				"POST" [ Actor/on-post ctx ]
			][
				ctx/state: 'data-received
				ctx/out/status: 405 ; Method Not Allowed
			]
		]

		On-List-Dir: func[
			ctx [object!] target [object!]
			/local path dir out size date files dirs
		][
			unless ctx/config/list-dir? [
				sys/log/more 'HTTPD ["Listing dir not allowed:^[[1m" mold target/file]
				ctx/out/status: 404 ; using not-found response!
				return false
			]
			dir: target/file
			path: join "/" find/match/tail dir ctx/config/root
		
			try/except [
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
			sys/log/more 'HTTPD ["Target not found:^[[1m" mold target/file]
			ctx/out/status: 404
		]
	]

	Status-Codes: make map! [
		100 "Continue"
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

	MIME-Types: make map! [
		%.txt   "text/plain"
		%.html  "text/html"
		%.htm   "text/html"
		%.js    "text/javascript"
		%.css   "text/css"
		%.csv   "text/csv"
		%.ics   "text/calendar"
		%.gif   "image/gif"
		%.png   "image/png"
		%.jpg   "image/jpeg"
		%.jpeg  "image/jpeg"
		%.ico   "image/x-icon"
		%.svg   "image/svg+xml"
		%.json  "application/json"
		%.pdf   "application/pdf"
		%.swf   "application/x-shockwave-flash"
		%.wasm  "application/wasm"
		%.xls   "application/vnd.ms-excel"
		%.xlsx  "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"
		%.zip   "application/zip"
		%.7z    "application/x-7z-compressed"
		%.doc   "application/msword"
		%.wav   "audio/wav"
		%.mid   "audio/midi"
		%.midi  "audio/x-midi"
		%.otf   "font/otf"
		%.ttf   "font/ttf"
		%.woff  "font/woff"
		%.woff2 "font/woff2"
		%.xhtml "application/xhtml+xml"
	]

	Respond: function [port [port!]][
		ctx: port/locals
		out: ctx/out
		sys/log/info 'HTTPD ["Respond:^[[22m" out/status status-codes/(out/status) length? out/content]
		; send the response header
		buffer: make binary! 1024
		append buffer ajoin ["HTTP/" ctx/inp/version #" " out/status #" " status-codes/(out/status) CRLF]

		unless out/header/Content-Type [
			if out/target [
				out/header/Content-Type: pick MIME-Types suffix? out/target
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

		try/except [
			write port buffer
		][
			;@@TODO: handle it without `print`; using on-error?
			print "** Write failed!"
			;probe copy/part buffer 100
			Awake-Client make event! [type: 'close port: port ]
		]
	]

	Do-log: function [ctx][
		try/except [
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
			if file? file: select ctx/config 'log-access [
				write/append file msg
			]
			if all [
				ctx/out/status >= 400
				file? file: select ctx/config 'log-errors
			][
				write/append file msg
			]
		][
			print "** Failed to write a log"
			print system/state/last-error
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
			ctx: port/locals
			inp: ctx/inp
			out: ctx/out

			sys/log/more 'HTTPD ["Awake:^[[1m" ctx/remote "^[[22m" event/type]

			ctx/timeout: now + 0:0:15

			switch event/type [
				READ [
					sys/log/more 'HTTPD ["bytes:^[[1m" length? port/data]
					either header-end: find/tail port/data CRLF2BIN [
						try/except [
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
								sys/log/info 'HTTPD ["Request header:^[[22m" ctx/inp/method mold ctx/inp/header]
								; on-header actor may be used for rewrite rules (redirection)
								actor/on-header ctx
								if ctx/out/status [
								; if status is defined (probably redirection), than we can send a response now
									respond port
									break
								]
							]
							actor/on-read port/locals
						][
							print system/state/last-error
							ctx/state: 'error
							ctx/out/status: 500 ; Internal Server Error
						]
						sys/log/debug 'HTTPD ["State:^[[1m" ctx/state "^[[22mstatus:^[[1m" out/status]
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
									try/except [
										write port buffer
									][
										print "** Write failed (2)!"
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
					sys/log/info 'HTTPD ["Closing:^[[22m" ctx/remote]
					if pos: find ctx/parent/locals/clients port [ remove pos ]
					close port
				]
			]
		]
	]

	Awake-Server: func [event [event!] /local ctx client config] [
		sys/log/debug 'HTTPD ["Awake (server):^[[22m" event/type]
		switch event/type [
			ACCEPT [ New-Client event/port ]
			CLOSE  [
				close event/port
				close event/port/locals/parent
			]
		]
		true
	]



	New-Client: func[port [port!] /local client info err][
		client: first port
		info: query client
		unless Actor/On-Accept info [
			; connection not allowed
			sys/log/info 'HTTPD ["Client not accepted:^[[22m" info/remote-ip]
			close client
			return false
		]
		client/awake: :Awake-Client
		client/locals: make object! [
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
		client/locals/config: port/locals/config
		append port/locals/clients client

		sys/log/info 'HTTPD ["New client:^[[1;31m" client/locals/remote]
		try/except [read client][
			print ["** Failed to read new client:" client/locals/remote]
			print system/state/last-error
		]
	]

	End-Client: function [port [port!]][
		ctx: port/locals
		Do-log ctx
		clients: ctx/parent/locals/clients
		keep-alive: ctx/config/keep-alive
		
		either all [
			keep-alive
			open? port
			ctx/requests <= keep-alive/2 ; limit to max requests
			"close" <> select port/locals/inp/Header 'Connection ; client don't want or cannot handle persistent connection
		][
			ctx/requests: ctx/requests + 1
			sys/log/info 'HTTPD ["Keep-alive:^[[22m" ctx/remote "requests:" ctx/requests]
			; reset client state
			foreach v ctx/inp [ctx/inp/:v: none]
			foreach v ctx/out [ctx/out/:v: none]
			ctx/out/Header: make map! 12
			ctx/state: none
			; set new timeout
			ctx/timeout: now + ctx/config/keep-alive/1 
			read port
		][
			; close client connection
			Awake-Client make event! [type: 'CLOSE port: port]
			;try [remove find clients port]
		]
		sys/log/debug 'HTTPD ["Ports open:" length? clients]
		if all [ctx/done? zero? length? clients][
			sys/log/info 'HTTPD "Server's job done, closing initiated"
			ctx/parent/data: ctx/done?
			Awake-Server make event! [type: 'CLOSE port: ctx/parent]
		]
	]

	Check-Clients: func[
		"Checks if some client connection isn't open too long time"
		port [port!] /local tm tmc
	][
		tm: now
		;sys/log/debug 'HTTPD ["Check-Clients:" length? port/state #"-" now]
		if block? port/state [
			foreach client reverse copy port/state [
				;sys/log/debug 'HTTPD ["Checking:" client/locals/remote client/locals/timeout]
				try [
					if all [
						date? tmc: client/locals/timeout
						tm >= tmc
					][
						Awake-Client make event! [type: 'CLOSE port: client]
					]
				]
			]
		]
	]
]

http-server: function [
	"Initialize simple HTTP server"
	port  [integer!]        "Port to listen"
	/config                 "Possibility to change default settings"
	spec  [block! object!]  "Can hold: root, index, keep-alive, server-name"
	/actor  "Redefine default actor actions"
	actions [block! object!] "Functions like: On-Get On-Post On-Post-Received On-Read On-List-Dir On-Not-Found"
	/no-wait "Will not enter wait loop"
][
	server: open join httpd://: port
	if config [
		if object? spec [ spec: body-of spec ]
		case [
			file? spec/root [spec/root: dirize clean-path spec/root]
			none? spec/root [spec/root: what-dir]
		]
		append server/locals/config spec
	]
	
	unless system/options/quiet [
		? server/locals/config
	]

	if actor [
		append server/actor either block? actions [
			reduce/no-set actions
		][	body-of actions ]
	]
	unless no-wait [
		forever [
			p: wait [server server/locals/subport 15]
			if all [port? p not open? p] [
				return p/data
			]
			server/scheme/Check-Clients server
		]
	]
	server
]