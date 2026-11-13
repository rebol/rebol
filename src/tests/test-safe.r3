Rebol [
	Title:    "Test SAFE codec port"
	Date:     10-Jul-2022
	Author:   "Oldes"
	File:     %test-safe.r3
	Version:  1.0.0
	Requires: 3.8.0
]

unless find system/codecs 'safe [
	ask as-red "SAFE codec not found!"
	quit
]

original: read any [
	system/options/script
	system/options/boot
]

en-safe: :system/codecs/safe/encode
de-safe: :system/codecs/safe/decode

print ["Encrypting and decrypting" length? original "bytes"]
foreach cipher system/catalog/ciphers [
	if cipher = 'chacha20-poly1305 [continue]
	prin pad cipher 19
	t1: dt [encrypted: en-safe/key/as original "my-pass" :cipher]
	t2: dt [decrypted: de-safe/key   encrypted "my-pass"]
	prin [pad t1 16 pad t2 16]
	print either decrypted [as-green " OK"][as-red reduce [" FAILED!" length? decrypted]]
]

if system/options/script [ask "DONE"]