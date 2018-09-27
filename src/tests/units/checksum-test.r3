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

~~~end-file~~~