Rebol [
	Title:   "Rebol checksum test script"
	Author:  "Oldes"
	File: 	 %checksum-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Checksum"

===start-group=== "Checksum of string"
	--test-- {checksum of string}
		--assert #{C000CBCECDCD2582B0418B8CF0301A8E} = checksum "ščř" 'md5
		--assert #{900150983CD24FB0D6963F7D28E17F72} = checksum "abc" 'md5
	--test-- {checksum/part of string}
		--assert #{900150983CD24FB0D6963F7D28E17F72} = checksum/part "abc123" 'md5 3
		--assert #{900150983CD24FB0D6963F7D28E17F72} = checksum/part skip "123abc" 3 'md5 3
===end-group===


===start-group=== "Checksum with key (issue #1910)"
;@@ https://github.com/Oldes/Rebol-issues/issues/1910
	--test-- "checksum-1"
		--assert #{800A1BC1B53CAA795F4DF39DC57652209239E1F1}
					= checksum/with "Hello world" 'sha1 "mykey"
		--assert #{800A1BC1B53CAA795F4DF39DC57652209239E1F1}
					= checksum/with to binary! "Hello world" 'sha1 "mykey"
		--assert #{800A1BC1B53CAA795F4DF39DC57652209239E1F1}
					= checksum/with to binary! "Hello world" 'sha1 to binary! "mykey"
	--test-- "checksum with unicode key"
		; any string key is converted to unicode
		--assert #{5EA5CFA243BE16926AF5B2620AE8D383} = checksum/with "a" 'md5 "č"
		--assert #{5EA5CFA243BE16926AF5B2620AE8D383} = checksum/with "a" 'md5 @č
		--assert #{5EA5CFA243BE16926AF5B2620AE8D383} = checksum/with "a" 'md5 %č
		--assert #{5EA5CFA243BE16926AF5B2620AE8D383} = checksum/with "a" 'md5 to binary! "č"

===end-group===


===start-group=== "Checksum basic"
	--test-- {checksum ""}
		str: ""
		--assert #{da39a3ee5e6b4b0d3255bfef95601890afd80709}
					= checksum str 'sha1
		--assert #{e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855}
					= checksum str 'sha256
		--assert #{38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b}
					= checksum str 'sha384
		--assert #{cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e}
					= checksum str 'sha512
		--assert  1 = checksum str 'adler32
	--test-- {checksum #{}}
		bin: #{}
		--assert #{da39a3ee5e6b4b0d3255bfef95601890afd80709}
					= checksum bin 'sha1
		--assert #{e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855}
					= checksum bin 'sha256
		--assert #{38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b}
					= checksum bin 'sha384
		--assert #{cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e}
					= checksum bin 'sha512
		--assert  1 = checksum bin 'adler32
	--test-- {checksum adler32}
		--assert       65537 = checksum "^@" 'adler32
		--assert    11731034 = checksum "X^A" 'adler32
		--assert   695534982 = checksum "message digest" 'adler32
		--assert  2329613580 = checksum "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" 'adler32
		--assert  2545291369 = checksum "12345678901234567890123456789012345678901234567890123456789012345678901234567890" 'adler32


===end-group===


===start-group=== "Checksum port"
	bin: #{0BAD}
	bin2: join bin bin
	--test-- "checksum-port-md5"
		port: open checksum://
		sum1: checksum bin 'md5
		sum2: checksum bin2 'md5
		--assert port? port
		--assert open? port
		--assert 'md5 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
		port: open checksum:md5
		--assert 'md5 = port/spec/method

		;; using just a name of the scheme...
		;@@ https://github.com/Oldes/Rebol-issues/issues/826
		--assert all [
			port? try [port: open 'checksum]
			'md5 = port/spec/method
		]

	--test-- "checksum-port-sha1"
		port: open checksum:sha1
		sum1: checksum bin 'sha1
		sum2: checksum bin2 'sha1
		--assert #{ED53B6E608B8E821640F4AC1278EE402E5EA0ED5} = sum1
		--assert port? port
		--assert open? port
		--assert 'sha1 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
if find system/catalog/checksums 'sha224 [
	--test-- "checksum-port-sha224"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2580
		port: open checksum:sha224
		sum1: checksum bin 'sha224
		sum2: checksum bin2 'sha224
		--assert sum1 = #{2CFF14B122C25DF72902A59A877E82A80CF637056A51AD5A343755B4}
		--assert sum2 = #{0D32498E6CAACAE9CAC68626A65ABE97690F6BB1BEF02868A97F46BB}
		--assert port? port
		--assert open? port
		--assert 'sha224 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]
	--test-- "checksum-port-sha256"
		port: open checksum:sha256
		sum1: checksum bin 'sha256
		sum2: checksum bin2 'sha256
		--assert sum1 = #{183559C9230A3361110FF397037E53E998B6166002BF0FC0603C8939CC89539A}
		--assert sum2 = #{5AC1B5FC2664C14E58969DAA340D7C180D64072E72FCD13B82CD89DCEC44CA14}
		--assert port? port
		--assert open? port
		--assert 'sha256 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port

	--test-- "checksum-port-sha384"
		port: open checksum:sha384
		sum1: checksum bin 'sha384
		sum2: checksum bin2 'sha384
		--assert sum1 = #{
B0C9ADA83C89485563049E5CF212911F334A788D47C97CC9A1D952C9E9EB8B5D
40FC4DAE76AF7024712A5BFC7DFA7BF4}
		--assert sum2 = #{
198C40C38637BF68323D005261066112ECD25FE90ABF491B458883D80A436C95
B9B8B9F1CEC9EFE43153694B5D1EDFD2}
		--assert port? port
		--assert open? port
		--assert 'sha384 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port

	--test-- "checksum-port-sha512"
		port: open checksum:sha512
		sum1: checksum bin 'sha512
		sum2: checksum bin2 'sha512
		--assert sum1 = #{
D2079D59D6984814DAC71CDEB38097DB52F77810391FD7B6F92FFBD64EA93DF8
7783EF1E4FEF4ABA834FF3C186A17B2E8DF7B08AF35A96E3802D280AB35BFE1B}
		--assert sum2 = #{
37195CDD26E210FAAE2684FF60526EF6163D1656158CA3534D32895C981D49AC
378ACE3B6EADA57F6A072CC95094EC09E1AD2A5F4BBA5B84B5969F5D418DED56}
		--assert port? port
		--assert open? port
		--assert 'sha512 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
if find system/catalog/checksums 'ripemd160 [
	;@@ https://github.com/Oldes/Rebol-issues/issues/2516
	--test-- "checksum-port-ripemd160"
		port: open checksum:ripemd160
		sum1: checksum bin 'ripemd160
		sum2: checksum bin2 'ripemd160
		--assert sum1 = #{595FEC4966B173C6CD00ECCAF1A007F3C6C5B938}
		--assert sum2 = #{F39EF68D81BFC2956AE08FB8BBA0347B3AC7A06A}
		--assert port? port
		--assert open? port
		--assert 'ripemd160 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]
if find system/catalog/checksums 'sha3-224 [
	--test-- "checksum-port-sha3-224"
		port: open checksum:sha3-224
		sum1: checksum bin 'sha3-224
		sum2: checksum bin2 'sha3-224
		--assert sum1 = #{4989EDA57367DA7AD56223D5D3CDB6B872D69E109D33453AA1E20BF4}
		--assert sum2 = #{2F21452C77B2E8C48B6EAD9AEF305DB9508D85E79B8E20CC177FA482}
		--assert port? port
		--assert open? port
		--assert 'sha3-224 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]
if find system/catalog/checksums 'sha3-256 [
	--test-- "checksum-port-sha3-256"
		port: open checksum:sha3-256
		sum1: checksum bin 'sha3-256
		sum2: checksum bin2 'sha3-256
		--assert sum1 = #{05A60FB439A3A289A3F6CA061F008CC921109AEB22D3DCDC89FDF061AF5053A4}
		--assert sum2 = #{AD98A5CE91DCC6151A3D2E40D538D145BB6BB20F2E0AD0148FD0B353E95E9044}
		--assert port? port
		--assert open? port
		--assert 'sha3-256 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]
if find system/catalog/checksums 'sha3-384 [
	--test-- "checksum-port-sha3-384"
		port: open checksum:sha3-384
		sum1: checksum bin 'sha3-384
		sum2: checksum bin2 'sha3-384
		--assert sum1 = #{
8508DCBB087FF43A43F2EDFCBFF613C8DA960922B920C7C49D65497B91714439
1CE6D5883737C1314E2939F009791616}
		--assert sum2 = #{
4EB270CF5530457330B251397F7FC1BE807F20D52A2E28EE4B8C1B5006FA1CB7
D62559A7BA788A602559715B0939F675}
		--assert port? port
		--assert open? port
		--assert 'sha3-384 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]
if find system/catalog/checksums 'sha3-512 [
	--test-- "checksum-port-sha3-512"
		port: open checksum:sha3-512
		sum1: checksum bin 'sha3-512
		sum2: checksum bin2 'sha3-512
		--assert sum1 = #{
C71260D0C2961CA303BAFB0F55EA436AF32FD635FD9D427DF0930B641797B033
9DBB60135BE90F0711B8A37A26CF2A0BF6FB3FC4AA57946901E4BCC78E54FDB6}
		--assert sum2 = #{
19AA22168178EC03C82E45ABB89971A645D2918C3BF043F15EF544C879EED740
C4294CF5AD661A4551844E9EEA3A1074188503B24349912BFAA21BD5FB4ACFBE}
		--assert port? port
		--assert open? port
		--assert 'sha3-512 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]

if find system/catalog/checksums 'xxh3 [
	--test-- "checksum-port-xxh3"
		port: open checksum:xxh3
		sum1: checksum bin 'xxh3
		sum2: checksum bin2 'xxh3
		--assert sum1 = #{9FEF7E6F59EC717A}
		--assert sum2 = #{078967DE7F525BA5}
		--assert port? port
		--assert open? port
		--assert 'xxh3 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]
if find system/catalog/checksums 'xxh32 [
	--test-- "checksum-port-xxh32"
		port: open checksum:xxh32
		sum1: checksum bin 'xxh32
		sum2: checksum bin2 'xxh32
		--assert sum1 = #{7E793E85}
		--assert sum2 = #{E0F8511E}
		--assert port? port
		--assert open? port
		--assert 'xxh32 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]
if find system/catalog/checksums 'xxh64 [
	--test-- "checksum-port-xxh64"
		port: open checksum:xxh64
		sum1: checksum bin 'xxh64
		sum2: checksum bin2 'xxh64
		--assert sum1 = #{1850B5D77D6C60CF}
		--assert sum2 = #{3FCEF9F54F46787C}
		--assert port? port
		--assert open? port
		--assert 'xxh64 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]
if find system/catalog/checksums 'xxh128 [
	--test-- "checksum-port-xxh128"
		port: open checksum:xxh128
		sum1: checksum bin 'xxh128
		sum2: checksum bin2 'xxh128
		--assert sum1 = #{55E28E62BBDB8B2D9FEF7E6F59EC717A}
		--assert sum2 = #{EB7A7F7736C45EEB1379AF60CEAB7F2A}
		--assert port? port
		--assert open? port
		--assert 'xxh128 = port/spec/method
		--assert port? write port bin
		--assert sum1 = read port
		--assert port? write port bin
		--assert sum2 = read port
		--assert sum2 = read port
		--assert not open? close port
		--assert port? write open port bin
		--assert port? update port
		--assert sum1 = port/data
		close port
]


	--test-- "checksum-write-refinements"
		port: open checksum://
		write/part port bin 1
		write/part port next bin 1
		sum1: checksum bin port/spec/method
		sum2: checksum join bin bin port/spec/method
		--assert sum1 = read port
		--assert sum1 = read write/part port bin 0
		--assert sum1 = read write/part port bin -1
		--assert sum2 = read write/part port tail bin -2
		port: open checksum://
		--assert sum1 = read write/seek/part port #{cafe0bad} 2 2
		;opening already opened port restarts computation
		--assert sum1 = read write/seek/part open port #{cafe0bad} 2 2
		--assert sum1 = read write/seek/part open port tail #{cafe0bad} -2 2

	--test-- "checksum port with invalid argument"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2553
		--assert all [error? e: try [write checksum:md5 1]  e/id = 'invalid-arg]
===end-group===

===start-group=== "Checksum HMAC SHA"
;@@ https://tools.ietf.org/html/rfc4231
	--test-- "test case 1"
		key: #{0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b}
		data: #{4869205468657265}
		--assert (checksum/with data 'sha256 key) = #{
B0344C61D8DB38535CA8AFCEAF0BF12B881DC200C9833DA726E9376C2E32CFF7}

		--assert (checksum/with data 'sha384 key) = #{
AFD03944D84895626B0825F4AB46907F15F9DADBE4101EC682AA034C7CEBC59C
FAEA9EA9076EDE7F4AF152E8B2FA9CB6}

		--assert (checksum/with data 'sha512 key) = #{
87AA7CDEA5EF619D4FF0B4241A1D6CB02379F4E2CE4EC2787AD0B30545E17CDE
DAA833B7D6B8A702038B274EAEA3F4E4BE9D914EEB61F1702E696C203A126854
}

	--test-- "test case 2"
		key: #{4a656665}
		data: #{7768617420646f2079612077616e7420 666f72206e6f7468696e673f}
		--assert (checksum/with data 'sha256 key) = #{
5BDCC146BF60754E6A042426089575C75A003F089D2739839DEC58B964EC3843}

		--assert (checksum/with data 'sha384 key) = #{
AF45D2E376484031617F78D2B58A6B1B9C7EF464F5A01B47E42EC3736322445E
8E2240CA5E69E2C78B3239ECFAB21649}

		--assert (checksum/with data 'sha512 key) = #{
164B7A7BFCF819E2E395FBE73B56E0A387BD64222E831FD610270CD7EA250554
9758BF75C05A994A6D034F65F8F0E6FDCAEAB1A34D4A6B4B636E070A38BCE737
}

===end-group===

===start-group=== "Checksum/hash"
;@@ https://github.com/Oldes/Rebol-issues/issues/1396
	--test-- "test case 1"
	res: true
	--assert repeat i 1024 [
		h: checksum/with to binary! i 'hash 64
		res: all [res h >= 0 h < 64]
	]
===end-group===


===start-group=== "file-checksum"
	--test-- "file-checksum with small file"
	--assert (file-checksum %units/files/pdf-maker-doc.pdf 'md5)    == #{6F782354D64B0B09CF103A9A129E1137}
	--assert (file-checksum %units/files/pdf-maker-doc.pdf 'sha1)   == #{A598B252C045ABF94EE5F034798B384056C57086}
	--assert (file-checksum %units/files/pdf-maker-doc.pdf 'sha256) == #{FA24645FE45C06DEB31DEC0B4478718A3ABE3F8C923A3B720B5564DAA2C9FC0F}
	--test-- "file-checksum with bigger file"
	--assert binary? file-checksum system/options/boot 'md5 ;; not testing result, because the binary changes.

	--test-- "checksum file!"
	;; when file argument is used with the `checksum` native, then above `file-checksum` function is used
	--assert (checksum %units/files/pdf-maker-doc.pdf 'md5)    == #{6F782354D64B0B09CF103A9A129E1137}
	--assert (checksum %units/files/pdf-maker-doc.pdf 'sha1)   == #{A598B252C045ABF94EE5F034798B384056C57086}
	--assert (checksum %units/files/pdf-maker-doc.pdf 'sha256) == #{FA24645FE45C06DEB31DEC0B4478718A3ABE3F8C923A3B720B5564DAA2C9FC0F}

	--test-- "checksum file! with refines"
	;; refines are not supported
	--assert all [error? e: try [checksum/part %units/files/pdf-maker-doc.pdf 'md5 1] e/id = 'bad-refines]
	--assert all [error? e: try [checksum/with %units/files/pdf-maker-doc.pdf 'md5 1] e/id = 'bad-refines]
===end-group===


===start-group=== "Checksum/with rfc4231"
;-- https://datatracker.ietf.org/doc/html/rfc4231
test-cases: [
	[
    Key:  #{0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b}
    Data: "Hi There"
    HMAC-SHA-224: #{896fb1128abbdf196832107cd49df33f
                     47b4b1169912ba4f53684b22}
    HMAC-SHA-256: #{b0344c61d8db38535ca8afceaf0bf12b
                     881dc200c9833da726e9376c2e32cff7}
    HMAC-SHA-384: #{afd03944d84895626b0825f4ab46907f
                     15f9dadbe4101ec682aa034c7cebc59c
                     faea9ea9076ede7f4af152e8b2fa9cb6}
    HMAC-SHA-512: #{87aa7cdea5ef619d4ff0b4241a1d6cb0
                     2379f4e2ce4ec2787ad0b30545e17cde
                     daa833b7d6b8a702038b274eaea3f4e4
                     be9d914eeb61f1702e696c203a126854}
    ][
    Key:  "Jefe"
    Data: "what do ya want for nothing?"
    HMAC-SHA-224: #{a30e01098bc6dbbf45690f3a7e9e6d0f
                   8bbea2a39e6148008fd05e44}
    HMAC-SHA-256: #{5bdcc146bf60754e6a042426089575c7
                   5a003f089d2739839dec58b964ec3843}
    HMAC-SHA-384: #{af45d2e376484031617f78d2b58a6b1b
                   9c7ef464f5a01b47e42ec3736322445e
                   8e2240ca5e69e2c78b3239ecfab21649}
    HMAC-SHA-512: #{164b7a7bfcf819e2e395fbe73b56e0a3
                   87bd64222e831fd610270cd7ea250554
                   9758bf75c05a994a6d034f65f8f0e6fd
                   caeab1a34d4a6b4b636e070a38bce737}
    ][
    Key:  #{aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa}
    Data: #{dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd}
    HMAC-SHA-224: #{7fb3cb3588c6c1f6ffa9694d7d6ad264
                   9365b0c1f65d69d1ec8333ea}
    HMAC-SHA-256: #{773ea91e36800e46854db8ebd09181a7
                   2959098b3ef8c122d9635514ced565fe}
    HMAC-SHA-384: #{88062608d3e6ad8a0aa2ace014c8a86f
                   0aa635d947ac9febe83ef4e55966144b
                   2a5ab39dc13814b94e3ab6e101a34f27}
    HMAC-SHA-512: #{fa73b0089d56a284efb0f0756c890be9
                   b1b5dbdd8ee81a3655f83e33b2279d39
                   bf3e848279a722c806b485a47e67c807
                   b946a337bee8942674278859e13292fb}
    ][
    Key:          #{0102030405060708090a0b0c0d0e0f10
                   111213141516171819} ;                (25 bytes)
    Data:        #{cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd
                   cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd
                   cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd
                   cdcd} ;                              (50 bytes)
 
    HMAC-SHA-224: #{6c11506874013cac6a2abc1bb382627c
                   ec6a90d86efc012de7afec5a}
    HMAC-SHA-256: #{82558a389a443c0ea4cc819899f2083a
                   85f0faa3e578f8077a2e3ff46729665b}
    HMAC-SHA-384: #{3e8a69b7783c25851933ab6290af6ca7
                   7a9981480850009cc5577c6e1f573b4e
                   6801dd23c4a7d679ccf8a386c674cffb}
    HMAC-SHA-512: #{b0ba465637458c6990e5a8c5f61d4af7
                   e576d97ff94b872de76f8050361ee3db
                   a91ca5c11aa25eb4d679275cc5788063
                   a5f19741120c4f2de2adebeb10a298dd}
    ][
    ;; Test with a truncation of output to 128 bits.
    Key:         #{0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c
                   0c0c0c0c} ; (20 bytes)
    Data:        #{546573742057697468205472756e6361 ; ("Test With Trunca")
                   74696f6e}                        ; ("tion")
 
    HMAC-SHA-224: #{0e2aea68a90c8d37c988bcdb9fca6fa8}
    HMAC-SHA-256: #{a3b6167473100ee06e0c796c2955552b}
    HMAC-SHA-384: #{3abf34c3503b2a23a46efc619baef897}
    HMAC-SHA-512: #{415fad6271580a531d4179bc891d87a6}
    ][
    ;; Test with a key larger than 128 bytes (= block-size of SHA-384 and
    ;; SHA-512).
    Key:         #{aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaa} ; (131 bytes)
    Data:        #{54657374205573696e67204c61726765 ; ("Test Using Large")
                   72205468616e20426c6f636b2d53697a ; ("r Than Block-Siz")
                   65204b6579202d2048617368204b6579 ; ("e Key - Hash Key")
                   204669727374}                    ; (" First")
 
    HMAC-SHA-224: #{95e9a0db962095adaebe9b2d6f0dbce2
                    d499f112f2d2b7273fa6870e}
    HMAC-SHA-256: #{60e431591ee0b67f0d8a26aacbf5b77f
                    8e0bc6213728c5140546040f0ee37f54}
    HMAC-SHA-384: #{4ece084485813e9088d2c63a041bc5b4
                    4f9ef1012a2b588f3cd11f05033ac4c6
                    0c2ef6ab4030fe8296248df163f44952}
    HMAC-SHA-512: #{80b24263c7c1a3ebb71493c1dd7be8b4
                    9b46d1f41b4aeec1121b013783f8f352
                    6b56d037e05f2598bd0fd2215d6a1e52
                    95e64f73f63f0aec8b915a985d786598}
    ][
    ;; Test with a key and data that is larger than 128 bytes (= block-size
    ;; of SHA-384 and SHA-512).
 
    Key:         #{aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                   aaaaaa} ;                           (131 bytes)
    Data:        #{54686973206973206120746573742075 ; ("This is a test u")
                   73696e672061206c6172676572207468 ; ("sing a larger th")
                   616e20626c6f636b2d73697a65206b65 ; ("an block-size ke")
                   7920616e642061206c61726765722074 ; ("y and a larger t")
                   68616e20626c6f636b2d73697a652064 ; ("han block-size d")
                   6174612e20546865206b6579206e6565 ; ("ata. The key nee")
                   647320746f2062652068617368656420 ; ("ds to be hashed ")
                   6265666f7265206265696e6720757365 ; ("before being use")
                   642062792074686520484d414320616c ; ("d by the HMAC al")
                   676f726974686d2e}                ; ("gorithm.")
 
    HMAC-SHA-224: #{3a854166ac5d9f023f54d517d0b39dbd
                    946770db9c2b95c9f6f565d1}
    HMAC-SHA-256: #{9b09ffa71b942fcb27635fbcd5b0e944
                    bfdc63644f0713938a7f51535c3a35e2}
    HMAC-SHA-384: #{6617178e941f020d351e2f254e8fd32c
                    602420feb0b8fb9adccebb82461e99c5
                    a678cc31e799176d3860e6110c46523e}
    HMAC-SHA-512: #{e37b6a775dc87dbaa4dfa9f96e5e3ffd
                    debd71f8867289865df5a32d20cdc944
                    b6022cac3c4982b10d5eeb55c3e4de15
                    134676fb6de0446065c97440fa8c6a58}
   ]
]

n: 1
foreach test test-cases [
	--test-- join "Test case " ++ n
	;; Using copy/part, because one test is with a truncation!
	--assert test/HMAC-SHA-224 == copy/part checksum/with test/data 'sha224 test/key length? test/HMAC-SHA-224 
	--assert test/HMAC-SHA-256 == copy/part checksum/with test/data 'sha256 test/key length? test/HMAC-SHA-256 
	--assert test/HMAC-SHA-384 == copy/part checksum/with test/data 'sha384 test/key length? test/HMAC-SHA-384 
	--assert test/HMAC-SHA-512 == copy/part checksum/with test/data 'sha512 test/key length? test/HMAC-SHA-512 
]
===end-group===

===start-group=== "Checksum/with rfc2202"
;-- https://datatracker.ietf.org/doc/html/rfc2202
sha1-tests: [
	[
	key: #{0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b}
	data: "Hi There"
	digest: #{b617318655057264e28bc0b6fb378c8ef146be00}
	][
	key: "Jefe"
	data: "what do ya want for nothing?"
	digest: #{effcdf6ae5eb2fa2d27416d5f184df9c259a7c79}
	][
	key: #{aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa}
	data: #{DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD}
	digest: #{125d7342b9ac11cd91a39af48aa17b4f63f175d3}
	][
	key: #{0102030405060708090a0b0c0d0e0f10111213141516171819}
	data: #{CDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCDCD}
	digest: #{4c9007f4026250c6bc8414f9bf50c86c2d7235da}
	][
	key: #{0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c}
	data: "Test With Truncation"
	digest: #{4c1a03424b55e07fe7f27be1d58bb9324a9a5a04}
	][
	key: #{
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA}
	data: "Test Using Larger Than Block-Size Key - Hash Key First"
	digest: #{aa4ae5e15272d00e95705637ce8a3b55ed402112}
	][
	key: #{
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA}
	data: "Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data"
	digest: #{e8e99d0f45237d786d6bbaa7965c7808bbff1a91}
	][
	key: #{
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA}
	data: "Test Using Larger Than Block-Size Key - Hash Key First"
	digest: #{aa4ae5e15272d00e95705637ce8a3b55ed402112}
	]
]
n: 1
foreach test sha1-tests [
	--test-- join "Test case " ++ n
	--assert test/digest == checksum/with test/data 'sha1 test/key
]
===end-group===


~~~end-file~~~