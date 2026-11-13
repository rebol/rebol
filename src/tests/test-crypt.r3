Rebol [
	Title:    "Test crypt port"
	Date:     3-Feb-2022
	Author:   "Oldes"
	File:     %test-crypt.r3
	Version:  1.0.0
	Requires: 3.8.0
]

my-secret: context [
	password:    checksum "my not much secret password" 'sha256
]
;- using just simple wrapper functions for [en/de]cryption
encrypt: function [data cipher [word!]] [
	binary/write vector: #{} [random-bytes 16]
	port: open [
		scheme:      'crypt
		algorithm:   :cipher
		key:         :my-secret/password
		init-vector: :vector
	]
	bytes: length? data
	check: checksum/with data 'sha256 my-secret/password
	output: make binary! 48 + bytes
	binary/write output [UI32 :bytes :check :vector]
	if bytes > 0 [
		write port data
		; here could be more writes, for example for showing a progress...
		update port ; finish the stream... adds padding if needed
		append output read port ; resolve encrypted data
	]
	close port
	output
]
decrypt: function [data cipher [word!]] [
	binary/read data [
		bytes:        UI32     ; real data size (ecrypted may be padded)
		expected-sum: BYTES 32 ; checksum using SHA256 and the password
		vector:       BYTES 16 ; random initial vector used for encryption
		pos:          INDEX
	]
	output: either bytes > 0 [
		port: open [
			scheme:      'crypt
			direction:   'decrypt
			algorithm:   :cipher
			key:         :my-secret/password
			init-vector: :vector
		]
		write port at data :pos ; decrypt the rest of data (skipping the header)
		; here could be more writes, for example for showing a progress...
		read update port        ; get decrypted data
	][	copy #{} ]              ; there were no data
	close port                  ; and close the port as is not needed anymore
	;; remove the padding
	clear skip output bytes
	real-sum: checksum/with output 'sha256 my-secret/password
	either real-sum = expected-sum [output][none]
]

original: read system/options/boot

print ["Encrypting and decrypting" length? original "bytes"]
foreach cipher system/catalog/ciphers [
	if cipher = 'chacha20-poly1305 [continue]
	prin pad cipher 19
	t1: dt [encrypted: encrypt original :cipher]
	t2: dt [decrypted: decrypt encrypted :cipher]
	prin [pad t1 16 pad t2 16]
	print either decrypted [as-green " OK"][as-red reduce [" FAILED!" length? decrypted]]
]

if system/options/script [ask "DONE"]