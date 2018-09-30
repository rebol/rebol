Rebol [
	Title:   "Rebol checksum test script"
	Author:  "Olds"
	File: 	 %checksum-test.red
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Checksum"

===start-group=== "Checksum with binary key (issue #1910)"
;@@ https://github.com/rebol/rebol-issues/issues/1910
	--test-- "checksum-1"
		--assert  #{800A1BC1B53CAA795F4DF39DC57652209239E1F1}
					 = checksum/key to binary! "Hello world" "mykey"
		--assert  #{800A1BC1B53CAA795F4DF39DC57652209239E1F1}
					 = checksum/key to binary! "Hello world" to binary! "mykey"
===end-group===


===start-group=== "Checksum basic"
	--test-- {checksum ""}
		bin: #{}
		--assert #{da39a3ee5e6b4b0d3255bfef95601890afd80709}
					= checksum/method bin 'sha1
		--assert #{e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855}
					= checksum/method bin 'sha256
		--assert #{38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b}
					= checksum/method bin 'sha384
		--assert #{cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e}
					= checksum/method bin 'sha512
===end-group===


===start-group=== "Checksum port"
	bin: #{0BAD}
	--test-- "checksum-port-md5"
		port: open checksum://
		sum1: checksum/method bin 'md5
		sum2: checksum/method join bin bin 'md5
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
		port: open checksum://md5
		--assert 'md5 = port/spec/method

	--test-- "checksum-port-sha1"
		port: open checksum://sha1
		sum1: checksum/method bin 'sha1
		sum2: checksum/method join bin bin 'sha1
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
		port: open checksum://sha256
		sum1: checksum/method bin 'sha256
		sum2: checksum/method join bin bin 'sha256
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
		port: open checksum://sha384
		sum1: checksum/method bin 'sha384
		sum2: checksum/method join bin bin 'sha384
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
		port: open checksum://sha512
		sum1: checksum/method bin 'sha512
		sum2: checksum/method join bin bin 'sha512
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
		sum1: checksum/method bin port/spec/method
		sum2: checksum/method join bin bin port/spec/method
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

~~~end-file~~~