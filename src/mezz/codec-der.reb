REBOL [
	title: "Codec: DER/BER structures"
	name: der
	type: module
	version: 0.3.1
	date:    18-Mar-2025
	author: "Oldes"	
	history: [
		0.1.0 17-Oct-2018 "Oldes" {Initial version with DECODE and IDENTIFY functions.}
		0.2.0 17-Feb-2022 "Oldes" {Including `form-id`}
		0.3.0 18-Mar-2025 "Oldes" {Decoding a few more common OIDs used in TLS}
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
	suffixes: [%.p12 %.pfx %.cer %.der %.jks %.p7s]
	decode: function[data [binary!]][
		if verbose > 0 [
			print ["^/^[[1;32mDecode DER data^[[m (^[[1m" length? data "^[[mbytes )"]
			; count maximal bytes width (used for padding)
			wl: length? form length? data
			wr: negate wl
		]

		case [
			all [data/1 = 0 data/2 = 48][data: next data]
			data/1 <> 48 [
				if verbose > 0 [
					prin "*** DER data does not start with SEQUENCE tag ***^/*** "
					probe copy/part data 10
				]
				return none
			]
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
						;@@if data/1 = 0 [data: next data]
						;data: enbase data 2
					]
					INTEGER [
						;@@ TODO: review if the null skipping is correct!
						;@@if data/1 = 0 [data: next data]
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
					#" " /bright-magenta wr /green ":d=" /bright-cyan 2 /green
					"hl=" /bright-green 2 /green
					"l=" /bold wl /green #" " -5
					#" " /bright-cyan 18 /reset] [
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

	form-OID: either find lib 'form-oid [
		; native version
		:lib/form-oid
	][
		function[
			"Return the x.y.z.... style numeric string for the given OID"
			oid [binary!]
		][
			len: length?  oid
			out: make string! 3 * len
			append out ajoin [to integer! oid/1 / 40  #"."  oid/1 % 40]
			++ oid
			value: 0
			while [not tail? oid][
				value: (value << 7)
				value: value + (oid/1 & 127)
				if oid/1 & 128 = 0 [
					append append out #"." value
					value: 0
				]
				++ oid
			]
			out
		]
	]
	decode-OID: function[
		"Convert given OID to its name if recognized or a numeric string"
		oid [binary!]
		/full "Returns name with group name as a string"
		/local main name warn
	][
		parse oid [
			#{2B0E0302} (main: "Oddball OIW OID")
			;; 1.3.14.3.2
			;= OIW Security Special Interest Group defined algorithms
			set n: skip (
				name: select #[
					0#01 rsa
					0#02 md4WithRSA
					0#03 md5WithRSA
					0#04 md4WithRSAEncryption
					0#06 desECB
					0#07 desCBC
					0#0B rsaSignature
					0#0C dsaSignature
					0#0D dsaWithSHA
					0#1A sha1
					0#1D sha1WithRSAEncryption
				] n
			) end
			|
			#{2B060105050701} (main: "PKIX private extension")
			;; 1.3.6.1.5.5.7.1
			;= Public-Key Infrastructure using X.509 (PKIX) certificate extensions
			[
				#"^(01)" (name: 'authorityInfoAccess) ;; Provides information about the issuer of the certificate
			]
			|
			#{2B060105050730} (main: "PKIX")
			;; 1.3.6.1.5.5.7.48
			;= access descriptor definitions
			set n: skip (
				name: select #[
					0#01 ocsp             ;; Online Certificate Status Protocol
					0#02 caIssuers        ;; Certificate authority issuers
					0#03 timeStamping     ;; Used for time-stamping services to ensure data integrity over time
					0#05 caRepository     ;; Represents a repository for CA-related data
				] n
			) end
			|
			#{2A8648CE3D} (main: "X9.62")
			;; 1.2.840.10045
			;; The Elliptic Curve Digital Signature Algorithm (ECDSA)
			 [
				  #{0201}   (name: 'ecPublicKey)
				| #{0301} [
					  #"^(07)"  (name: 'secp256r1)
					;| #"^(02)"  (name: 'prime192v2)
					;| #"^(03)"  (name: 'prime192v3)  
					| #"^(01)"  (name: 'secp192r1)
				]
				| #{0403} [
					  #"^(01)" (name: 'ecdsa-with-SHA224)
					| #"^(02)" (name: 'ecdsa-with-SHA256)
					| #"^(03)" (name: 'ecdsa-with-SHA384)
					| #"^(04)" (name: 'ecdsa-with-SHA512)
				]
			]
			|
			#{2A864886F70D01} ;= 1.2.840.113549.1 belongs to the RSA Data Security, Inc.
			                  ;; PKCS (Public-Key Cryptography Standards) family
			[
				#"^(01)" (main: "PKCS #1")
				;; 1.2.840.113549.1.1
				;= RSA Cryptography Standard
				set n: skip (
					name: select #[
						0#01 rsaEncryption
						0#02 md2WithRSAEncryption
						0#03 md4withRSAEncryption
						0#04 md5withRSAEncryption
						0#05 sha1WithRSAEncrption
						0#0B sha256WithRSAEncryption
						0#0C sha384WithRSAEncryption
						0#0D sha512WithRSAEncryption
						0#0E sha224WithRSAEncryption
					] n
				) end
				|
				#"^(07)" (main: "PKCS #7")
				;; 1.2.840.113549.1.7
				;= Cryptographic Message Syntax, CMS
				set n: skip (
					name: select #[
						0#01 data
						0#02 signedData
						0#03 envelopedData
						0#04 signedAndEnvelopedData
						0#05 digestedData
						0#06 encryptedData
					] n
				) end
				|
				#"^(09)" (main: "PKCS #9")
				;; 1.2.840.113549.1.9
				;= Attributes like email, signing time, etc.
				set n: skip (
					name: select #[
						0#01 emailAddress         ;; Email address of the entity
						0#02 unstructuredName     ;; Human-readable name
						0#03 contentType          ;; Type of signed data
						0#04 messageDigest        ;; Message hash used in signing
						0#05 signingTime          ;; Timestamp when signing occurred
						0#06 counterSignature     ;; Countersignature on a signed message
						0#07 challengePassword    ;; Password for certificate request authentication
						0#08 unstructuredAddress  ;; Unformatted address
						0#0E extensionRequest     ;; Used in PKCS #10 for requesting X.509 extensions
					] n
				) end
				|
				#"^(0C)" (main: "PKCS #12")
				;; 1.2.840.113549.1.12
				;= Personal Information Exchange Syntax
				[	  #{0106}   (name: 'pbeWithSHAAnd40BitRC2-CBC)
					| #{0103}   (name: 'pbeWithSHAAnd3-KeyTripleDES-CBC)
					| #{0A0102} (name: 'pkcs-12-pkcs-8ShroudedKeyBag)
				] end
			] end
			|
			#{2A864886F70D03} (main: "Encryption algorithm")
			;http://oid-info.com/get/1.2.840.113549.3
			;= Symmetric encryption algorithms
			set n: skip (
				name: select #[
					0#02 rc2CBC
					0#03 rc2ECB
					0#04 rc4
					0#05 rc4WithMAC
					0#06 DESx
					0#07 tripleDES-CBC
					0#08 rc5CBC
					0#09 rc5ECB
				] n
			) end
			|
			#{2B810400} (main: "SECG curve")
			;; ANSI X9.62 standard, which defines elliptic curves for cryptographic use
			;; https://oid-base.com/get/1.3.132.0
			set n: skip (
				name: select #[
					0#01 secp192r1 ;(NIST P-192)
					0#02 sect163k1
					0#03 sect163r1
					0#04 sect239k1
					0#05 sect283k1
					0#06 sect283r1
					0#07 secp160k1
					0#08 secp160r1
					0#09 secp160r2
					0#0A secp192k1
					0#0F secp256k1
					0#10 sect233k1
					0#11 sect233r1
					0#21 secp224r1 ;(NIST P-224)
					0#22 secp384r1 ;(NIST P-384)
					0#23 secp521r1 ;(NIST P-521)
				] n
			) end
			|
			#{5504} (main: "X.520 DN component")
			;; 2.5.4
			set n: skip (
				name: select #[
					0#03 commonName
					0#06 countryName
					0#07 localityName
					0#08 stateOrProvinceName
					0#09 streetAddress
					0#0A organizationName
					0#0B organizationalUnitName
					0#0C title
					0#0D description
					0#0E searchGuide 
					0#0F businessCategory
					0#10 postalAddress
					0#11 spostalCode
					0#12 postOfficeBox
					0#13 physicalDeliveryOfficeName
					0#14 telephoneNumber
				] n
			) end
			|
			#{551D} (main: "X.509 extension")
			;; 2.5.29
			;= Additional attributes for X.509 certificates used in PKI (Public Key Infrastructure). 
			set n: skip (
				name: select #[
					0#01 authorityKeyIdentifier ;; Deprecated, use 2 5 29 35 instead
					0#02 keyAttributes          ;; Obsolete!
					0#03 certificatePolicies    ;; Old version  
					0#04 keyUsageRestriction    ;; Obsolete, use keyUsage/extKeyUsage instead
					0#0E subjectKeyIdentifier   ;; Unique ID of the public key 
					0#0F keyUsage               ;; Defines permitted key operations, e.g., signing, encryption
					0#11 subjectAltName         ;; Alternative names, such as multiple domains or emails
					0#12 issuerAlternativeName  ;; Alternative names for the certificate issuer
					0#13 basicConstraints       ;; Defines whether the certificate is a CA or end-entity
					0#1E nameConstraints        ;; Defines allowed or disallowed name spaces
					0#1F CRLDistributionPoints  ;; Defines where to check for revoked certificates
					0#20 certificatePolicies    ;; Defines policies under which the certificate is issued
					0#21 policyMappings         ;; Links different policy OIDs for compatibility
					0#23 authorityKeyIdentifier ;; Identifies the CA’s public key
					0#24 policyConstraints      ;; Defines restrictions on policy inheritance
					0#25 extendedKeyUsage       ;; Defines additional usage, e.g., SSL, email protection, code signing
					0#2E freshestCRL            ;; Defines the newest Certificate Revocation List
				] n
			) end
			|
			#{2B060105050703} (main: "PKIX key purpose")
			;; 1.3.6.1.5.5.7.3
			;= The extended key usage (EKU) field in X.509 certificates
			set n: skip (
				name: select #[
					0#01 serverAuth
					0#02 clientAuth
					0#03 codeSigning
					0#04 emailProtection
					;0#05 ipsecEndSystem
					;0#06 ipsecTunnel
					;0#07 ipsecUser
					0#08 timeStamping
					0#09 OCSPSigning
					;0#0A dvcs
					;0#0B sbgpCertAAServerAuth
					;0#0C scvp
					;0#0D eapOverPPP
					;0#0E eapOverLAN
					;0#0F scvpServer
					;0#10 scvpClient
					;0#11 ipsecIKE
					;0#12 capwapAC
					;0#13 capwapWTP
					;0#14 sipDomain
					;0#15 secureShellClient
					;0#16 secureShellServer
					;0#17 sendRouter
					;0#18 sendProxiedRouter
					;0#19 sendOwner
					;0#1A sendProxiedOwner
					;0#1B cmcCA
					;0#1C cmcRA
					;0#1D cmcArchive
					;0#1E bgpsec-router
					;0#1F BrandIndicatorforMessageIdentification
					;0#20 cmKGA
					;0#21 rpcTLSClient
					;0#22 rpcTLSServer
					;0#23 bundleSecurity
				] n
			) end
			|
			#{2B06010401} [;= 1.3.6.1.4.1 -> Private Enterprise Numbers
				#{82370201} (main: "Microsoft") [
					#"^(15)" (name: 'individualCodeSigning)
				]
				|
				#{D679} (main: "Google") [
					#{020402} (name: 'X509Extension)
				]
			] end
			|
			#{0992268993F22C6401} (main: "Attribute") [
				; http://oid-info.com/cgi-bin/display?tree=0.9.2342.19200300.100.1.1
				#"^(01)" (name: 'uid)
			] end
		]
		;?? main
		;?? name
		;if warn [?? warn]

		either all [main name] [
			either full [
				rejoin [ any [name "<?name>"] " (" any [main "<?main>"] ")"]
			][	name ]
		][
			log-trace 'DER ["Failed to decode OID" oid "->" form-oid oid]
			form-oid oid
		]
	]

	system/options/log/der: verbose: 0
]
