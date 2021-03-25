Rebol [
	Title:   "Rebol parse test script"
	Author:  "Oldes"
	File: 	 %parse-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Parse"

===start-group=== "Basic string based parsing"
--test-- "parse string! using any-string!"
;@@ https://github.com/Oldes/Rebol-wishes/issues/36
	--assert      parse "ab"   ["ab"]
	--assert      parse "abc"  [%abc]
	--assert      parse "a@bc" [a@bc]
	--assert try [parse "abc"  [@abc]]
	--assert try [parse "a:bc" [a:bc]]
	--assert      parse "<a>"  [<a> ]
===end-group===


===start-group=== "TO/THRU"
--test-- "TO/THRU with bitset!"
;@@ https://github.com/Oldes/Rebol-issues/issues/1457
	a: charset "a"
	--assert parse "a" [thru a]
	--assert not parse "a" [thru a skip]
	--assert parse "a" [and thru a skip]
	--assert parse "ba" [to a skip]
	--assert not parse "ba" [to a "ba"]
	--assert parse/case "a" [thru a]
	--assert not parse/case "A" [thru a]
--test-- "TO/THRU with unsuported rule"
;@@ https://github.com/Oldes/Rebol-issues/issues/2129
	--assert all [
		e: try [parse "foo" [thru some "0"]] ;@@ Red supports `some` right after `to`!
		e/id = 'parse-rule
	]
	--assert all [
		e: try [parse "foo" [thru 1.2]]
		e/id = 'parse-rule
	]
===end-group===


===start-group=== "THEN"
--test-- "then"
;@@ https://github.com/Oldes/Rebol-issues/issues/1394
	--assert parse "ab" ["a" then "b" | "c"]
	--assert parse "c"  ["a" then "b" | "c"]
===end-group===


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

--test-- "CHANGE undefined"
;@@  https://github.com/Oldes/Rebol-issues/issues/1418
	--assert all [
		error? e: try [parse "abc" ["a" change skip undefined-word]]
		e/id = 'no-value
	]

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

===start-group=== "REMOVE"
--test-- "remove"
	;--assert parse v: "yx" [some [remove #"y" | #"x"] ]
	;--assert v = "x"
	;--assert parse "yx" [copy v any [ remove #"y" | #"x" ] ]
	;--assert v = "x"
	remove-any-y: [while [remove #"y" | #"x"]] 
	--assert parse v: "" remove-any-y
	--assert parse v: "yx" remove-any-y
	--assert v = "x"
	--assert parse v: "yxxyyx" remove-any-y
	--assert v = "xxx"

--test-- "remove & insert"
;@@ https://github.com/Oldes/Rebol-issues/issues/1251
	--assert parse v: "a" [remove skip insert "xxx"]
	--assert v = "xxx"
	--assert parse v: "a" [[remove skip] insert "xxx"]
	--assert v = "xxx"

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


===start-group=== "Other parse issues"

--test-- "issue-215"
;@@ https://github.com/Oldes/Rebol-issues/issues/215
	--assert ["a" "b" "c"] = parse     "a/b/c" #"/"
	--assert ["a" "b" "c"] = parse     "a/b/c"  "/"
	--assert ["a" "b" "c"] = parse/all "a/b/c" #"/"
	--assert ["a" "b" "c"] = parse/all "a/b/c"  "/"

--test-- "issue-394"
;@@ https://github.com/Oldes/Rebol-issues/issues/394
	--assert parse #{001122} [#{00} #{11} #{22}]

--test-- "issue-529"
;@@ https://github.com/Oldes/Rebol-issues/issues/529
	a: context [b: string!]
	--assert parse ["test"] [a/b]

--test-- "issue-591"
;@@ https://github.com/Oldes/Rebol-issues/issues/591
	--assert all [
		error? e: try [parse " " [0]]
		e/id = 'parse-end
		e/arg1 = 0
	]

--test-- "issue-753"
;@@ https://github.com/Oldes/Rebol-issues/issues/753
	ws: to-bitset rejoin[ tab newline cr sp ]
	abc: charset [ "a" "b" "c" ]
	rls: [ "a" some ws copy b some abc some ws "c" ]
	rla: [ "a"  any ws copy b some abc  any ws "c" ]
	--assert     parse/all "a b c" rls
	--assert     parse/all "a b c" rla
	--assert not parse/all "a b"   rls
	--assert not parse/all "a b"   rla


--test-- "issue-2130"
;@@ https://github.com/Oldes/Rebol-issues/issues/2130
	--assert parse [x][set val pos: word!]
	--assert pos = [x]

--test-- "issue-2138"
;@@ https://github.com/Oldes/Rebol-issues/issues/2138
	--assert not parse "test<a>" [ copy content to [<a>] ]
	--assert content = "test"

--test-- "issue-1938"
;@@ https://github.com/Oldes/Rebol-issues/issues/1938
	--assert parse "a" reduce [charset "a"]
	--assert parse/case "a" reduce [charset "a"]
	--assert parse "A" reduce [charset "a"]
	--assert not parse/case "A" reduce [charset "a"]

--test-- "issue-2141"
;@@ https://github.com/Oldes/Rebol-issues/issues/2141
	x: charset "x"
	y: charset "y"
	--assert parse "x" [thru [x | y]]
	--assert parse "y" [thru [x | y]]
	--assert parse "zx" [thru [x | y]]
	--assert parse "zy" [thru [x | y]]
	--assert parse "xz" [thru [x | y] #"z"]
	--assert parse "yz" [thru [x | y] #"z"]
	--assert parse "xy" [some thru [x | y]]

--test-- "issue-206"
;@@ https://github.com/Oldes/Rebol-issues/issues/206
	any-char: complement charset ""
	--assert parse/all "^(80)" [any-char]

--test-- "issue-1895"
;@@ https://github.com/Oldes/Rebol-issues/issues/1895
	--assert not parse #{f0} reduce [charset [#{d0}]]

--test-- "issue-297"
;@@ https://github.com/Oldes/Rebol-issues/issues/297
	a: none
	--assert all [
		parse "" [some [(a: true)]] ;- no infinite loop as in R2!
		a
	]
--test-- "set/copy set-word"
;@@ https://github.com/Oldes/Rebol-issues/issues/2023
	a: none
	--assert all [
		true? parse [42] [set a: integer!]
		a = 42
	]
	--assert all [
		true? parse [42] [copy a: integer!]
		a = [42]
	]
--test-- "issue-2130"
;@@ https://github.com/Oldes/Rebol-issues/issues/2130
	--assert all [
		parse [x][set val pos: word!]
		pos = [x]
	]
	--assert all [
		not parse ser: "foo" [copy val pos: skip]
		val = "f"
		pos = ser
	]
	--assert all [
		not parse ser: "foo" [copy val: pos: skip]
		val = "f"
		pos = ser
	]
--test-- "get-word use"
;@@ https://github.com/Oldes/Rebol-issues/issues/2269
	s: copy "ab"
	--assert all [
		parse s [p: to end :p insert "x" to end]
		s = "xab"
		p = s
	]
	s: copy "ab"
	--assert all [
		parse s [p: 1 skip :p insert "x" to end]
		s = "xab"
		p = s
	]
	s: copy "ab"
	--assert all [
		parse s [1 skip p: 1 skip :p insert "x" to end]
		s = "axb"
		p = "xb"
	]
	s: copy "abcd"
	--assert all [
		error? e: try [parse s [x: "ab" thru :s "abcd"]]
		e/id = 'parse-rule
	]
	--assert parse s [x: "ab" :s thru "abcd"]
	s: copy "abcd" parse s ["ab" p: "c" :p copy x to end]
	--assert all [p = "cd" x = "cd"]
	s: copy "abcd" parse s ["ab" p: "c" :p set x to end]
	--assert all [p = "cd" x = #"c"] 
	--assert all [
		; get-word used in middle of the rule
		error? e: try [parse "abcd" [x: "ab" copy y :s thru "abcd"]]
		e/id = 'parse-rule
		e/arg1 = quote :s
	]

--test-- "issue-1253"
;@@  https://github.com/Oldes/Rebol-issues/issues/1253
	--assert all [
		parse [base: [specs]] [set-word! set c opt word! block!]
		none? c
	]

;@@ not using this tests because it allocates so many values,
;@@ that I cannot see in results if there is any new problem!
;------------------------------------------------------------
;@@ --test-- "issue-1480"
;@@ ;@@ https://github.com/Oldes/Rebol-issues/issues/1480
;@@ 	c: make string! 255
;@@ 	--assert 256 = length? for i 0 255 1 [append c to-char i]
;@@ 	--assert 16646400 = length? data: to-string array/initial 255 * 255 random c 
;@@ 	--assert (length? parse data "^/") = (length? parse data "^/")

--test-- "issue-1614"
;@@ https://github.com/Oldes/Rebol-issues/issues/1614
	--assert not parse "a b c" ["a" "b" "c"]

===end-group===

~~~end-file~~~
