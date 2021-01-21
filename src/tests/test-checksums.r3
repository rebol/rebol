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

adler32   0:00:00.001521
crc24     0:00:00.011748
crc32     0:00:00.010814
tcp       0:00:00.001168
md4       0:00:00.006888
md5       0:00:00.010386
sha1      0:00:00.010025
sha224    0:00:00.026268
sha256    0:00:00.027214
sha384    0:00:00.015616
sha512    0:00:00.016317
ripemd160 0:00:00.014123
------------------------
total:    0:00:00.152088
	}
]

bin: read system/options/boot
num: 10

print ajoin ["^/Testing " num "x checksum of " length? bin " bytes.^/"] 

total: 0:0:0
foreach m system/catalog/checksums [
	t: dt [loop num [checksum bin m]]
	printf [10] reduce [m t]
	total: total + t
]
print  "------------------------"
print ["total:   " total LF]

ask "DONE"