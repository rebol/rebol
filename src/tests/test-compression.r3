Rebol [
	Title:    "Test compression"
	Date:     24-Dec-2025
	Author:   "Oldes"
	File:     %test-compression.r3
	Version:  0.0.2
;;	Requires: 3.11.0
	Note: {}
]

foreach m [brotli zstd zlib-ng deflate][
	attempt [import (m)]
]

;; Using molded system as a test input (large text data).
;; When running this script from REPL console,
;; make sure that we don't mold the system multiple times,
;; else its size would be significantly bigger!
unless binary? :bin [bin: to binary! mold system]
sum: checksum bin 'sha256 ;; Used to validate decompressed result
len: length? bin          ;; Used as a hint for the decompression

foreach level [1 5 9 22][
	print as-green ajoin ["^/Testing compression of " length? bin " bytes with level " level ".^/"]

	print as-yellow {Method    c.size    factor    com.time    dec.time    valid}
	foreach m system/catalog/compressions [
		t1: attempt [ to decimal! dt [out: compress/level bin m level] ]
		sz: attempt [ length? out ]
		fc: attempt [ round/to (len / sz) 0.001 ]
		t2: attempt [ to decimal! dt [out: decompress/size out m len] ]
		ok: attempt [ equal? sum checksum out 'sha256 ]
		printf [10 10 10 12 12] [m sz fc t1 t2 ok]
	]
	print  "------------------------"
]

if system/options/script [ask "DONE"]
