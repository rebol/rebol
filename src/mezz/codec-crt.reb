REBOL [
	title: "REBOL 3 codec for X.509 Public Key Infrastructure Certificate"
	name: 'codec-crt
	author: rights: "Oldes"
	version: 0.0.1
]

register-codec [
	name:  'crt
	type:  'cryptography
	title: "Internet X.509 Public Key Infrastructure Certificate and Certificate Revocation List (CRL) Profile"
	suffixes: [%.crt]
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
			/local pkix version serialNumber issuer subject validity
		][
			try [all [
				; as there seems to be no standard, the *.crt file
				; may be actually in pkix format, so try it first...
				pkix: codecs/pkix/decode data
				pkix/label = "CERTIFICATE"
				data: pkix/binary
			]]
			if binary? data [ data: der-codec/decode data ]
			if all [
				2 = length? data
				'SEQUENCE = data/1 
				block? data/2 
			] [data: data/2]

			result: object [
				version:
				serial-number:
				signature:
				issuer:
				valid-from:
				valid-to:
				subject:
				public-key:
				issuer-id:
				subject-id:
				extensions:
					none
			]

			parse data [
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
					AlgorithmIdentifier ( result/signature: copy *blk )

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
				AlgorithmIdentifier ( result/signature: copy *blk )
				to end
			]
			if verbose > 0 [
				prin "^/^[[1;32mCRT "
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
	fingerprint: function [
		"Computes Certificate Fingerprint of the cert's data sequence"
		data   [binary!] "Raw CRT data"
		/method          "If not used, default is sha256"
			m [word!]    "One of: sha256, sha1 or md5"
	][
		der: binary data
		binary/read der [UB 2 BIT tag: UB 5 LENGTH pos: INDEX]
		if tag <> 16 [return none]
		binary/read der [UB 2 BIT tag: UB 5 length: LENGTH]
		if not find [sha256 sha1 md5] m [m: 'sha256]
		hash: open join checksum:// m
		write/part hash (at data pos) (pos + length + 5)
		read hash
	]

	verbose: 0
]