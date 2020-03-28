Rebol [
	Title:   "Rebol parse test script"
	Author:  "Oldes"
	File: 	 %parse-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Parse"

===start-group=== "CHANGE"

--test-- "CHANGE rule value (same size)"
	r: ["b"]
	--assert parse s: "abc" [some ["a" change r "X" | skip]]
	--assert s = "aXc"

	--assert parse s: "abc" [some ["a" change copy v r "X" | skip]]
	--assert v = "b"
	--assert s = "aXc"

--test-- "CHANGE rule value (different size)"
	r: ["b"]
	--assert parse s: "abc" [some ["a" change r "XX" | skip]]
	--assert s = "aXXc"

	--assert parse s: "abc" [some ["a" change copy v r "XX" | skip]]
	--assert v = "b"
	--assert s = "aXXc"

--test-- "CHANGE rule (expression)"
	--assert parse s: "abc" [some ["a" change copy v r (uppercase v) | skip]]
	--assert v = "B"
	--assert s = "aBc"
	--assert parse s: "abc" [some ["a" change copy v ["b" "c"] (reverse copy v) | skip]]
	--assert v = "bc"
	--assert s = "acb"

--test-- "CHANGE block"
	--assert parse b: [1] [change integer! (1 + 1) to end]
	--assert b = [2]

===end-group===

===start-group=== "INSERT"

--test-- "INSERT value"
	--assert parse s: "abc" [some ["a" insert "a" | skip]]
	--assert s = "aabc"
	--assert parse s: "abc" [some ["a" insert "X" | "b" insert "YY" | 1 skip]]
	--assert s = "aXbYYc"

--test-- "INSERT expresion"
	--assert parse s: "abc" [some ["a" insert (uppercase "x") | skip]]
	--assert s = "aXbc"

--test-- "INSERT block"
	--assert parse b: [1] [insert (1 + 2) insert 2 to end]
	--assert b = [3 2 1]

===end-group===

===start-group=== "Modifiers on protected series"
;@@ https://github.com/Oldes/Rebol-issues/issues/2290
	s: protect "aaa"
	b: protect [1 2]
	--test-- "INSERT"
		--assert error? err: try [parse s [insert "a" to end]]
		--assert err/id = 'protected
		--assert error? err: try [parse b [insert "a" to end]]
		--assert err/id = 'protected

	--test-- "REMOVE"
		--assert error? err: try [parse s [remove "a" to end]]
		--assert err/id = 'protected
		--assert error? err: try [parse b [remove integer! to end]]
		--assert err/id = 'protected

	--test-- "CHANGE"
		--assert error? err: try [parse s [change "a" "b" to end]]
		--assert err/id = 'protected
		--assert error? err: try [parse b [change integer! "b" to end]]
		--assert err/id = 'protected

===end-group===

===start-group=== "DO"

--test-- "issue-2083"
;@@ https://github.com/Oldes/Rebol-issues/issues/2083
;@@ https://github.com/Oldes/Rebol-issues/issues/2085
	--assert error? err: try [parse [1 + 1] [set result do integer!]]
	--assert err/id = 'parse-rule

===end-group===


~~~end-file~~~
