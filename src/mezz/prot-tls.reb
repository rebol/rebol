REBOL [
	Title: "Rebol3 TLSv1.2 protocol scheme"
	Name:  tls
	Type:  module
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2022 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: ["Richard 'Cyphre' Smolak" "Oldes" "Brian Dickens (Hostilefork)"]
	Version: 0.9.2
	Date: 24-May-2022
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
		0.8.0 "Oldes" "Using new `crypt` port introduced in Rebol 3.8.0"
		0.9.0 "Oldes" "Added (limited) support for a `server` role"
		0.9.1 "Oldes" "Improved initialization to be able reuse already opened TCP port"
		0.9.2 "Oldes" "Added support for GCM and SHA384 crypt modes"
	]
	todo: {
		* cached sessions
		* automagic cert data lookup
		* add more cipher suites (based on DSA, 3DES, ECDSA, ...)
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

		https://hpbn.co/transport-layer-security-tls/
		
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

TLS-context: context [
	tcp-port:
	tls-port:
	encrypt-port:
	decrypt-port: 
	sha256-port:   ;used for progressive checksum computations
	sha384-port:   ;used for progressive checksum computations with SHA384 cipher modes
	sha-port:      ;one of the above
	md5-port:      ;used for progressive checksum computations (in TLSv1.0)

	version: none  ; TLS version (currently just TLSv1.2)

	in:  binary 16104 ;input binary codec
	out: binary 16104 ;output binary codec
	bin: binary 64    ;temporary binary

	port-data:   make binary! 32000 ;this holds received decrypted application data
	rest:        make binary! 8 ;packet may not be fully processed, this value is used to keep temporary leftover
	reading?:       false  ;if client is reading or writing data
	server?:        false  ;always FALSE now as we have just a client
	protocol:       none   ;current protocol state. One of: [HANDSHAKE APPLICATION ALERT]
	state:         'lookup ;current state in context of the protocol
	state-prev:     none   ;previous state

	error:                 ;used to hold Rebol error object (for reports to above layer)
	critical-error:        ;used to signalize error state
	cipher-suite:   none
	cipher-spec-set: 0     ;stores state of cipher spec exchange (0 = none, 1 = client, 2 = both)

	;- values defined inside TLS-init-cipher-suite:
	key-method:            ; one of: [RSA DH_DSS DH_RSA DHE_DSS DHE_RSA]
	hash-method:           ; one of: [MD5 SHA1 SHA256 SHA384]
	crypt-method:   none
	is-aead?:       false  ; crypt-method with "Authenticated Encryption with Additional Data" (not yet supported!)
	aad-length:
	tag-length:
	   IV-size:            ; The amount of data needed to be generated for the initialization vector.
	   IV-size-dynamic:
	  mac-size:            ; Size of message authentication code
	crypt-size:            ; The number of bytes from the key_block that are used for generating the write keys.
	block-size:  0         ; The amount of data a block cipher enciphers in one chunk; a block
						   ; cipher running in CBC mode can only encrypt an even multiple of
						   ; its block size.

	local-IV:
	local-mac:
	local-key:
	local-random:

	remote-IV:
	remote-mac:
	remote-key:
	remote-random:

	dh-key:
	aead: none ; used now for chacha20/poly1305 combo

	session-id: none        ; https://hpbn.co/transport-layer-security-tls/#tls-session-resumption
	server-certs: copy []
	extensions: copy []

	seq-read:  0 ; sequence counters
	seq-write: 0

	
	pre-secret:
	master-secret:
	certificate:
	pub-key: pub-exp:
	key-data:
		none
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
;   Elyptic curves:

	TLS_ECDHE_RSA_WITH_CHACHA20-POLY1305_SHA256:   #{CCA8}
	TLS_ECDHE_ECDSA_WITH_CHACHA20-POLY1305_SHA256: #{CCA9}
	TLS_ECDHE_RSA_WITH_AES-256-CBC_SHA384:         #{C028}
	TLS_ECDHE_RSA_WITH_AES-128-GCM_SHA256:         #{C02F}
	TLS_ECDHE_RSA_WITH_AES-256-GCM_SHA384:         #{C030}
	TLS_ECDHE_ECDSA_WITH_AES-128-GCM_SHA256:       #{C02B}
	TLS_ECDHE_ECDSA_WITH_AES-256-GCM_SHA384:       #{C02C}
	TLS_ECDHE_RSA_WITH_AES-128-CBC_SHA256:         #{C027}
	TLS_ECDHE_ECDSA_WITH_AES-256-CBC_SHA384:       #{C024}
	TLS_ECDHE_ECDSA_WITH_AES-128-CBC_SHA256:       #{C023}
	TLS_ECDHE_RSA_WITH_AES-128-CBC_SHA:            #{C013}
	TLS_ECDHE_ECDSA_WITH_AES-128-CBC_SHA:          #{C009}
	TLS_ECDHE_RSA_WITH_AES-256-CBC_SHA:            #{C014}
	TLS_ECDHE_ECDSA_WITH_AES-256-CBC_SHA:          #{C00A}

	TLS_ECDH_ECDSA_WITH_AES-256-GCM_SHA384: #{C02E} ; ECDH does not support Perfect Forward Secrecy (PFS)


	TLS_DHE_RSA_WITH_AES-128-CCM:     #{C09E}
	TLS_ECDHE_ECDSA_WITH_AES_256_CCM: #{C0AD}

;   The following CipherSuite definitions require that the server provide
;   an RSA certificate that can be used for key exchange.  The server may
;   request any signature-capable certificate in the certificate request
;   message.

	TLS_RSA_WITH_AES-128-GCM_SHA256:     #{009C}

	TLS_RSA_WITH_NULL_MD5:               #{0001}
	TLS_RSA_WITH_NULL_SHA:               #{0002}
	TLS_RSA_WITH_NULL_SHA256:            #{003B}
	TLS_RSA_WITH_RC4-128_MD5:            #{0004}
	TLS_RSA_WITH_RC4-128_SHA:            #{0005}
	TLS_RSA_WITH_3DES-EDE-CBC_SHA:       #{000A}
	TLS_RSA_WITH_AES-128-CBC_SHA:        #{002F}
	TLS_RSA_WITH_AES-256-CBC_SHA:        #{0035}
	TLS_RSA_WITH_AES-128-CBC_SHA256:     #{003C}
	TLS_RSA_WITH_AES-256-CBC_SHA256:     #{003D}

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

	TLS_DH_DSS_WITH_3DES-EDE-CBC_SHA:    #{000D}
	TLS_DH_RSA_WITH_3DES-EDE-CBC_SHA:    #{0010}
	TLS_DHE_DSS_WITH_3DES-EDE-CBC_SHA:   #{0013}
	TLS_DHE_RSA_WITH_3DES-EDE-CBC_SHA:   #{0016}
	TLS_DH_DSS_WITH_AES-128-CBC_SHA:     #{0030}
	TLS_DH_RSA_WITH_AES-128-CBC_SHA:     #{0031}
	TLS_DHE_DSS_WITH_AES-128-CBC_SHA:    #{0032}
	TLS_DHE_RSA_WITH_AES-128-CBC_SHA:    #{0033}
	TLS_DH_DSS_WITH_AES-256-CBC_SHA:     #{0036}
	TLS_DH_RSA_WITH_AES-256-CBC_SHA:     #{0037}
	TLS_DHE_DSS_WITH_AES-256-CBC_SHA:    #{0038}
	TLS_DHE_RSA_WITH_AES-256-CBC_SHA:    #{0039}
	TLS_DH_DSS_WITH_AES-128-CBC_SHA256:  #{003E}
	TLS_DH_RSA_WITH_AES-128-CBC_SHA256:  #{003F}
	TLS_DHE_DSS_WITH_AES-128-CBC_SHA256: #{0040}
	TLS_DHE_RSA_WITH_AES-128-CBC_SHA256: #{0067}
	TLS_DH_DSS_WITH_AES-256-CBC_SHA256:  #{0068}
	TLS_DH_RSA_WITH_AES-256-CBC_SHA256:  #{0069}
	TLS_DHE_DSS_WITH_AES-256-CBC_SHA256: #{006A}
	TLS_DHE_RSA_WITH_AES-256-CBC_SHA256: #{006B}

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

	TLS_DH_anon_WITH_RC4-128_MD5:        #{0018}
	TLS_DH_anon_WITH_3DES-EDE-CBC_SHA:   #{001B}
	TLS_DH_anon_WITH_AES-128-CBC_SHA:    #{0034}
	TLS_DH_anon_WITH_AES-256-CBC_SHA:    #{003A}
	TLS_DH_anon_WITH_AES-128-CBC_SHA256: #{006C}
	TLS_DH_anon_WITH_AES-256-CBC_SHA256: #{006D}

] 'TLS-Cipher-suite

*EllipticCurves: enum [
	secp192r1:  19 ;#{0013}
	secp224k1:  20 ;#{0014}
	secp224r1:  21 ;#{0015}
	secp256k1:  22 ;#{0016}
	secp256r1:  23 ;#{0017}
	secp384r1:  24 ;#{0018}
	secp521r1:  25 ;#{0019}
	bp256r1:    26 ;#{001A}
	bp384r1:    27 ;#{001B}
	bp512r1:    28 ;#{001C}
	curve25519: 29 ;#{001D} ;? or x25519
	curve448:   30 ;#{001E} ;? or x448
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
	ServerName:            #{0000}
	SupportedGroups:       #{000A}
	SupportedPointFormats: #{000B}
	SignatureAlgorithms:   #{000D}
	SCT:                   #{0012} ;Signed_certificate_timestamp -> https://www.rfc-editor.org/rfc/rfc6962.html
	EncryptThenMAC:        #{0016}
	ExtendedMasterSecret:  #{0017}
	KeyShare:              #{0033}
	RenegotiationInfo:     #{FF01} ;@@ https://tools.ietf.org/html/rfc5746
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
_log-----: does [print-horizontal-line]

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

decode-cipher-suites: function[
	bin [binary!]
][
	num: (length? bin) >> 1
	out: make block! num
	bin: binary bin
	loop num [
		append out cipher: *Cipher-suite/name binary/read bin 'UI16
		log-debug ["Cipher-suite:" cipher]
	]
	out
]
decode-supported-groups: function[
	bin [binary!]
][
	num: (length? bin) >> 1
	blk: make block! num
	loop num [
		append blk *EllipticCurves/name binary/read bin 'UI16
		bin: skip bin 2
	]
	log-debug ["SupportedGroups:" mold blk]
	blk
]
decode-extensions: function[
	bin [binary!]
][
	bin: binary bin
	out: make block! 4
	while [not empty? bin/buffer][
		binary/read bin [
			ext-type: UI16
			ext-data: UI16BYTES
		]
		ext-type: any [*TLS-Extension/name ext-type  ext-type]
		log-more ["Extension:" ext-type "bytes:" length? ext-data mold ext-data ]
		switch ext-type [
			SupportedGroups [ ext-data: decode-supported-groups skip ext-data 2 ]
		]
		repend out [ext-type ext-data]
	]
	out
]

;-- list of supported suites as a single binary
; This list is sent to the server when negotiating which one to use.  Hence
; it should be ORDERED BY CLIENT PREFERENCE (more preferred suites first).
; Use https://ciphersuite.info for security info!

suported-cipher-suites-binary: make binary! 60
if find system/catalog/ciphers 'chacha20-poly1305 [
	binary/write tail suported-cipher-suites-binary [
	UI16BE 52393 ;= CCA9 ;TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256 ;= recommended
	UI16BE 52392 ;= CCA8 ;TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256   ;= secure
	]
]
if find system/catalog/ciphers 'aes-128-gcm [
	binary/write tail suported-cipher-suites-binary [
	UI16BE 49195 ;= C02B ;TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 ;= recommended
	UI16BE 49196 ;= C02C ;TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 ;= recommended
	UI16BE 49199 ;= C02F ;TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256   ;= secure
	UI16BE 49200 ;= C030 ;TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384   ;= secure
	UI16BE   156 ;= 009C ;TLS_RSA_WITH_AES_128_GCM_SHA256         ;= weak
	]
]
if find system/catalog/ciphers 'aes-128-cbc [
	binary/write tail suported-cipher-suites-binary [
	;- CBC mode is considered to be weak, but still used!
	UI16BE 49192 ;= C028 ;TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384
	UI16BE 49188 ;= C024 ;TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384
	UI16BE 49191 ;= C027 ;TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
	UI16BE 49187 ;= C023 ;TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256
	UI16BE 49172 ;= C014 ;TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
	UI16BE 49171 ;= C013 ;TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
	UI16BE 49162 ;= C00A ;TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA
	UI16BE 49161 ;= C009 ;TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA
;	UI16BE   106 ;= 006A ;TLS_DHE_DSS_WITH_AES_256_CBC_SHA256
	UI16BE   107 ;= 006B ;TLS_DHE_RSA_WITH_AES_256_CBC_SHA256
	UI16BE   103 ;= 0067 ;TLS_DHE_RSA_WITH_AES_128_CBC_SHA256
	UI16BE    61 ;= 003D ;TLS_RSA_WITH_AES_256_CBC_SHA256
	UI16BE    60 ;= 003C ;TLS_RSA_WITH_AES_128_CBC_SHA256
	UI16BE    53 ;= 0035 ;TLS_RSA_WITH_AES_256_CBC_SHA
	UI16BE    47 ;= 002F ;TLS_RSA_WITH_AES_128_CBC_SHA
;	UI16BE    56 ;= 0038 ;TLS_DHE_DSS_WITH_AES_256_CBC_SHA
;	UI16BE    50 ;= 0032 ;TLS_DHE_DSS_WITH_AES_128_CBC_SHA
	UI16BE    57 ;= 0039 ;TLS_DHE_RSA_WITH_AES_256_CBC_SHA
	UI16BE    51 ;= 0033 ;TLS_DHE_RSA_WITH_AES_128_CBC_SHA
	]
]

;- RC4 is prohibited by https://tools.ietf.org/html/rfc7465 for insufficient security
;if native? :rc4 [
;	binary/write tail suported-cipher-suites-binary [
;	UI16BE 4 ;= 0004 ;TLS_RSA_WITH_RC4_128_MD5 
;	UI16BE 5 ;= 0005 ;TLS_RSA_WITH_RC4_128_SHA
;	]
;]

suported-cipher-suites: decode-cipher-suites suported-cipher-suites-binary

supported-signature-algorithms: rejoin [
;@@ TODO: review this list!
	;#{0703} ; curve25519 (EdDSA algorithm)
	#{0601} ; rsa_pkcs1_sha512
	;#{0602} ; SHA512 DSA
	#{0603} ; ecdsa_secp521r1_sha512
	#{0501} ; rsa_pkcs1_sha384
	;#{0502} ; SHA384 DSA
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

supported-elliptic-curves: make binary! 22
foreach [curve id] [
;;  curves in the prefered order!
;	curve448   30 ;#{001E}
;	curve25519 29 ;#{001D} ;= needs some work - signature-algorithm 0703 not done!
	secp521r1  25 ;#{0019}
;	bp512r1    28 ;        ;= needs tests!
;	bp384r1    27 ;        ;= needs tests!
;	bp256r1    26 ;        ;= needs tests!
	secp384r1  24 ;#{0018}
	secp256r1  23 ;#{0017}
	secp256k1  22 ;#{0016}
	secp224r1  21 ;#{0015}
	secp224k1  20 ;#{0014}
	secp192r1  19 ;#{0013}
][
	if find system/catalog/elliptic-curves curve [
		binary/write tail supported-elliptic-curves [UI16BE :id]
	]
]


TLS-init-cipher-suite: func [
	"Initialize context for current cipher-suite. Returns false if unknown suite is used."
	ctx          [object!]
	/local suite key-method cipher
][
	cipher: ctx/cipher-suite
	suite: *Cipher-suite/name :cipher
	unless suite [
		; unknown suite... convert back to binary befor reporting
		binary/write suite: #{} [UI16 :cipher]
		log-error ["Unknown cipher suite:" suite]
		return false
	]
	unless find suported-cipher-suites suite [
		unless ctx/server? [log-error ["Server requests" suite "cipher suite!"]]
		return false
	]

	log-info ["Init TLS Cipher-suite:^[[35m" suite "^[[22m" skip to binary! cipher 6]

	parse form suite [
		opt "TLS_"
		copy key-method to "_WITH_" 6 skip
		copy cipher [
			  "CHACHA20-POLY1305" (ctx/crypt-size: 32 ctx/IV-size: 12 ctx/block-size: 16  )
			| "AES-256-GCM"  (ctx/crypt-size: 32 ctx/IV-size: 4 ctx/IV-size-dynamic: 8 ctx/tag-length: ctx/block-size: 16 ctx/aad-length: 13 )
			| "AES-128-GCM"  (ctx/crypt-size: 16 ctx/IV-size: 4 ctx/IV-size-dynamic: 8 ctx/tag-length: ctx/block-size: 16 ctx/aad-length: 13 )
			| "AES-128-CBC"  (ctx/crypt-size: 16 ctx/IV-size: 16 ctx/block-size: 16  ) ; more common than AES-256-CBC
			| "AES-256-CBC"  (ctx/crypt-size: 32 ctx/IV-size: 16 ctx/block-size: 16  )
			;| "3DES-EDE-CBC" (ctx/crypt-size: 24 ctx/IV-size: 8  ctx/block-size: 8   )
			| "RC4-128"      (ctx/crypt-size: 16 ctx/IV-size: 0  ctx/block-size: none)
			| "NULL"         (ctx/crypt-size: 0  ctx/IV-size: 0  ctx/block-size: none)
		] #"_" [
			  "SHA384" end (ctx/hash-method: 'SHA384 ctx/mac-size: 48)
			| "SHA256" end (ctx/hash-method: 'SHA256 ctx/mac-size: 32)
			| "SHA"    end (ctx/hash-method: 'SHA1   ctx/mac-size: 20)
			| "SHA512" end (ctx/hash-method: 'SHA512 ctx/mac-size: 64)
			| "MD5"    end (ctx/hash-method: 'MD5    ctx/mac-size: 16)
			| "NULL"   end (ctx/hash-method: none    ctx/mac-size: 0 )
			;NOTE: in RFC mac-size is named mac_length and there is also mac_key_length, which has same value
		]
		(
			ctx/key-method:   to word! key-method
			ctx/crypt-method: to word! cipher
			ctx/is-aead?: to logic! find [AES-128-GCM AES-256-GCM CHACHA20-POLY1305] ctx/crypt-method

			either ctx/hash-method = 'SHA384 [
				; upgrade hashing from default sha256 to sha384
				close ctx/sha256-port
				ctx/sha256-port: none
				ctx/sha-port: ctx/sha384-port
			][
				; the sha384 hasning is not needed anymore
				close ctx/sha384-port
				ctx/sha384-port: none
			]

			log-more [
				"Key:^[[1m" ctx/key-method
				"^[[22mcrypt:^[[1m" ctx/crypt-method
				"^[[22msize:^[[1m" ctx/crypt-size
				"^[[22mIV:^[[1m" ctx/IV-size 
			]
		)
	]
]

cause-TLS-error: func [
	name [word!]
	/local message
][
	message: replace/all form name #"_" #" "

	log-error join "ERROR: " message

	do make error! [
		type: 'Access
		id: 'Protocol
		arg1: message
	]
]

change-state: function [
	ctx [object!]
	new-state [word!]
][
	ctx/state-prev: ctx/state
	if ctx/state <> new-state [
		log-more ["New state:^[[33m" new-state "^[[22mfrom:" ctx/state]
		ctx/state: new-state
	]
]

assert-prev-state: function [
	ctx [object!]
	legal-states [block!]
][
	if not find legal-states ctx/state-prev [
		log-error ["State" ctx/state "is not expected after" ctx/state-prev]
		cause-TLS-error 'Internal_error
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
		ctx/sha256-port: open checksum:sha256
		ctx/sha384-port: open checksum:sha384
		ctx/sha-port: ctx/sha256-port
		log-more ["Initialized SHA method:" ctx/sha-port/spec/method]
	]
	unless ctx/hash-method [ write/part ctx/sha384-port msg len ]
	write/part ctx/sha-port msg len
	log-debug ["messages-hash:" read ctx/sha-port]
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
			ctx/tcp-port
			host-name: ctx/tcp-port/spec/host
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

		;- encrypt_then_mac extension
		;append extensions #{0016 0000} ;???
		;- extended_master_secret extension
		;append extensions #{0017 0000} ;???
		;- session ticket extension
		;append extensions #{0023 0000} ;empty!

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
		length-message:    41 + length-extensions + length? suported-cipher-suites-binary
		length-record:      4 + length-message

		binary/write out [
			UI8       22                  ; protocol type (22=Handshake)
			UI16      :version            ; protocol version (minimal supported)
			UI16      :length-record      ; length of SSL record data
			;client-hello message:
			UI8       1                   ; protocol message type	(1=ClientHello)
			UI24      :length-message     ; protocol message length
			UI16      :version            ; max supported version by client
			
			;-- It's not recommended to use unix-time timestamp here anymore!
			;-- https://datatracker.ietf.org/doc/html/draft-mathewson-no-gmtunixtime-00
			;@@ UNIXTIME-NOW RANDOM-BYTES 28  ; random struct
			;-- So let's use just fully random struct..
			RANDOM-BYTES 32               ; random struct

			UI8       0                   ; session ID length
			UI16BYTES :suported-cipher-suites-binary
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
		
		local-random: copy/part (at out/buffer 12) 32
		TLS-update-messages-hash ctx (at out/buffer 6) (4 + length-message)
		log-more [
			"W[" ctx/seq-write "] Bytes:" length? out/buffer "=>"
			"record:"     length-record
			"message:"    length-message
			"extensions:" length-extensions
			"signatures:" length-signatures
		]
		log-more ["W[" ctx/seq-write "] CRandom:^[[32m" local-random]
		ctx/seq-write: ctx/seq-write + 1
	]
]

server-hello: function [ctx [object!]][
	change-state ctx 'SERVER_HELLO
	with ctx [
		binary/init out none ;reset output buffer

		session-id: checksum to-binary 1 'sha256 ;@@ real number!
		extensions: #{00000000FF01000100000B000403000102} ;@@ real!

		binary/write out [
		 pos-start:
			UI8       22        ; protocol type (22=Handshake)
			UI16      :version  ; protocol version (minimal supported)
		 pos-record-len:
			UI16      0         ; will be set later
		 pos-record:
			;server-hello message:
			UI8       2         ; protocol message type (2=ServerHello)
		 pos-message-len:
			UI24      0         ; will be set later
			UI16      :version  ; prefered version by server
			UNIXTIME-NOW RANDOM-BYTES 28  ; random struct
			UI8BYTES  :session-id
			UI16      :cipher-suite
			UI8       0         ;no compression
			UI16BYTES :extensions
		 pos-end:
		]
		local-random: copy/part (at out/buffer 12) 32
		log-more ["W[" ctx/seq-write "] SRandom:^[[32m" local-random]
		
		;; fill the missing lengths
		binary/write out compose [
			AT :pos-record-len  UI16 (length-record:  pos-end - pos-record)
			AT :pos-message-len UI24 (length-message: length-record - 4)
			AT :pos-end
		]
		;; and count record's hash
		TLS-update-messages-hash ctx (at head out/buffer :pos-record) :length-record
		log-more [
			"W[" ctx/seq-write "] Bytes:" pos-end - pos-start "=>"
			"record:"     length-record
			"message:"    length-message
		]
	]
]
server-certificate: function [ctx [object!]][
	change-state ctx 'CERTIFICATE
	with ctx [
		; certificates are stored in the server (tpc's parent) settings
		certificates: tcp-port/parent/state/certificates
		binary/write out [
		 pos-start:
			UI8       22        ; protocol type (22=Handshake)
			UI16      :version  ; protocol version (minimal supported)
		 pos-record-len:
			UI16      0         ; will be set later
		 pos-record:
			;certificate message:
			UI8       11         ; protocol message type (11=Certificate)
		 pos-message-len:
			UI24      0         ; will be set later
			UI24BYTES :certificates
		 pos-end:
		]
		;; fill the missing lengths
		binary/write out compose [
			AT :pos-record-len  UI16 (length-record:  pos-end - pos-record)
			AT :pos-message-len UI24 (length-message: length-record - 4)
			AT :pos-end
		]
		;; and count record's hash
		TLS-update-messages-hash ctx (at head out/buffer :pos-record) :length-record
		log-more [
			"W[" ctx/seq-write "] Bytes:" pos-end - pos-start "=>"
			"record:"     length-record
			"message:"    length-message
		]
		;? key-method
		if find [ECDHE_RSA ECDHE_ECDSA DHE_RSA] key-method [
			;; is emphemeral cipher
			change-state ctx 'SERVER_KEY_EXCHANGE
			binary/write out [
			 pos-start:
				UI8       22        ; protocol type (22=Handshake)
				UI16      :version  ; protocol version (minimal supported)
			 pos-record-len:
				UI16      0         ; will be set later
			 pos-record:
				UI8       12         ; protocol message type (12=ServerKeyExchange)
			 pos-message-len:
				UI24      0         ; will be set later
			]

			switch key-method [
				ECDHE_RSA [
					spec: TCP-port/parent/state
					;@@ TODO: make sure, that curve is supported by client!
					curve: first spec/elliptic-curves
					dh-key: ecdh/init none curve
					pub-key: ecdh/public dh-key

					;log-more ["Server's ECDHE" curve "pub-key:^[[1m" mold pub-key]

					curve: *EllipticCurves/:curve  ; converted to integer for writting
					;@@ TODO: detect sign-algorithm from the certificate!
					sign-algorithm: *ClientCertificateType/rsa_sign
					hash-method-int: *HashAlgorithm/:hash-method

					binary/write message: #{} [
						BYTES :remote-random
						BYTES :local-random
					 pos-msg:
						UI8      3 ;= ECDHE_RSA
						UI16     :curve
						UI8BYTES :pub-key
					]
					signature: rsa/sign/hash spec/private-key :message :hash-method
					remove/part message (pos-msg - 1) ; removing random bytes
					binary/write out [
						BYTES     :message
						UI8       :hash-method-int
						UI8       :sign-algorithm
						UI16BYTES :signature
					 pos-end:
					]
				] 
			]
			;; fill the missing lengths
			binary/write out compose [
				AT :pos-record-len  UI16 (length-record:  pos-end - pos-record)
				AT :pos-message-len UI24 (length-message: length-record - 4)
				AT :pos-end
			]
			;; and count record's hash
			TLS-update-messages-hash ctx (at head out/buffer :pos-record) :length-record
			log-more [
				"W[" ctx/seq-write "] Bytes:" pos-end - pos-start "=>"
				"record:"     length-record
				"message:"    length-message
			]
		]
	]
]
server-hello-done: function [ctx [object!]][
	change-state ctx 'SERVER_HELLO_DONE
	with ctx [
		binary/write out [
		 pos-start:
			UI8       22        ; protocol type (22=Handshake)
			UI16      :version  ; protocol version (minimal supported)
		 pos-record-len:
			UI16      0         ; will be set later
		 pos-record:
			;server-hello-done message:
			UI8       14        ; protocol message type (14=ServerHelloDone)
		 pos-message-len:
		 	UI24      0         ; will be set later
		 pos-end:
		]
		;; fill the missing lengths
		binary/write out compose [
			AT :pos-record-len  UI16 (length-record:  pos-end - pos-record)
			AT :pos-message-len UI24 (length-message: length-record - 4)
			AT :pos-end
		]
		;; and count record's hash
		TLS-update-messages-hash ctx (at head out/buffer :pos-record) :length-record
		log-more [
			"W[" ctx/seq-write "] Bytes:" pos-end - pos-start "=>"
			"record:"     length-record
			"message:"    length-message
		]
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
				log-more ["W[" seq-write "] Using ECDH key-method"]
				key-data-len-bytes: 1
			]
			RSA [
				log-more ["W[" seq-write "] Using RSA key-method"]

				; generate pre-secret
				binary/write bin [
					UI16 :version RANDOM-BYTES 46 ;writes genereted secret (first 2 bytes are version)
				]
				; read the temprary random bytes back to store them for client's use
				binary/read bin [pre-secret: BYTES 48]
				binary/init bin 0 ;clears temp bin buffer


				log-more ["W[" seq-write "] pre-secret:" mold pre-secret]

				;log-debug "encrypting pre-secret:"

				;?? pre-secret
				;?? pub-key
				;?? pub-exp

				rsa-key: rsa-init pub-key pub-exp

				; supply encrypted pre-secret to server
				key-data: rsa/encrypt rsa-key pre-secret
				key-data-len-bytes: 2
				log-more ["W[" seq-write "] key-data:" mold key-data]
				release :rsa-key ; don't wait on GC and release it immediately
			]
			DHE_DSS
			DHE_RSA [
				log-more ["W[" seq-write "] Using DH key-method"]
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
		TLS-key-expansion ctx
		TLS-update-messages-hash ctx (at head out/buffer pos-record) length-record
		ctx/seq-write: ctx/seq-write + 1
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
	log-debug "application-data"
	log-more ["W[" ctx/seq-write "] application data:" length? message "bytes"]
	;prin "unencrypted: " ?? message
	message: encrypt-data ctx to binary! message
	;prin "encrypted: " ?? message
	with ctx [
		binary/write out [
			UI8       23        ; protocol type (23=Application)
			UI16      :version  ; protocol version
			UI16BYTES :message 
		]
		++ seq-write
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
	change-state ctx 'FINISHED
	ctx/seq-write: 0

	seed: read ctx/sha-port

	unencrypted: rejoin [
		#{14}		; protocol message type	(20=Finished)
		#{00000C}   ; protocol message length (12 bytes)
		prf :ctx/sha-port/spec/method either ctx/server? ["server finished"]["client finished"] seed ctx/master-secret  12
	]
	
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
	;?? unencrypted
	encrypted: encrypt-data/type ctx unencrypted 22
	;?? encrypted
	;? ctx/out
	with ctx [
		binary/write out [
			UI8 22					; protocol type (22=Handshake)
			UI16 :version			; protocol version
			UI16BYTES :encrypted
		]
		++ seq-write
	]
]

decrypt-msg: function [
	ctx [object!]
	data [binary!]
	type [integer!]
][
	;print "CRYPTED message!" 
	with ctx [
		binary/write bin compose [
			UI64  :seq-read
			UI8   :type
			UI16  :version
		]
		either is-aead? [

			if crypt-method <> 'CHACHA20-POLY1305 [
				change/part skip remote-IV 4 take/part data 8 8
				;? remote-IV
				modify decrypt-port 'iv remote-IV

				log-more ["Remote-IV:^[[32m" remote-IV]
			]

			binary/write bin reduce ['UI16 (length? data) - 16]
			write  decrypt-port  bin/buffer ; AAD chunk

			log-more ["AAD:      ^[[32m" bin/buffer]

			mac: take/last/part data 16 ; expected mac
			data: read write decrypt-port data
			if mac <> tag: take decrypt-port [
				log-debug "Failed to validate MAC after decryption!"
				log-debug ["Expected:" mac]
				log-debug ["Counted: " tag]
				critical-error: 'Bad_record_MAC
			]
		][
			if block-size [
				;server's initialization vector is new with each message
				remote-IV: take/part data block-size
			]
			;?? data
			;? decrypt-port
			;? block-size
			modify decrypt-port 'init-vector remote-IV
			data: read update write decrypt-port :data

			;change data decrypt-data ctx data
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
				;?? bin/buffer
				mac-check: checksum/with bin/buffer hash-method remote-mac

				;?? mac
				;?? mac-check

				if mac <> mac-check [ critical-error: 'Bad_record_MAC ]
				
				unset 'remote-IV ;-- avoid reuse in TLS 1.1 and above
			]
		]
		binary/init  bin 0 ;clear the temp bin buffer
	]
	unless data [
	;	critical-error: 'Bad_record_MAC
	]
	;? data
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
		; record header
		binary/write bin compose [
			UI64  :seq-write
			UI8   :msg-type
			UI16  :version
			UI16  (length? content)
		]
		either is-aead? [
			aad: bin/buffer
			either crypt-method = 'CHACHA20-POLY1305 [
				write encrypt-port :aad
				; on next line are 3 ops.. encrypting content, counting its MAC and getting the result  
				encrypted: read update write encrypt-port content
			][
				; update dynamic part of the IV
				binary/write local-IV [ATz :IV-size UI64be :seq-write]
				
				log-more ["Local-IV:   ^[[32m" local-IV]
				log-more ["AAD:        ^[[32m" bin/buffer]
				
				modify encrypt-port 'iv local-IV
				write  encrypt-port  :aad
				encrypted: read update write encrypt-port content
				if IV-size-dynamic > 0 [
					insert encrypted copy/part skip local-IV :IV-size :IV-size-dynamic
				]

			]
		][

			;@@ GenericBlockCipher: https://tools.ietf.org/html/rfc5246#section-6.2.3.2
			; "The Initialization Vector (IV) SHOULD be chosen at random, and
			;  MUST be unpredictable.  Note that in versions of TLS prior to 1.1,
			;  there was no IV field, and the last ciphertext block of the
			;  previous record (the "CBC residue") was used as the IV.  This was
			;  changed to prevent the attacks described in [CBCATT].  For block
			;  ciphers, the IV length is SecurityParameters.record_iv_length,
			;  which is equal to the SecurityParameters.block_size."
			;
			binary/write clear local-IV [RANDOM-BYTES :block-size]
			modify encrypt-port 'init-vector local-IV

			;?? ctx/seq-write
			log-more ["Local-IV:   ^[[32m" local-IV]
			log-more ["Local-mac:  ^[[32m" local-mac]
			log-more ["Hash-method:^[[32m" hash-method]

			; Message Authentication Code
			; https://tools.ietf.org/html/rfc5246#section-6.2.3.1

			binary/write bin content
			; computing MAC on the header + content 
			;?? bin/buffer
			MAC: checksum/with bin/buffer ctx/hash-method ctx/local-mac
			; padding the message to achieve a multiple of block length
			len: length? append content MAC
			;?? MAC ?? content ??  block-size ?? len
			if block-size [
				; add the padding data in CBC mode (PKCS5 Padding)
				padding: block-size - ((len + 1) % block-size)
				insert/dup tail content padding padding + 1
				;?? padding
			]
			;?? content

			; on next line are 3 ops.. encrypting content, padding and getting the result  
			encrypted: read update write encrypt-port content

			;-- TLS versions 1.1 and above include the local-IV in plaintext.
			insert encrypted local-IV
			;clear local-IV ;-- avoid accidental reuse
		]
		binary/init bin 0 ;reset the bin buffer
	]
	encrypted
]

prf: function [
	{(P)suedo-(R)andom (F)unction, generates arbitrarily long binaries}
	hash    [word!]
	label   [string! binary!]
	seed    [binary!]
	secret  [binary!]
	output-length [integer!]
][
	; The seed for the underlying P_<hash> is the PRF's seed appended to the
	; label.  The label is hashed as-is, so no null terminator.
	;
	; PRF(secret, label, seed) = P_<hash>(secret, label + seed)
	;

	log-more ["PRF" hash mold label "len:" output-length]
	seed: join to binary! label seed

	; TLS 1.2 includes the pseudorandom function as part of its cipher
	; suite definition.  No cipher suites assume the md5/sha1 combination
	; used above by TLS 1.0 and 1.1.  All cipher suites listed in the
	; TLS 1.2 spec use `P_SHA256`, which is driven by the single SHA256
	; hash function: https://tools.ietf.org/html/rfc5246#section-5

	p-sha256: make binary! output-length
	a: seed ; A(0)
	while [output-length >= length? p-sha256][
		a: checksum/with a :hash :secret
		append p-sha256 checksum/with append copy :a :seed :hash :secret
		;?? p-sha256
	]
	;trim the result to required output length
	clear at p-sha256 (1 + output-length)
	;log-more ["PRF result length:" length? p-sha256 mold p-sha256]
	p-sha256
]

TLS-key-expansion: func[
	ctx [object!]
	/local rnd1 rnd2 key-expansion sha
][
	with ctx [
		;-- make all secure data
		either server? [
			rnd1: append copy ctx/remote-random ctx/local-random
			rnd2: append copy ctx/local-random ctx/remote-random
		][
			rnd2: append copy ctx/remote-random ctx/local-random
			rnd1: append copy ctx/local-random ctx/remote-random
		]
		sha: ctx/sha-port/spec/method
		master-secret: prf :sha "master secret" rnd1 pre-secret 48
		key-expansion: prf :sha "key expansion" rnd2 master-secret (mac-size + crypt-size + iv-size) * 2

		either server? [
			unless is-aead? [
				remote-mac: take/part key-expansion mac-size
				local-mac:  take/part key-expansion mac-size
			]

			remote-key: take/part key-expansion crypt-size
			local-key:  take/part key-expansion crypt-size

			remote-IV: take/part key-expansion iv-size
			local-IV:  take/part key-expansion iv-size
		][
			unless is-aead? [
				local-mac:  take/part key-expansion mac-size
				remote-mac: take/part key-expansion mac-size
			]

			local-key:  take/part key-expansion crypt-size
			remote-key: take/part key-expansion crypt-size

			local-IV:  take/part key-expansion iv-size
			remote-IV: take/part key-expansion iv-size
		]
		if IV-size-dynamic > 0 [
			append/dup local-IV  0 IV-size-dynamic
			append/dup remote-IV 0 IV-size-dynamic
		]
		log-more ["Local-IV:   ^[[32m" local-IV]
		log-more ["Local-mac:  ^[[32m" local-mac]
		log-more ["Local-key:  ^[[32m" local-key]
		log-more ["Remote-IV:  ^[[32m" remote-IV]
		log-more ["Remote-mac: ^[[32m" remote-mac]
		log-more ["Remote-key: ^[[32m" remote-key]

		encrypt-port: open [
			scheme:      'crypt
			algorithm:   :crypt-method
			init-vector: :local-IV
			key:         :local-key
		]
		decrypt-port: open [
			scheme:      'crypt
			direction:   'decrypt
			algorithm:   :crypt-method
			init-vector: :remote-IV
			key:         :remote-key
		]

		;@@ TODO: could be supported in the spec directly, but that is not implemented yet!
		modify encrypt-port 'aad-length :aad-length
		modify decrypt-port 'aad-length :aad-length
		if tag-length > 0 [
			modify decrypt-port 'tag-length :tag-length
			modify encrypt-port 'tag-length :tag-length
		]

		; not needed anymore...
		pre-secret: local-key: remote-key: none	
	]
]

;----------------------------------------------;
;--- TLS scheme -------------------------------;
;----------------------------------------------;

TLS-init-context: func [
	"Resets existing TLS context"
	ctx [object!]
][
	ctx/seq-read: ctx/seq-write: 0
	ctx/protocol: ctx/state: ctx/state-prev: none
	ctx/cipher-spec-set: 0 ;no encryption yet
	clear ctx/server-certs
]

TLS-init-connection: function [ctx [object!]][
	binary/init ctx/out none ;reset output buffer
	client-hello ctx
	do-TCP-write ctx
	
	false
]

TLS-read-data: function [
	ctx       [object!]
	tcp-data  [binary!] 
][
;@@ NOTE: this is not the best solution! Complete stream is collected in the `inp` buffer,
;@@       but we need just parts of it, before it is decrypted! Unfortunatelly the current
;@@       bincode does not allow shrinking of the buffer :-/  NEEDS REWRITE!!!            

	log-debug ["read-data:^[[1m" length? tcp-data "^[[22mbytes previous rest:" length? ctx/rest]
	inp: ctx/in

	binary/write inp ctx/rest  ;- possible leftover from previous packet
	binary/write inp tcp-data  ;- fills input buffer with received data
	clear tcp-data
	clear ctx/rest

	ctx/reading?: true

	while [ctx/reading? and ((available: length? inp/buffer) >= 5)][

		;log-debug ["Data starts: " copy/part inp/buffer 16]

		binary/read inp [
			start:          INDEX
			type:           UI8
			server-version: UI16
			len:            UI16
		]
		available: available - 5

		log-debug ["fragment type: ^[[1m" type "^[[22mver:^[[1m" server-version *Protocol-version/name server-version "^[[22mbytes:^[[1m" len "^[[22mbytes"]

		if ctx/version <> server-version [
			log-error ["Version mismatch:^[[22m" ctx/version "<>" server-version]
			ctx/critical-error: 'Internal_error
			return false
		]

		if available < len [
			;probe inp/buffer
			log-debug ["Incomplete fragment:^[[22m available^[[1m" available "^[[22mof^[[1m" len "^[[22mbytes"]
			;?? inp/buffer
			binary/read inp [AT :start] ;resets position
			log-debug ["Data starts: " copy/part inp/buffer 10]
			return false
		]

		*protocol-type/assert type
		*protocol-version/assert server-version

		protocol: *protocol-type/name type
		version:  *protocol-version/name server-version

		ctx/seq-read: ctx/seq-read + 1

		end: start + len + 5 ; header size is 5 bytes

		;log-debug "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
		log-more ["^[[22mR[" ctx/seq-read "] Protocol^[[1m" protocol "^[[22mbytes:^[[1m" len "^[[22mfrom^[[1m" start "^[[22mto^[[1m" end]

		ctx/protocol: protocol

		switch protocol [
			APPLICATION [
				; first one, as it's the most common
				change-state ctx 'APPLICATION
				assert-prev-state ctx [APPLICATION ALERT FINISHED]
				binary/read inp [data: BYTES :len]
				;?? data
				;?? ctx/cipher-spec-set
				if ctx/cipher-spec-set > 1 [
					if data: decrypt-msg ctx data :type [
						append ctx/port-data data
						;@@ TODO: the parent scheme (HTTPS) should be notified here,
						;@@ that there are already some decrypted data available!   
						;@@ Now it is awaked only when data are complete :-/        
					]
				]
			]
			HANDSHAKE [
				binary/read inp [data: BYTES :len]
				;?? data
				;? ctx/cipher-spec-set
				either ctx/cipher-spec-set > 1 [
					ctx/seq-read: 0
					data: decrypt-msg ctx data :type
					;ctx/protocol: 'APPLICATION
					;change-state ctx 'APPLICATION
					ctx/reading?: ctx/server?
					;?? data
				][
					TLS-update-messages-hash ctx data len
				]
				; process the handshake message, set `critical-error` if there is any
				unless empty? data [
					ctx/critical-error: TLS-parse-handshake-message ctx data
				]
			]
			CHANGE_CIPHER_SPEC [
				binary/read inp [value: UI8]
				either 1 = value [
					ctx/cipher-spec-set: 2
					ctx/seq-read: 0
					ctx/reading?: true ;ctx/server?
					;ctx/protocol: 'APPLICATION
					;change-state ctx 'APPLICATION
				][
					log-error ["*** CHANGE_CIPHER_SPEC value should be 1 but is:" value]
				]
			]
			ALERT [
				log-debug ["ALERT len:" :len "ctx/cipher-spec-set:" ctx/cipher-spec-set]
				binary/read inp [data: BYTES :len]
				if ctx/cipher-spec-set > 1 [
					log-debug ["Decrypting ALERT message:" mold data]
					data: decrypt-msg ctx data :type
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
				name:  any [*Alert/name id  'Unknown]
				
				;@@ do some actions here....
				ctx/critical-error: either level = 'WARNING [false][name]
				either id = 0 [
					; server done
					ctx/reading?: false
					ctx/protocol: 'APPLICATION
					log-info "Server done"
				][
					log-more ["ALERT:" level "-" replace/all form name #"_" #" "]
				]
			]
		]

		;?? ctx/critical-error
		if ctx/critical-error [ return false ]

		if end <> index? inp/buffer [
			log-error ["Record end mismatch:^[[22m" end "<>" index? inp/buffer]
			ctx/critical-error: 'Record_overflow
			return false
		]

		unless ctx/reading? [
			log-debug ["Reading finished!"]
			;log-----
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
	;@@ There may be more fragments in one message!
	while [4 <= length? msg/buffer][
	binary/read msg [type: UI8 len: UI24 start: INDEX]
	ends: start + len
	log-debug ["R[" ctx/seq-read "] length:" len "start:" start "ends:" ends "type:" type]

	change-state ctx *Handshake/name type

	;log-more ["R[" ctx/seq-read "] Handshake fragment:" ctx/state "length:" len "start:" start "ends:" ends]

	switch/default ctx/state [
		;----------------------------------------------------------
		CLIENT_HELLO [	;- this is the initial message from client to server
			binary/read msg [
				client-version: UI16 ; max supported version by client
				;client-hello message:
				remote-random: BYTES 32 ; random struct
				session-id:    UI8BYTES
				cipher-suites: UI16BYTES
				compressions:  UI8BYTES
				extensions:    UI16BYTES
			]
			log-debug ["Client requests:" *Protocol-version/name :client-version]
			log-debug ["Client random:^[[1m" remote-random]
			ctx/remote-random: remote-random
			unless empty? session-id [
				;@@ could be used for Session Resumption
				;https://hpbn.co/transport-layer-security-tls/#tls-session-resumption
				log-debug ["Client session:" session-id]
			]

			client-cipher-suites: decode-cipher-suites :cipher-suites
			foreach cipher client-cipher-suites [
				if find suported-cipher-suites cipher [
					; store as an integer
					ctx/cipher-suite: *Cipher-suite/:cipher
					TLS-init-cipher-suite ctx
					break
				]
			]
			; now we should have initialized cipher suites, return error, if not
			unless ctx/crypt-method [
				log-error "No supported cipher-suite!"
				return 'Handshake_failure
			]

			if #{00} <> compressions [
				log-error ["Client requests compression:" compressions]
				return 'Unexpected_message
			]
			;? extensions
			extensions: decode-extensions :extensions
			;if groups: select extensions 'SupportedGroups [
			;]
			;'clear TCP-port/data
			ctx/reading?: false
		]
		SERVER_HELLO [ ;- this is the server's response to clients initial message
			;NOTE: `len` should be now >= 38 for TLS and 41 for DTLS
			assert-prev-state ctx [CLIENT_HELLO]

			with ctx [
				if any [
					error? try [
						binary/read msg [
							server-version: UI16
							remote-random:  BYTES 32
							session-id:     UI8BYTES						
							cipher-suite:   UI16 2
							compressions:   UI8BYTES ;<- must be empty
							extensions:     UI16BYTES
							pos:            INDEX
						]
					]
					32 < length? session-id  ;@@ limit session-id size; TLSe has it max 32 bytes
				][
					log-error "Failed to read server hello."
					return 'Handshake_failure
				]

				log-more ["R[" seq-read "] Version:" *Protocol-version/name server-version "len:" len "cipher-suite:" *Cipher-suite/name cipher-suite]
				log-more ["R[" seq-read "] SRandom:^[[32m" remote-random ]
				log-more ["R[" seq-read "] Session:^[[32m" session-id]

				if server-version <> version [
					log-error [
						"Version required by server:" server-version
						"is not same like clients:" version
					]
					
					; protocol downgrade (to v1.1) is not allowed now, would have to find out, how to make it
					if server-version <> version [
						return 'Protocol_version
					]

					version: server-version
				]

				unless empty? compressions [
					log-more ["R[" seq-read "] Compressions:^[[1m" compressions ]
					log-error "COMPRESSION NOT SUPPORTED"
					return 'Decompression_failure
				]

				unless TLS-init-cipher-suite ctx [
					log-error "Unsupported cipher suite!"
					return 'Handshake_failure
				]

				;-- extensions handling
				extensions: decode-extensions :extensions
				false ;= no error
			]
		]
		;----------------------------------------------------------
		CERTIFICATE [
			assert-prev-state ctx [SERVER_HELLO CLIENT_HELLO]
			tmp: binary/read msg [UI24 INDEX]
			if ends <> (tmp/1 + tmp/2) [
				log-error ["Improper certificate list end?" ends "<>" (tmp/1 + tmp/2)]
				return 'Handshake_failure
			]
			;i: 0
			while [ends > index? msg/buffer][
				;probe copy/part msg/buffer 10
				binary/read msg [cert: UI24BYTES]
				;probe cert
				;i: i + 1 write rejoin [%/x/cert i %.crt] cert
				append ctx/server-certs decode 'CRT cert
			]
			log-more ["Received" length? ctx/server-certs "server certificates."]
			;? ctx/server-certs
			try/with [
				;?? ctx/server-certs/1/public-key
				;?? ctx/server-certs/1
				key: ctx/server-certs/1/public-key
				switch key/1 [
					ecPublicKey [
						ctx/pub-key: key/3
						ctx/pub-exp: key/2      ;curve name
						if 0 = ctx/pub-key/1 [remove ctx/pub-key]
					]
					rsaEncryption [
						ctx/pub-key: key/2/1
						ctx/pub-exp: key/2/2
					]
				]
			][
				log-error "Missing public key in certifiate"
				return 'Bad_certificate
			]
			;@@TODO: certificate validation
		]
		;----------------------------------------------------------
		SERVER_KEY_EXCHANGE [
			assert-prev-state ctx [CERTIFICATE SERVER_HELLO]
			log-more ["R[" ctx/seq-read "] Using key method:^[[1m" ctx/key-method]
			;?? msg
			switch ctx/key-method [
				ECDHE_RSA
				ECDHE_ECDSA [
					;? msg/buffer
					try/with [
						binary/read msg [
						  s: INDEX
							ECCurveType: UI8  
							ECCurve:     UI16     ; IANA CURVE NUMBER
							pub_key:     UI8BYTES 
						  e: INDEX
						]
					][
						log-error "Error reading elyptic curve"
						return 'User_cancelled
					]

					if any [
						3 <> ECCurveType
						4 <> first pub_key
						none? curve: *EllipticCurves/name ECCurve
					][
						log-error ["Unsupported ECurve type:" ECCurveType ECCurve ]
						return 'User_cancelled
					]
					log-more ["R[" ctx/seq-read "] Elyptic curve type:" ECCurve "=>" curve]
					log-more ["R[" ctx/seq-read "] Elyptic curve data:" mold pub_key]
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
			;print ["DH:" mold dh_p mold dh_g mold pub_key] 
			;?? message
			
			;-- check signature
			hash-algorithm:         *HashAlgorithm/name binary/read msg 'UI8
			sign-algorithm: *ClientCertificateType/name binary/read msg 'UI8
			log-more ["R[" ctx/seq-read "] Using algorithm:" hash-algorithm "with" sign-algorithm]
			if hash-algorithm = 'md5_sha1 [
				;__private_rsa_verify_hash_md5sha1
				log-error "legacy __private_rsa_verify_hash_md5sha1 not implemented yet!"
				return 'Decode_error
			]
			binary/read msg [signature: UI16BYTES]
			;?? signature
			insert message join ctx/local-random ctx/remote-random
			;?? message
			if any [
				error? valid?: try [
					switch sign-algorithm [
						rsa_sign [
							log-more "Checking signature using RSA"
							;decrypt the `signature` with server's public key
							rsa-key: apply :rsa-init ctx/server-certs/1/public-key/rsaEncryption
							also rsa/verify/hash rsa-key message signature hash-algorithm
							     release :rsa-key ; release it immediately, don't wait on GC
						]
						rsa_fixed_dh [
							log-more "Checking signature using RSA_fixed_DH"
							;@@ TODO: rewrite ecdsa/verify to count the hash automatically like it is in rsa/verify now?
							message-hash: checksum message hash-algorithm
							; test validity:
							ecdsa/verify/curve ctx/pub-key message-hash signature ctx/pub-exp
						]
					]
				]
				not valid?
			][
				log-error "Failed to validate signature"
				if error? valid? [print valid?]
				return 'Decode_error
			]
			log-more "Signature valid!"
			if ends > pos: index? msg/buffer [
				len: ends - pos
				binary/read msg [extra: BYTES :len]
				log-error ["Extra" len "bytes at the end of message:" ellipsize form extra 40]
				return 'Decode_error
			]

			if dh_p [
				dh-key: dh-init dh_g dh_p
				ctx/pre-secret: dh/secret dh-key pub_key
				log-more ["DH common secret:" mold ctx/pre-secret]
				ctx/key-data: dh/public :dh-key
				; release immediately, don't wait on GC
				release :dh-key dh-key: none
			]
			if curve [
				;- elyptic curve init
				;curve is defined above (sent from server as well as server's public key)
				dh-key: ecdh/init none curve
				ctx/pre-secret: ecdh/secret dh-key pub_key
				log-more ["ECDH common secret:^[[32m" mold  ctx/pre-secret]
				; resolve the public key to supply it to server
				ctx/key-data: ecdh/public :dh-key
				; release immediately, don't wait on GC
				release :dh-key dh-key: none
			]
		]
		CLIENT_KEY_EXCHANGE [
			unless ctx/server? [
				log-error "This message is expected on server!"
				return 'Decode_error
			]
			switch ctx/key-method [
				ECDHE_RSA
				ECDHE_ECDSA [
					key-data: binary/read msg 'UI8BYTES
					ctx/pre-secret: ecdh/secret ctx/dh-key key-data 
					log-more ["ECDH common secret:^[[32m" ctx/pre-secret]
				]
				DHE_DSS
				DHE_RSA [
					;- has DS params
					key-data: binary/read msg 'UI8BYTES
					;@@TODO!!!
				]
				RSA [
					key-data: binary/read msg 'UI16BYTES 
					;@@TODO!!!
				]
			]
			TLS-key-expansion ctx
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
				return 'Decode_error
			]
		]
		;----------------------------------------------------------
		SERVER_HELLO_DONE [
			ctx/reading?: false
		]

		FINISHED [
			binary/read msg [verify-data: BYTES] ;rest of data
			seed: read ctx/sha-port
			result: prf :ctx/sha-port/spec/method either ctx/server? ["client finished"]["server finished"] seed ctx/master-secret  12
			;? verify-data ? result
			if result <> verify-data [
				return 'Handshake_failure
			]
		]
	][
		log-error ["Unknown state: " ctx/state "-" type]
		return 'Unexpected_message
	]
	;; validate fragment's tail..
	if ends <> i: index? msg/buffer [
		log-error ["Wrong fragment message tail!" ends "<>" i]
		log-error ["in/buffer starts:" copy/part msg/buffer 20]
		return 'Record_overflow
	]
	] ;; more messages?

	log-more ["R[" ctx/seq-read "] DONE: handshake^[[1m" ctx/state] log-----
	false ;= no error
]

send-event: function[
	event  [word!]
	target [port!]
][
	log-debug ["Send-event:^[[1m" pad event 8 "^[[m->" target/spec/ref]
	either all [
		port? target/parent
		function? :target/parent/awake
	][  ;; If there is parent scheme, send the event to its awake function 
		target/parent/awake make event! [ type: event port: target ]
	][  ;; If not, insert the event into the system port's que
		insert system/ports/system make event! [ type: event port: target ]
	]
]


TLS-server-client-awake: function [event [event!]][
	TCP-port: event/port
	;? TCP-port
	ctx: TCP-port/extra
	log-debug ["Server's client awake event:" event/type "state:" ctx/state]
	switch event/type [
		read [
			error: try [
				complete?: TLS-read-data ctx TCP-port/data
				if ctx/critical-error [ cause-TLS-error ctx/critical-error ]
				log-debug ["Read complete?" complete?]
				either complete? [
					switch ctx/state [
						CLIENT_HELLO [
							server-hello ctx
							server-certificate ctx
							server-hello-done ctx
							write TCP-port head ctx/out/buffer
						]
						FINISHED [
							binary/init ctx/out none
							change-cipher-spec ctx
							finished ctx
							ctx/cipher-spec-set: 2
							ctx/seq-read: 0
							ctx/seq-write: 1 ;@@TODO: it cannot be here and it must be incremented on each write!
							write TCP-port head ctx/out/buffer
						]
						APPLICATION [
							;? ctx
							;print to-string ctx/port-data
							;send-event 'read TCP-port/parent
							;? TCP-port
							;? TCP-port/extra
							;? TCP-port/parent/state
							TCP-port/parent/actor/On-Read TCP-port
							;do-TLS-close TCP-port
						]
					]
				][
					read TCP-port
				]
				return false
			]
			; on error:
			if ctx [ log-error ctx/error: error ]
			;send-event 'error TLS-port
			do-TLS-close TCP-port
			return true
		]
		wrote [
			either ctx/protocol = 'APPLICATION [
				TCP-port/parent/actor/On-Wrote TCP-port
			][
				read TCP-port
			]
			return false
		]
		close [
			do-TLS-close TCP-port
			return true
		]
	]
	false
]
TLS-server-awake: func [event /local port info serv] [
	log-more ["AWAKE Server:^[[1m" event/type]
	switch event/type [
		accept [
			serv: event/port
			port: first serv
			info: query port
			;? info
			;? serv
			port/extra: make TLS-context [
				tcp-port: port
				tls-port: serv/parent
				server?:  true
				state:   'CLIENT_HELLO
				version:  serv/extra/version
			]
			port/spec/title: "TLS Server's client"
			port/spec/ref: rejoin [tcp:// info/remote-ip #":" info/remote-port]
			port/awake: :TLS-server-client-awake
			;? port
			read port
		]
	]
	false
]


TLS-client-awake: function [event [event!]][
	log-debug ["AWAKE Client:^[[1m" event/type]
	TCP-port: event/port
	ctx: TCP-port/extra
	TLS-port: ctx/TLS-port

	if all [
		ctx/protocol = 'APPLICATION
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
			TLS-init-context ctx
			return false
		]
		connect [
			if none? ctx [return true] ;- probably closed meanwhile
			return TLS-init-connection ctx
		]
		wrote [
			switch ctx/protocol [
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
				log-debug ["READ TCP" length? TCP-port/data "bytes proto-state:" ctx/protocol]
				;@@ This part deserves a serious review!                         
				complete?: TLS-read-data ctx TCP-port/data
				;? port
				if ctx/critical-error [ cause-TLS-error ctx/critical-error ]
				log-debug ["Read complete?" complete?]
				unless complete? [
					read TCP-port
					return false
				]
				TLS-port/data: ctx/port-data
				binary/init ctx/in none ; resets input buffer
				;? ctx/protocol
				switch ctx/protocol [
					APPLICATION [
						;- report that we have data to higher layer
						send-event 'read TLS-port
						return true ;= wake up
					]
					HANDSHAKE [
						switch ctx/state [
							SERVER_HELLO_DONE [
								binary/init ctx/out none ;reset output buffer
								client-key-exchange ctx
								change-cipher-spec ctx
								finished ctx
								do-TCP-write ctx
								return false
							]
							FINISHED [
								change-state ctx ctx/protocol: 'APPLICATION
								send-event 'connect TLS-port
								;print-horizontal-line
								return true ;= wake up
							]
						]
					]
				]

				read TCP-port
				return false
			]
			; on error:
			if ctx [ log-error ctx/error: error ]
			send-event 'error TLS-port
			return true
		]
		close [
			send-event 'close TLS-port
			return true
		]
		error [
			unless ctx/error [
				ctx/error: case [
					ctx/state = 'lookup [
						make error! [
							code: 500 type: 'access id: 'cannot-open
							arg1: TCP-port/spec/ref
						]
					]
					'else [
						;@@ needs better error (unknown reason)
						; So far this error is used, when we try to write
						; application data larger than 16KiB!
						make error! [
							code: 500 type: 'access id: 'protocol
							arg1: TCP-port/spec/ref
						]
					]
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


do-TLS-open: func [
	port [port!]
	/local spec conn config certs bin der key
][
	log-debug "OPEN"
	if port/state [return port]
	spec: port/spec

	either port? conn: select spec 'conn [
		;- reusing already prepared TCP connection      
		spec/host: conn/spec/host
		spec/port: conn/spec/port
		if block? spec/ref [
			spec/ref: rejoin [tls:// any [spec/host ""] ":" spec/port]
		]
	][
		;- opening new low level TCP connection         
		conn: make port! [
			scheme: 'tcp
			host:    spec/host
			port:    spec/port
			ref:     rejoin [tcp:// any [host ""] ":" port]
		]
		if port/parent [
			conn/state: port/parent/state
		]
		conn/parent: port
	]

	either spec/host [
		;- CLIENT connection ---------------------------
		port/extra: conn/extra: make TLS-context [
			tcp-port: conn
			tls-port: port
			version: *Protocol-version/TLS1.2
		]
		port/data: conn/extra/port-data
		conn/awake: :TLS-client-awake
	][
		;- SERVER connection ---------------------------
		spec/ref: rejoin [tls://: spec/port]
		port/spec/title: "TLS Server"
		conn/spec/title: "TLS Server (internal)"
		port/state: conn/extra: object [
			TCP-port: conn
			certificates: none
			private-key:  none
			elliptic-curves: decode-supported-groups :supported-elliptic-curves
			version: *Protocol-version/TLS1.2
		]
		;? spec
		if config: select spec 'config [
			certs: any [select config 'certificates []]
			unless block? certs [certs: to block! certs]
			bin: binary 4000
			foreach file certs [
				try/with [
					der: select decode 'pkix read file 'binary
					binary/write bin [UI24BYTES :der]
				][
					log-error ["Failed to import certificate:" file]
				]
			]
			port/state/certificates: bin/buffer

			if key: select config 'private-key [
				if file? key [try [key: load key]]
				either handle? key [
					port/state/private-key: key
				][	log-error ["Failed to import private key:" key] ]
			]
		]
		port/actor: context [
			On-Read: func [port [port!]][
				log-debug "TLS On-Read"
				print to string! port/extra/port-data
				do-TLS-write port "HTTP/1.1 200 OK^M^/Content-type: text/plain^M^/^M^/Hello from Rebol using TLS v1.2"
			]
			On-Wrote: func [port [port!]][
				send-event 'close port
			]
		]
		conn/parent: port
		conn/awake: :TLS-server-awake
	]
	either open? conn [
		TLS-init-context    conn/extra
		TLS-init-connection conn/extra
	][
		open conn
	]
	port
]
do-TLS-close: func [port [port!] /local ctx parent][
	log-debug "CLOSE"
	unless ctx: port/extra [return port]
	parent: port/parent
	log-debug "Closing port/extra/tcp-port"
	close ctx/tcp-port
	if port? ctx/encrypt-port [ close ctx/encrypt-port ]
	if port? ctx/decrypt-port [ close ctx/decrypt-port ]
	ctx/encrypt-port: none
	ctx/decrypt-port: none
	ctx/tcp-port/awake: none
	ctx/tcp-port: none
	ctx/tls-port: none
	port/extra: none
	log-more "Port closed"
	if parent [
		insert system/ports/system make event! [type: 'close port: parent]
	]
	port
]
do-TLS-read: func [port [port!]][
	log-debug "READ"
	read port/extra/tcp-port
	port
]
do-TLS-write: func[port [port!] value [any-string! binary!] /local ctx][
	log-debug "WRITE"
	ctx: port/extra
	if ctx/protocol = 'APPLICATION [
		binary/init ctx/out none ;resets the output buffer
		;@@ There is a size limit for application data 16KiB!
		while [not tail? value][
			application-data ctx copy/part :value 16000
			value: skip value 16000
		]
		do-TCP-write ctx
		return port
	]
]

do-TCP-write: func[ctx][
	log-debug ["Writing bytes:" length? ctx/out/buffer]
	;?? ctx/out/buffer
	;ctx/out/buffer: head ctx/out/buffer
	clear ctx/port-data
	write ctx/tcp-port ctx/out/buffer

	ctx/reading?: true
	binary/init ctx/out none ;resets the output buffer
]

sys/make-scheme [
	name: 'tls
	title: "TLS protocol v1.2"
	spec: make system/standard/port-spec-net []
	actor: reduce/no-set [
		read:    :do-TLS-read
		write:   :do-TLS-write
		open:    :do-TLS-open
		close:   :do-TLS-close
		query:   func [port [port!]][all [port/extra query port/extra/tcp-port]]
		open?:   func [port [port!]][all [port/extra open? port/extra/tcp-port]]
		copy:    func [port [port!]][if port/data [copy port/data]]
		length?: func [port [port!]][either port/data [length? port/data][0]]
	]
	set-verbose: :tls-verbosity
]
