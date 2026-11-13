Rebol [
	Title:  "WebSocket scheme and codec"
	Type:    module
	Name:    websocket
	Date:    24-Oct-2025
	Version: 0.3.0
	Author:  @Oldes
	Home:    https://github.com/Oldes/Rebol-WebSocket
	Rights:  http://opensource.org/licenses/Apache-2.0
	Purpose: {Communicate with a server over WebSocket's connection.}
	History: [
		01-Jan-2024 "Oldes" {Initial version}
	]
	Needs: [3.11.0] ;; used bit/hexadecimal integer syntax
]

;--- WebSocket Codec --------------------------------------------------
system/options/log/ws: 2
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
		"Decodes WebSocket frames from a given input."
		data [binary!] "Consumed data are removed! (modified)"
	][
		out: copy []
		bin: binary data
		;; minimal WebSocket message has 2 bytes at least (when no masking involved)
		unless while [2 < length? bin/buffer][
			msg-start: bin/buffer
			;@@TODO: Rewrite when bincode supports reading bits.
			binary/read bin [b1: UI8 b2: UI8]
			final?: b1 & 2#10000000 == 2#10000000
			opcode: b1 & 2#00001111
			mask?:  b2 & 2#10000000 == 2#10000000
			len:    b2 & 2#01111111

			case [
				len = 126 [
					;; there must be at least 2 bytes for the message length
					if 2 >= length? bin/buffer [break/return false]
					len: binary/read bin 'UI16
				]
				len = 127 [
					if 8 >= length? bin/buffer [break/return false]
					len: binary/read bin 'UI64
				]
			]

			sys/log/debug 'WS ["opcode:" opcode "final?" final? "mask?" mask? "len:" pad len 6 "avail:" length? bin/buffer]

			if ((pick [4 0] mask?) + length? bin/buffer) < len [break/return false]
			
			either mask? [
				masks: binary/read bin 4
				temp: masks xor binary/read bin :len
				if len < 4 [clear skip temp len] ;; the mask was longer then the message
			][
				temp: binary/read bin :len
			]
			if all [final? opcode = 1] [try [temp: to string! temp]]
			append append append out :final? :opcode :temp
		][
			sys/log/debug 'WS ["Need data:" len "has:" length? bin/buffer]
			bin/buffer: msg-start ;; reset position to the head of the message
		]
		data: truncate at data index? bin/buffer 
		out
	]
]

ws-encode: :codecs/ws/encode
ws-decode: :codecs/ws/decode

;--- WebSocket Scheme -------------------------------------------------
ws-conn-awake: func [event /local port extra parent spec temp] [
	port: event/port
	;; wakeup if there is no parent
	unless parent: port/parent [return true]
	extra: parent/extra
	sys/log/debug 'WS ["==TCP-event:" as-red event/type]
	either extra/handshake [
		switch event/type [
			read [
				;print ";; TCP read" probe port/data 
				append extra/buffer take/all port/data
			]
		]
		insert system/ports/system make event! [ type: event/type port: parent ]
		port
	][
		switch/default event/type [
			;- Upgrading from HTTP to WS...
			read [
				;print ["^/read:" length? port/data]
				append extra/buffer port/data
				clear port/data
				;probe to string! parent/data
				either find extra/buffer #{0D0A0D0A} [
					;; parse response header...
					try/with [
						;; skip the first line and construct response fields
						extra/fields: temp: construct find/tail extra/buffer #{0D0A}
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
					clear extra/buffer
					extra/handshake: true
					insert system/ports/system make event! [ type: 'connect port: parent ]
				][
					;; missing end of the response header...			
					read port ;; keep reading...
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
	awake: func [event /local port ctx raw frames] [
		;; This is just a default awake handler...
		;; one may want to redefine it for a real life use!
		port: event/port
		ctx: port/extra
		raw: ctx/buffer  ;; used to store unprocessed raw data
		sys/log/more 'WS ["WS-event:" as-red event/type port/spec/ref]
		switch event/type [
			read  [
				;sys/log/debug 'WS ["== raw-data:" as-blue mold/flat/part raw 100]
				;; Decode Websocket frames
				if empty? frames: ws-decode raw [
					sys/log/debug 'WS "data not complete..."
					read port    ;; keep reading...
					return false ;; don't wake up yet...
				]
				;?? frames
				
				;; A WebSocket message can be split into multiple frames (fragments).
				;; The first frame in the sequence has an opcode for the data type (text or binary),
				;; and subsequent frames (except the last) are continuation frames with opcode 0.
				;; The final frame in the fragmented sequence has the FIN bit set to 1,
				;; signaling the end of the message.
				foreach [fin op msg] frames [
					if ctx/fragment-type [
						;; Append fragment data to an existing fragment buffer
						append ctx/fragment msg
						either fin [
							;; Final fragment, so prepare complete message
							msg: copy ctx/fragment
							op:  ctx/fragment-type
							;; And clear the fragment buffer state
							clear ctx/fragment
							ctx/fragment-type: false
						][  continue ]
					]
					either fin [
						;; Complete message
						if op == 1 [try/with [msg: to string! msg][ sys/log/error system/state/last-error] ]
						;; Queue for the parent actor
						append port/data msg
					][
						;; First frame
						ctx/fragment-type: op
						append ctx/fragment msg
					]
				]
				sys/log/more 'WS ["Queued messages:" as-yellow length? port/data]
				; Notify the parent port
				insert system/ports/system make event! [ type: 'read port: port/parent ]
			]
			wrote [
				;; don't wake up and instead wait for a response...
				read port
				return false
			]
			connect [
				;; optional validation of response headers
				sys/log/debug 'WS ["Connect response:" mold/flat ctx/fields]
			]
			error [
				sys/log/info 'WS "Closing..."
				try [close ctx/connection]
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
				buffer:   make binary! 200 ;; used to hold undecoded raw websocket data
				fragment: make binary! 100 ;; used to hold fragmented message
				fragment-type: false ;; type of the fragmented message 
			]
			port/data: copy [] ;; used to hold decoeded packets

			;; `ref` is used in logging and errors
			conn: make port/spec [ref: none]
			conn/scheme: 'tcp
			port-spec: if spec/port [join #":" spec/port]
			conn/ref: as url! ajoin [conn/scheme "://" spec/host port-spec]
			unless url? spec/ref [
				spec/ref: as url! ajoin ["ws://" spec/host port-spec spec/path spec/target]
			]
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
			sys/log/debug 'WS ["Write:" as-green mold/flat data]
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