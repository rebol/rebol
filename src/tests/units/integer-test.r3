Rebol [
	Title:   "Rebol3 integer test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %integer-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "integer"

===start-group=== "shift op!"
	--test-- "shift-op-left"
		--assert 2 << 3 = 16
		--assert 1 <<  0 = 1
		--assert 1 <<  1 = 2
		--assert 1 << 64 = 1
		--assert 1 << 65 = 2
		--assert #{8000000000000000} = to-binary ((to-integer #{4000000000000000}) << 1)
	--test-- "shift-op-right"
		--assert 1024 >> 1 = 512
		--assert 2 >> 0 = 2
		--assert 2 >> 1 = 1
		--assert 2 >> 2 = 0
		--assert 2 >> 3 = 0

	--test-- "shift overflow"
		;@@ https://github.com/rebol/rebol-issues/issues/2067
		--assert -9223372036854775808 = shift/logical 1 63
		--assert error? try [shift 1 63] ; overflow
		--assert 0 = shift 1 -100

===end-group===
	
~~~end-file~~~
