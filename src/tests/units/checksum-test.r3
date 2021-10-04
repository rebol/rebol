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
		--assert -1965353716 = checksum "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" 'adler32
		--assert -1749675927 = checksum "12345678901234567890123456789012345678901234567890123456789012345678901234567890" 'adler32


===end-group===


===start-group=== "Checksum port"
	bin: #{0BAD}
	--test-- "checksum-port-md5"
		port: open checksum://
		sum1: checksum bin 'md5
		sum2: checksum join bin bin 'md5
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

	--test-- "checksum-port-sha1"
		port: open checksum:sha1
		sum1: checksum bin 'sha1
		sum2: checksum join bin bin 'sha1
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

	--test-- "checksum-port-sha256"
		port: open checksum:sha256
		sum1: checksum bin 'sha256
		sum2: checksum join bin bin 'sha256
		--assert #{183559C9230A3361110FF397037E53E998B6166002BF0FC0603C8939CC89539A} = sum1
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
		sum2: checksum join bin bin 'sha384
		--assert #{
B0C9ADA83C89485563049E5CF212911F334A788D47C97CC9A1D952C9E9EB8B5D
40FC4DAE76AF7024712A5BFC7DFA7BF4} = sum1
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
		sum2: checksum join bin bin 'sha512
		--assert #{
D2079D59D6984814DAC71CDEB38097DB52F77810391FD7B6F92FFBD64EA93DF8
7783EF1E4FEF4ABA834FF3C186A17B2E8DF7B08AF35A96E3802D280AB35BFE1B} = sum1
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

~~~end-file~~~