Rebol [
	Title:    "Test compression"
	Date:     28-Jun-2023
	Author:   "Oldes"
	File:     %test-compression.r3
	Version:  0.0.1
;;	Requires: 3.11.0
	Note: {}
]

bin: to binary! mold system

foreach level [1 5 9][
	print ajoin ["^/Testing compression of " length? bin " bytes with level " level ".^/"]

	sum: checksum bin 'sha256

	foreach m system/catalog/compressions [
		t1: attempt [ dt [out: compress/level bin m level] ]
		sz: attempt [ length? out                          ]
		t2: attempt [ dt [out: decompress out m]           ]
		ok: attempt [ equal? sum checksum out 'sha256      ]
		printf [10 10 20 20] reduce [m sz t1 t2 ok]
	]
	print  "------------------------"
]

if system/options/script [ask "DONE"]
total