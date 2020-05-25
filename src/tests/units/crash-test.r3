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

--test-- "2x mold system"
	;@@ https://github.com/Oldes/Rebol-issues/issues/14
	;@@ https://github.com/Oldes/Rebol-issues/issues/73
	--assert string? mold system
	--assert integer? length? loop 2 [mold system]

--test-- "self in object!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/595
	--assert error? try [o: make object! [self: 0]]
	--assert error? try [o: make object! [] extend o 'self 0]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1050
	--assert error? try [o: make object! [] append o 'self]

--test-- "issue-143"
	;@@ https://github.com/Oldes/Rebol-issues/issues/143
	d: system/options/decimal-digits
	system/options/decimal-digits: 100
	--assert not error? try [mold 1.7976931348623157E+308] ;- no crash!
	system/options/decimal-digits: d

--test-- "issue-599"
	;@@ https://github.com/Oldes/Rebol-issues/issues/599
	--assert error? try [action []] ;- no crash!
	--assert error? try [make action! []]

--test-- "issue-170"
	;@@ https://github.com/Oldes/Rebol-issues/issues/170
	--assert error? try [foreach :a [1 2 3 4] [print a]] ;- no crash!
	;@@ https://github.com/Oldes/Rebol-issues/issues/179
	a: [x] b: "" foreach :a [1 2 3][append b x]
	--assert "123" = b

--test-- "issue-188"
	;@@ https://github.com/Oldes/Rebol-issues/issues/188
	--assert try [c: closure [n][if n > 0 [c n - 1]] c 306 true] ;- no crash

--test-- "issue-198"
	;@@ https://github.com/Oldes/Rebol-issues/issues/198
	--assert #00000000 = repeat i 1000 [to-hex 0.0.0.0] ;- no crash

--test-- "issue-229"
	;@@ https://github.com/Oldes/Rebol-issues/issues/229
	f1: func [][exit]
;	f2: func [][break] ; this one is impossible to catch by `try` so hard to test
	f3: func [][return 0]
	--assert not error? try [trace on f1 trace off] ;- no crash
;	--assert not error? try [trace on f2 trace off] ;- no crash
	--assert not error? try [trace on f3 trace off] ;- no crash
	trace off

--test-- "issue-231"
	;@@ https://github.com/Oldes/Rebol-issues/issues/231
	clear head s: tail "s"
	--assert {""} = mold s ;- no crash

--test-- "issue-261"
	;@@ https://github.com/Oldes/Rebol-issues/issues/261
	--assert percent? p: to percent! to decimal! #{7FEFFFFFFFFFFFFF} ;- no crash
	--assert #{7FEFFFFFFFFFFFFF} = to binary! to decimal! p

--test-- "issue-2417"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2417
	--assert block? protect/words [aaaa]
	--assert block? protect/words/deep [bbbb]         ;- no crash
	cccc: 1
	dddd: 2
	--assert block? protect/words [cccc]
	--assert block? protect/words/deep [dddd] ;- no crash

--test-- "issue-371"
	;@@ https://github.com/Oldes/Rebol-issues/issues/371
	--assert module? module [] ['self]        ;- no crash
	--assert module? module [] [test: [self]] ;- no crash

--test-- "issue-1934"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1934
	--assert error? try [do reduce [1 :+ 2]] ;- no crash

--test-- "issue-1865"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1865
	--assert object? resolve/extend/only context [] context [a1: a2: a3: a4: a5: a6: none] [a] ;- no crash!
	
--test-- "issue-2017"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2017
	--assert object? resolve/extend/only context [] context [a: none] [a] ;- no crash!

===end-group===

~~~end-file~~~