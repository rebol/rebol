Rebol [
	Title:   "Rebol3 crash test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %dh-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Crash tests"

===start-group=== "Crashing issues"

--test-- "DH keys generation"
	;@@ situation fixed in: https://github.com/zsx/r3/commit/cc625bebcb6038b9282876954f929c9d80048d2b

	a: copy ""
	insert/dup a #"a" to integer! #10ffff
	take/part a 3
	take/part a to integer! #f0010

	insert/dup a #"b" 10
	a: 1    ;force a to recycle
	recycle ;@@ <-- it was crashing here
	--assert 1 = a

--test-- "issue-1977"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1977
	a: func [/b] [1]
	--assert error? try [a/b/%] ;- no crash, just error!

--test-- "issue-2190"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2190
	catch/quit [ attempt [ quit ] ]
	--assert error? try [print x] ;- no crash, just error!

===end-group===

~~~end-file~~~