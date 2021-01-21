Rebol [
	Title:    "Test checksum speeds"
	Date:     21-Jan-2021
	Author:   "Oldes"
	File:     %test-checksums.r3
	Version:  0.0.1
	Requires: 3.4.0
	Note: {
My results:
Testing 10x checksum of 473088 bytes.

adler32   0:00:00.001465
crc24     0:00:00.011457
crc32     0:00:00.010477
tcp       0:00:00.001447
md4       0:00:00.006927
md5       0:00:00.010525
sha1      0:00:00.009612
sha224    0:00:00.027495
sha256    0:00:00.026062
sha384    0:00:00.01497
sha512    0:00:00.015317
ripemd160 0:00:00.013626
------------------------
total:    0:00:00.152634
	}
]

bin: read system/options/boot
num: 10

print ajoin ["^/Testing " num "x checksum of " length? bin " bytes.^/"] 

total: dt [
	foreach m system/catalog/checksums [
		t: dt [loop num [checksum bin m]]
		printf [10] reduce [m t]
	]
]
print  "------------------------"
print ["total:   " total LF]

ask "DONE"