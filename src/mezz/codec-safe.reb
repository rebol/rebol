REBOL [
	Name:    safe
	Version: 1.0.0
	Title:   "Codec: SAFE"
	Author:  "Oldes"
	History: [10-Jul-2022 "Oldes" "Initial version"]
	File:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-safe.reb
]

register-codec [
	name:  'safe
	type:  'crypt
	title: "Encrypted file storage"
	suffixes: [%.safe]

	encode: function [
		data  [any-type!]
		/key
		 password [any-string! binary!]
		/as
		 cipher [word!]
	][
		unless cipher [
			cipher: any [
				; choose by preference from available ciphers...
				foreach method [
					chacha20
					aes-192-gcm
					aes-192-ccm
					aes-192-cbc
				][
					if find system/catalog/ciphers method [
						break/return method
					]
				]
				; just use the first one available...
				first system/catalog/ciphers
			]
		]

		either binary? data [
			flags: 0
		][
			data: compress to binary! mold/all data 'zlib
			flags: 3
		]

		password: any [password ask-password]
		unless binary? password [
			password: checksum password 'sha256
		]

		bytes:  length? data
		check:  checksum data 'sha256
		output: make binary! 48 + bytes
		method: form cipher

		binary/write output [
			BYTES    :id
			UI16     :flags  ; so far only if data are compressed and molded (3)
			UI32     :bytes  ; original length of data
			BYTES    :check  ; having the checksum before method, so it's part is used as IV
			UI8BYTES :method 
		]
		if bytes > 0 [
			port: open [scheme: 'crypt algorithm: :cipher key: :password]
			modify port 'init-vector copy/part output 16
			bytes: length? data: take write port data
			close port
		]
		binary/write tail output [UI32 :bytes BYTES :data]
		output
	]

	decode: function [
		data  [binary!]
		/key
		 password [any-string! binary!]
	][
		unless parse data [id data: to end][return none]
		binary/read data [
			flags:        UI16
			bytes:        UI32     ; real data size (ecrypted may be padded)
			expected-sum: BYTES 32 ; checksum using SHA256
			method:       UI8BYTES
			length:       UI32
			pos:          INDEX
		]
		cipher: to word! to string! method

		password: any [password ask-password]
		unless binary? password [
			password: checksum password 'sha256
		]

		either bytes > 0 [
			port: open [
				scheme:    'crypt
				direction: 'decrypt
				algorithm: :cipher
				key:       :password
			]
			modify port 'init-vector copy/part head data 16

			output: take write/part port at head data :pos :length; decrypt the data
			clear skip output bytes ; remove optional padding
			close port
		][	output: copy #{} ]  ; there were no data
		
		real-sum: checksum output 'sha256
		unless equal? real-sum expected-sum [ return none ]
		if flags == 3 [output: load decompress output 'zlib]
		output
	]

	id: "SAFE 1.0"
	ask-password: does [
		any [
			get-env  "REBOL_SAFE_PASS"
			ask/hide "Enter SAFE Password: "
		]
	]

	;probe decode encode #(a: 1)
	;probe decode encode #(a: 1 b: 2)
	;probe decode encode #{DEAD}
]

