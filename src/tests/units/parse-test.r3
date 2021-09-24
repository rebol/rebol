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

--test-- "changing input"
;@@ https://github.com/Oldes/Rebol-issues/issues/1263
	b: "this"
	--assert parse "test" ["test" :b "this"]
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
--test-- "TO/THRU with tag!"
	--assert parse "<a>" [thru <a>]
	--assert parse "a<a>" [thru [<a>]]
	--assert parse "a<a>" [to <a> 3 skip]
	--assert parse "a<a>" [to [<a>] to end]
	--assert parse "a<a>" [thru [<b> | <a>]]
	--assert all [parse "11<b>xx</b>22" [thru <b> copy x to </b> to end] x = "xx"]

--test-- "TO/THRU datatype"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1282
	--assert parse [1 2 a] [thru word!]
	--assert parse [1 2 a] [to word! 1 skip]

--test-- "TO/THRU end"
	;@@ https://github.com/Oldes/Rebol-issues/issues/295
	--assert all [parse "xyz" [copy f to   end]  f = "xyz"]
	--assert all [parse "xyz" [copy f thru end]  f = "xyz"]
	--assert error? try [parse "xyz" [copy f thru to end]]
	--assert error? try [parse "xyz" [copy f thru thru end]]
===end-group===


===start-group=== "NOT"
--test-- "not not"
;@@ https://github.com/Oldes/Rebol-issues/issues/1246
	--assert parse "1" [not not "1" "1"]
	--assert parse "1" [not [not "1"] "1"]
	--assert not parse "" [not 0 "a"]
	--assert not parse "" [not [0 "a"]]
===end-group===

===start-group=== "AND / AHEAD"
;@@ https://github.com/Oldes/Rebol-issues/issues/2095
--test-- "and"
	--assert parse "a" [and #"a" skip]
	--assert all [parse "abc" [#"a" and #"b" copy x to end]  x = "bc"]
	--assert parse [1] [and integer! skip]
	--assert parse [1 hi] [integer! and word! skip]
	--assert all [parse [hi @bob] ['hi and ref! set x skip] x = @bob]
	--assert not parse "a" [and #"b" skip]
	--assert not parse [1] [and word! skip]
--test-- "ahead"
	--assert parse "a" [ahead #"a" skip]
	--assert all [parse "abc" [#"a" ahead #"b" copy x to end]  x = "bc"]
	--assert parse [1] [ahead integer! skip]
	--assert parse [1 hi] [integer! ahead word! skip]
	--assert all [parse [hi @bob] ['hi ahead ref! set x skip] x = @bob]
	--assert not parse "a" [ahead #"b" skip]
	--assert not parse [1] [ahead word! skip]
--test-- "issue-1238"
;@@ https://github.com/Oldes/Rebol-issues/issues/1238
	--assert not parse "ab" [and "ab" "ac"]
	--assert not parse "ac" [and "ab" "ac"]
	--assert not parse "ab" [ahead "ab" "ac"]
	--assert not parse "ac" [ahead "ab" "ac"]
===end-group===


===start-group=== "THEN"
--test-- "then"
;@@ https://github.com/Oldes/Rebol-issues/issues/1394
	--assert parse "ab" ["a" then "b" | "c"]
	--assert parse "c"  ["a" then "b" | "c"]
===end-group===


===start-group=== "LIMIT"
--test-- "limit"
;@@ https://github.com/Oldes/Rebol-issues/issues/1300
	--assert all [
		error? e: try [limit: ["123"] parse "123" [limit]]
		e/id = 'not-done
	]
===end-group===


===start-group=== "CHANGE"

--test-- "CHANGE string"
;@@ https://github.com/Oldes/Rebol-issues/issues/1245
	--assert parse s: "(1)" [change "(1)" "()"]
	--assert s = "()"

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
;@@ https://github.com/Oldes/Rebol-issues/issues/1279
	--assert parse s: "abc" [some ["a" change copy v r (uppercase v) | skip]]
	--assert v = "B"
	--assert s = "aBc"
	--assert parse s: "abc" [some ["a" change copy v ["b" "c"] (reverse copy v) | skip]]
	--assert v = "bc"
	--assert s = "acb"
	--assert all [parse s: [1] [change set n integer! (n * 10)]  s = [10]]

--test-- "CHANGE block"
	--assert parse b: [1] [change integer! (1 + 1) to end]
	--assert b = [2]

--test-- "CHANGE undefined"
;@@  https://github.com/Oldes/Rebol-issues/issues/1418
	--assert all [
		error? e: try [parse "abc" ["a" change skip undefined-word]]
		e/id = 'no-value
	]

--test-- "CHANGE into"
;@@ https://github.com/Oldes/Rebol-issues/issues/1266
	--assert all [parse s: [[a b]][change  into ['a 'b]  [z p]]  s = [z p]]
	--assert all [parse s: [[a b]][change [into ['a 'b]] [z p]]  s = [z p]]
	--assert all [parse s:  [a b] [change  some word!    [z p]]  s = [z p]]

--test-- "CHANGE lit-word"
;@@ https://github.com/Oldes/Rebol-issues/issues/1265
	--assert all [parse s: [a][change 'a 'z]  s = [z]]

--test-- "CHANGE only"
;@@ https://github.com/Oldes/Rebol-issues/issues/1264
	--assert error? try [parse s: [a b][change only ['a 'b] [z p]]]
	--assert all [parse s: [a b][change ['a 'b] only [z p]]  s = [[z p]]]

===end-group===

===start-group=== "INSERT"

--test-- "INSERT value"
	--assert parse s: "abc" [some ["a" insert "a" | skip]]
	--assert s = "aabc"
	--assert parse s: "abc" [some ["a" insert "X" | "b" insert "YY" | 1 skip]]
	--assert s = "aXbYYc"

--test-- "INSERT expresion"
;@@ https://github.com/Oldes/Rebol-issues/issues/1279
	--assert parse s: "abc" [some ["a" insert (uppercase "x") | skip]]
	--assert s = "aXbc"

--test-- "INSERT block"
	--assert parse b: [1] [insert (1 + 2) insert 2 to end]
	--assert b = [3 2 1]

===end-group===

===start-group=== "REMOVE"
--test-- "remove"
;@@ https://github.com/Oldes/Rebol-issues/issues/2452
	--assert parse v: "yx" [some [remove #"y" | #"x"]]
	--assert v = "x"
	--assert parse "yx" [copy v any [ remove #"y" | #"x" ]]
	--assert v = "x"
	--assert parse v: "yx" [some [change #"y" "" | #"x"]]
	--assert v = "x"
	--assert parse v: "ab" [any [s: 1 skip e: (e: remove/part s e) :e | skip]]
	--assert empty? v
	; and also:
	lr: [s: #L integer! e: (s: remove/part s 2) :s]
	--assert parse v: [#L 1 "a" #L 2 "b"][some [lr | string!]]
	--assert v = ["a" "b"]
	--assert parse v: [#L 1 "a" #L 2 "b"][some [string! | lr]]
	--assert v = ["a" "b"]

--test-- "while .. remove"
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

--test-- "remove copy"
;@@ https://github.com/Oldes/Rebol-issues/issues/1244
	--assert not parse a: "12" [remove copy v skip]
	--assert a = "2"
	--assert v = "1"
	--assert not parse a: "12" [remove [copy v skip]]
	--assert a = "2"
	--assert v = "1"

--test-- "remove into"
;@@ https://github.com/Oldes/Rebol-issues/issues/1266
	--assert all [parse s: [[a b]][remove into ['a 'b]]  s = []]

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

--test-- "issue-367"
;@@ https://github.com/Oldes/Rebol-issues/issues/367
	--assert error? try [parse [1 2 3] [(1 / 0)] false]
	
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


--test-- "issue-967"
;@@ https://github.com/Oldes/Rebol-issues/issues/967
	x: 0
	--assert all [parse "" [some [(x: 1) break]]  x = 1]
	--assert all [parse "" [any  [(x: 2) break]]  x = 2]


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

--test-- "issue-1068"
;@@ https://github.com/Oldes/Rebol-issues/issues/1068
	foreach x [[a b] (a b) a/b 'a/b :a/b a/b:] [
		parse x [y:]
		--assert same? x y
	]

--test-- "issue-1298"
;@@ https://github.com/Oldes/Rebol-issues/issues/1298
	cset: charset [#"^(01)" - #"^(FF)"]
	--assert parse "a" ["a" any cset]
	cset: charset [#"^(00)" - #"^(FE)"]
	--assert parse "a" ["a" any cset]
	cset: charset [#"^(00)" - #"^(FF)"]
	--assert parse "a" ["a" any cset]

--test-- "integer! repeat"
;@@ https://github.com/Oldes/Rebol-issues/issues/1237
	--assert     parse [a] [1 'a]
	--assert not parse [a] [9 'a]
	--assert     parse [a] [1 3 'a]
	--assert not parse [a] [2 3 'a]

===end-group===

~~~end-file~~~
