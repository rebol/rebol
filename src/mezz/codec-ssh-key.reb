REBOL [
	Title:   "Codec: Secure Shell Key"
	Author:  "Oldes"
	Rights:  "Copyright (C) 2020 Oldes. All rights reserved."
	License: "BSD-3"
	Test:    %tests/units/crypt-test.r3
	Note: {
		* it extract (and inits) only RSA keys so far
		* encrypted keys only with `AES-128-CBC` encryption
	}
]

wrap [
	init-from-ssh2-key: function [data][
		try [
			binary/read data [
				v: UI32BYTES
				e: UI32BYTES
				n: UI32BYTES
			]
			v: to string! v
			if v = "ssh-rsa" [
				return rsa-init n e
			]
		]
		sys/log/error 'REBOL ["Not RSA key! (" v ")"]
		none
	]
	init-rsa-public-key: function [data [block!]][
		parse data [
			'SEQUENCE into [
				'INTEGER set n:   binary! ;modulus        
				'INTEGER set e:   binary! ;publicExponent 
			]
		]
		rsa-init n e
	]
	init-rsa-private-key: function [data [block!]][
		parse data [
			'SEQUENCE into [
				'INTEGER set v:   binary! ;version
				'INTEGER set n:   binary! ;modulus
				'INTEGER set e:   binary! ;publicExponent
				'INTEGER set d:   binary! ;privateExponent
				'INTEGER set p:   binary! ;prime1
				'INTEGER set q:   binary! ;prime2
				;- dp, dq and qp are computed when initialized, so not used here 
				;'INTEGER set dp:  binary! ;exponent1       d mod (p-1)
				;'INTEGER set dq:  binary! ;exponent2       d mod (q-1)
				;'INTEGER set qp:  binary! ;coefficient     (inverse of q) mod p
				to end
			]
			to end
		]
		rsa-init/private n e d p q
	]

	register-codec [
		name: 'ssh-key
		type: 'cryptography
		title: "Secure Shell Key"
		suffixes: [%.key]

		decode: function [
			"Decodes and initilize SSH key"
			key [binary! string! file!]
			/password p [string! binary! none!] "Optional password"
		][
			case [
				file?   key [ key: read key ]
				string? key [ key: to binary! key ]
			]
			; try to load key as a PKIX structure
			try [ pkix: codecs/pkix/decode key ]
			if none? pkix [
				; if failed to load as PKIX, try to treat it as a *.PUB file
				return either parse key [
					"ssh-rsa " copy data to [#" " | end] to end
				][	init-from-ssh2-key debase data 64
				][	init-from-ssh2-key key ]
			]
			if "4,ENCRYPTED" = select pkix/header "Proc-Type" [
				sysl/log/info 'REBOL "ENCRYPTED key!"
				try/with [
					dek-info: select pkix/header "DEK-Info"
					sysl/log/info 'REBOL ["Using:" dek-info]
					parse dek-info [
						"AES-128-CBC" #"," copy iv to end
					]
					iv: debase iv 16
					unless p [p: ask/hide "Pasword: "]
					p: checksum (join to binary! p copy/part iv 8) 'md5
					d: aes/key/decrypt p iv
					pkix/binary: aes/stream d pkix/binary
				][	return none ]
			]
			sys/log/info 'REBOL ["Importing:" pkix/label]

			switch pkix/label [
				"SSH2 PUBLIC KEY" [
					return init-from-ssh2-key pkix/binary
				]
			]
			; decode DER structure from decoded PKIX binary
			try/with [
				data: codecs/der/decode pkix/binary
			][
				sys/log/error 'REBOL "Failed to decode DER day for RSA key!"
				sys/log/error 'REBOL system/state/last-error
				return none
			]
			
			switch pkix/label [
				"PUBLIC KEY"
				"PRIVATE KEY" [
					; resolve key data from the DER structure (PKCS#1)
					return attempt [
						parse data [
							'SEQUENCE into [
								opt ['INTEGER binary!]
								'SEQUENCE into [
									'OBJECT_IDENTIFIER set oid: binary! ; AlgorithmIdentifier 
									to end ;'NULL binary!
								]
								['BIT_STRING | 'OCTET_STRING] set data: binary! ; PublicKey
								(
									data: codecs/der/decode data
								)
							]
						]
						switch/default oid [
							#{2A864886F70D010101} [ ;= rsaEncryption
								return either pkix/label = "PUBLIC KEY" [
									init-rsa-public-key  data
								][  init-rsa-private-key data ]
							]
							#{2A8648CE3D0201} [ ;= ecPublicKey
								return either pkix/label = "PUBLIC KEY" [
									init-rsa-public-key  data
								][  init-rsa-private-key data ]
							]
						][
							sys/log/error 'REBOL ["Unknown key type:" codecs/der/decode-OID oid]
							none
						]
					]
				]
				"RSA PUBLIC KEY"  [ return init-rsa-public-key  data ]
				"RSA PRIVATE KEY" [ return init-rsa-private-key data ]
				"DH PARAMETERS" [
					parse data [
						'SEQUENCE into [
							'INTEGER set p: binary!
							'INTEGER set g: binary!
						] (
							return dh-init :g :p
						)
					]
					
				]
			]
			none ; no success!
		]
	]
]