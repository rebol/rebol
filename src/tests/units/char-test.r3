Rebol [
	Title:   "Rebol3 char test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %char-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "char"

===start-group=== "char column width"
	--test-- "wide chars"
		s: "a⚡b"
		--assert 2 = s/2/width
===end-group===

===start-group=== "++ & --"
	--test-- "++ and -- char!"
		a: #"a"
		--assert #"a" == ++ a
		--assert #"b" == -- a
		--assert #"a" == a

===end-group===

~~~end-file~~~