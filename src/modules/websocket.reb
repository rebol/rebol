Rebol [
	Title:  "WebSocket scheme and codec"
	Type:    module
	Name:    websocket
	Date:    01-Jan-2024
	Version: 0.1.0
	Author:  @Oldes
	Exports: [http-server decode-target to-CLF-idate]
	Home:    https://github.com/Oldes/Rebol-WebSocket
	Rights:  http://opensource.org/licenses/Apache-2.0
	Purpose: {Communicate with a server over WebSocket's connection.}
	History: [
		01-Jan-2024 "Oldes" {Initial version}
	]
	Needs: [3.11.0]
]

;--- WebSocket Codec --------------------------------------------------
append system/options/log [ws: 4]
system/options/quiet: false
register-codec [
	name:  'ws
	type:  'text
	title: "WebSocket"

	encode: function/with [
		"Encodes one WebSocket message."
		data [binary! any-string! word! map!]
		/no-mask
	][
		case [
			data = 'ping  [return #{81801B1F519C}]
			data = 'close [return #{888260D19A196338}]
			map?  data [data: to-json data]
			word? data [data: form data]
		]
		out: clear #{}
		;; first byte has FIN bit and an opcode (if data are string or binary data)
		byte1: either binary? data [2#10000010][2#10000001] ;; final binary/string
		unless binary? data [data: to binary! data]
		len:   length? data
		either no-mask [
			binary/write out case [
				len <  0#007E [[UI8 :byte1 UI8 :len :data]]
				len <= 0#FFFF [[UI8 :byte1 UI8 126 UI16 :len :data]]
				'else         [[UI8 :byte1 UI8 127 UI64 :len :data]]
			]
		][
			;; update a mask...
			repeat i 4 [mask/:i: 1 + random 254] ;; avoiding zero
			data: data xor mask
			binary/write out case [
				len <  0#007E [byte2: 2#10000000 | len [UI8 :byte1 UI8 :byte2 :mask :data]]
				len <= 0#FFFF [[UI8 :byte1 UI8 254 UI16 :len :mask :data]]
				'else         [[UI8 :byte1 UI8 255 UI64 :len :mask :data]]
			]
		]
		out
	][
		mask: #{00000000}
		out:  make binary! 100
	]

	decode: function [
		"Decodes WebSocket messages from a given input."
		data [binary!] "Consumed data are removed! (modified)"
	][
		out: clear []
		;; minimal WebSocket message has 2 bytes at least (when no masking involved)
		while [2 < length? data][
			final?: data/1 & 2#10000000 = 2#10000000
			opcode: data/1 & 2#00001111
			mask?:  data/2 & 2#10000000 = 2#10000000
			len:    data/2 & 2#01111111
			data: skip data 2

			;@@ Not removing bytes until we make sure, that there is enough data!
			case [
				len = 126 [
					;; there must be at least 2 bytes for the message length
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
				masks: take/part data 4
				temp: masks xor take/part data len
				if len < 4 [truncate/part temp len] ;; the mask was longer then the message
			][	temp: take/part data len ]
			if all [final? opcode = 1] [try [temp: to string! temp]]
			append append append out :final? :opcode :temp
		]
		out
	]
]

ws-encode: :codecs/ws/encode
ws-decode: :codecs/ws/decode

;--- WebSocket Scheme -------------------------------------------------
ws-conn-awake: func [event /local port extra parent spec temp] [
	port: event/port
	unless parent: port/parent [return true]
	extra: parent/extra
	sys/log/more 'WS ["==TCP-event:" as-red event/type]
	either extra/handshake [
		switch event/type [
			read [
				append parent/data port/data
				clear port/data
			]
		]
		insert system/ports/system make event! [ type: event/type port: parent ]
		port
	][
		switch/default event/type [
			;- Upgrading from HTTP to WS...]
			read [
				;print ["^/read:" length? port/data]
				append parent/data port/data
				clear port/data
				;probe to string! parent/data
				either find parent/data #{0D0A0D0A} [
					;; parse response header...
					try/with [
						;; skip the first line and construct response fields
						extra/fields: temp: construct find/tail parent/data #{0D0A}
						unless all [
							"websocket" = select temp 'Upgrade
							"Upgrade"   = select temp 'Connection
							extra/key   = select temp 'Sec-WebSocket-Accept
						][
							insert system/ports/system make event! [ type: 'error port: parent ]
							return true
						]
						
					] :print

					clear port/data
					clear parent/data
					extra/handshake: true
					insert system/ports/system make event! [ type: 'connect port: parent ]
				][
					;; missing end of the response header...			
					read port
				]
			]
			wrote  [read port]
			lookup [open port]
			connect [
				spec: parent/spec
				extra/key: enbase/part checksum form now/precise 'sha1 64 16
				write port ajoin [
					{GET } spec/path spec/target { HTTP/1.1} CRLF
					{Host: } spec/host if spec/port [join #":" spec/port] CRLF
					{Upgrade: websocket} CRLF
					{Connection: Upgrade} CRLF
					{Sec-WebSocket-Key: } extra/key CRLF
					{Sec-WebSocket-Protocol: chat, superchat} CRLF
					{Sec-WebSocket-Version: 13} CRLF
					CRLF
				]
				extra/key: enbase checksum join extra/key "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" 'sha1 64
			]
		][true]
	]
]
sys/make-scheme [
	name: 'ws
	title: "Websocket"
	spec: make system/standard/port-spec-net []
	awake: func [event /local port extra parent spec temp] [
		port: event/port
		sys/log/debug 'WS ["== WS-event:" as-red event/type]
		switch event/type [
			read  [
				sys/log/debug 'WS ["== raw-data:" as-blue port/data]
				ws-decode port/data 
			]
			wrote []
			connect [
				;; optional validation of response headers
				?? port/extra/fields
			]
			error [
				print "closing..."
				try [close port/extra/connection]
				;wait port/extra/connection
			]
		]
		true
	]
	actor: [
		open: func [port [port!] /local spec host conn port-spec][
			spec: port/spec
			port/extra: context [
				connection:
				key:
				handshake:
				fields: none
			]
			port/data: make binary! 200
			;; `ref` is used in logging and errors
			conn: make port/spec [ref: none]
			conn/scheme: 'tcp
			port-spec: if spec/port [join #":" spec/port]
            conn/ref: as url! ajoin [conn/scheme "://" spec/host port-spec]
            spec/ref: as url! ajoin ["ws://" spec/host port-spec]
			port/extra/connection: conn: make port! conn
			conn/parent: port
			conn/awake: :ws-conn-awake
			open conn
			port
		]
		open?: func[port /local ctx][
			all [
				ctx: port/extra
				ctx/handshake
				open? ctx/connection
			]
		]
		close: func[port][
			close port/extra/connection
		]
		write: func[port data][
			sys/log/debug 'WS ["write:" as-green mold data]
			either open? port [
				write port/extra/connection ws-encode data
			][	sys/log/error 'WS "Not open!"]
			
		]
		read: func[port][
			either open? port [
				read port/extra/connection
			][	sys/log/error 'WS "Not open!"]
			
		]
	]
]