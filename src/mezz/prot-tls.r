REBOL [
	title: "REBOL 3 TLSv1.0 protocol scheme"
	name: 'tls
	type: 'module
	author: rights: "Richard 'Cyphre' Smolak"
	version: 0.6.1
	todo: {
		-cached sessions
		-automagic cert data lookup
		-add more cipher suites (based on DSA, 3DES, ECDH, ECDHE, ECDSA, SHA256, SHA384 ...)
		-server role support
		-SSL3.0, TLS1.1/1.2 compatibility
		-cert validation
	}
]

; support functions

debug:
;:print
none

emit: func [
	ctx [object!]
	code [block! binary!]
] [
	repend ctx/msg code
]

to-bin: func [
	val [integer!]
	width [integer!]
] [
	skip tail to binary! val negate width
]

make-tls-error: func [
	"Make an error for the TLS protocol"
	message [string! block!]
] [
	if block? message [message: ajoin message]
	make error! [
		type: 'Access
		id: 'Protocol
		arg1: message
	]
]

tls-error: func [
	"Throw an error for the TLS protocol"
	message [string! block!]
] [
	do make-tls-error message
]

cipher-suites: make object! [
	TLS_RSA_WITH_RC4_128_MD5:				#{00 04}
	TLS_RSA_WITH_RC4_128_SHA:				#{00 05}
	TLS_RSA_WITH_AES_128_CBC_SHA:			#{00 2F}
	TLS_RSA_WITH_AES_256_CBC_SHA:			#{00 35}
	TLS_DHE_DSS_WITH_AES_128_CBC_SHA:		#{00 32}
	TLS_DHE_DSS_WITH_AES_256_CBC_SHA:		#{00 38}
	TLS_DHE_RSA_WITH_AES_128_CBC_SHA:		#{00 33}
	TLS_DHE_RSA_WITH_AES_256_CBC_SHA:		#{00 39}
]

; ASN.1 format parser code

universal-tags: [
	eoc
	boolean
	integer
	bit-string
	octet-string
	null
	object-identifier
	object-descriptor
	external
	real
	enumerated
	embedded-pdv
	utf8string
	relative-oid
	undefined
	undefined
	sequence
	set
	numeric-string
	printable-string
	t61-string
	videotex-string
	ia5-string
	utc-time
	generalized-time
	graphic-string
	visible-string
	general-string
	universal-string
	character-string
	bmp-string
]

class-types: [universal application context-specific private]

parse-asn: func [
	data [binary!]
	/local
		mode d constructed? class tag ln length result val
] [
	result: make block! 16
	mode: 'type

	while [d: data/1] [
		switch mode [
			type [
				constructed?: not zero? (d and 32)
				class: pick class-types 1 + shift d -6

				switch class [
					universal [
						tag: pick universal-tags 1 + (d and 31)
					]
					context-specific [
						tag: class
						val: d and 31
					]
				]
				mode: 'length
			]

			length [
				length: d and 127
				unless zero? (d and 128) [
					; long form
					ln: length
					length: to integer! copy/part next data length
					data: skip data ln
				]
				either zero? length [
					append/only result compose/deep [(tag) [(either constructed? ["constructed"] ["primitive"]) (index? data) (length) #[none]]]
					mode: 'type
				] [
					mode: 'value
				]
			]

			value [
				switch class [
					universal [
						val: copy/part data length
						append/only result compose/deep [(tag) [(either constructed? ["constructed"] ["primitive"]) (index? data) (length) (either constructed? [none] [val])]]
						if constructed? [
							poke second last result 4
							parse-asn val
						]
					]

					context-specific [
						append/only result compose/deep [(tag) [(val) (length)]]
						parse-asn copy/part data length
					]
				]

				data: skip data length - 1
				mode: 'type
			]
		]

		data: next data
	]
	result
]

; protocol state handling

read-proto-states: [
	client-hello [server-hello]
	server-hello [certificate]
	certificate [server-hello-done server-key-exchange]
	server-key-exchange [server-hello-done]
	server-hello-done [#complete]
	finished [change-cipher-spec alert]
	change-cipher-spec [encrypted-handshake]
	encrypted-handshake [application #complete]
	application [application alert #complete]
	alert [#complete]
	close-notify [alert]
]

write-proto-states: [
	server-hello-done [client-key-exchange]
	client-key-exchange [change-cipher-spec]
	change-cipher-spec [finished]
	encrypted-handshake [application]
	application [application alert]
	alert [close-notify]
	close-notify []
]

get-next-proto-state: func [
	ctx [object!]
	/write-state "default is read state"
	/local
		next-state
] [
	all [
		next-state: select/only/skip either write-state [write-proto-states] [read-proto-states] ctx/protocol-state 2
		not empty? next-state
		next-state
	]
]

update-proto-state: func [
	ctx [object!]
	new-state [word!]
	/write-state
	/local
		next-state
] [
	debug [ctx/protocol-state "->" new-state write-state]
	either any [
		none? ctx/protocol-state
		all [
			next-state: apply :get-next-proto-state [ctx write-state]
			find next-state new-state
		]
	] [
		debug ["new-state:" new-state]
		ctx/protocol-state: new-state
	] [
		do make error! "invalid protocol state"
	]
]

; TLS protocol code

client-hello: func [
	ctx [object!]
	/local
		beg len cs-data
] [
	; generate client random struct
	ctx/client-random: to-bin to integer! difference now/precise 1-Jan-1970 4
	random/seed now/time/precise
	loop 28 [append ctx/client-random (random/secure 256) - 1]

	cs-data: rejoin values-of cipher-suites

	beg: length? ctx/msg
	emit ctx [
		#{16}						; protocol type (22=Handshake)
		ctx/version					; protocol version (3|1 = TLS1.0)
		#{00 00}					; length of SSL record data
		#{01}						; protocol message type	(1=ClientHello)
		#{00 00 00} 				; protocol message length
		ctx/version					; max supported version by client (TLS1.0)
		ctx/client-random			; random struct (4 bytes gmt unix time + 28 random bytes)
		#{00}						; session ID length
		to-bin length? cs-data 2	; cipher suites length
		cs-data						; cipher suites list
		#{01}						; compression method length
		#{00}						; no compression
	]

	; set the correct msg lengths
	change at ctx/msg beg + 7 to-bin len: length? at ctx/msg beg + 10 3
	change at ctx/msg beg + 4 to-bin len + 4 2

	append clear ctx/handshake-messages copy at ctx/msg beg + 6

	return ctx/msg
]

client-key-exchange: func [
	ctx [object!]
	/local
	rsa-key key-data beg len
] [
	switch ctx/key-method [
		rsa [
			; generate pre-master-secret
			ctx/pre-master-secret: copy ctx/version
			random/seed now/time/precise
			loop 46 [append ctx/pre-master-secret (random/secure 256) - 1]

			; encrypt pre-master-secret
			rsa-key: rsa-make-key
			rsa-key/e: ctx/pub-exp
			rsa-key/n: ctx/pub-key

			; supply encrypted pre-master-secret to server
			key-data: rsa ctx/pre-master-secret rsa-key
		]
		dhe-dss dhe-rsa [
			; generate public/private keypair
			dh-generate-key ctx/dh-key

			; supply the client's public key to server
			key-data: ctx/dh-key/pub-key

			; generate pre-master-secret
			ctx/pre-master-secret: dh-compute-key ctx/dh-key ctx/dh-pub
		]
	]

	beg: length? ctx/msg
	emit ctx [
		#{16}						; protocol type (22=Handshake)
		ctx/version					; protocol version (3|1 = TLS1.0)
		#{00 00}					; length of SSL record data
		#{10}						; protocol message type	(16=ClientKeyExchange)
		#{00 00 00} 				; protocol message length
		to-bin length? key-data 2	; length of the key (2 bytes)
		key-data
	]

	; set the correct msg lengths
	change at ctx/msg beg + 7 to-bin len: length? at ctx/msg beg + 10 3
	change at ctx/msg beg + 4 to-bin len + 4 2

	; make all secure data
	make-master-secret ctx ctx/pre-master-secret

	make-key-block ctx

	; update keys
	ctx/client-mac-key: copy/part ctx/key-block ctx/hash-size
	ctx/server-mac-key: copy/part skip ctx/key-block ctx/hash-size ctx/hash-size
	ctx/client-crypt-key: copy/part skip ctx/key-block 2 * ctx/hash-size ctx/crypt-size
	ctx/server-crypt-key: copy/part skip ctx/key-block 2 * ctx/hash-size + ctx/crypt-size ctx/crypt-size

	if ctx/block-size [
		ctx/client-iv: copy/part skip ctx/key-block 2 * (ctx/hash-size + ctx/crypt-size) ctx/block-size
		ctx/server-iv: copy/part skip ctx/key-block 2 * (ctx/hash-size + ctx/crypt-size) + ctx/block-size ctx/block-size
	]

	append ctx/handshake-messages copy at ctx/msg beg + 6

	return ctx/msg
]


change-cipher-spec: func [
	ctx [object!]
] [
	emit ctx [
		#{14}			; protocol type (20=ChangeCipherSpec)
		ctx/version		; protocol version (3|1 = TLS1.0)
		#{00 01}		; length of SSL record data
		#{01}			; CCS protocol type
	]
	return ctx/msg
]

encrypted-handshake-msg: func [
	ctx [object!]
	message [binary!]
	/local
		plain-msg
] [
	plain-msg: message
	message: encrypt-data/type ctx message #{16}
	emit ctx [
		#{16}						; protocol type (22=Handshake)
		ctx/version					; protocol version (3|1 = TLS1.0)
		to-bin length? message 2	; length of SSL record data
		message
	]
	append ctx/handshake-messages plain-msg
	return ctx/msg
]

application-data: func [
	ctx [object!]
	message [binary! string!]
] [
	message: encrypt-data ctx to binary! message
	emit ctx [
		#{17}						; protocol type (23=Application)
		ctx/version					; protocol version (3|1 = TLS1.0)
		to-bin length? message 2	; length of SSL record data
		message
	]
	return ctx/msg
]

alert-close-notify: func [
	ctx [object!]
] [
	message: encrypt-data ctx #{0100} ; close notify
	emit ctx [
		#{15}						; protocol type (21=Alert)
		ctx/version					; protocol version (3|1 = TLS1.0)
		to-bin length? message 2	; length of SSL record data
		message
	]
	return ctx/msg
]


finished: func [
	ctx [object!]
] [
	ctx/seq-num-w: 0
	return rejoin [
		#{14}		; protocol message type	(20=Finished)
		#{00 00 0c} ; protocol message length (12 bytes)
		prf ctx/master-secret either ctx/server? ["server finished"] ["client finished"] rejoin [
			checksum/method ctx/handshake-messages 'md5 checksum/method ctx/handshake-messages 'sha1
		] 12
	]
]

encrypt-data: func [
	ctx [object!]
	data [binary!]
	/type
		msg-type [binary!] "application data is default"
	/local
		mac padding len
] [
		data: rejoin [
		data
		; MAC code
		mac: checksum/method/key rejoin [
			to-bin ctx/seq-num-w 8				; sequence number (64-bit int in R3)
			any [msg-type #{17}]				; msg type
			ctx/version							; version
			to-bin length? data 2				; msg content length
			data								; msg content
		] ctx/hash-method decode 'text ctx/client-mac-key
	]

	if ctx/block-size [
		; add the padding data in CBC mode
		padding: ctx/block-size - (1 + (length? data) // ctx/block-size)
		len: 1 + padding
		append data head insert/dup make binary! len to-bin padding 1 len
	]

	switch ctx/crypt-method [
		rc4 [
			unless ctx/encrypt-stream [
				ctx/encrypt-stream: rc4/key ctx/client-crypt-key
			]
			rc4/stream ctx/encrypt-stream data
		]
		aes [
			unless ctx/encrypt-stream [
				ctx/encrypt-stream: aes/key ctx/client-crypt-key ctx/client-iv
			]
			data: aes/stream ctx/encrypt-stream data
		]
	]

	return data
]

decrypt-data: func [
	ctx [object!]
	data [binary!]
	/local
		crypt-data
] [
	switch ctx/crypt-method [
		rc4 [
			unless ctx/decrypt-stream [
				ctx/decrypt-stream: rc4/key ctx/server-crypt-key
			]
			rc4/stream ctx/decrypt-stream data
		]
		aes [
			unless ctx/decrypt-stream [
				ctx/decrypt-stream: aes/key/decrypt ctx/server-crypt-key ctx/server-iv
			]
			data: aes/stream ctx/decrypt-stream data
		]
	]

	return data
]

protocol-types: [
	20 change-cipher-spec
	21 alert
	22 handshake
	23 application
]

message-types: [
	0 hello-request
	1 client-hello
	2 server-hello
	11 certificate
	12 server-key-exchange
	13 certificate-request
	14 server-hello-done
	15 certificate-verify
	16 client-key-exchange
	20 finished
]

alert-descriptions: [
	0 "Close notify"
	10 "Unexpected message"
	20 "Bad record MAC"
	21 "Decryption failed"
	22 "Record overflow"
	30 "Decompression failure"
	40 "Handshake failure"
	41 "No certificate"
	42 "Bad certificate"
	43 "Unsupported certificate"
	44 "Certificate revoked"
	45 "Certificate expired"
	46 "Certificate unknown"
	47 "Illegal parameter"
	48 "Unknown CA"
	49 "Access denied"
	50 "Decode error"
	51 "Decrypt error"
	60 "Export restriction"
	70 "Protocol version"
	71 "Insufficient security"
	80 "Internal error"
	90 "User cancelled"
   100 "No renegotiation"
   110 "Unsupported extension"
]

parse-protocol: func [
	data [binary!]
	/local proto
] [
	unless proto: select protocol-types data/1 [
		do make error! "unknown/invalid protocol type"
	]
	return context [
		type: proto
		version: pick [ssl-v3 tls-v1.0 tls-v1.1] data/3 + 1
		length: to integer! copy/part at data 4 2
		messages: copy/part at data 6 length
	]
]

parse-messages: func [
	ctx [object!]
	proto [object!]
	/local
		result data msg-type len clen msg-content mac msg-obj
] [
	result: make block! 8
	data: proto/messages

	if ctx/encrypted? [
		change data decrypt-data ctx data
		debug ["decrypting..."]
		if ctx/block-size [
			; deal with padding in CBC mode
			data: copy/part data (length? data) - 1 - (to integer! last data)
			debug ["depadding..."]
		]
		debug ["data:" data]
	]
	debug [ctx/seq-num-r ctx/seq-num-w "READ <--" proto/type]

	unless proto/type = 'handshake [
		update-proto-state ctx proto/type
	]

	switch proto/type [
		alert [
			append result reduce [
				context [
					level: any [pick [warning fatal] data/1 'unknown]
					description: any [select alert-descriptions data/2 "unknown"]
				]
			]
		]
		handshake [
			while [data/1] [
				msg-type: select message-types data/1

				update-proto-state ctx either ctx/encrypted? ['encrypted-handshake] [msg-type]

				len: to integer! copy/part at data 2 3
				append result switch msg-type [
					server-hello [
						msg-content: copy/part at data 7 len

						msg-obj: context [
							type: msg-type
							version: pick [ssl-v3 tls-v1.0 tls-v1.1] data/6 + 1
							length: len
							server-random: copy/part msg-content 32
							session-id: copy/part at msg-content 34 msg-content/33
							cipher-suite: copy/part at msg-content 34 + msg-content/33 2
							compression-method-length: first at msg-content 36 + msg-content/33
							compression-method: either compression-method-length = 0 [none] [copy/part at msg-content 37 + msg-content/33 compression-method-length]
						]
						ctx/cipher-suite: msg-obj/cipher-suite

						; note: the cipher-suite config will be more automatized in later versions
						switch/default ctx/cipher-suite reduce bind [
							TLS_RSA_WITH_RC4_128_SHA [
								ctx/key-method: 'rsa
								ctx/crypt-method: 'rc4
								ctx/crypt-size: 16
								ctx/hash-method: 'sha1
								ctx/hash-size: 20
							]
							TLS_RSA_WITH_RC4_128_MD5 [
								ctx/key-method: 'rsa
								ctx/crypt-method: 'rc4
								ctx/crypt-size: 16
								ctx/hash-method: 'md5
								ctx/hash-size: 16
							]
							TLS_RSA_WITH_AES_128_CBC_SHA [
								ctx/key-method: 'rsa
								ctx/crypt-method: 'aes
								ctx/crypt-size: 16
								ctx/block-size: 16
								ctx/iv-size: 16
								ctx/hash-method: 'sha1
								ctx/hash-size: 20
							]
							TLS_RSA_WITH_AES_256_CBC_SHA [
								ctx/key-method: 'rsa
								ctx/crypt-method: 'aes
								ctx/crypt-size: 32
								ctx/block-size: 16
								ctx/iv-size: 16
								ctx/hash-method: 'sha1
								ctx/hash-size: 20
							]
							TLS_DHE_DSS_WITH_AES_128_CBC_SHA [
								ctx/key-method: 'dhe-dss
								ctx/crypt-method: 'aes
								ctx/crypt-size: 16
								ctx/block-size: 16
								ctx/iv-size: 16
								ctx/hash-method: 'sha1
								ctx/hash-size: 20
							]
							TLS_DHE_DSS_WITH_AES_256_CBC_SHA [
								ctx/key-method: 'dhe-dss
								ctx/crypt-method: 'aes
								ctx/crypt-size: 32
								ctx/block-size: 16
								ctx/iv-size: 16
								ctx/hash-method: 'sha1
								ctx/hash-size: 20
							]
							TLS_DHE_RSA_WITH_AES_128_CBC_SHA [
								ctx/key-method: 'dhe-rsa
								ctx/crypt-method: 'aes
								ctx/crypt-size: 16
								ctx/block-size: 16
								ctx/iv-size: 16
								ctx/hash-method: 'sha1
								ctx/hash-size: 20
							]
							TLS_DHE_RSA_WITH_AES_256_CBC_SHA [
								ctx/key-method: 'dhe-rsa
								ctx/crypt-method: 'aes
								ctx/crypt-size: 32
								ctx/block-size: 16
								ctx/iv-size: 16
								ctx/hash-method: 'sha1
								ctx/hash-size: 20
							]
						] cipher-suites [
							do make error! rejoin ["Current version of TLS scheme doesn't support ciphersuite: " mold ctx/cipher-suite]
						]

						ctx/server-random: msg-obj/server-random
						msg-obj
					]
					certificate [
						msg-content: copy/part at data 5 len
						msg-obj: context [
							type: msg-type
							length: len
							certificates-length: to integer! copy/part msg-content 3
							certificate-list: make block! 4
							while [msg-content/1] [
								if 0 < clen: to integer! copy/part skip msg-content 3 3 [
									append certificate-list copy/part at msg-content 7 clen
								]
								msg-content: skip msg-content 3 + clen
							]
						]
						; no cert validation - just set it to be used
						ctx/certificate: parse-asn msg-obj/certificate-list/1

						switch/default ctx/key-method [
							rsa [
								; get the public key and exponent (hardcoded for now)
								ctx/pub-key: parse-asn next
;								ctx/certificate/1/sequence/4/1/sequence/4/6/sequence/4/2/bit-string/4
								ctx/certificate/1/sequence/4/1/sequence/4/7/sequence/4/2/bit-string/4
								ctx/pub-exp: ctx/pub-key/1/sequence/4/2/integer/4
								ctx/pub-key: next ctx/pub-key/1/sequence/4/1/integer/4
							]
						] [
							; for DH cipher suites the certificate is used just for signing the key exchange data
						]
						msg-obj
					]
					server-key-exchange [
						switch/default ctx/key-method [
							dhe-dss dhe-rsa [
								msg-content: copy/part at data 5 len
								msg-obj: context [
									type: msg-type
									length: len
									p-length: to integer! copy/part msg-content 2
									p: copy/part at msg-content 3 p-length
									g-length: to integer! copy/part at msg-content 3 + p-length 2
									g: copy/part at msg-content 3 + p-length + 2 g-length
									ys-length: to integer! copy/part at msg-content 3 + p-length + 2 + g-length 2
									ys: copy/part at msg-content 3 + p-length + 2 + g-length + 2 ys-length
									signature-length: to integer! copy/part at msg-content 3 + p-length + 2 + g-length + 2 + ys-length 2
									signature: copy/part at msg-content 3 + p-length + 2 + g-length + 2 + ys-length + 2 signature-length
								]

								ctx/dh-key: dh-make-key
								ctx/dh-key/p: msg-obj/p
								ctx/dh-key/g: msg-obj/g
								ctx/dh-pub: msg-obj/ys

								; TODO: the signature sent by server should be verified using DSA or RSA algorithm to be sure the dh-key params are safe
								msg-obj
							]
						] [
							do make error! "Server-key-exchange message has been sent illegally."
						]
					]
					server-hello-done [
						context [
							type: msg-type
							length: len
						]
					]
					client-hello [
						msg-content: copy/part at data 7 len
						context [
							type: msg-type
							version: pick [ssl-v3 tls-v1.0 tls-v1.1] data/6 + 1
							length: len
							content: msg-content
						]
					]
					finished [
						ctx/seq-num-r: 0
						msg-content: copy/part at data 5 len
						either msg-content <> prf ctx/master-secret either ctx/server? ["client finished"] ["server finished"] rejoin [checksum/method ctx/handshake-messages 'md5 checksum/method ctx/handshake-messages 'sha1] 12 [
							do make error! "Bad 'finished' MAC"
						] [
							debug "FINISHED MAC verify: OK"
						]
						context [
							type: msg-type
							length: len
							content: msg-content
						]
					]
				]

				append ctx/handshake-messages copy/part data len + 4

				data: skip data len + either ctx/encrypted? [
					; check the MAC
					mac: copy/part skip data len + 4 ctx/hash-size
					if mac <> checksum/method/key rejoin [
							to-bin ctx/seq-num-r 8					; sequence number (64-bit int in R3)
							#{16}									; msg type
							ctx/version								; version
							to-bin len + 4 2						; msg content length
							copy/part data len + 4
						] ctx/hash-method decode 'text ctx/server-mac-key
					[
						do make error! "Bad handshake record MAC"
					]
					4 + ctx/hash-size
				] [
					4
				]
			]
		]
		change-cipher-spec [
			ctx/encrypted?: true
			append result context [
				type: 'ccs-message-type
			]
		]
		application [
			append result msg-obj: context [
				type: 'app-data
				content: copy/part data (length? data) - ctx/hash-size
			]
			len: length? msg-obj/content
			mac: copy/part skip data len ctx/hash-size
			; check the MAC
			if mac <> checksum/method/key rejoin [
				to-bin ctx/seq-num-r 8	; sequence number (64-bit int in R3)
				#{17}					; msg type
				ctx/version				; version
				to-bin len 2			; msg content length
				msg-obj/content			; content
			] ctx/hash-method decode 'text ctx/server-mac-key
			[
				do make error! "Bad application record MAC"
			]
		]
	]
	ctx/seq-num-r: ctx/seq-num-r + 1
	return result
]

parse-response: func [
	ctx [object!]
	msg [binary!]
	/local
		proto messages
] [
	proto: parse-protocol msg
	either empty? messages: parse-messages ctx proto [
		do make error! "unknown/invalid protocol message"
	] [
		proto/messages: messages
	]

	debug ["processed protocol type:" proto/type "messages:" length? proto/messages]

	unless tail? skip msg proto/length + 5 [do make error! "invalid length of response fragment"]

	return proto
]

prf: func [
	secret [binary!]
	label [string! binary!]
	seed [binary!]
	output-length [integer!]
	/local
		len mid s-1 s-2 a p-sha1 p-md5
] [
	len: length? secret
	mid: to integer! .5 * (len + either odd? len [1] [0])

	s-1: copy/part secret mid
	s-2: copy at secret mid + either odd? len [0] [1]

	seed: rejoin [#{} label seed]

	p-md5: clear #{}
	a: seed ; A(0)
	while [output-length > length? p-md5] [
		a: checksum/method/key a 'md5 decode 'text s-1 ; A(n)
		append p-md5 checksum/method/key rejoin [a seed] 'md5 decode 'text s-1

	]

	p-sha1: clear #{}
	a: seed ; A(0)
	while [output-length > length? p-sha1] [
		a: checksum/method/key a 'sha1 decode 'text s-2 ; A(n)
		append p-sha1 checksum/method/key rejoin [a seed] 'sha1 decode 'text s-2
	]
	return ((copy/part p-md5 output-length) xor copy/part p-sha1 output-length)
]

make-key-block: func [
	ctx [object!]
] [
	ctx/key-block: prf ctx/master-secret "key expansion" rejoin [ctx/server-random ctx/client-random] ctx/hash-size + ctx/crypt-size + (either ctx/block-size [ctx/iv-size] [0]) * 2
]

make-master-secret: func [
	ctx [object!]
	pre-master-secret [binary!]
] [
	ctx/master-secret: prf pre-master-secret "master secret" rejoin [ctx/client-random ctx/server-random] 48
]

do-commands: func [
	ctx [object!]
	commands [block!]
	/no-wait
	/local arg cmd
] [
	clear ctx/msg
	parse commands [
		some [
			set cmd [
				'client-hello (client-hello ctx)
				| 'client-key-exchange (client-key-exchange ctx)
				| 'change-cipher-spec (change-cipher-spec ctx)
				| 'finished (encrypted-handshake-msg ctx finished ctx)
				| 'application  set arg [string! | binary!] (application-data ctx arg)
				| 'close-notify (alert-close-notify ctx)
			] (
				debug [ctx/seq-num-r ctx/seq-num-w "WRITE -->" cmd]
				ctx/seq-num-w: ctx/seq-num-w + 1
				update-proto-state/write-state ctx cmd
			)
		]
	]
	debug ["writing bytes:" length? ctx/msg]
	ctx/resp: clear []
	write ctx/connection ctx/msg

	unless no-wait [
		unless port? wait [ctx/connection 30] [do make error! "port timeout"]
	]
	ctx/resp
]

; TLS scheme

tls-init: func [
	ctx [object!]
] [
	ctx/seq-num-r: 0
	ctx/seq-num-w: 0
	ctx/protocol-state: none
	ctx/encrypted?: false

	switch ctx/crypt-method [
		rc4 [
			if ctx/encrypt-stream [
				ctx/encrypt-stream: rc4/stream ctx/encrypt-stream none
			]
			if ctx/decrypt-stream [
				ctx/decrypt-stream: rc4/stream ctx/decrypt-stream none
			]
		]
	]
]

tls-read-data: func [
	ctx [object!]
	port-data [binary!]
	/local len data fragment next-state
] [
	debug ["tls-read-data:" length? port-data "bytes"]
	data: append ctx/data-buffer port-data
	clear port-data

	while [
		5 = length? copy/part data 5
	] [
		len: 5 + to integer! copy/part at data 4 2

		debug ["reading bytes:" len]

		fragment: copy/part data len

		if len > length? fragment [
			debug ["incomplete fragment: read" length? fragment "of" len "bytes"]
			break
		]

		debug ["received bytes:" length? fragment newline "parsing response..."]

		append ctx/resp parse-response ctx fragment

		next-state: get-next-proto-state ctx

		debug ["State:" ctx/protocol-state "-->" next-state]

		data: skip data len

		if all [tail? data find next-state #complete] [
			debug ["READING FINISHED" length? head ctx/data-buffer index? data same? tail ctx/data-buffer data]
			clear ctx/data-buffer
			return true
		]
	]

	debug ["CONTINUE READING..."]
	clear change ctx/data-buffer data
	return false
]

tls-awake: funct [event [event!]] [
	debug ["TLS Awake-event:" event/type]
	port: event/port
	tls-port: port/locals
	tls-awake: :tls-port/awake

	if all [
		tls-port/state/protocol-state = 'application
		not port/data
	] [
		; reset the data field when interleaving port r/w states
		tls-port/data: none
	]

	switch/default event/type [
		lookup [
			open port
			tls-init tls-port/state
			insert system/ports/system make event! [type: 'lookup port: tls-port]
			return false
		]
		connect [
			do-commands tls-port/state [client-hello]

			if tls-port/state/resp/1/type = 'handshake [
				do-commands tls-port/state [
					client-key-exchange
					change-cipher-spec
					finished
				]
			]
			insert system/ports/system make event! [type: 'connect port: tls-port]
			return false
		]
		wrote [
			switch tls-port/state/protocol-state [
				close-notify [
					return true
				]
				application [
					insert system/ports/system make event! [type: 'wrote port: tls-port]
					return false
				]
			]
			read port
			return false
		]
		read [
			debug ["Read" length? port/data "bytes proto-state:" tls-port/state/protocol-state]
			complete?: tls-read-data tls-port/state port/data
			application?: false
			foreach proto tls-port/state/resp [
				switch proto/type [
					application [
						foreach msg proto/messages [
							if msg/type = 'app-data [
								unless tls-port/data [tls-port/data: clear tls-port/state/port-data]
								append tls-port/data msg/content
								application?: true
								msg/type: none
							]
						]
					]
					alert [
						foreach msg proto/messages [
							if msg/description = "Close notify" [
								do-commands tls-port/state [close-notify]
								insert system/ports/system make event! [type: 'read port: tls-port]
								return true
							]
						]
					]
				]
			]
			debug ["data complete?:" complete? "application?:" application?]
			either application? [
				insert system/ports/system make event! [type: 'read port: tls-port]
			] [
				read port
			]
			return complete?
		]
		close [
			insert system/ports/system make event! [type: 'close port: tls-port]
			return true
		]
	] [
		close port
		do make error! rejoin ["Unexpected TLS event: " event/type]
	]
	false
]


sys/make-scheme [
	name: 'tls
	title: "TLS protocol v1.0"
	spec: make system/standard/port-spec-net []
	actor: [
		read: func [
			port [port!]
			/local
				resp data msg
		] [
			debug ["READ" open? port/state/connection]
			read port/state/connection
			return port
		]

		write: func [port [port!] value [any-type!]] [
			if find [encrypted-handshake application] port/state/protocol-state [
				do-commands/no-wait port/state compose [
					application (value)
				]
				return port
			]
		]
		open: func [port [port!] /local conn] [
			if port/state [return port]

			if none? port/spec/host [tls-error "Missing host address"]

			port/state: context [
				data-buffer: make binary! 32000
				port-data: make binary! 32000
				resp: none

				version: #{03 01} ; protocol version used

				server?: false

				protocol-state: none

				key-method:

				hash-method:
				hash-size:

				crypt-method:
				crypt-size:
				block-size:
				iv-size:

				cipher-suite: none


				client-crypt-key:
				client-mac-key:
				client-iv:
				server-crypt-key:
				server-mac-key:
				server-iv: none

				seq-num-r: 0
				seq-num-w: 0

				msg: make binary! 4096
				handshake-messages: make binary! 4096 ; all messages from Handshake records except 'HelloRequest's

				encrypted?: false

				client-random: server-random: pre-master-secret: master-secret:
				key-block:
				certificate: pub-key: pub-exp:
				dh-key: dh-pub: none

				encrypt-stream: decrypt-stream: none

				connection: none
			]

			port/state/connection: conn: make port! [
				scheme: 'tcp
				host: port/spec/host
				port-id: port/spec/port-id
				ref: rejoin [tcp:// host ":" port-id]
			]

			port/data: port/state/port-data

			conn/awake: :tls-awake
			conn/locals: port
			open conn
			port
		]
		open?: func [port [port!]] [
			found? all [port/state open? port/state/connection]
		]
		close: func [port [port!]] [
			if port/state [
				close port/state/connection
				debug "TLS/TCP port closed"
				port/state/connection/awake: none
				port/state: none
			]
			port
		]
		copy: func [port [port!]] [
			if port/data [copy port/data]
		]
		query: func [port [port!]] [
			all [port/state query port/state/connection]
		]
		length?: func [port [port!]] [
			either port/data [length? port/data] [0]
		]
	]
]
