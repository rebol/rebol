REBOL [
	title: "REBOL3 TLSv1.2 protocol scheme"
	name: 'tls
	type: 'module
	author: rights: ["Richard 'Cyphre' Smolak" "Oldes" "Brian Dickens (Hostilefork)"]
	version: 0.7.4
	history: [
		0.6.1 "Cyphre" "Initial implementation used in old R3-alpha"
		0.7.0 "Oldes" {
			* Rewritten almost from scratch to support TLSv1.2
			* Using BinCode (binary) dialect to deal with buffers input/output.
			* Code is now having a lots of traces for deep study of the process.
			Special thanks to Hostile Fork for implementing TLSv1.2 for his Ren-C before me.
		}
		0.7.1 "Oldes" {
			* Added Server Name Indication extension into TLS scheme
			* Fixed RSA/SHA message signatures
		}
		0.7.2 "Oldes" {
			* Basic support for EllipticCurves (x25519 still missing)
			* Added support for Chacha20-Poly1305 cipher suite
		}
		0.7.3 "Oldes" "Fixed RSA memory leak"
		0.7.4 "Oldes" "Pass data to parent handler even when ALERT message is not decoded"
	]
	todo: {
		* cached sessions
		* automagic cert data lookup
		* add more cipher suites (based on DSA, 3DES, ECDSA, ...)
		* server role support
		* TLS1.3 support
		* cert validation
	}
	references: [
		;The Transport Layer Security (TLS) Protocol v1.2
		https://tools.ietf.org/html/rfc5246

		https://testssl.sh/openssl-rfc.mapping.html
		https://fly.io/articles/how-ciphersuites-work/
		https://tls12.ulfheim.net/
		https://tls13.ulfheim.net/
		
		; If you want to get a report on what suites a particular site has:
		https://www.ssllabs.com/ssltest/analyze.html
	]
	notes: {
		Tested with:
			TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256
			TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA
			TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA
			TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256
			TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
			TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
			TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
			TLS_RSA_WITH_AES_128_CBC_SHA256
			TLS_RSA_WITH_AES_256_CBC_SHA256
			TLS_RSA_WITH_AES_128_CBC_SHA
			TLS_RSA_WITH_AES_256_CBC_SHA
			TLS_DHE_RSA_WITH_AES_128_CBC_SHA
			TLS_DHE_RSA_WITH_AES_256_CBC_SHA256

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

;   Elyptic curves:

	TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256:   #{CCA8}
	TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256: #{CCA9}
	TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384:         #{C028}
	TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256:         #{C02F}
	TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384:         #{C030}
	TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:       #{C02B}
	TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:       #{C02C}
	TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256:         #{C027}
	TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA:            #{C013}
	TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA:          #{C009}
	TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA:            #{C014}
	TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA:          #{C00A}

] 'TLS-Cipher-suite

*EllipticCurves: enum [
	secp192r1: #{0013}
	secp224k1: #{0014}
	secp224r1: #{0015}
	secp256k1: #{0016}
	secp256r1: #{0017}
	secp384r1: #{0018}
	secp521r1: #{0019}
	x25519:    #{001D}
] 'EllipticCurves

*HashAlgorithm: enum [
	none:       0
	md5:        1
	sha1:       2
	sha224:     3
	sha256:     4
	sha384:     5
	sha512:     6
	md5_sha1: 255
] 'TLSHashAlgorithm

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
	ServerName:          #{0000}
	SupportedGroups:     #{000A}
	SignatureAlgorithms: #{000D}
	KeyShare:            #{0033}
	RenegotiationInfo:   #{FF01} ;@@ https://tools.ietf.org/html/rfc5746
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
	#{CCA9} ;TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256
	#{CCA8} ;TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256
	;#{C02F} ;TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
	;#{C030} ;TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
	;#{C02B} ;TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
	;#{C02C} ;TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384
	;#{C028} ;TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384 ;need test with https://snappygoat.com/b/8d6492a33fee8f8f0ea289203fdf080608d9d61d
	#{C027} ;TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
	#{C014} ;TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
	#{C013} ;TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
	#{C00A} ;TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA
	#{C009} ;TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA
	;#{006A} ;TLS_DHE_DSS_WITH_AES_256_CBC_SHA256
	#{006B} ;TLS_DHE_RSA_WITH_AES_256_CBC_SHA256
	#{0067} ;TLS_DHE_RSA_WITH_AES_128_CBC_SHA256
	#{003D} ;TLS_RSA_WITH_AES_256_CBC_SHA256
	#{003C} ;TLS_RSA_WITH_AES_128_CBC_SHA256
	#{0035} ;TLS_RSA_WITH_AES_256_CBC_SHA
	#{002F} ;TLS_RSA_WITH_AES_128_CBC_SHA
	#{0038} ;TLS_DHE_DSS_WITH_AES_256_CBC_SHA
	#{0032} ;TLS_DHE_DSS_WITH_AES_128_CBC_SHA
	#{0039} ;TLS_DHE_RSA_WITH_AES_256_CBC_SHA
	#{0033} ;TLS_DHE_RSA_WITH_AES_128_CBC_SHA
	;- RC4 is prohibited by https://tools.ietf.org/html/rfc7465 for insufficient security
	;#{0004} ;TLS_RSA_WITH_RC4_128_MD5 
	;#{0005} ;TLS_RSA_WITH_RC4_128_SHA
]

supported-signature-algorithms: rejoin [
	;#{0703} ; curve25519 (EdDSA algorithm)
	#{0601} ; rsa_pkcs1_sha512
	#{0602} ; SHA512 DSA
	;#{0603} ; ecdsa_secp521r1_sha512
	#{0501} ; rsa_pkcs1_sha384
	#{0502} ; SHA384 DSA
	;#{0503} ; ecdsa_secp384r1_sha384
	#{0401} ; rsa_pkcs1_sha256
	#{0402} ; SHA256 DSA
	#{0403} ; ecdsa_secp256r1_sha256
	;#{0301} ; SHA224 RSA
	;#{0302} ; SHA224 DSA
	;#{0303} ; SHA224 ECDSA
	#{0201} ; rsa_pkcs1_sha1
	#{0202} ; SHA1 DSA
	;#{0203} ; ecdsa_sha1
]

supported-elliptic-curves: rejoin [
	;#{001D} ; x25519
	;#{0019} ; secp521r1
	;#{0018} ; secp384r1
	#{0017} ; secp256r1
	#{0015} ; secp224r1
	#{0014} ; secp224k1
	#{0013} ; secp192r1
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
			  "CHACHA20_POLY1305" (ctx/crypt-size: 32 ctx/IV-size: 12 ctx/block-size: 16  )
			;| "AES_256_GCM"  (ctx/crypt-size: 32 ctx/IV-size: 4  ctx/block-size: 16  )
			;| "AES_128_GCM"  (ctx/crypt-size: 16 ctx/IV-size: 4  ctx/block-size: 16  )
			| "AES_128_CBC"  (ctx/crypt-size: 16 ctx/IV-size: 16 ctx/block-size: 16  ) ; more common than AES_256_CBC
			| "AES_256_CBC"  (ctx/crypt-size: 32 ctx/IV-size: 16 ctx/block-size: 16  )
			;| "3DES_EDE_CBC" (ctx/crypt-size: 24 ctx/IV-size: 8  ctx/block-size: 8   )
			| "RC4_128"      (ctx/crypt-size: 16 ctx/IV-size: 0  ctx/block-size: none)
			| "NULL"         (ctx/crypt-size: 0  ctx/IV-size: 0  ctx/block-size: none)
		] #"_" [
			  "SHA384" end (ctx/hash-method: 'SHA384 ctx/mac-size: 48)
			| "SHA256" end (ctx/hash-method: 'SHA256 ctx/mac-size: 32)
			| "SHA"    end (ctx/hash-method: 'SHA1   ctx/mac-size: 20)
			| "MD5"    end (ctx/hash-method: 'MD5    ctx/mac-size: 16)
			| "NULL"   end (ctx/hash-method: none    ctx/mac-size: 0 )
			;NOTE: in RFC mac-size is named mac_length and there is also mac_key_length, which has same value
		]
		(
			ctx/key-method:   to word! key-method
			ctx/crypt-method: to word! cipher
			ctx/is-aead?: to logic! find [AES_128_GCM AES_256_GCM CHACHA20_POLY1305] ctx/crypt-method

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
][
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
		TLS-error *Alert/Internal_error
	]
]

; TLS protocol code

TLS-update-messages-hash: function [
	ctx [object!]
	msg [binary!]
	len [integer!]
][
	log-more ["Update-messages-hash bytes:" len "hash:" all [ctx/sha-port ctx/sha-port/spec/method]]
	if none? ctx/sha-port [
		either ctx/legacy? [
			ctx/sha-port: open checksum:sha1
			ctx/md5-port: open checksum:md5
		][
			ctx/sha-port: open either ctx/mac-size = 48 [checksum:sha384][checksum:sha256]
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
][
	change-state ctx 'CLIENT_HELLO
	with ctx [

		extensions: make binary! 100

		;- Server Name Indication (extension)
		;  https://tools.ietf.org/html/rfc6066#section-3
		if all [
			ctx/connection
			host-name: ctx/connection/spec/host
		][
			host-name: to binary! host-name
			length-name: length? host-name

			binary/write extensions compose [
				UI16  0                ; extension type (server_name=0)
				UI16 (5 + length-name) 
				UI16 (3 + length-name)
				UI8   0
				UI16  :length-name
				BYTES :host-name
			]
		]
		;- Supported Groups (extension)
		binary/write tail extensions compose [
			#{000A} ; assigned value for extension "supported groups"
			UI16 (2 + length? supported-elliptic-curves)
			UI16 (    length? supported-elliptic-curves)
			:supported-elliptic-curves
		]

		;- Supported Point Formats Extension
		append extensions #{000B 0002 01 00} ;only "uncompressed"

		;- Renegotiation Info (extension)
		; The presence of this extension prevents a type of attack performed with TLS renegotiation. 
		; https://kryptera.se/Renegotiating%20TLS.pdf
		; Advertise it, but refuse renegotiation
		append extensions #{ff01 0001 00} ; (extensionID, 1 byte length, zero byte)

		;- Signed certificate timestamp (extension)
		; The client provides permission for the server to return a signed certificate timestamp. 

		; This form of the client sending an empty extension is necessary because it is a fatal error
		; for the server to reply with an extension that the client did not provide first. Therefore 
		; the client sends an empty form of the extension, and the server replies with the extension 
		; populated with data, or changes behavior based on the client having sent the extension.
		append extensions #{0012 0000}


		;precomputing the extension's lengths so I can write them in one WRITE call
		length-signatures:  2 + length? supported-signature-algorithms
		length-extensions:  4 + length-signatures + length? extensions
		length-message:    41 + length-extensions + length? suported-cipher-suites
		length-record:      4 + length-message

		binary/write out [
			UI8       22                  ; protocol type (22=Handshake)
			UI16      :version            ; protocol version (minimal supported)
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
			BYTES     :extensions
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
][
	log-debug ["client-key-exchange -> method:" ctx/key-method "key-data:" mold ctx/key-data]

	change-state ctx 'CLIENT_KEY_EXCHANGE
	assert-prev-state ctx [CLIENT_CERTIFICATE SERVER_HELLO_DONE SERVER_HELLO]

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
			ECDHE_ECDSA
			ECDHE_RSA [
				log-more ["W[" ctx/seq-write "] Using ECDH key-method"]
				insert key-data #{04} ; ECDH key seems to have this byte at its head
				key-data-len-bytes: 1
			]
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
				key-data-len-bytes: 2
				log-more ["W[" ctx/seq-write "] key-data:" mold key-data]
				rsa rsa-key none ;@@ releases the internal RSA data, should be done by GC one day!
			]
			DHE_DSS
			DHE_RSA [
				log-more ["W[" ctx/seq-write "] Using DH key-method"]
				key-data-len-bytes: 2
			]
		]

		;compute used lengths
		length-message: key-data-len-bytes + length? key-data
		length-record:  4 + length-message

		;and write them with key data
		binary/write out compose [
			AT :pos-record-len UI16 :length-record
			AT :pos-message    UI24 :length-message
			; for ECDH only 1 byte is used to store length!
			AT :pos-key (pick [UI8BYTES UI16BYTES] key-data-len-bytes) :key-data
		]


		;-- make all secure data
		if ctx/version >= *Protocol-version/TLS1.0 [
			; NOTE: key-expansion is used just to generate keys so it does not need to be stored in context!
			ctx/master-secret: prf "master secret" legacy? (join ctx/client-random ctx/server-random) pre-master-secret 48
				key-expansion: prf "key expansion" legacy? (join ctx/server-random ctx/client-random) master-secret 
								   (mac-size + crypt-size + iv-size) * 2

			pre-master-secret: none ;-- not needed anymore

			;?? master-secret
			;?? key-expansion
			;?? mac-size
			;?? crypt-size
			;?? iv-size
		]

		unless is-aead? [
			client-mac-key: take/part key-expansion mac-size
			server-mac-key: take/part key-expansion mac-size
		]

		client-crypt-key: take/part key-expansion crypt-size
		server-crypt-key: take/part key-expansion crypt-size

		log-more ["Client-mac-key:   " mold client-mac-key  ]
		log-more ["Server-mac-key:   " mold server-mac-key  ]
		log-more ["Client-crypt-key: " mold client-crypt-key]
		log-more ["Server-crypt-key: " mold server-crypt-key]

		client-iv: take/part key-expansion iv-size
		server-iv: take/part key-expansion iv-size
		
		log-more ["Client-IV: " mold client-iv]
		log-more ["Server-IV: " mold server-iv]


		key-expansion: none

		switch crypt-method [
			CHACHA20_POLY1305 [
				aead: chacha20poly1305/init none client-crypt-key client-iv server-crypt-key server-iv 
			]
			RC4_128 [
				decrypt-stream: rc4/key server-crypt-key
			]
			;AES_128_GCM
			;AES_256_GCM [
			;	local-aead-iv:  copy client-iv
			;	remote-aead-iv: copy server-iv
			;]
		]

		TLS-update-messages-hash ctx (at head out/buffer pos-record) length-record
	]
]


change-cipher-spec: function [
	ctx [object!]
][
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
][
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
][
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
][
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
][
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
	;print "CRYPTED message!" ? data
	with ctx [
		binary/write bin compose [
			UI64  :seq-read
			UI8   23
			UI16  :version
		]
		either is-aead? [
			switch crypt-method [
				CHACHA20_POLY1305 [
					binary/write bin reduce ['UI16 (length? data) - 16]
					data: chacha20poly1305/decrypt aead data bin/buffer
				]
			]
		][
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
					UI16BYTES :data
				]
				mac-check: checksum/with bin/buffer hash-method server-mac-key

				;?? mac
				;?? mac-check

				if mac <> mac-check [ critical-error: *Alert/Bad_record_MAC ]
				
				if version > *Protocol-version/TLS1.0 [
					unset 'server-iv ;-- avoid reuse in TLS 1.1 and above
				]
			]
		]
		binary/init  bin 0 ;clear the temp bin buffer
	]
	unless data [
	;	critical-error: *Alert/Bad_record_MAC
	]
	data
]

encrypt-data: function [
	ctx     [object!]
	content [binary!]
	/type
		msg-type [integer!] "application data is default"
][
	;log-debug "--encrypt-data--"
	;? ctx
	msg-type: any [msg-type 23] ;-- default application

	;?? content

	with ctx [
		binary/write bin compose [
			UI64  :seq-write
			UI8   :msg-type
			UI16  :version
			UI16  (length? content)
		]
		either is-aead? [

			switch crypt-method [
				CHACHA20_POLY1305 [
					cipher: chacha20poly1305/encrypt aead content bin/buffer
				]
				AES_256_GCM	
				AES_128_GCM [
					;@@ TODO: needs AES_GCM computation			
					log-error ["Not yet implemented crypt-method:" crypt-method]
				]
			]

		][

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

			binary/write bin content

			MAC: checksum/with bin/buffer ctx/hash-method ctx/client-mac-key

			;?? MAC
			data: rejoin [content MAC]
			;??  block-size

			if block-size [
				; add the padding data in CBC mode
				padding: block-size - (remainder (1 + length? data) block-size)
				insert/dup tail data (to char! padding) (padding + 1)
			]

			switch crypt-method [
			; no need for default in this SWITCH as only available methods will pass
			; thru TLS-init-cipher-suite function call
				AES_256_CBC
				AES_128_CBC [
					unless encrypt-stream [
						encrypt-stream: aes/key client-crypt-key client-iv
					]
					cipher: aes/stream encrypt-stream data

					if version > *Protocol-version/TLS1.0 [
						; encrypt-stream must be reinitialized each time with the
						; new initialization vector.
						encrypt-stream: none
					]
				]
				RC4_128 [
					unless encrypt-stream [
						encrypt-stream: rc4/key client-crypt-key
					]
					cipher: rc4/stream encrypt-stream data
				]
			]

			;-- TLS versions 1.1 and above include the client-iv in plaintext.
			if version > *Protocol-version/TLS1.0 [
				insert cipher client-iv
				unset 'client-iv ;-- avoid accidental reuse
			]
		]
		binary/init bin 0 ;clear the bin buffer
	]
	cipher
]

decrypt-data: func [
	ctx [object!]
	data [binary!]
	/local
		crypt-data
][
	;print "DECRYPT DATA" ? data
	switch ctx/crypt-method [
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
		RC4_128 [
			rc4/stream ctx/decrypt-stream data
		]
	]
	data
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
		while [output-length > length? p-md5][
			a: checksum/with a 'md5 s-1 ; A(n)
			append p-md5 checksum/with rejoin [a seed] 'md5 s-1
		]

		p-sha1: copy #{}
		a: seed ; A(0)
		while [output-length > length? p-sha1][
			a: checksum/with a 'sha1 s-2 ; A(n)
			append p-sha1 checksum/with rejoin [a seed] 'sha1 s-2
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
	while [output-length >= length? p-sha256][
		a: checksum/with a 'sha256 secret
		append p-sha256 checksum/with rejoin [a seed] 'sha256 secret
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
][
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
		unless port? wait [ctx/connection 130][
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
	rest:        make binary! 8 ;packet may not be fully processed, this value is used to keep temporary leftover
	reading?:       false  ;if client is reading or writing data
	;server?:       false  ;always FALSE now as we have just a client
	protocol:       none   ;current protocol state. One of: [HANDSHAKE APPLICATION ALERT]
	state:         'lookup ;current state in context of the protocol
	state-prev:     none   ;previous state

	error:                 ;used to hold Rebol error object (for reports to above layer)
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

	aead: none ; used now for chacha20/poly1305 combo

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
	key-data:

	encrypt-stream:
	decrypt-stream: none

	connection: none
]]

TLS-init: func [
	"Resets existing TLS context"
	ctx [object!]
][
	ctx/seq-read: ctx/seq-write: 0
	ctx/protocol: ctx/state: ctx/state-prev: none
	ctx/cipher-spec-set: 0 ;no encryption yet

	ctx/legacy?: (255 & ctx/version) < 3 ;- TLSv1.1 and older

	clear ctx/server-certs

	;@@ review code bellow if reset for other crypt method is not needed:
	switch ctx/crypt-method [
		RC4_128 [
			if ctx/encrypt-stream [
				rc4/stream ctx/encrypt-stream none
				rc4/stream ctx/decrypt-stream none
				ctx/encrypt-stream: none
				ctx/decrypt-stream: none
			]
		]
	]
]


TLS-read-data: function [
	ctx       [object!]
	tcp-data  [binary!] 
][
	log-more ["read-data:^[[1m" length? tcp-data "^[[22mbytes previous rest:" length? ctx/rest]

	inp: ctx/in

	binary/write inp ctx/rest  ;- possible leftover from previous packet
	binary/write inp tcp-data  ;- fills input buffer with received data
	clear tcp-data
	clear ctx/rest

	ctx/reading?: true

	while [ctx/reading? and ((available: length? inp/buffer) >= 5)][

		log-debug ["Data starts: " mold copy/part inp/buffer 16]

		binary/read inp [
			start:   INDEX
			type:    UI8
			version: UI16
			len:     UI16
		]
		available: available - 5

		log-debug ["fragment type: ^[[1m" type "^[[22mver:^[[1m" version *Protocol-version/name version "^[[22mbytes:^[[1m" len "^[[22mbytes"]

		if all [
			ctx/server-version
			version <> ctx/server-version
		][
			log-error ["Version mismatch:^[[22m" version "<>" ctx/server-version]
			ctx/critical-error: *Alert/Internal_error
			return false
		]

		if available < len [
			;probe inp/buffer
			log-info ["Incomplete fragment:^[[22m available^[[1m" available "^[[22mof^[[1m" len "^[[22mbytes"]
			;?? inp/buffer
			binary/read inp [AT :start] ;resets position
			log-debug ["Data starts: " mold copy/part inp/buffer 10]
			return false
		]

		*protocol-type/assert type
		*protocol-version/assert version

		protocol: *protocol-type/name type
		version:  *protocol-version/name version

		ctx/seq-read: ctx/seq-read + 1

		end: start + len + 5 ; header size is 5 bytes

		;log-debug "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
		log-info ["^[[22mR[" ctx/seq-read "] Protocol^[[1m" protocol "^[[22m" version "bytes:^[[1m" len "^[[22mfrom^[[1m" start "^[[22mto^[[1m" end]

		ctx/protocol: protocol

		switch protocol [
			HANDSHAKE [
				binary/read inp [data: BYTES :len]
				
				either ctx/cipher-spec-set > 1 [
					data: decrypt-msg ctx data
				][
					TLS-update-messages-hash ctx data len
				]
				
				ctx/critical-error: TLS-parse-handshake-message ctx data
			]
			ALERT [
				log-debug ["ALERT len:" :len "ctx/cipher-spec-set:" ctx/cipher-spec-set]
				binary/read inp [data: BYTES :len]
				if ctx/cipher-spec-set > 1 [
					log-debug ["Decrypting ALERT message:" mold data]
					data: decrypt-msg ctx data
					unless data [
						log-error "Failed to decode ALERT message!"
						;@@ TODO: inspect how it's possible that decrypt failes
						;@@ problem is when CHACHA20_POLY1305 is used.
						ctx/critical-error: none
						ctx/protocol: 'APPLICATION ; content is reported to higher level
						continue
					]
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
				][
					log-error ["*** CHANGE_CIPHER_SPEC value should be 1 but is:" value]
				]
			]
			APPLICATION [
				change-state ctx 'APPLICATION
				assert-prev-state ctx [APPLICATION ALERT FINISHED]
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
	unless empty? ctx/in/buffer [
		; keeping rest of unprocessed data for later use
		ctx/rest: copy ctx/in/buffer
	]
	return true
]

TLS-parse-handshake-message: function [
	ctx [object!]
	data [binary!]
][
	msg: binary data
	binary/read msg [type: UI8 len: UI24 start: INDEX]
	ends: start + len
	log-debug ["R[" ctx/seq-read "] length:" len "start:" start "ends:" ends "type:" type]

	change-state ctx *Handshake/name type

	;log-more ["R[" ctx/seq-read "] Handshake fragment:" ctx/state "length:" len "start:" start "ends:" ends]

	switch/default ctx/state [
		;----------------------------------------------------------
		SERVER_HELLO [
			;NOTE: `len` should be now >= 38 for TLS and 41 for DTLS
			assert-prev-state ctx [CLIENT_HELLO]

			if ctx/critical-error: with ctx [
				if any [
					error? try [
						binary/read msg [
							server-version: UI16
							server-random:  BYTES 32
							server-session: UI8BYTES						
							cipher-suite:   BYTES 2
							compressions:   UI8BYTES ;<- must be empty
						]
					]
					32 < length? server-session  ;@@ limit session-id size; TLSe has it max 32 bytes
				][
					log-error "Failed to read server hello."
					return *Alert/Handshake_failure
				]

				log-more ["R[" seq-read "] Version:" *Protocol-version/name server-version "len:" len "cipher-suite:" cipher-suite]
				log-more ["R[" seq-read "] Random: ^[[1m" mold server-random ]
				log-more ["R[" seq-read "] Session:^[[1m" mold server-session]

				if server-version <> version [
					log-error [
						"Version required by server:" server-version
						"is not same like clients:" version
					]
					
					; protocol downgrade (to v1.1) is not allowed now, would have to find out, how to make it
					if server-version <> version [
						return *Alert/Protocol_version
					]

					version: server-version
					;ctx/legacy?: (255 & version) < 3
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
				if ends > (i: binary/read msg 'index) [
					pos: i + 2 + binary/read msg 'ui16

					if ends <> pos [
						log-error  "Warning: unexpected number of extension bytes in SERVER_HELLO fragment!"
						log-error ["Expected position:" ends "got:" pos]
					]
					while [ 4 <= (ends - binary/read msg 'index)][
						binary/read msg [
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
			][; ctx
				; WITH block catches RETURNs so just throw it again
				return ctx/critical-error
			]
		]
		;----------------------------------------------------------
		CERTIFICATE [
			assert-prev-state ctx [SERVER_HELLO]
			;probe msg/buffer
			tmp: binary/read msg [UI24 INDEX]
			if ends <> (tmp/1 + tmp/2) [
				log-error ["Improper certificate list end?" ends "<>" (tmp/1 + tmp/2)]
				return *Alert/Handshake_failure
			]
			;i: 0
			while [ends > index? msg/buffer][
				;probe copy/part msg/buffer 10
				binary/read msg [cert: UI24BYTES]
				;probe cert
				;i: i + 1 write rejoin [%/x/cert i %.der] cert
				append ctx/server-certs decode 'CRT cert
			]
			log-more ["Received" length? ctx/server-certs "server certificates."]
			;? ctx/server-certs
			try/except [
				;?? ctx/server-certs/1/public-key
				;?? ctx/server-certs/1
				key: ctx/server-certs/1/public-key
				switch key/1 [
					ecPublicKey [
						ctx/pub-key: key/3
						remove ctx/pub-key  ;don't include the first byte 0x04
						ctx/pub-exp: key/2      ;curve name
					]
					rsaEncryption [
						ctx/pub-key: key/2/1
						ctx/pub-exp: key/2/2
					]
				]
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
				ECDHE_RSA
				ECDHE_ECDSA [
					;? msg/buffer
					try/except [
						binary/read msg [
						  s: INDEX
							ECCurveType: UI8  
							ECCurve:     UI16     ; IANA CURVE NUMBER
							pub_key:     UI8BYTES 
						  e: INDEX
						]
					][
						log-error "Error reading elyptic curve"
						return *Alert/User_cancelled
					]

					if any [
						3 <> ECCurveType
						4 <> take pub_key
						none? curve: *EllipticCurves/name ECCurve
					][
						log-error ["Unsupported ECurve type:" ECCurveType ECCurve ]
						return *Alert/User_cancelled
					]
					log-more ["R[" ctx/seq-read "] Elyptic curve type:" ECCurve "=>" curve]
					log-more ["R[" ctx/seq-read "] Elyptic curve data:" pub_key]
				]
				DHE_DSS
				DHE_RSA [
					;- has DS params
					binary/read msg [
						s: INDEX
							dh_p:    UI16BYTES
							dh_g:    UI16BYTES
							pub_key: UI16BYTES
						e: INDEX
					]
				]
			]

			;store the complete message for signature test later
			message-len: e - s
			binary/read msg [
				AT :s
				message: BYTES :message-len
			]
			;print ["DH:" dh_p dh_g dh_Ys] 
			;?? message
			
			hash-algorithm: 'md5_sha1
			sign-algorithm: 'rsa_sign
			;-- check signature
			unless ctx/legacy? [
				;signature
				hash-algorithm:         *HashAlgorithm/name binary/read msg 'UI8
				sign-algorithm: *ClientCertificateType/name binary/read msg 'UI8
				log-more ["R[" ctx/seq-read "] Using algorithm:" hash-algorithm "with" sign-algorithm]
				binary/read msg [signature: UI16BYTES]
				;? signature
				insert message rejoin [
					ctx/client-random
					ctx/server-random
				]
				if hash-algorithm = 'md5_sha1 [
					;__private_rsa_verify_hash_md5sha1
					log-error "legacy __private_rsa_verify_hash_md5sha1 not implemented yet!"
					return *Alert/Decode_error
				]
				message-hash: checksum message hash-algorithm
				;? message-hash
				if any [
					error? valid?: try [
						switch sign-algorithm [
							rsa_sign [
								log-more "Checking signature using RSA"
								;decrypt the `signature` with server's public key
								rsa-key: apply :rsa-init ctx/server-certs/1/public-key/rsaEncryption
								signature: rsa/verify rsa-key signature
								rsa rsa-key none ;@@ releases the internal RSA data, should be done by GC one day!
								;?? signature
								signature: decode 'der signature
								;note tls1.3 is different a little bit here!
								; test validity:
								message-hash == signature/sequence/octet_string
							]
							rsa_fixed_dh [
								log-more "Checking signature using RSA_fixed_DH"
								der: decode 'der signature
								signature: join der/2/2 der/2/4
								;? ctx/pub-key
								;? signature
								;? ctx/pub-key

								; test validity:
								ecdsa/verify/curve ctx/pub-key message-hash signature ctx/pub-exp
							]
						]
					]
					not valid?
				][
					log-error "Failed to validate signature"
					if error? valid? [print valid?]
					return *Alert/Decode_error
				]
				log-more "Signature valid!"
				if ends > pos: index? msg/buffer [
					len: ends - pos
					binary/read msg [extra: BYTES :len]
					log-error [
						"Extra" len "bytes at the end of message:"
						mold extra
					]
					return *Alert/Decode_error
				]

				if dh_p [
					dh-key: dh-init dh_g dh_p
					ctx/pre-master-secret: dh/secret dh-key pub_key
					log-more ["DH common secret:" mold ctx/pre-master-secret]
					ctx/key-data: dh/public/release dh-key
				]
				if curve [
					;- elyptic curve init
					;curve is defined above (send from server as well as server's public key)
					dh-key: ecdh/init none curve
					ctx/pre-master-secret: ecdh/secret dh-key pub_key
					log-more ["ECDH common secret:" mold ctx/pre-master-secret]
					; resolve the public key to supply it to server
					ctx/key-data: ecdh/public/release dh-key

				]
			]
		]
		;----------------------------------------------------------
		CERTIFICATE_REQUEST [
			assert-prev-state ctx [SERVER_HELLO SERVER_KEY_EXCHANGE CERTIFICATE]
			binary/read msg [
				certificate_types:  UI8BYTES
				supported_signature_algorithms: UI16BYTES
			]
			log-more ["R[" ctx/seq-read "] certificate_types:   " certificate_types]
			log-more ["R[" ctx/seq-read "] signature_algorithms:" supported_signature_algorithms]
			if ends > binary/read msg 'index [
				certificate_authorities: binary/read msg 'UI16BYTES
			]
			if ends <> index? msg/buffer [
				log-error ["Positions:" ends  index? msg/buffer]
				log-error  "Looks we should read also something else!"
				return *Alert/Decode_error
			]
		]
		;----------------------------------------------------------
		SERVER_HELLO_DONE [
			ctx/reading?: false
		]

		FINISHED [
			binary/read msg [verify-data: BYTES] ;rest of data
			;? verify-data
			seed: either ctx/legacy? [
				rejoin [
					read ctx/md5-port
					read ctx/sha-port
				]
			][	read ctx/sha-port ]
			;?? seed

			result: prf "client finished" ctx/legacy? seed ctx/master-secret  12
			;? result
			;ask ""



		]
	][
		log-error ["Unknown state: " ctx/state "-" type]
		return *Alert/Unexpected_message
	]

	if ends <> i: index? msg/buffer [
		log-error ["Wrong fragment message tail!" ends "<>" i]
		log-error ["in/buffer starts:" mold copy/part msg/buffer 20]
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

TLS-awake: function [event [event!]][
	log-more ["AWAKE:^[[1m" event/type]
	TCP-port:   event/port
	TLS-port:   TCP-port/locals
	TLS-awake: :TLS-port/awake

	if all [
		TLS-port/state/protocol = 'APPLICATION
		not TCP-port/data
	][
		; reset the data field when interleaving port r/w states
		;@@ TODO: review this part
		log-debug ["reseting data -> " mold TLS-port/data] 
		TLS-port/data: none
	]

	switch/default event/type [
		lookup [
			open TCP-port
			TLS-init TLS-port/state
			send-event 'lookup TLS-port
			return false
		]
		connect [
			error: try [
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
				if open? TLS-port [
					send-event 'connect TLS-port
					return false
				]
				TLS-error *Alert/Close_notify
			]
			if error? TLS-port/locals/state [
				; upper protocol was already closed and reports the error in its state
				; it's safe to throw the error now
				do TLS-port/locals/state
			]
			; in case that the upper protocol is not yet closed, store error and report it
			if TLS-port/state [ TLS-port/state/error: error ]
			send-event 'error TLS-port
			return true
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
			read TCP-port
			return false
		]
		read [
			error: try [
				log-info ["READ TCP" length? TCP-port/data "bytes proto-state:" TLS-port/state/protocol]
				;dump/fmt TCP-port/data
				complete?: TLS-read-data TLS-port/state TCP-port/data
				;? port
				if error-id: TLS-port/state/critical-error [
					;? TLS-port
					;? TLS-port/spec
					;? TLS-port/state/connection
					; trying to close internal connection before throwing the error
					TLS-error error-id
				]
				log-debug ["Read complete?" complete?]
				unless complete? [
					read TCP-port
					return false
				]
				TLS-port/data: TLS-port/state/port-data
				binary/init TLS-port/state/in none ; resets input buffer
				;?? TLS-port/state/protocol
				either 'APPLICATION = TLS-port/state/protocol [
					;print "------------------"
					;- report that we have data to higher layer
					send-event 'read TLS-port 
				][	read TCP-port ]
				return true
			]
			; on error:
			if TLS-port/state [ TLS-port/state/error: error ]
			send-event 'error TLS-port
			return true
		]
		close [
			log-info "CLOSE"
			send-event 'close TLS-port
			return true
		]
		error [
			if all [
				TLS-port/state
				TLS-port/state/state = 'lookup
			][
				TLS-port/state/error: make error! [
					code: 500 type: 'access id: 'cannot-open
					arg1: TCP-port/spec/ref
				]
			]
			send-event 'error TLS-port
			return true
		]
	][
		close TCP-port
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
		][
			log-more "READ"
			;? port
			read port/state/connection
			return port
		]

		write: func [port [port!] value [any-type!]][
			log-more "WRITE"
			;?? port/state/protocol
			if port/state/protocol = 'APPLICATION [ ;encrypted-handshake?
				do-commands/no-wait port/state compose [
					application (value)
				]
				return port
			]
		]

		open: func [port [port!] /local conn][
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
		open?: func [port [port!]][
			all [port/state open? port/state/connection true]
		]
		close: func [port [port!] /local ctx check1 check2][
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
		copy: func [port [port!]][
			if port/data [copy port/data]
		]
		query: func [port [port!]][
			all [port/state query port/state/connection]
		]
		length?: func [port [port!]][
			either port/data [length? port/data][0]
		]
	]
	set-verbose: :tls-verbosity
]
