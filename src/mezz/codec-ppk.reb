REBOL [
	Title:   "REBOL 3 codec: PuTTY Private Key"
	Author:  "Oldes"
	Rights:  "Copyright (C) 2020 Oldes. All rights reserved."
	License: "BSD-3"
	Test:    %tests/units/rsa-test.r3
	Note: {
		* it extract (and inits) only RSA keys so far
	}
]

register-codec [
	name: 'ppk
	type: 'cryptography
	title: "PuTTY Private Key"
	suffixes: [%.ppk]

	decode: function [
		"Decodes PuTTY key file"
		data [binary! string! file!]
		/password pass [string! binary!] "Optional password for encrypted keys"
		/local type encr comm line pmac
		;return: [handle! none!] "RSA private key handle on success"
	][
		if file?   data [data: read/string data]
		if binary? data [data: to string! data]
		 sp:   charset " ^-^/^M"
		!sp:   complement sp
		!crlf: complement charset "^M^/"
		try/except [
			parse data [
				"PuTTY-User-Key-File-" ["1:" (vers: 1) | "2:" (vers: 2)]
					any sp copy type some !sp some sp
				"Encryption:"
					any sp copy encr some !sp some sp
				"Comment: "
					any sp copy comm some !crlf some sp
				"Public-Lines:"
					any sp copy num some !sp some sp
				(
					num: to integer! to string! num
					pub: make binary! 64 * num
				)
				num [ copy line any !crlf some sp (append pub line) ]
				"Private-Lines:"
					any sp copy num some !sp some sp
				(
					num: to integer! to string! num
					pri: make binary! 64 * num
				)
				num [ copy line any !crlf some sp (append pri line) ]
				["Private-MAC:" (mac?: true) | "Private-Hash:" (mac?: false)]
					any sp copy pmac some !sp any sp

				|

				"---- BEGIN SSH2 PUBLIC KEY ----" to end (
					return codecs/ssh-key/decode/password data pass
				)

			]
			pub: debase pub 64
			pri: debase pri 64

			if encr = "aes256-cbc" [
				try/except [
					pass: either password [copy pass][
						ask/hide ajoin ["Key password for " mold comm ": "]
					]
					key: join checksum join #{00000000} pass 'sha1
					          checksum join #{00000001} pass 'sha1
					key: aes/decrypt/key copy/part key 32 none
					pri: aes/decrypt/stream key pri
				][
					;clean pass data in memory
					forall pass [pass/1: random 255]
					print "Failed to decrypt private key!"
					return none
				]
			]

			macdata: either vers = 1 [ pri ][
				select binary/write 800 [
					UI32BYTES :type
					UI32BYTES :encr
					UI32BYTES :comm
					UI32BYTES :pub
					UI32BYTES :pri
				] 'buffer
			]
			mackey: checksum join "putty-private-key-file-mac-key" any [pass ""] 'sha1
			if pass [forall pass [pass/1: random 255]]
			if pmac <> form either mac? [
				checksum/with macdata 'sha1 mackey
			][	checksum      macdata 'sha1 ] [
				print either key ["Wrong password!"]["MAC failed!"]
				return none
			]
			binary/read pub [
				t: UI32BYTES	
				e: UI32BYTES ;exponent
				n: UI32BYTES ;modulus
			]
			binary/read pri [
				d: UI32BYTES
				p: UI32BYTES
				q: UI32BYTES
				i: UI32BYTES
			]
			if "ssh-rsa" = to string! t [
				return rsa-init/private n e d p q none none i
			]
		][
			print system/state/last-error
		]
		none
	]
]