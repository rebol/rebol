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

===end-group===

~~~end-file~~~