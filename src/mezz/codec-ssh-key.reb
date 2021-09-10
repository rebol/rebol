REBOL [
	Title:   "REBOL 3 codec: Secure Shell Key"
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
		print ["Not RSA key! (" v ")"]
		none
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
				print "ENCRYPTED key!"
				try/except [
					dek-info: select pkix/header "DEK-Info"
					;probe dek-info
					parse dek-info [
						"AES-128-CBC" #"," copy iv to end
					]
					iv: debase iv 16
					unless p [p: ask/hide "Pasword: "]
					p: checksum
						join to binary! p copy/part iv 8
						'md5
					d: aes/key/decrypt p iv
					pkix/binary: aes/stream d pkix/binary
				][	return none ]
			]

			switch pkix/label [
				"SSH2 PUBLIC KEY" [
					return init-from-ssh2-key pkix/binary
				]
			]
			; decode DER structure from decoded PKIX binary
			try/except [
				data: codecs/der/decode pkix/binary
			][
				print "Failed to decode DER day for RSA key!"
				probe system/state/last-error
				return none
			]
			
			switch pkix/label [
				"PUBLIC KEY" [
					; resolve RSA public data from the DER structure (PKCS#1)
					all [
						parse data [
							'SEQUENCE into [
								'SEQUENCE   set v:    block!  ; AlgorithmIdentifier 
								'BIT_STRING set data: binary! ; PublicKey
								(
									data: codecs/der/decode data
								)
							]
						]
						v/OBJECT_IDENTIFIER = #{2A864886F70D010101} ;= rsaEncryption
						parse data [
							'SEQUENCE into [
								'INTEGER set n:   binary! ;modulus        
								'INTEGER set e:   binary! ;publicExponent 
							]
						]
					]
					; resolve RSA handle from parsed data
					return rsa-init n e
				]
				"RSA PUBLIC KEY" [
					; resolve RSA public data from the DER structure (PKCS#1)
					parse data [
						'SEQUENCE into [
							'INTEGER set n:   binary! ;modulus        
							'INTEGER set e:   binary! ;publicExponent 
						]
					]
					; resolve RSA handle from parsed data
					return rsa-init n e
				]
				"RSA PRIVATE KEY" [
					; resolve RSA private data from the DER structure (PKCS#1)
					parse data [
						'SEQUENCE into [
							'INTEGER set v:   binary! ;version
							'INTEGER set n:   binary! ;modulus        
							'INTEGER set e:   binary! ;publicExponent 
							'INTEGER set d:   binary! ;privateExponent
							'INTEGER set p:   binary! ;prime1         
							'INTEGER set q:   binary! ;prime2         
							'INTEGER set dp:  binary! ;exponent1       d mod (p-1)
							'INTEGER set dq:  binary! ;exponent2       d mod (q-1)
							'INTEGER set inv: binary! ;coefficient     (inverse of q) mod p
							to end
						]
						to end
					]
					; resolve RSA handle from parsed data
					return rsa-init/private n e d p q dp dq inv
				]
			]
			none ; no success!
		]
	]
]