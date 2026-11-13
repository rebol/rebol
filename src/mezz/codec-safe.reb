REBOL [
	Title:   "Codec: SAFE"
	Name:    safe
	type:    module
	Version: 1.0.0
	Author:  "Oldes"
	History: [
		10-Jul-2022 "Oldes" "Initial version"
		12-Jul-2022 "Oldes" "Included SAFE scheme"
	]
	File:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-safe.reb
	TODO: [
		"The scheme could require to enter the password again after some time of inactivity."
	]
]

register-codec [
	name:  'safe
	type:  'crypt
	title: "Encrypted file storage"
	suffixes: [%.safe]

	encode: function [
		data  [any-type!]
		/key
		 password [any-string! binary! none!]
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
		data  [binary! file! url!]
		/key
		 password [any-string! binary! none!]
	][
		unless binary? data [ data: read/binary data ]
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
]


sys/make-scheme [
	title: "Persistent key/value storage"
	name: 'safe
	actor: [
		open: func [port [port!] /local spec host][
			spec: port/spec
			spec/ref: rejoin either/only host: select spec 'host [
				https:// host
				select spec 'path
				select spec 'target %""
			][
				any [select spec 'path   system/options/home]
				any [select spec 'target %.safe]
			]
			if %.safe <> suffix? spec/ref [
				append spec/ref %.safe
			]

			port/data: object [
				data: none
				pass: any [
					select spec 'pass
					ask/hide "Enter password: "
				]
				file: port/spec/ref
				date: none
				get:  func[key /local value][
					; returned values are always copied, if possible, so
					; they are not modified by accident from outside
					either find copyable! type? value: select data :key [
						copy/deep :value
					][	:value ]
				]
				set:  func[key [word!] val [any-type!]][put data :key :val]
				rem:  func[key][remove/key data :key #(unset!)]
				load: does [
					date: modified? file
					data: system/codecs/safe/decode/key :file :pass
					unless data [
						print [as-purple "*** Failed to decrypt data from:" as-red file]
					]
				]
				save: does [
					if url? file [
						print as-purple "*** Saving to URL is not yet implemented!"
						exit
					]
					;@@ TODO: prevent collision when 2 processes tries to write in the same time!
					write/binary file system/codecs/safe/encode/key :data :pass
					date: modified? file
				]
				sync: func[/close /local modf] [
					if data [
						;; There are already some data..
						case [
							not exists? file [ save ]
							date > modf: modified? file [ save ]
							date < modf [ load ]
						]
						data
					]
					case [
						close [
							data: date: pass: none
						]
						none? data [
							;; There are no data yet, so load it or make a new map!
							either exists? file [ load ][
								data: make map! 4
								save
							]
						]
					]
					file
				]
				open?: does [ map? data ]
				change-pass: func[new [string! binary!]][
					either pass = ask/hide "Old password: " [
						pass: new
						date: now/precise
						sync
						true
					][
						sys/log/error 'REBOL "Password validation failed!"
						false
					]
				]
				
				sync

				protect/words/hide [data pass load save]
				protect/words [get set rem sync open? change-pass]
			]

			; make only these required values availeble in the spec:
			set port/spec: object [title: scheme: ref: none] spec

			;- It is not possible to protect data so far, because of:
			;- https://github.com/Oldes/Rebol-issues/issues/1148     
			;protect/words port/data
			port
		]
		; We must use the inner functions bellow, because the inner data are hidden!
		open?: func[port][
			port/data/open?
		]
		close: func[port][
			port/data/sync/close
		]
		put:
		poke: func[port key value][
			port/data/date: now/precise
			port/data/set :key :value
		]
		pick:
		select: func[port key][
			port/data/get :key
		]
		remove: func[port /part range /key key-arg][
			port/data/date: now/precise
			port/data/rem :key-arg
		]
		update: func[port][
			port/data/sync
		]
		modify: func[port field value][
			if field = 'password [port/data/change-pass :value]
		]
	]
]