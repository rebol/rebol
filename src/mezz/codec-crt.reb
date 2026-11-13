REBOL [
	title:   "Codec: CRT"
	name:    crt
	type:    module
	version: 0.0.1
	Purpose: "Codec for X.509 Public Key Infrastructure Certificate"
	author:  "Oldes"
]

register-codec [
	name:  'crt
	type:  'cryptography
	title: "Internet X.509 Public Key Infrastructure Certificate and Certificate Revocation List (CRL) Profile"
	suffixes: [%.crt]

	get-fingerprint: function[data [binary!] method [word!]][
		bin: binary data
		loop 2 [
			binary/read bin [
				flags:    UI8
				length:   LENGTH
			]
			if any [
				flags <> 48 ; 0x30 = class universal, constructed, SEQUENCE
				length > length? bin/buffer
			][	return none ]
		]
		checksum/part at data 5 :method (length + 4)
	]
	decode: wrap [
		*oid:
		*val:
		*bin: none
		*bool: false
		*new: none
		*blk: copy []

		der-codec: system/codecs/DER
		
		Name: [
			(clear *blk)
			'SEQUENCE into [
				some [
					'SET into [
						'SEQUENCE into [
							'OBJECT_IDENTIFIER set *oid binary!
							['PRINTABLE_STRING | 'UTF8_STRING | 'IA5_STRING | 'T61_STRING | 'BMP_STRING]
							set *val string!
							(
								*oid: der-codec/decode-OID *oid
								if word? *oid [*oid: to set-word! *oid]
								;?? *oid
								repend *blk [*oid *val]
							)
						]
					]
				]
			]
			(new-line/skip *blk true 2)
		]

		AlgorithmIdentifier: [
			(clear *blk)
			'SEQUENCE into [
				'OBJECT_IDENTIFIER set *oid binary!
				[	;- optional parameters
					'NULL binary! (*val: none)
					|
					copy *val to end ;@@ TODO: decode it too?
				]
				(
					append *blk der-codec/decode-OID *oid
					if *val [append *blk *val]
				)
			]
			(new-line/skip *blk true 2)
		]

		Extensions: [
			(clear *blk)
			'SEQUENCE into [
				some [
					(*bool: false)
					'SEQUENCE into [
						'OBJECT_IDENTIFIER set *oid binary!
						opt ['BOOLEAN #{FF} (*bool: true)]
						'OCTET_STRING set *val binary! (
							*oid: der-codec/decode-OID *oid
							if all [not empty? *val *val/1 = 48] [*val: der-codec/decode *val]
							switch *oid [
								extKeyUsage [
									*val: der-codec/decode-OID *val/SEQUENCE/OBJECT_IDENTIFIER
								]
								subjectAltName [
									*new: copy []
									parse *val [
										'SEQUENCE into [
											any ['CS2 set *bin binary! (append *new to string! *bin)]
										]
									]
									*val: new-line/all *new true
								]
								;authorityInfoAccess [
								;	probe *val
								;]

							]
							repend *blk [*oid reduce [*bool *val]]
							new-line last *blk true
						)
					]
				]
			]
			(new-line/skip *blk true 2)
		]

		func [
			data [binary! block!]
			/local pkix version serialNumber issuer subject validity der
		][
			try [all [
				; as there seems to be no standard, the *.crt file
				; may be actually in pkix format, so try it first...
				pkix: codecs/pkix/decode data
				pkix/label = "CERTIFICATE"
				data: pkix/binary
			]]

			der: either binary? data [
				der-codec/decode data
			][	data ]
			if all [
				2 = length? der
				'SEQUENCE = der/1 
				block? der/2 
			] [der: der/2]

			result: construct [
				version:
				serial-number:
				fingerprint:
				algorithm:
				issuer:
				valid-from:
				valid-to:
				subject:
				public-key:
				issuer-id:
				subject-id:
				extensions:
				signature:
			]

			parse der [
				'SEQUENCE into [
					;-- version:
					'CS0 into [
						'INTEGER set *val binary! (
							result/version: to integer! *val
						)
					]

					;-- serial number:
					'INTEGER set *val binary! (	result/serial-number: *val	)
					
					;-- signature:
					AlgorithmIdentifier ( result/algorithm: copy *blk )

					;-- issuer:
					Name (result/issuer: copy *blk)
					
					;-- validity:
					'SEQUENCE into [
						'UTC_TIME set *val date! (result/valid-from: *val)
						'UTC_TIME set *val date! (result/valid-to:   *val)
					]
					;-- subject:
					Name (result/subject: copy *blk)

					;-- subject public key info:
					'SEQUENCE into [
						AlgorithmIdentifier ( result/public-key: copy *blk )
						'BIT_STRING set *val binary! (
							append/only result/public-key switch/default *blk/1 [
								rsaEncryption [
									tmp: der-codec/decode *val
									reduce [copy tmp/2/2 copy tmp/2/4] ;@@ or don't copy?
								]
								ecPublicKey [
									remove next result/public-key
									result/public-key/2: der-codec/decode-OID result/public-key/2
									copy *val
								]
							][	copy *val ]
							
						)
					]

					opt ['BIT_STRING set *val binary! ( result/issuer-id: *val )]
					opt ['BIT_STRING set *val binary! ( result/subject-id: *val )]
					opt [
						;-- extensions
						'CS3 into [
							Extensions ( result/extensions: copy *blk )
						]
					]
					to end
				]

				;-- signature:
				AlgorithmIdentifier (
					; MUST be same like result/algorithm!
					either *blk <> result/algorithm [
						print "Invalid certificate! Signature alrgorithm mischmasch."
						? result/algorithm
						? *blk
						result/algorithm: none
					][
						if 1 = length? result/algorithm [
							result/algorithm: first result/algorithm
						]
					]
				)
				'BIT_STRING set *val binary! ( result/signature: *val )
				to end
			]
			if all [
				binary? data
				hash: select [
					sha256WithRSAEncryption sha256
					sha384WithRSAEncryption sha384
					sha512WithRSAEncryption sha512
					md5withRSAEncryption    md5
					md4withRSAEncryption    md4
					ecdsa-with-SHA224       sha224
					ecdsa-with-SHA256       sha256
					ecdsa-with-SHA384       sha384
					ecdsa-with-SHA512       sha512
					sha1WithRSAEncrption    sha1
				] result/algorithm
			][
				try [
					result/fingerprint: get-fingerprint :data :hash
				]
			]
			if verbose > 0 [
				prin "^/^[[1;32mCRT"
				either verbose > 1 [
					?? result
				][
					print " result:^[[0m"
					print dump-obj result
				]
			]
			result
		]
	]
	verbose: 0
]