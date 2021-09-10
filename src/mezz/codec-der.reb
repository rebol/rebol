REBOL [
	title: "REBOL3 codec for DER/BER structures"
	name: 'codec-der
	author: "Oldes"
	version: 0.1.0
	date:    17-Oct-2018
	history: [
		0.1.0 17-Oct-2018 "Oldes" {Initial version with DECODE and IDENTIFY functions.}
	]
	notes: {
	Useful command for cross-testing:
		openssl asn1parse -inform DER -in test.pfx

	Current output is as a tree structure, but I'm thinking about using flat structure too
	as it may be easier (and resource friendlier) to deal with in practical use scenarios.
	}
]

register-codec [
	name:  'der
	type:  'cryptography
	title: "Distinguished Encoding Rules"
	suffixes: [%.p12 %.pfx %.cer %.der %.jks]
	decode: function[data [binary!]][
		if verbose > 0 [
			print ["^/^[[1;32mDecode DER data^[[m (^[[1m" length? data "^[[mbytes )"]
			; count maximal bytes width (used for padding)
			wl: length? form length? data
			wr: negate wl
		]
		if data/1 <> 48 [
			if verbose > 0 [
				prin "*** DER data does not start with SEQUENCE tag ***^/*** "
				probe copy/part data 10
			]
			return none
		]

		der: binary data
		
		result: out: make block! 32

		tails:  make block! 8 ;que for finding closings
		blocks: make block! 8 ;que for constructed data

		insert/only blocks out

		while [not tail? der/buffer][
			;?? tails
			depth: length? blocks
			;?? depth
			binary/read der [
				tag-pos:  INDEX
				class:    UB 2    ;- Class encoding [universal application context-specific private]
				constr:   BIT     ;- Method: FALSE = primitive, TRUE = constructed
				tag:      UB 5    ;- Tag
				length:   LENGTH
				data-pos: INDEX
			]
			
			tag-name: switch class [
				0 [ DER-tags/(tag + 1) ]
				1 [ to word! join "AP" tag ]
				2 [ to word! join "CS" tag ]
				3 [ to word! join "PR" tag ]
			]
			
			if closing-pos: tails/1 [
				while [tails/1 = tag-pos][
					;print ["--- closing constructed" tails/1]
					remove tails
					remove blocks
					out: blocks/1
					;?? tails
				]
			]

			data: none

			either constr [
				;-constructed
				repend out [
					tag-name
					out: make block! 32
				]
				insert/only blocks out
				insert tails (data-pos + length)
			][
				;- primitive
				;print ["tag:" tag-name "len: " length length? der/buffer]
				if length > length? der/buffer [
					print "Tag length expects more bytes than available!"
					length: length? der/buffer
				]
				binary/read der [data: BYTES :length]
				switch tag-name [
					OBJECT_IDENTIFIER [
						;data: decode-OID data
					]
					UTC_TIME [
						data: system/codecs/utc-time/decode data
					]
					UTF8_STRING
					PRINTABLE_STRING
					IA5_STRING
					T61_STRING
					BMP_STRING [
						data: to string! data
					]
					;OCTET_STRING [
						;binary/read der [AT :data-pos] 
						;data: make block! 8
						;repend out [tag-name data]
						;out: data
						;insert/only blocks out
						;insert tails (data-pos + length)
						;data: none
					;]
					BIT_STRING [
						if data/1 = 0 [data: next data]
						;data: enbase data 2
					]
					INTEGER [
						if data/1 = 0 [data: next data]
					]
				]
				if data [
					repend out [tag-name data]
				]
			]
			if verbose > 0 [
				if all [series? data empty? data] [data: none]
				if tag-name = 'OBJECT_IDENTIFIER [
					data: decode-OID/full data
				]
				if all [binary? data verbose < 3 94 < length? data][
					data: mold copy/part data 94
					change skip tail data -2 " ..."
				]
				printf [
					#" " $1.35 wr $.32 ":d=" $1.36 2 $.32
					"hl=" $1.32 2 $.32
					"l=" $1 wl $.32 #" " -5
					#" " $1.36 18 $.32 $0] reduce [
					tag-pos  - 1         ; tag start position
					depth    - 1         ; current depth
					data-pos - tag-pos   ; length of header 
					length               ; length of data
					pick ["cons:" "prim:"] constr
					tag-name
					either binary? data[ mold data ][ any [data ""] ]
				]
			]
		]
		;?? tails
		;?? blocks
		result
	]
	
	identify: function[data [binary!]][
		any [
			data/1 = 48
		]
	]
	
	DER-tags: [
		END_OF_CONTENTS   ;= 00
		BOOLEAN           ;= 01
		INTEGER           ;= 02
		BIT_STRING        ;= 03
		OCTET_STRING      ;= 04
		NULL              ;= 05
		OBJECT_IDENTIFIER ;= 06
		OBJECT_DESCRIPTOR ;= 07
		EXTERNAL          ;= 08
		REAL              ;= 09
		ENUMERATED        ;= 0a
		EMBEDDED_PDV      ;= 0b
		UTF8_STRING       ;= 0c
		RELATIVE_OID      ;= 0d
		UNDEFINED
		UNDEFINED
		SEQUENCE          ;= 10
		SET               ;= 11
		NUMERIC_STRING    ;= 12
		PRINTABLE_STRING  ;= 13
		T61_STRING        ;= 14
		VIDEOTEX_STRING   ;= 15
		IA5_STRING        ;= 16
		UTC_TIME          ;= 17
		GENERALIZED_TIME  ;= 18
		GRAPHIC_STRING    ;= 19
		VISIBLE_STRING    ;= 1a Visible string (ASCII subset)
		GENERAL_STRING    ;= 1b
		UNIVERSAL_STRING  ;= 1c
		CHARACTER_STRING  ;= 1d
		BMP_STRING        ;= 1e Basic Multilingual Plane/Unicode string
	]

	decode-OID: function[
		oid [binary!]
		/full "Returns name with group name as a string"
		/local main name warn
	][
		parse/all oid [
			#{2B0E0302} (main: "Oddball OIW OID") [
				;http://oid-info.com/get/1.3.14.3.2
				  #"^(01)" (name: 'rsa)
				| #"^(02)" (name: 'md4WitRSA)
				| #"^(03)" (name: 'md5WithRSA)
				| #"^(04)" (name: 'md4WithRSAEncryption)
				| #"^(06)" (name: 'desECB)
				| #"^(07)" (name: 'desCBC)
				| #"^(0B)" (name: 'rsaSignature)
				| #"^(1A)" (name: 'sha1)
				| #"^(1D)" (name: 'sha1WithRSAEncryption)
			]
			|
			#{2B060105050701} (main: "PKIX private extension") [
				#"^(01)" (name: 'authorityInfoAccess)
			]
			|
			#{2B060105050730} (main: "PKIX") [
				;- access descriptor definitions
				  #"^(01)" (name: 'ocsp)         ; Online Certificate Status Protocol
				| #"^(02)" (name: 'caIssuers)    ; Certificate authority issuers 
				| #"^(03)" (name: 'timeStamping)
				| #"^(05)" (name: 'caRepository)
			]
			|
			#{2A8648CE3D} (main: "X9.62") [
				  #{0201}   (name: 'ecPublicKey)
				| #{0301} [
					  #"^(07)"  (name: 'secp256r1)
					;| #"^(02)"  (name: 'prime192v2)
					;| #"^(03)"  (name: 'prime192v3)  
					| #"^(01)"  (name: 'secp192r1)
				]
			]
			|
			#{2A864886F70D01} [
				#{01} (main: "PKCS #1") [
					  #"^(01)" (name: 'rsaEncryption)
					| #"^(02)" (name: 'md2WithRSAEncryption)
					| #"^(03)" (name: 'md4withRSAEncryption)
					| #"^(04)" (name: 'md5withRSAEncryption)
					| #"^(05)" (name: 'sha1WithRSAEncrption)
					| #"^(0B)" (name: 'sha256WithRSAEncryption)

				] end
				|
				#{07} (main: "PKCS #7") [
					  #"^(01)" (name: 'data)
					| #"^(02)" (name: 'signedData)
					| #"^(06)" (name: 'encryptedData)
				] end
				|
				#{09} (main: "PKCS #9") [
				;http://oid-info.com/get/1.2.840.113549.1.9
					  #"^(01)" (name: 'emailAddress warn: "Deprecated, use an altName extension instead")
					| #"^(03)" (name: 'contentType)
					| #"^(04)" (name: 'messageDigest)
					| #"^(05)" (name: 'signingTime)
					| #"^(0F)" (name: 'smimeCapabilities) 
					| #"^(14)" (name: 'friendlyName)
					| #"^(15)" (name: 'localKeyID)
					| #"^(34)" (name: 'CMSAlgorithmProtect)
				] end
				|
				#{0C} (main: "PKCS #12") [
					  #{0106}   (name: 'pbeWithSHAAnd40BitRC2-CBC)
					| #{0103}   (name: 'pbeWithSHAAnd3-KeyTripleDES-CBC)
					| #{0A0102} (name: 'pkcs-12-pkcs-8ShroudedKeyBag)
				] end
			] end
			|
			#{2A864886F70D03} (main: "Encryption algorithm") [
			;http://oid-info.com/get/1.2.840.113549.3
				  #"^(02)" (name: "rc2CBC")
				| #"^(03)" (name: "rc2ECB")
				| #"^(04)" (name: "rc4")
				| #"^(07)" (name: "des-ede3-cbc")
			] end
			|

			#{5504} (main: "X.520 DN component") [
				  #"^(03)" (name: 'commonName)
				| #"^(06)" (name: 'countryName)
				| #"^(07)" (name: 'localityName)
				| #"^(08)" (name: 'stateOrProvinceName)
				| #"^(0A)" (name: 'organizationName)
				| #"^(0B)" (name: 'organizationalUnitName)
				| #"^(0D)" (name: 'description)
				| #"^(0F)" (name: 'businessCategory)
			] end
			|
			#{551D} (main: "X.509 extension") [
				  #"^(01)" (name: 'authorityKeyIdentifier warn: "Deprecated, use 2 5 29 35 instead")
				| #"^(04)" (name: 'keyUsageRestriction warn: "Obsolete, use keyUsage/extKeyUsage instead")
				| #"^(0E)" (name: 'subjectKeyIdentifier)
				| #"^(0F)" (name: 'keyUsage)
				| #"^(11)" (name: 'subjectAltName)
				| #"^(13)" (name: 'basicConstraints)
				| #"^(1F)" (name: 'cRLDistributionPoints)
				| #"^(20)" (name: 'certificatePolicies)
				| #"^(23)" (name: 'authorityKeyIdentifier)
				| #"^(25)" (name: 'extKeyUsage)
			] end
			|
			#{2B060105050703} (main: "PKIX key purpose") [
				  #"^(01)" (name: 'serverAuth)
				  #"^(02)" (name: 'clientAuth)
				| #"^(03)" (name: 'codeSigning)
			] end
			|
			#{2B0601040182370201} (main: "Microsoft") [
				  #"^(15)" (name: 'individualCodeSigning)
			] end
		]
		;?? main
		;?? name
		;if warn [?? warn]

		either all [main name] [
			either full [
				rejoin [ any [name "<?name>"] " (" any [main "<?main>"] ")"]
			][	name ]
		][ oid ]
	]

	verbose: 0
]

register-codec [
	name:  'mobileprovision
	type:  'cryptography
	title: "Apple's mobileprovision file"
	suffixes: [%.mobileprovision]
	decode: function[data [binary!]][
		try [
			der: codecs/DER/decode data
			result: to string! der/sequence/cs0/sequence/sequence/cs0/2
		]
		result
	]
]