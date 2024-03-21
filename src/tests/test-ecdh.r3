Rebol [
	Title:    "Test ECDH exchange"
	Date:     14-Jan-2022
	Author:   "Oldes"
	File:     %test-ecdh.r3
	Version:  0.0.1
	Requires: 3.4.0
]

k1: none
k2: none
foreach curve [
	secp192r1      ; 192-bit curve defined by FIPS 186-4 and SEC1
	secp224r1      ; 224-bit curve defined by FIPS 186-4 and SEC1
	secp256r1      ; 256-bit curve defined by FIPS 186-4 and SEC1
	secp384r1      ; 384-bit curve defined by FIPS 186-4 and SEC1
	secp521r1      ; 521-bit curve defined by FIPS 186-4 and SEC1
	bp256r1        ; 256-bit Brainpool curve
	bp384r1        ; 384-bit Brainpool curve
	bp512r1        ; 512-bit Brainpool curve
	curve25519     ; Curve25519
	secp192k1      ; 192-bit "Koblitz" curve
	secp224k1      ; 224-bit "Koblitz" curve
	secp256k1      ; 256-bit "Koblitz" curve
	curve448       ; Curve448
][
	prin pad curve 12
	tm: dt [
		ok: attempt [all [
			k1: ecdh/init k1 curve
			k2: ecdh/init k2 curve
			curve = ecdh/curve k1
			curve = ecdh/curve k2
			p1: ecdh/public k1
			p2: ecdh/public k2
			s1: ecdh/secret k1 p2
			s2: ecdh/secret k2 p1
			s1 = s2
		]]
	]
	print either ok [[as-green "OK" tm]][as-red "NA"]
]

if system/options/script [ask "DONE"]
()