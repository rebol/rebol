REBOL [
	title: "REBOL3 TLSv1.2 protocol scheme"
	name: 'tls
	type: 'module
	author: rights: ["Richard 'Cyphre' Smolak" "Oldes" "Brian Dickens (Hostilefork)"]
	version: 0.6.1
	history: [
		0.6.1 "Cyphre" "Initial implementation used in old R3-alpha"
		0.7.0 "Oldes" {
			* Rewritten almost from scratch to support TLSv1.2
			* Using BinCode (binary) dialect to deal with buffers input/output.
			* Code is now having a lots of traces for deep study of the process.
			Special thanks to Hostile Fork for implementing TLSv1.2 for his Ren-C before me.
		}
	]
	todo: {
		* cached sessions
		* automagic cert data lookup
		* add more cipher suites (based on DSA, 3DES, ECDH, ECDHE, ECDSA, SHA384 ...)
		* server role support
		* TLS1.3 support
		* cert validation
	}
	references: [
		;The Transport Layer Security (TLS) Protocol v1.2
		https://tools.ietf.org/html/rfc5246

		https://testssl.sh/openssl-rfc.mapping.html
		https://fly.io/articles/how-ciphersuites-work/
		
		; If you want to get a report on what suites a particular site has:
		https://www.ssllabs.com/ssltest/analyze.html
	]
	notes: {
		Tested with:
			TLS_RSA_WITH_AES_128_CBC_SHA256
			TLS_RSA_WITH_AES_256_CBC_SHA256
			TLS_RSA_WITH_AES_128_CBC_SHA
			TLS_RSA_WITH_AES_256_CBC_SHA
			TLS_DHE_RSA_WITH_AES_128_CBC_SHA
	}
]

*Protocol-type: enum [
	CHANGE_CIPHER_SPEC: 20
	ALERT:              21
	HANDSHAKE:          22 ;0x16
	APPLICATION:        23
] 'TLS-protocol-type

*Protocol-version: enum [
	SSLv3:  #{0300}
	TLS1.0: #{0301}
	TLS1.1: #{0302}
	TLS1.2: #{0303}
	;TLS1.3: #{0304}
	;DTLS1.0: #{FEFF}
	;DTLS1.2: #{FEFD}
	;DTLS1.3: #{FEFC}
] 'TLS-Protocol-version

*Handshake: enum [
	HELLO_REQUEST:       0
	CLIENT_HELLO:        1
	SERVER_HELLO:        2
	CERTIFICATE:         11
	SERVER_KEY_EXCHANGE: 12
	CERTIFICATE_REQUEST: 13
	SERVER_HELLO_DONE:   14
	CERTIFICATE_VERIFY:  15
	CLIENT_KEY_EXCHANGE: 16 ;0x10
	FINISHED:            20
] 'TLS-Handshake-type

*Cipher-suite: enum [

;   The following CipherSuite definitions require that the server provide
;   an RSA certificate that can be used for key exchange.  The server may
;   request any signature-capable certificate in the certificate request
;   message.

	TLS_RSA_WITH_NULL_MD5:               #{0001}
	TLS_RSA_WITH_NULL_SHA:               #{0002}
	TLS_RSA_WITH_NULL_SHA256:            #{003B}
	TLS_RSA_WITH_RC4_128_MD5:            #{0004}
	TLS_RSA_WITH_RC4_128_SHA:            #{0005}
	TLS_RSA_WITH_3DES_EDE_CBC_SHA:       #{000A}
	TLS_RSA_WITH_AES_128_CBC_SHA:        #{002F}
	TLS_RSA_WITH_AES_256_CBC_SHA:        #{0035}
	TLS_RSA_WITH_AES_128_CBC_SHA256:     #{003C}
	TLS_RSA_WITH_AES_256_CBC_SHA256:     #{003D}

;   The following cipher suite definitions are used for server-
;   authenticated (and optionally client-authenticated) Diffie-Hellman.
;   DH denotes cipher suites in which the server's certificate contains
;   the Diffie-Hellman parameters signed by the certificate authority
;   (CA).  DHE denotes ephemeral Diffie-Hellman, where the Diffie-Hellman
;   parameters are signed by a signature-capable certificate, which has
;   been signed by the CA.  The signing algorithm used by the server is
;   specified after the DHE component of the CipherSuite name.  The
;   server can request any signature-capable certificate from the client
;   for client authentication, or it may request a Diffie-Hellman
;   certificate.  Any Diffie-Hellman certificate provided by the client
;   must use the parameters (group and generator) described by the
;   server.

	TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA:    #{000D}
	TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA:    #{0010}
	TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA:   #{0013}
	TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA:   #{0016}
	TLS_DH_DSS_WITH_AES_128_CBC_SHA:     #{0030}
	TLS_DH_RSA_WITH_AES_128_CBC_SHA:     #{0031}
	TLS_DHE_DSS_WITH_AES_128_CBC_SHA:    #{0032}
	TLS_DHE_RSA_WITH_AES_128_CBC_SHA:    #{0033}
	TLS_DH_DSS_WITH_AES_256_CBC_SHA:     #{0036}
	TLS_DH_RSA_WITH_AES_256_CBC_SHA:     #{0037}
	TLS_DHE_DSS_WITH_AES_256_CBC_SHA:    #{0038}
	TLS_DHE_RSA_WITH_AES_256_CBC_SHA:    #{0039}
	TLS_DH_DSS_WITH_AES_128_CBC_SHA256:  #{003E}
	TLS_DH_RSA_WITH_AES_128_CBC_SHA256:  #{003F}
	TLS_DHE_DSS_WITH_AES_128_CBC_SHA256: #{0040}
	TLS_DHE_RSA_WITH_AES_128_CBC_SHA256: #{0067}
	TLS_DH_DSS_WITH_AES_256_CBC_SHA256:  #{0068}
	TLS_DH_RSA_WITH_AES_256_CBC_SHA256:  #{0069}
	TLS_DHE_DSS_WITH_AES_256_CBC_SHA256: #{006A}
	TLS_DHE_RSA_WITH_AES_256_CBC_SHA256: #{006B}

;   The following cipher suites are used for completely anonymous
;   Diffie-Hellman communications in which neither party is
;   authenticated.  Note that this mode is vulnerable to man-in-the-
;   middle attacks.  Using this mode therefore is of limited use: These
;   cipher suites MUST NOT be used by TLS 1.2 implementations unless the
;   application layer has specifically requested to allow anonymous key
;   exchange.  (Anonymous key exchange may sometimes be acceptable, for
;   example, to support opportunistic encryption when no set-up for
;   authentication is in place, or when TLS is used as part of more
;   complex security protocols that have other means to ensure
;   authentication.)

	TLS_DH_anon_WITH_RC4_128_MD5:        #{0018}
	TLS_DH_anon_WITH_3DES_EDE_CBC_SHA:   #{001B}
	TLS_DH_anon_WITH_AES_128_CBC_SHA:    #{0034}
	TLS_DH_anon_WITH_AES_256_CBC_SHA:    #{003A}
	TLS_DH_anon_WITH_AES_128_CBC_SHA256: #{006C}
	TLS_DH_anon_WITH_AES_256_CBC_SHA256: #{006D}

] 'TLS-Cipher-suite

*SignatureAlgorithm: enum [
	none:       0
	md5:        1
	sha1:       2
	sha224:     3
	sha256:     4
	sha384:     5
	sha512:     6
	md5_sha1: 255
] 'TLSSignatureAlgorithm

*ClientCertificateType: enum [
	rsa_sign:                  1
	dss_sign:                  2
	rsa_fixed_dh:              3
	dss_fixed_dh:              4
	rsa_ephemeral_dh_RESERVED: 5
	dss_ephemeral_dh_RESERVED: 6
	fortezza_dms_RESERVED:     20
	ecdsa_sign:                64
	rsa_fixed_ecdh:            65
	ecdsa_fixed_ecdh:          66
] 'TLSClientCertificateType

*Alert-level: enum [
	WARNING: 1
	FATAL:   2
] 'TLS-Alert-level

*Alert: enum [
	Close_notify:             0
	Unexpected_message:      10
	Bad_record_MAC:          20
	Decryption_failed:       21
	Record_overflow:         22
	Decompression_failure:   30
	Handshake_failure:       40
	No_certificate:          41
	Bad_certificate:         42
	Unsupported_certificate: 43
	Certificate_revoked:	 44
	Certificate_expired:	 45
	Certificate_unknown:	 46
	Illegal_parameter:       47
	Unknown_CA:              48
	Access_denied:           49
	Decode_error:            50
	Decrypt_error:           51
	Export_restriction:      60
	Protocol_version:        70
	Insufficient_security:   71
	Internal_error:          80
	User_cancelled:          90
	No_renegotiation:       100
	Unsupported_extension:  110
] 'TLS-Alert

*TLS-Extension: enum [
	RenegotiationInfo: #{FF01} ;@@ https://tools.ietf.org/html/rfc5746
] 'TLS-Extension


_log-error: func[msg][
	if block? msg [msg: reform msg]
	print rejoin [" ^[[1;33m[TLS] ^[[35m" msg "^[[0m"]
]
_log-info: func[msg][
	if block? msg [msg: reform msg]
	print rejoin [" ^[[1;33m[TLS] ^[[36m" msg "^[[0m"]
]
_log-more: func[msg][
	if block? msg [msg: reform msg]
	print rejoin [" ^[[33m[TLS] ^[[0;36m" msg "^[[0m"]
]
_log-debug: func[msg][
	if block? msg [msg: reform msg]
	print rejoin [" ^[[33m[TLS] ^[[0;32m" msg "^[[0m"]
]
_log-----: does [print "----------------------------------------------------------------"]

log-error: log-info: log-more: log-debug: log-----: none

tls-verbosity: func[
	"Turns ON and OFF various log traces"
	verbose [integer!] "Verbosity level"
][
	log-error: log-info: log-more: log-debug: log-----: none
	case/all [
		verbose >= 0 [log-error: :_log-error ]
		verbose >= 1 [log-info:  :_log-info  ]
		verbose >= 2 [log-more:  :_log-more
		              log-----:  :_log-----  ]
		verbose >= 3 [log-debug: :_log-debug ]
	]
]

log-error: :_log-error ;- use error logs by default
;tls-verbosity 3

;-- list of supported suites as a single binary
; This list is sent to the server when negotiating which one to use.  Hence
; it should be ORDERED BY CLIENT PREFERENCE (more preferred suites first).
suported-cipher-suites: rejoin [
	#{003D} ;TLS_RSA_WITH_AES_256_CBC_SHA256
	#{003C} ;TLS_RSA_WITH_AES_128_CBC_SHA256
	#{002F} ;TLS_RSA_WITH_AES_128_CBC_SHA
	#{0035} ;TLS_RSA_WITH_AES_256_CBC_SHA
	#{0032} ;TLS_DHE_DSS_WITH_AES_128_CBC_SHA
	#{0038} ;TLS_DHE_DSS_WITH_AES_256_CBC_SHA
	#{0033} ;TLS_DHE_RSA_WITH_AES_128_CBC_SHA
	#{0039} ;TLS_DHE_RSA_WITH_AES_256_CBC_SHA
	;- RC4 is prohibited by https://tools.ietf.org/html/rfc7465 for insufficient security
	;#{0004} ;TLS_RSA_WITH_RC4_128_MD5 
	;#{0005} ;TLS_RSA_WITH_RC4_128_SHA
]

supported-signature-algorithms: rejoin [
	;#{0601} ; rsa_pkcs1_sha512
	#{0602} ; SHA512 DSA
	;#{0603} ; ecdsa_secp521r1_sha512
	#{0501} ; rsa_pkcs1_sha384
	#{0502} ; SHA384 DSA
	;#{0503} ; ecdsa_secp384r1_sha384
	#{0401} ; rsa_pkcs1_sha256
	#{0402} ; SHA256 DSA
	;#{0403} ; ecdsa_secp256r1_sha256
	;#{0301} ; SHA224 RSA
	;#{0302} ; SHA224 DSA
	;#{0303} ; SHA224 ECDSA
	#{0201} ; rsa_pkcs1_sha1
	#{0202} ; SHA1 DSA
	;#{0203} ; ecdsa_sha1
]


TLS-init-cipher-suite: func [
	"Initialize context for current cipher-suite. Returns false if unknown suite is used."
	ctx          [object! ]
	cipher-suite [binary!]
	/local suite key-method cipher
][
	unless find/skip suported-cipher-suites cipher-suite 2 [
		log-error "Requested cipher suite is not supported!"
		return false
	]

	suite: *Cipher-suite/name to integer! cipher-suite
	
	if none? suite [
		log-error ["Unknown cipher suite:" cipher-suite]
		return false
	]

	log-info ["Init TLS Cipher-suite:^[[35m" suite "^[[22m" cipher-suite]

	parse form suite [
		opt "TLS_"
		copy key-method to "_WITH_" 6 skip
		copy cipher [
			  "NULL"         (ctx/crypt-size: 0  ctx/IV-size: 0  ctx/block-size: none)
			| "RC4_128"      (ctx/crypt-size: 16 ctx/IV-size: 0  ctx/block-size: none)
			| "3DES_EDE_CBC" (ctx/crypt-size: 24 ctx/IV-size: 8  ctx/block-size: 8   )
			| "AES_128_CBC"  (ctx/crypt-size: 16 ctx/IV-size: 16 ctx/block-size: 16  )
			| "AES_256_CBC"  (ctx/crypt-size: 32 ctx/IV-size: 16 ctx/block-size: 16  )
		] #"_" [
			  "NULL"   end (ctx/hash-method: none    ctx/mac-size: 0 )
			| "MD5"    end (ctx/hash-method: 'MD5    ctx/mac-size: 16)
			| "SHA"    end (ctx/hash-method: 'SHA1   ctx/mac-size: 20)
			| "SHA256" end (ctx/hash-method: 'SHA256 ctx/mac-size: 32)
			| "SHA384" end (ctx/hash-method: 'SHA384 ctx/mac-size: 48)
			;NOTE: in RFC mac-size is named mac_length and there is also mac_key_length, which has same value
		]
		(
			ctx/key-method:   to word! key-method
			ctx/crypt-method: to word! cipher
			ctx/is-aead?: found? find [AES_128_GCM AES_256_GCM CHACHA20_POLY1305] ctx/crypt-method

			log-more [
				"Key:^[[1m" ctx/key-method
				"^[[22mcrypt:^[[1m" ctx/crypt-method
				"^[[22msize:^[[1m" ctx/crypt-size
				"^[[22mIV:^[[1m" ctx/IV-size 
			]
		)
	]
]

make-TLS-error: func [
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

TLS-error: function [
	id [integer!]
][
	message: to string! any [*Alert/name id  "unknown"]
	replace/all message #"_" #" "

	log-error join "ERROR: " message

	;@@TODO: send it to server and close!
	do make-TLS-error message
]

change-state: function [
	ctx [object!]
	new-state [word!]
][
	ctx/state-prev: ctx/state
	if ctx/state <> new-state [
		;log-info "_________________________________________________________"
		log-info ["New state:^[[33m" new-state "^[[22mfrom:" ctx/state]
		ctx/state: new-state
	]
]

assert-prev-state: function [
	ctx [object!]
	legal-states [block!]
][
	if not find legal-states ctx/state-prev [
		log-error ["State" ctx/state "is not expected after" ctx/state-prev]
		do make error! "invalid protocol state... TODO: close port!"
	]
]

; TLS protocol code

TLS-update-messages-hash: function [
	ctx [object!]
	msg [binary!]
	len [integer!]
][
	;prin  join "^/^/^/" either ctx/reading? [#"R"][#"W"] 
	;print ["[TLS] TLS-update-messages-hash bytes:" len "hash:" all [ctx/sha-port ctx/sha-port/spec/method]]
	if none? ctx/sha-port [
		either ctx/legacy? [
			ctx/sha-port: open checksum://sha1
			ctx/md5-port: open checksum://md5
		][
			ctx/sha-port: open either ctx/mac-size = 48 [checksum://sha384][checksum://sha256]
		]
		log-more ["Initialized SHA method:" ctx/sha-port/spec/method]
	]
	write/part ctx/sha-port msg len
	if ctx/legacy? [;TLS1.1 and older
		write/part ctx/md5-port msg len
	]
]

client-hello: function [
	ctx [object!]
] [
	change-state ctx 'CLIENT_HELLO
	with ctx [
		;initialize checksum port(s)

		;precomputing the extension's lengths so I can write them in one WRITE call
		length-signatures:  2 + length? supported-signature-algorithms
		length-extensions:  4 + length-signatures
		length-message:    41 + length-extensions + length? suported-cipher-suites
		length-record:      4 + length-message

		binary/write out [
			UI8       22                  ; protocol type (22=Handshake)
			UI16      :version            ; protocol version
			UI16      :length-record      ; length of SSL record data
			;client-hello message:
			UI8       1                   ; protocol message type	(1=ClientHello)
			UI24      :length-message     ; protocol message length
			UI16      :version            ; max supported version by client
			UNIXTIME-NOW RANDOM-BYTES 28  ; random struct
			UI8       0                   ; session ID length
			UI16BYTES :suported-cipher-suites
			UI8       1                   ; compression method length
			UI8       0                   ; no compression
			;extensions
			UI16      :length-extensions
			UI16      13                  ; extension type: signature-algorithms
			UI16      :length-signatures  ; note: there is another length following
			UI16BYTES :supported-signature-algorithms
		]

		out/buffer: head out/buffer
		
		client-random: copy/part (at out/buffer 12) 32
		TLS-update-messages-hash ctx (at out/buffer 6) (4 + length-message)
		log-more [
			"W[" ctx/seq-write "] Bytes:" length? out/buffer "=>"
			"record:"     length-record
			"message:"    length-message
			"extensions:" length-extensions
			"signatures:" length-signatures
		]
		log-more ["W[" ctx/seq-write "] Client random:" mold client-random]
	]
]

client-key-exchange: function [
	ctx [object!]
] [
	;log-debug ["client-key-exchange -> method:" ctx/key-method]

	change-state ctx 'CLIENT_KEY_EXCHANGE
	assert-prev-state ctx [CLIENT_CERTIFICATE SERVER_HELLO_DONE]

	with ctx [

		binary/write out [
				UI8  22          ; protocol type (22=Handshake)
				UI16 :version    ; protocol version
			pos-record-len:
				UI16 0           ; length of the (following) record data
			pos-record:
				UI8  16          ; protocol message type (16=ClientKeyExchange)
			pos-message:
				UI24 0           ; protocol message length
			pos-key: 
		]

		switch key-method [
			RSA [
				log-more ["W[" ctx/seq-write "] Using RSA key-method"]

				; generate pre-master-secret
				binary/write bin [
					UI16 :version RANDOM-BYTES 46 ;writes genereted secret (first 2 bytes are version)
				]
				; read the temprary random bytes back to store them for client's use
				binary/read bin [pre-master-secret: BYTES 48]
				binary/init bin 0 ;clears temp bin buffer


				log-more ["W[" ctx/seq-write "] pre-master-secret:" mold pre-master-secret]

				;log-debug "encrypting pre-master-secret:"

				;?? pre-master-secret
				;?? pub-key
				;?? pub-exp

				rsa-key: rsa-init pub-key pub-exp

				; supply encrypted pre-master-secret to server
				key-data: rsa/encrypt rsa-key pre-master-secret

				log-more ["W[" ctx/seq-write "] key-data:" mold key-data]
			]
			DHE_DSS
			DHE_RSA [
				log-more ["W[" ctx/seq-write "] Using DH key-method"]
				; resolve the public key to supply it to server
				key-data: dh/public dh-key
				; and release the dh-key handle (as not needed anymore)
				dh/release dh-key
				dh-key: none
			]
		]

		;compute used lengths
		length-message: 2 + length? key-data
		length-record:  4 + length-message

		;and write them with key data
		binary/write out [
			AT :pos-record-len UI16 :length-record
			AT :pos-message    UI24 :length-message
			AT :pos-key   UI16BYTES :key-data
		]


		;-- make all secure data
		if ctx/version >= *Protocol-version/TLS1.0 [
			; NOTE: key-expansion is used just to generate keys so it does not need to be stored in context!
			ctx/master-secret: prf "master secret" legacy? (join ctx/client-random ctx/server-random) pre-master-secret 48
				key-expansion: prf "key expansion" legacy? (join ctx/server-random ctx/client-random) master-secret 
								   (mac-size + crypt-size + iv-size) * 2

			pre-master-secret: none ;-- not needed anymore
			;@@TODO: should it be released more sefely? Like first overwriting the series with other data?

			;?? master-secret
			;?? key-expansion
			;?? mac-size
			;?? crypt-size
			;?? iv-size
		]

		client-mac-key:   take/part key-expansion mac-size
		server-mac-key:   take/part key-expansion mac-size
		client-crypt-key: take/part key-expansion crypt-size
		server-crypt-key: take/part key-expansion crypt-size

		log-more ["Client-mac-key:   " mold client-mac-key  ]
		log-more ["Server-mac-key:   " mold server-mac-key  ]
		log-more ["Client-crypt-key: " mold client-crypt-key]
		log-more ["Server-crypt-key: " mold server-crypt-key]

		if block-size [
			either *Protocol-version/TLS1.0 = version [
				; Block ciphers in TLS 1.0 used an implicit initialization vector
				; (IV) to seed the encryption process.  This has vulnerabilities.
				client-iv: take/part key-expansion block-size
				server-iv: take/part key-expansion block-size
				
			][
				; Each encrypted message in TLS 1.1 and above carry a plaintext
				; initialization vector, so the ctx does not use one for the whole
				; session.  Unset it to make sure.
				unset system/contexts/lib/in ctx 'client-iv
				unset system/contexts/lib/in ctx 'server-iv
			]
		]

		key-expansion: none

		;@@ TODO: aead will need this:
		;@@	switch crypt-method [
		;@@		CHACHA20_POLY1305 [
		;@@			local-nonce:  copy client-iv
		;@@			remote_nonce: copy server-iv
		;@@		]
		;@@		AES_128_GCM
		;@@		AES_256_GCM [
		;@@			local-aead-iv:  copy client-iv
		;@@			remote-aead-iv: copy server-iv
		;@@		]
		;@@	]

		TLS-update-messages-hash ctx (at head out/buffer pos-record) length-record
	]
]


change-cipher-spec: function [
	ctx [object!]
] [
	;@@ actually this is not just state, but its own "protocol"
	;@@ https://tools.ietf.org/html/rfc5246#section-7.1

	change-state ctx 'CHANGE_CIPHER_SPEC

	with ctx [
		binary/write out [
			UI8  20        ; protocol type (20=ChangeCipherSpec)
			UI16 :version  ; protocol version
			UI16 1         ; length of SSL record data
			UI8  1         ; CCS protocol type
		]
	]
	ctx/cipher-spec-set: 1
	ctx/seq-write: 0
]

application-data: func [
	ctx [object!]
	message [binary! string!]
] [
	log-more "application-data"
	;prin "unencrypted: " ?? message
	message: encrypt-data ctx to binary! message
	;prin "encrypted: " ?? message
	with ctx [
		binary/write out [
			UI8       23        ; protocol type (23=Application)
			UI16      :version  ; protocol version
			UI16BYTES :message 
		]
	]
]

alert-close-notify: func [
	ctx [object!]
] [
	;@@ Not used/tested yet! It should be replaced with ALERT-notify with CLOSE as possible type
	log-more "alert-close-notify"
	message: encrypt-data ctx #{0100} ; close notify
	with ctx [
		binary/write out [
			UI8       21        ; protocol type (21=Alert)
			UI16      :version  ; protocol version
			UI16BYTES :message
		]
	]
]

finished: function [
	ctx [object!]
] [
	log-info ["FINISHED^[[22m write sequence:" ctx/seq-write]
	ctx/seq-write: 0

	seed: either ctx/legacy? [
		rejoin [
			read ctx/md5-port
			read ctx/sha-port
		]
	][	read ctx/sha-port ]
	;?? seed

	unencrypted: rejoin [
		#{14}		; protocol message type	(20=Finished)
		#{00000C}   ; protocol message length (12 bytes)
		prf "client finished" ctx/legacy? seed ctx/master-secret  12
	]
	;?? unencrypted
	
	TLS-update-messages-hash ctx unencrypted length? unencrypted
	encrypt-handshake-msg ctx unencrypted
]

encrypt-handshake-msg: function [
	ctx [object!]
	unencrypted [binary!]
	/local
		plain-msg
] [
	log-more ["W[" ctx/seq-write "] encrypting-handshake-msg"]

	encrypted: encrypt-data/type ctx unencrypted 22
	;?? encrypted

	with ctx [
		binary/write out [
			UI8 22					; protocol type (22=Handshake)
			UI16 :version			; protocol version
			UI16BYTES :encrypted
		]
	]
]

decrypt-msg: function [
	ctx [object!]
	data [binary!]
][
	;print "CRYPTED message!"
	;?? data
	with ctx [
		if all [
			block-size
			version > *Protocol-version/TLS1.0
		][
			;server's initialization vector is new with each message
			server-iv: take/part data block-size
		]
		;?? data
		change data decrypt-data ctx data
		;?? data
		if block-size [
			; deal with padding in CBC mode
			; the padding length is stored in the last byte
			clear skip tail data (-1 - (to integer! last data))
			; and at tail of data is MAC value....
			mac: take/last/part data mac-size
			;  which MUST be same like following checksum:
			binary/write bin [
				UI64      :seq-read
				UI8       23
				UI16      :version
				UI16BYTES :data
			]
			mac-check: checksum/method/key bin/buffer hash-method server-mac-key
			binary/init bin 0 ;clear the temp bin buffer

			;?? mac
			;?? mac-check

			if mac <> mac-check [ critical-error: *Alert/Bad_record_MAC ]
			
			if version > *Protocol-version/TLS1.0 [
				unset 'server-iv ;-- avoid reuse in TLS 1.1 and above
			]
		]
	]
	data
]

encrypt-data: function [
	ctx     [object!]
	content [binary!]
	/type
		msg-type [integer!] "application data is default"
] [
	;log-debug "--encrypt-data--"
	;? ctx
	msg-type: any [msg-type 23] ;-- default application

	;?? content

	with ctx [
		;@@ GenericBlockCipher: https://tools.ietf.org/html/rfc5246#section-6.2.3.2

		if version > *Protocol-version/TLS1.0 [
			;
			; "The Initialization Vector (IV) SHOULD be chosen at random, and
			;  MUST be unpredictable.  Note that in versions of TLS prior to 1.1,
			;  there was no IV field, and the last ciphertext block of the
			;  previous record (the "CBC residue") was used as the IV.  This was
			;  changed to prevent the attacks described in [CBCATT].  For block
			;  ciphers, the IV length is SecurityParameters.record_iv_length,
			;  which is equal to the SecurityParameters.block_size."
			;
			client-iv: make binary! block-size
			binary/write client-iv [RANDOM-BYTES :block-size]
		]

		;?? ctx/seq-write
		log-more ["Client-iv:     " client-iv]
		log-more ["Client-mac-key:" client-mac-key]
		log-more ["Hash-method:   " hash-method]

		; Message Authentication Code
		; https://tools.ietf.org/html/rfc5246#section-6.2.3.1

		binary/write bin [
			UI64      :seq-write
			UI8       :msg-type
			UI16      :version
			UI16BYTES :content
		]

		MAC: checksum/method/key bin/buffer ctx/hash-method ctx/client-mac-key

		binary/init bin 0 ;clear the bin buffer

		;?? MAC
		data: rejoin [content MAC]
		;??  block-size

		if block-size [
			; add the padding data in CBC mode
			padding: block-size - (remainder (1 + length? data) block-size)
			insert/dup tail data (to char! padding) (padding + 1)
		]

		switch/default crypt-method [
			RC4_128 [
				unless encrypt-stream [
					encrypt-stream: rc4/key client-crypt-key
				]
				rc4/stream encrypt-stream data
			]
			AES_128_CBC
			AES_256_CBC [
				unless encrypt-stream [
					encrypt-stream: aes/key client-crypt-key client-iv
				]
				data: aes/stream encrypt-stream data

				if version > *Protocol-version/TLS1.0 [
					; encrypt-stream must be reinitialized each time with the
					; new initialization vector.
					encrypt-stream: none
				]
			]
		][
			;@@ remove this part.. the check must be done sooner!
			log-error ["Unsupported TLS crypt-method:" crypt-method]
			halt
		]

		;-- TLS versions 1.1 and above include the client-iv in plaintext.
		if version > *Protocol-version/TLS1.0 [
			insert data client-iv
			unset 'client-iv ;-- avoid accidental reuse
		]
		;print ["encrypted data size:" length? data]
	]
	data
]

decrypt-data: func [
	ctx [object!]
	data [binary!]
	/local
		crypt-data
] [
	switch ctx/crypt-method [
		RC4_128 [
			unless ctx/decrypt-stream [
				ctx/decrypt-stream: rc4/key ctx/server-crypt-key
			]
			rc4/stream ctx/decrypt-stream data
		]
		AES_128_CBC
		AES_256_CBC [
			unless ctx/decrypt-stream [
				ctx/decrypt-stream: aes/key/decrypt ctx/server-crypt-key ctx/server-iv
			]
			data: aes/stream ctx/decrypt-stream data

			;-- TLS 1.1 and above must use a new initialization vector each time
			if ctx/version > *Protocol-version/TLS1.0 [
				;@@TODO: it must be possible to reuse the stream instead of recreating it on each message!
				ctx/decrypt-stream: none
			]
		]
	]

	return data
]


prf: function [
	{(P)suedo-(R)andom (F)unction, generates arbitrarily long binaries}
	label   [string! binary!]
	legacy  [logic!] "TRUE for TLS 1.1 and older"
	seed    [binary!]
	secret  [binary!]
	output-length [integer!]
][
	; The seed for the underlying P_<hash> is the PRF's seed appended to the
	; label.  The label is hashed as-is, so no null terminator.
	;
	; PRF(secret, label, seed) = P_<hash>(secret, label + seed)
	;

	log-more ["PRF" mold label "len:" output-length]
	seed: join to binary! label seed
	if legacy [
		;
		; Prior to TLS 1.2, the pseudo-random function was driven by a strange
		; mixed method that's half MD5 and half SHA-1 hashing, regardless of
		; cipher suite used: https://tools.ietf.org/html/rfc4346#section-5

		len: length? secret
		mid: to integer! (len + 1) * 0.5

		s-1: copy/part secret mid
		s-2: copy at secret (len - mid + 1)

		p-md5: copy #{}
		a: seed ; A(0)
		while [output-length > length? p-md5] [
			a: checksum/method/key a 'md5 s-1 ; A(n)
			append p-md5 checksum/method/key rejoin [a seed] 'md5 s-1
		]

		p-sha1: copy #{}
		a: seed ; A(0)
		while [output-length > length? p-sha1] [
			a: checksum/method/key a 'sha1 s-2 ; A(n)
			append p-sha1 checksum/method/key rejoin [a seed] 'sha1 s-2
		]
		return (
			(copy/part p-md5 output-length)
			xor+ (copy/part p-sha1 output-length)
		)
	]

	; TLS 1.2 includes the pseudorandom function as part of its cipher
	; suite definition.  No cipher suites assume the md5/sha1 combination
	; used above by TLS 1.0 and 1.1.  All cipher suites listed in the
	; TLS 1.2 spec use `P_SHA256`, which is driven by the single SHA256
	; hash function: https://tools.ietf.org/html/rfc5246#section-5

	p-sha256: make binary! output-length
	a: seed ; A(0)
	while [output-length >= length? p-sha256] [
		a: checksum/method/key a 'sha256 secret
		append p-sha256 checksum/method/key rejoin [a seed] 'sha256 secret
		;?? p-sha256
	]
	;trim the result to required output length
	clear at p-sha256 (1 + output-length)
	;log-more ["PRF result length:" length? p-sha256 mold p-sha256]
	p-sha256
]

do-commands: func [
	ctx [object!]
	commands [block!]
	/no-wait
	/local arg cmd
] [
	binary/init ctx/out none ;reset output buffer

	parse commands [
		some [
			set cmd [
				  'client-hello        ( client-hello            ctx )
				| 'client-key-exchange ( client-key-exchange     ctx )
				| 'change-cipher-spec  ( change-cipher-spec      ctx )
				| 'finished            ( finished                ctx )
				| 'application set arg [string! | binary!] 
									   ( application-data        ctx arg )
				| 'close-notify        ( alert-close-notify      ctx )
			] (
				ctx/seq-write: ctx/seq-write + 1
			)
		]
	]
	;ctx/out/buffer: head ctx/out/buffer
	;?? ctx/out/buffer
	log-info ["Writing bytes:" length? ctx/out/buffer]
	ctx/out/buffer: head ctx/out/buffer
	write ctx/connection ctx/out/buffer

	;?? ctx/out/buffer

	unless no-wait [
		log-more "Waiting for responses"
		unless port? wait [ctx/connection 30] [
			log-error "Timeout"
			;? ctx
			? ctx/connection
			send-event 'close ctx/connection/locals
			do make error! "port timeout"
		]
	]
	ctx/reading?: true
	binary/init ctx/out none ;resets the output buffer
]

;----------------------------------------------;
;--- TLS scheme -------------------------------;
;----------------------------------------------;

make-TLS-ctx: does [ context [
	version:  *Protocol-version/TLS1.2
	server-version: none

	legacy?: false

	in:  binary 16104 ;input binary codec
	out: binary 16104 ;output binary codec
	bin: binary 64    ;temporary binary

	port-data:   make binary! 32000 ;this holds received decrypted application data

	reading?:       false  ;if client is reading or writing data
	;server?:       false  ;always FALSE now as we have just a client
	protocol:       none   ;current protocol state. One of: [HANDSHAKE APPLICATION ALERT]
	state:          none   ;current state in context of the protocol
	state-prev:     none   ;previous state

	critical-error: none   ;used to signalize error state
	cipher-spec-set: 0     ;stores state of cipher spec exchange (0 = none, 1 = client, 2 = both)

	sha-port:       none   ;used for progressive checksum computations
	md5-port:       none   ;used for progressive checksum computations (in TLSv1.0)

	;- values defined inside TLS-init-cipher-suite:
	key-method:            ; one of: [RSA DH_DSS DH_RSA DHE_DSS DHE_RSA]
	hash-method:           ; one of: [MD5 SHA1 SHA256 SHA384]
	crypt-method:   none
	is-aead?:       false  ; crypt-method with "Authenticated Encryption with Additional Data" (not yet supported!)
	mac-size:            ; Size of message authentication code
	crypt-size:            ; The number of bytes from the key_block that are used for generating the write keys.
	block-size:            ; The amount of data a block cipher enciphers in one chunk; a block
						   ; cipher running in CBC mode can only encrypt an even multiple of
						   ; its block size.
	IV-size: 0             ; The amount of data needed to be generated for the initialization vector.
	

	client-crypt-key:
	server-crypt-key:
	client-mac-key:
	server-mac-key:
	client-iv:
	server-iv: none

	server-extensions: copy []

	seq-read:  0 ; sequence counters
	seq-write: 0

	server-certs: copy []

	client-random:
	server-random:
	server-session:
	pre-master-secret:
	master-secret:
	certificate:
	pub-key: pub-exp:
	dh-key:		 ;DH key calculated from provided G and P values from server
	dh-pub: none ;Server's public DH key

	encrypt-stream:
	decrypt-stream: none

	connection: none
]]

TLS-init: func [
	"Resets existing TLS context"
	ctx [object!]
] [
	ctx/seq-read: ctx/seq-write: 0
	ctx/protocol: ctx/state: ctx/state-prev: none
	ctx/cipher-spec-set: 0 ;no encryption yet

	ctx/legacy?: (255 & ctx/version) < 3 ;- TLSv1.1 and older

	clear ctx/server-certs

	;@@ review code bellow:
	switch ctx/crypt-method [
		RC4_128 [
			ctx/encrypt-stream: none
			ctx/decrypt-stream: none
		]
	]
]


TLS-read-data: function [
	ctx       [object!]
	port-data [binary!] 
] [
	;log-more ["read-data:^[[1m" length? port-data "^[[22mbytes"]

	;probe copy/part ctx/in/buffer 10

	binary/write ctx/in port-data ;- fills input buffer with received data
	clear port-data

	ctx/reading?: true

	inp: ctx/in

	while [ctx/reading? and ((available: length? inp/buffer) >= 5)][
		;?? available
		log-debug ["Data starts: " mold copy/part inp/buffer 10]

		binary/read inp [
			start:   INDEX
			type:    UI8
			version: UI16
			len:     UI16
		]
		log-debug ["fragment type: ^[[1m" type "^[[22mver:^[[1m" version "^[[22mbytes:^[[1m" len "^[[22mbytes"]

		if available < len [
			;probe inp/buffer
			log-info ["Incomplete fragment:^[[22m available^[[1m" available "^[[22mof^[[1m" len "^[[22mbytes"]
			;?? inp/buffer
			binary/read inp [AT :start] ;resets position
			log-debug ["Data starts: " mold copy/part inp/buffer 10]
			return false
		]

		start: start + 5 ;header size

		*protocol-type/assert type
		*protocol-version/assert version

		protocol: *protocol-type/name type
		version:  *protocol-version/name version

		ctx/seq-read: ctx/seq-read + 1

		end: start + len

		;log-debug "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
		log-info ["^[[22mR[" ctx/seq-read "] Protocol^[[1m" protocol "^[[22m" version "bytes:^[[1m" len "^[[22mfrom^[[1m" start "^[[22mto^[[1m" end]

		ctx/protocol: protocol

		switch protocol [
			HANDSHAKE [
				msg-start: inp/buffer
				TLS-update-messages-hash ctx msg-start len
				ctx/critical-error: TLS-read-handshake-message ctx
			]
			ALERT [
				binary/read inp [data: BYTES :len]
				if ctx/cipher-spec-set > 1 [
					data: decrypt-msg ctx data
					;print "DECRYPTED ALERT"
					;?? data
				]
				level: data/1
				id:    data/2

				level: any [*Alert-level/name level  join "Alert-" level ]
				description: any [*Alert/name id  "Unknown"]
				description: replace/all form description #"_" #" "
				
				;@@ do some actions here....
				ctx/critical-error: either level = 'WARNING [false][id]
				either id = 0 [
					; server done
					ctx/reading?: false
					ctx/protocol: 'APPLICATION
					log-info "Server done"
				][
					log-more ["ALERT:" level "-" description]
				]
			]
			CHANGE_CIPHER_SPEC [
				binary/read inp [value: UI8]
				either 1 = value [
					ctx/cipher-spec-set: 2
					ctx/seq-read: 0
					ctx/reading?: false
					ctx/protocol: 'APPLICATION
					change-state ctx 'APPLICATION
					;? ctx
				][
					log-error ["*** CHANGE_CIPHER_SPEC value should be 1 but is:" value]
				]
			]
			APPLICATION [
				change-state ctx 'APPLICATION
				assert-prev-state ctx [APPLICATION ALERT]

				binary/read inp [data: BYTES :len]

				if ctx/cipher-spec-set > 1 [
					if data: decrypt-msg ctx data [
						append ctx/port-data data
					]
				]
			]
		]

		;?? ctx/critical-error
		if ctx/critical-error [ return false ]

		if end <> index? inp/buffer [
			log-error ["Record end mismatch:^[[22m" end "<>" index? inp/buffer]
			ctx/critical-error: *Alert/Record_overflow
			return false
		]

		if not ctx/reading? [
			;? ctx
			;print "^/================================================================"
			log-more ["Reading finished!"]
			log-----

			return true
		]
	]

	;?? ctx/state
	log-debug "continue reading..."
	return true
]

TLS-read-handshake-message: function [
	ctx [object!]
][
	binary/read ctx/in [type: UI8 len: UI24 start: INDEX]
	ends: start + len
	log-debug ["R[" ctx/seq-read "] length:" len "start:" start "ends:" ends]

	change-state ctx *Handshake/name type

	;log-more ["R[" ctx/seq-read "] Handshake fragment:" ctx/state "length:" len "start:" start "ends:" ends]
	;probe copy/part ctx/in/buffer len

	switch/default ctx/state [
		;----------------------------------------------------------
		SERVER_HELLO [
			;NOTE: `len` should be now >= 38 for TLS and 41 for DTLS
			assert-prev-state ctx [CLIENT_HELLO]

			if ctx/critical-error: with ctx [
				;@@TODO: do check if server does not report lengths for session and compressions
				;@@      above given bounds!
				binary/read in [
					server-version: UI16
					server-random:  BYTES 32
					server-session: UI8BYTES						
					cipher-suite:   BYTES 2
					compressions:   UI8BYTES ;<- must be empty
				]

				log-more ["R[" seq-read "] Version:" *Protocol-version/name server-version "len:" len "cipher-suite:" cipher-suite]
				log-more ["R[" seq-read "] Random: ^[[1m"      mold server-random ]
				log-more ["R[" seq-read "] Session:^[[1m"      mold server-session] ;@@ limit session-id size? TLSe has it max 32 bytes

				if server-version <> version [
					print [
						"***** Version required by server is not same like clients:" 
						server-version version
					]

					tmp: *Protocol-version/name server-version

					if any [
						none? tmp
						server-version > version
					] [
						return *Alert/Protocol_version
					]

					version: server-version
				]

				unless empty? compressions [
					log-more ["R[" seq-read "] Compressions:^[[1m" mold compressions  ]
					log-error "COMPRESSION NOT SUPPORTED"
					return *Alert/Decompression_failure
				]

				unless TLS-init-cipher-suite ctx cipher-suite [
					log-error "Unsupported cipher suite!"
					return *Alert/Handshake_failure
				]

				;-- extensions handling
				clear server-extensions
				if ends > (i: binary/read in 'index) [
					pos: i + 2 + binary/read in 'ui16

					if ends <> pos [
						log-error  "Warning: unexpected number of extension bytes in SERVER_HELLO fragment!"
						log-error ["Expected position:" ends "got:" pos]
					]
					while [ 4 <= (ends - binary/read in 'index)][
						binary/read in [
							ext-type: UI16
							ext-data: UI16BYTES
						]
						log-more [
							"R[" seq-read "] Extension:" any [*TLS-Extension/name ext-type  ext-type]
							"bytes:" length? ext-data
						]
						repend server-extensions [
							ext-type ext-data
						]
					]
				]
				false ;= no error
			] [; ctx
				; WITH block catches RETURNs so just throw it again
				return ctx/critical-error
			]
		]
		;----------------------------------------------------------
		CERTIFICATE [
			assert-prev-state ctx [SERVER_HELLO]
			;probe ctx/in/buffer
			tmp: binary/read ctx/in [UI24 INDEX]
			if ends <> (tmp/1 + tmp/2) [
				log-error ["Improper certificate list end?" ends "<>" (tmp/1 + tmp/2)]
				return *Alert/Handshake_failure
			]
			i: 0
			while [ends > index? ctx/in/buffer][
				;probe copy/part ctx/in/buffer 10
				binary/read ctx/in [cert: UI24BYTES]
				;probe cert
				i: i + 1 write rejoin [%/x/cert i %.der] cert
				append ctx/server-certs decode 'CRT cert
			]
			log-more ["Received" length? ctx/server-certs "server certificates."]
			if error? try [
				;?? ctx/server-certs/1/public-key
				key: ctx/server-certs/1/public-key/2
				ctx/pub-key: key/1
				ctx/pub-exp: key/2
			][
				log-error "Missing public key in certifiate"
				return *Alert/Bad_certificate
			]
			;@@TODO: certificate validation
		]
		;----------------------------------------------------------
		SERVER_KEY_EXCHANGE [
			assert-prev-state ctx [CERTIFICATE SERVER_HELLO]
			log-more ["R[" ctx/seq-read "] Using key method:^[[1m" ctx/key-method]
			switch ctx/key-method [
				DHE_DSS
				DHE_RSA [
					binary/read ctx/in [
						s: INDEX
							dh_p:  UI16BYTES
							dh_g:  UI16BYTES
							dh_Ys: UI16BYTES
						e: INDEX
					]
					;store the complete message for signature test later
					message-len: e - s
					binary/read ctx/in [
						AT :s
						message: BYTES :message-len
					]
					;print ["DH:" dh_p dh_g dh_Ys] 
					;?? message
					
					hash-algorithm: 'md5_sha1
					sign-algorithm: 'rsa_sign


					;-- check signature

					if not ctx/legacy? [
						;signature
						hash-algorithm: *SignatureAlgorithm/name    binary/read ctx/in 'UI8
						sign-algorithm: *ClientCertificateType/name binary/read ctx/in 'UI8
						log-more ["R[" ctx/seq-read "] Using algorithm:" hash-algorithm "with" sign-algorithm]

						binary/read ctx/in [signature: UI16BYTES]
						;?? signature

						insert message rejoin [
							ctx/client-random
							ctx/server-random
						]
						message-hash: checksum/method message 'sha256 
						;print ["??? signature message length:" length? message]
						;?? message-hash

						either hash-algorithm = 'md5_sha1 [
							;__private_rsa_verify_hash_md5sha1
							log-error "legacy __private_rsa_verify_hash_md5sha1 not implemented yet!"
							;halt
						][
							log-more "Checking signature using RSA"
							if any [
								error? err: try [
									;decrypt the `signature` with server's public key
									rsa-key: apply :rsa-init ctx/server-certs/1/public-key/rsaEncryption
									signature: rsa/verify rsa-key signature
									;?? signature
									signature: decode 'der signature
								]
								;note tls1.3 is different a little bit here!
								message-hash <> signature/sequence/octet_string
							][
								log-error "Failed to validate signature"
								if error? err [print err]
								halt
								;@@TODO: alret: TLS_BROKEN_PACKET 
							]
							log-more "Signature valid!"
						]
					]

					if ends > pos: binary/read ctx/in 'index [
						len: ends - pos
						binary/read ctx/in [extra: BYTES :len]
						log-error [
							"Extra" len "bytes at the end of message:"
							mold extra
						]
						halt
					]

					ctx/dh-key: dh-init dh_g dh_p
					ctx/pre-master-secret: dh/secret ctx/dh-key dh_Ys
					log-more ["DH common secret:" mold ctx/pre-master-secret]
				]
				;@@ ECDHE_RSA ECDHE_ECDSA []
			]
		]
		;----------------------------------------------------------
		CERTIFICATE_REQUEST [
			assert-prev-state ctx [SERVER_HELLO SERVER_KEY_EXCHANGE CERTIFICATE]
			binary/read ctx/in [
				certificate_types:  UI8BYTES
				supported_signature_algorithms: UI16BYTES
			]
			log-more ["R[" ctx/seq-read "] certificate_types:   " certificate_types]
			log-more ["R[" ctx/seq-read "] signature_algorithms:" supported_signature_algorithms]
			if ends > binary/read ctx/in 'index [
				certificate_authorities: binary/read ctx/in 'UI16BYTES
			]
			if ends <> index? ctx/in/buffer [
				log-error ["Positions:" ends  index? ctx/in/buffer]
				log-error  "Looks we should read also something else!"
				halt
			]
		]
		;----------------------------------------------------------
		SERVER_HELLO_DONE [
			ctx/reading?: false
		]
	][
		log-error ["Unknown state: " new-state "-" type]
		return *Alert/Unexpected_message
	]

	if ends <> i: index? ctx/in/buffer [
		log-error ["Wrong fragment message tail!" ends "<>" i]
		log-error ["in/buffer starts:" mold copy/part ctx/in/buffer 20]
		return *Alert/Record_overflow
	]
	log-more ["R[" ctx/seq-read "] DONE: handshake^[[1m" ctx/state] log-----
	false ;= no error
]

send-event: function[
	type [word!]
	port [port!]
][
	log-debug ["Send-event:^[[1m" type]
	insert system/ports/system make event! compose [
		type: (to lit-word! type)
		port: (port)
	]
]

TLS-awake: function [event [event!]] [
	log-more ["AWAKE:^[[1m" event/type]
	port:       event/port
	TLS-port:   port/locals
	TLS-awake: :TLS-port/awake

	if all [
		TLS-port/state/protocol = 'APPLICATION
		not port/data
	] [
		; reset the data field when interleaving port r/w states
		;@@ TODO: review this part
		log-debug ["reseting data -> " mold TLS-port/data] 
		TLS-port/data: none
	]

	switch/default event/type [
		lookup [
			open port
			TLS-init TLS-port/state
			send-event 'lookup TLS-port
			return false
		]
		connect [
			do-commands TLS-port/state [client-hello]
			if none? TLS-port/state [return true] ;- probably closed meanwhile
			log-info ["CONNECT^[[22m: client-hello done; protocol:^[[1m" TLS-port/state/protocol]
			if TLS-port/state/protocol = 'HANDSHAKE [
				do-commands TLS-port/state [
					client-key-exchange
					change-cipher-spec
					finished
				]
			]
			send-event 'connect TLS-port
			return false
		]
		wrote [
			switch TLS-port/state/protocol [
				CLOSE-NOTIFY [
					return true
				]
				APPLICATION [
					send-event 'wrote TLS-port
					return false
				]
			]
			read port
			return false
		]
		read [
			;ask "????????????????????????"
			log-info ["READ" length? port/data "bytes proto-state:" TLS-port/state/protocol]
			complete?: TLS-read-data TLS-port/state port/data
			;? port
			if error-id: TLS-port/state/critical-error [
				;? TLS-port
				;? TLS-port/spec
				;? TLS-port/state/connection
				; trying to close internal connection before throwing the error
				if open? TLS-port [ try [close TLS-port] ]
				TLS-error error-id
			]
			log-debug ["Read complete?" complete?]
			if complete? [
				;? TLS-Port/state
				;? TLS-port/state/connection 
				TLS-port/data: TLS-port/state/port-data
				binary/init TLS-port/state/in none ; resets input buffer
			]
			either 'APPLICATION = TLS-port/state/protocol [
				send-event 'read TLS-port
			] [
				read port
			]
			return complete?
		]
		close [
			log-info "CLOSE"
			send-event 'close TLS-port
			return true
		]
	] [
		;try [close port/state/connection]
		close port
		do make error! rejoin ["Unexpected TLS event: " event/type]
	]
	false
]


sys/make-scheme [
	name: 'tls
	title: "TLS protocol v1.2"
	spec: make system/standard/port-spec-net []
	parent: none
	actor: [
		read: func [
			port [port!]
			/local
				resp data msg
		] [
			log-more "READ"
			;? port
			read port/state/connection
			return port
		]

		write: func [port [port!] value [any-type!]] [
			log-more "WRITE"
			;?? port/state/protocol
			if port/state/protocol = 'APPLICATION [ ;encrypted-handshake?
				do-commands/no-wait port/state compose [
					application (value)
				]
				return port
			]
		]

		open: func [port [port!] /local conn] [
			log-more "OPEN"
			if port/state [return port]

			if none? port/spec/host [TLS-error "Missing host address"]

			port/state: make-TLS-ctx

			port/state/connection: conn: make port! [
				scheme: 'tcp
				host:    port/spec/host
				port-id: port/spec/port-id
				ref:     rejoin [tcp:// host ":" port-id]
			]

			port/data: port/state/port-data

			conn/awake: :TLS-awake
			conn/locals: port
			open conn
			port
		]
		open?: func [port [port!]] [
			found? all [port/state open? port/state/connection]
		]
		close: func [port [port!] /local ctx] [
			log-more "CLOSE"
			;? port ? port/scheme
			unless port/state [return port]

			log-debug "Closing port/state/connection"
			;?? port/state/connection
			close port/state/connection

			; The symmetric ciphers used by TLS are able to encrypt chunks of
			; data one at a time.  It keeps the progressive state of the
			; encryption process in the -stream variables, which under the
			; hood are memory-allocated items stored as a HANDLE!.  The
			; memory they represent will not be automatically freed by
			; garbage collection.
			;
			; Calling the encryption functions with NONE! as the data to
			; input will assume you are done, and will free the handle.
			;
			; !!! Is there a good reason for not doing this with an ordinary
			; OBJECT! containing a BINARY! ?
			;
			switch port/state/crypt-method [
				RC4_128 [
					if port/state/encrypt-stream [
						rc4/stream port/state/encrypt-stream none
					]
					if port/state/decrypt-stream [
						rc4/stream port/state/decrypt-stream none
					]
				]
				AES_128_CBC
				AES_256_CBC [
					if port/state/encrypt-stream [
						aes/stream port/state/encrypt-stream none
					]
					if port/state/decrypt-stream [
						aes/stream port/state/decrypt-stream none
					]
				]
			]

			log-more "Port closed"
			port/state/connection/awake: none
			port/state: none
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
	set-verbose: :tls-verbosity
]
