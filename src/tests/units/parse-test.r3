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

--test-- "TO/THRU with integer!"
;@@ https://github.com/Oldes/Rebol-issues/issues/1964
	--assert all [
		parse "abcd" [to 1 "abcd"]
		parse "abcd" [to 2  "bcd"]
		parse "abcd" [to 3   "cd"]
		parse "abcd" [to 4    "d"]
	]
	--assert all [
		parse "abcd" [thru 0 "abcd"]
		parse "abcd" [thru 1  "bcd"]
		parse "abcd" [thru 2   "cd"]
		parse "abcd" [thru 3    "d"]
		parse "abcd" [thru 4    end]
	]
	--assert parse "abcd" ["ab" to 1 "abcd"]

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


===start-group=== "REJECT"
--test-- "reject"
;@@ https://github.com/Oldes/Rebol-issues/issues/2394
	--assert not parse "aa" [some [#"a"] reject]
	--assert     parse "aabb" [opt  [#"a" reject] to end]
	--assert not parse "aabb" [some [#"a" reject] to end]
	--assert not parse "aabb" [some  #"a" reject  to end]
	--assert not parse "aabb" [[#"a" reject | "aabb"]]
	--assert     parse "aabb" [[#"a" reject] | "aabb"]
===end-group===


===start-group=== "RETURN"
;@@ https://github.com/Oldes/Rebol-issues/issues/2238
--test-- "parse return"
	--assert [1 2]  = parse ["a" 1 2][string! return to end]
	--assert "done" = parse ["a" 1 2][string! return ("done")]
	--assert "positive" = parse [ 2][set n integer! if (n > 0) return ("positive")]
	--assert          not parse [-2][set n integer! if (n > 0) return ("positive")]

===end-group===


===start-group=== "COLLECT/KEEP"
;@@ https://github.com/Oldes/Rebol-issues/issues/2471
--test-- "collect/keep block!"
	--assert     []  = parse []     [collect []]
	--assert     []  = parse [1]    [collect []]
	--assert     [1] = parse [1]    [collect [keep skip]]
	--assert     [1] = parse [1 2]  [collect  keep integer! ]
	--assert     [1] = parse [1 2]  [collect [keep integer!]]
	--assert     [1] = parse [1 2]  [collect [[keep integer!]]]
	--assert     [1] = parse [1]    [collect any [keep any integer! | skip]]
	--assert     [1] = parse [1 %a] [collect any [keep any integer! | skip]]
	--assert [[1 1]] = parse [1 1]  [collect any [keep any integer! | skip]]
	--assert [[1 1]] = parse [1 1 %a] [collect any [keep any integer! | skip]]
	--assert ["a"]   = parse [1 "a"][collect any [integer! | keep string!]]
	--assert [1 2 3] = parse [1 2 3][collect  some [keep integer!]]
	--assert [1 2 3] = parse [1 2 3][collect [some [keep integer!]]]

	--assert [1]     = parse [    1][collect [opt [keep "A"] keep integer!]]
	--assert ["A" 1] = parse ["A" 1][collect [opt [keep "A"] keep integer!]]

	--assert [1 [2 3]] = parse [1 2 3]   [collect [keep integer! keep 2 integer!]]
	--assert [[1 2] 3] = parse [1 2 3]   [collect [keep 2 integer! keep integer!]]
	--assert [[b b b]] = parse [a b b b] [collect [skip keep some 'b]]
	--assert [1 [2 2] 3] = parse [1 "a" 2 2 "b" 3] [collect any [keep some integer! | skip]]

--test-- "block collect conditional"
	--assert [2] = parse [1 2 3] [collect [some [keep [set v integer! if (even? v)] | skip]]]

--test-- "block collect copy"
	--assert all [ [[1] [2] [3]] = parse [1 2 3][collect some [keep copy _ integer!]] _ = [3]]

--test-- "block collect keep pick"
	--assert [[1 2]] = parse [1 2][collect some [keep 2 integer!]]
	--assert [ 1 2 ] = parse [1 2][collect some [keep pick 2 integer!]]

--test-- "block collect nested"
	--assert [[[]]] = parse [][collect [collect [collect []]]]
	--assert [[     ]] = parse [   ][collect [collect [keep 2 skip]]]
	--assert [[[1 2]]] = parse [1 2][collect [collect [keep 2 skip]]]
	--assert [[ 1 2 ]] = parse [1 2][collect [collect [keep pick 2 skip]]]
	--assert [[ 1 2 ]] = parse [1 2][collect [collect [keep integer! keep integer!]]]
	--assert all [x: 0  [[] 1] = parse [1 2][collect [collect [] (x: x + 1) keep (x)]] ]
	--assert all [x: 0  [[] 1] = parse [1 2][collect some [collect [] (x: x + 1) keep (x)]] ]
	--assert all [x: 0  [[] 1 [] 2] = parse [1 2][collect 2 [collect [] (x: x + 1) keep (x)]] ]
	
--test-- "block collect nested (known issues)"
	;; following tests produces empty block at tail :-/
	--assert [[1] [2]] = parse [1 2][collect some [collect keep integer!]]
	--assert [[1] a [2] a] = parse [1 2][collect some [collect keep integer! keep ('a)]]

--test-- "block collect bizzar"
	--assert [[1 2] [3]] = parse [1 2 3] [collect [keep 2 integer!] collect [keep integer!]]
	--assert [1 [[2 3]]] = parse [1 2 3] [collect [keep integer!] collect [keep 2 integer!]]
	--assert [[1]] = parse [1] [opt [collect string!] collect keep integer!]

--test-- "block collect keep paren"
	--assert [3] = parse [1][collect [integer! keep (1 + 2)]]
	--assert [3 "A"] = parse [1][collect [integer! keep (1 + 2) keep ("A")]]

--test-- "block collect set"
	a: none --assert all [#(true)  = parse [ ]   [collect set a []] a = []]
	a: none --assert all [#(true)  = parse [1]   [collect set a [keep skip]] a = [1]]
	a: none --assert all [#(false) = parse [1 2] [collect set a [keep skip]] a = [1]]
	a: none --assert all [
		[] = parse [1] [collect [collect set a keep skip]]
		a = [1]
	]
	a: none --assert all [
		[] = parse [1] [collect [collect set a [keep skip]]]
		a = [1]
	]
	a: none --assert all [
		[[1]] = parse [1] [collect [collect [set a keep skip]]] ;set is not related to collect!
		a = 1
	]
	a: none --assert all [
		#(true) = parse [1] [collect set a [collect set a keep skip]]
		a = [1]
	]
	a: b: none --assert all [
		#(true) = parse [1] [collect set a [collect set b keep skip]]
		a = []
		b = [1]
	]

--test--  "block collect into"
	;;  Inserts collected values into a series referred by a word, resets series' index to the head.
	--assert all [a: [ ] parse [ ] [collect into a []] a = []]
	--assert all [a: [ ] parse [1] [collect into a [keep skip]] [1  ] = a [1] = head a]
	--assert all [a: [x] parse [1] [collect into a [keep skip]] [1 x] = a]
	--assert all [a: tail [x] parse [1] [collect into a [keep skip]] [1] = a  [x 1] = head a]
	--assert all [a: tail [x] parse [1 2] [collect into a [keep 2 skip]] [[1 2]] = a  [x [1 2]] = head a]
	--assert all [a: tail [x] parse [1 2] [collect into a [keep pick 2 skip]] [1 2] = a  [x 1 2] = head a]
	--assert all [
		list: next [1 2 3]
		parse [a 4 b 5 c] [collect into list [some [keep word! | skip]]]
		list = [a b c 2 3]
		[1 a b c 2 3] = head list
	]

--test-- "block collect after"
	;; Inserts collected values into a series referred by a word, moves series' index past the insertion.
	--assert all [a: [] parse [1] [collect after a [keep skip]] [] = a [1] = head a]
	--assert all [a: [x] parse [1 2] [collect after a some [keep skip]] [x] = a [1 2 x] = head a]
	--assert all [
		list: next [1 2 3]
		parse [a 4 b 5 c] [collect after list [some [keep word! | skip]]]
		list = [2 3]
		[1 a b c 2 3] = head list
	]

--test-- "string collect/keep"
	--assert [] = parse "" [collect []]
	--assert [] = parse "a" [collect []]
	--assert [#"a"] = parse "a" [collect [keep skip]]
	--assert [#"a" #"b" #"c"] = parse "abc" [collect any [keep skip]]
	--assert ["ab" #"c"] = parse "abc" [collect any [keep 1 2 skip]]
	--assert [%ab  #"c"] = parse %abc  [collect any [keep 1 2 skip]]
	--assert [@ab  #"c"] = parse @abc  [collect any [keep 1 2 skip]]
	--assert [#{0102} 3] = parse #{010203} [collect any [keep 1 2 skip]]
	--assert ["aa" "bbb"] = parse "aabbb" [collect [keep some "a" keep some #"b"]]

	digit: :system/catalog/bitsets/numeric
	--assert [#"1" #"2" #"3"] = parse "123" [collect [some [keep digit]]]
	--assert [#"2"] = parse "123" [collect [some [keep [copy v digit if (even? to integer! v)] | skip]]]
	--assert [1 2 3] = parse "123" [collect [some [copy d digit keep (to integer! d)]]]
	alpha: :system/catalog/bitsets/alpha
	--assert ["abc" "def"] = parse "abc|def" [collect [any [keep some alpha | skip]]]

--test-- "string collect copy"
	--assert all [ ["a" "b"] = parse "ab" [collect some [keep copy _ skip]] _ = "b"]
	--assert all [ [@a  @b ] = parse @ab  [collect some [keep copy _ skip]] _ = @b ]

--test-- "binary collect copy"
	--assert all [ [#{01} #{02}] = parse #{0102} [collect some [keep copy _ skip]] _ = #{02}]

--test-- "string collect keep pick"
	--assert ["ab"] = parse "ab" [collect [keep 2 skip]]
	--assert [#"a" #"b"] = parse "ab" [collect [keep pick 2 skip]]
	--assert [#"a" #"b"] = parse @ab  [collect [keep pick 2 skip]]

--test-- "binary collect keep pick"
	--assert [#{0102}] = parse #{0102} [collect [keep 2 skip]]
	--assert [1 2] = parse #{0102} [collect [keep pick 2 skip]]

--test-- "string collect set"
	a: none --assert all [#(true)  = parse ""   [collect set a []] a = []]
	a: none --assert all [#(true)  = parse "1"  [collect set a [keep skip]] a = [#"1"]]
	a: none --assert all [#(false) = parse "12" [collect set a [keep skip]] a = [#"1"]]
	a: none --assert all [
		[] = parse "1" [collect [collect set a keep skip]]
		a = [#"1"]
	]
	a: none --assert all [
		[] = parse "1" [collect [collect set a [keep skip]]]
		a = [#"1"]
	]
	a: none --assert all [
		[[#"1"]] = parse "1" [collect [collect [set a keep skip]]] ;set is not related to collect!
		a = #"1"
	]
	a: none --assert all [
		#(true) = parse "1" [collect set a [collect set a keep skip]]
		a = [#"1"]
	]
	a: b: none --assert all [
		#(true) = parse "1" [collect set a [collect set b keep skip]]
		a = []
		b = [#"1"]
	]
--test-- "string collect into"
	;;  Inserts collected values into a series referred by a word, resets series' index to the head.
	--assert all [a: "" parse "" [collect into a []] a = ""]
	--assert all [a: "" parse "1" [collect into a [keep skip]] "1" = a "1" = head a]
	--assert all [a: "" parse "š" [collect into a [keep skip]] "š" = a "š" = head a]
	--assert all [a: [] parse "1" [collect into a [keep skip]] [#"1"] = a [#"1"] = head a]
	--assert all [a: [] parse "š" [collect into a [keep skip]] [#"š"] = a [#"š"] = head a]
	--assert all [a: quote () parse #{01} [collect into a [keep skip]] a = quote (1)]
	--assert all [
		list: next [1 2 3]
		parse [a 4 b 5 c] [collect into list [some [keep word! | skip]]]
		list = [a b c 2 3]
		[1 a b c 2 3] = head list
	]
	;; Inserting unicode to ascii (internal target widening)
	--assert all [a: "" parse "š"  [collect into a keep skip] a = "š"]
	--assert all [a: "" parse "šo" [collect into a keep to end] a = "šo"]

--test-- "string collect after"
	;; Inserts collected values into a series referred by a word, moves series' index past the insertion.
	--assert all [a: "" parse "1" [collect after a [keep skip]]  "" = a    "1"  = head a]
	--assert all [a: [] parse "1" [collect after a [keep skip]]  [] = a  [#"1"] = head a]
	--assert all [
		a: next "11"
		b: next "22"
		[x] = parse "ab" [collect [keep ('x) collect into a keep skip collect after b keep to end]]
		a = "a1"
		b =  "2"
		"1a1" = head a
		"2b2" = head b
	]

--test-- "string collect into/after compatibility test"
	;; any-string! to any-string!
	--assert all [a:  "x" parse "1" [collect into a keep skip] a =  "1x"]
	--assert all [a: %"x" parse "1" [collect into a keep skip] a = %"1x"]
	--assert all [a:  <x> parse "1" [collect into a keep skip] a =  <1x>]
	--assert all [a:  @x  parse "1" [collect into a keep skip] a =  @1x ] ;ref
	--assert all [a: x@x  parse "1" [collect into a keep skip] a =  1x@x] ;email
	--assert all [a:  "x" parse <1> [collect into a keep skip] a =  "1x"]
	;; binary to binary is allowed..
	--assert all [a: #{}  parse #{01} [collect into a keep skip] a = #{01}]
	;; these will throw an error:
	--assert all [error? e: try [a:  1  parse "1"   [collect into a keep skip]] e/id = 'parse-into-type]
	--assert all [error? e: try [a: #{} parse "1"   [collect into a keep skip]] e/id = 'parse-into-type]
	--assert all [error? e: try [a: "1" parse #{01} [collect into a keep skip]] e/id = 'parse-into-type]
	--assert all [error? e: try [a: "1" parse []    [collect into a keep skip]] e/id = 'parse-into-type]


--test-- "string collect complex"
	; Taken from: https://www.red-lang.org/2013/11/041-introducing-parse.html
	html: {
		<html>
			<head><title>Test</title></head>
			<body><div><u>Hello</u> <b>World</b></div></body>
		</html>
	}
	ws: :system/catalog/bitsets/whitespace
	res: parse html tags: [
		collect [any [
			ws
			| "</" thru ">" break
			| "<" copy name to ">" skip keep (load name) opt tags
			| keep to "<"
		]]
	]
	--assert res = [html [head [title ["Test"]] body [div [u ["Hello"] b ["World"]]]]]

--test-- "string collect with fail"
	--assert all [
		alpha: system/catalog/bitsets/alpha
		numer: system/catalog/bitsets/numeric
		#(true) = parse "11ab2c33" [
			collect set res [
				  keep (quote alpha:  ) collect [some [keep some alpha | skip] fail]
				| keep (quote numeric:) collect [some [keep some numer | skip]]
			]
		]
		res = [alpha: ["ab" #"c"] numeric: ["11" #"2" "33"]]
	]

--test-- "collect/keep expression"
	--assert [1] = parse [][collect keep (1)]
	--assert [1] = parse [][collect keep pick (1)]
	--assert [[1]] = parse [][collect keep ([1])]
	--assert [[1]] = parse [][collect keep pick ([1])] ;@@ no difference?

--test-- "collect/keep set expression"
	--assert [1] = parse [][collect keep (1)]

--test-- "collect/keep errors"
	--assert all [error? e: try [parse [1] [keep skip]   ] e/id = 'parse-no-collect]
	--assert all [error? e: try [parse [1] [keep]        ] e/id = 'parse-end]
	--assert all [error? e: try [parse [1] [collect keep]] e/id = 'parse-end]
	--assert all [error? e: try [parse [1] [collect]     ] e/id = 'parse-end]
	--assert all [error? e: try [parse [1] [collect integer! keep (1)]] e/id = 'parse-no-collect]
	--assert all [error? e: try [collect [parse "abc" [any [keep 1 2 skip]]] e/id = 'parse-no-collect]] ;<--- requires parse's collect!

===end-group===


===start-group=== "CASE / NO-CASE"
;@@ https://github.com/Oldes/Rebol-issues/issues/1898
--test-- "case/no-case 1"
	--assert parse      "aAaBbBcccDDD" [no-case "AAA" case "BbB" "ccc" no-case "ddd"]
	--assert parse/case "aAaBbBcccDDD" [no-case "AAA" case "BbB" "ccc" no-case "ddd"]
--test-- "case/no-case 2"
	--assert all [parse b: ["aAa"][case ["AAA" | change "aAa" "AAA"]] b == ["AAA"]]
	--assert all [parse b: ["aAa"][case "AAA" | no-case change "aaa" "AAA"] b == ["AAA"]]
--test-- "case/no-case 3"
	--assert parse "aaaAB" [case thru #"A" #"B"]
	--assert parse "aaaAB" [case thru #"A" no-case #"b"]
--test-- "case/no-case 4"
	--assert     parse to binary! "aaaAB" [thru #"A" #"B"]
	--assert     parse to binary! "aaaAB" [thru #"A" no-case #"b"]
	--assert not parse to binary! "aaaAB" [thru #"A" #"b"]

;@@ https://github.com/Oldes/Rebol-issues/issues/2552
--test-- "parse/case word"
	--assert     parse [a]['a]
	--assert     parse [a]['A]
	--assert     parse/case [a]['a]
	--assert not parse/case [a]['A]
--test-- "parse/case quoted word"
	--assert     parse [a][quote a]
	--assert     parse [a][quote A]
	--assert     parse/case [a][quote a]
	--assert not parse/case [a][quote A]
--test-- "parse/case path"
	--assert     parse [p/a]['p/a]
	--assert     parse [p/a]['p/A]
	--assert     parse/case [p/a]['p/a]
	--assert not parse/case [p/a]['p/A]
--test-- "parse/case quoted word"
	--assert     parse [p/a][quote p/a]
	--assert     parse [p/a][quote p/A]
	--assert     parse/case [p/a][quote p/a]
	--assert not parse/case [p/a][quote p/A]

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

--test-- "remove using series' index"
;@@ https://github.com/Oldes/Rebol-issues/issues/2541
	--assert all [
		parse s: "abcd" [skip p: 2 skip remove p skip]
		s == "ad"
	]
	--assert all  [
		parse s: "abcd" [skip (p: tail s) remove p]
		s == "a"
	]
	--assert all [
		parse s: [a b c d] [skip p: 2 skip remove p skip]
		s == [a d]
	]
	--assert all  [
		parse s: [a b c d] [skip (p: tail s) remove p]
		s == [a]
	]
	;; here the word `p` is used, but not as an index, but as a value
	--assert all  [
		p: "bc" parse s: "abcd" [skip remove p skip]
		s == "ad"
	]
	p: "xx"
	--assert not parse s: "abcd" [skip remove p skip]

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

===start-group=== "Parse complex tests"
--test-- "brain-fuck"
	; Taken from: https://www.red-lang.org/2013/11/041-introducing-parse.html
	bf: function [prog [string!]][
		size: 3000
		cells:  make string! size
		output: make string! 20
		append/dup cells null size
		all [
			parse prog [
				some [
					  ">" (cells: next cells)
					| "<" (cells: back cells)
					| "+" (cells/1: cells/1 + 1)
					| "-" (cells/1: cells/1 - 1)
					| "." (append output cells/1)
					| "," (cells/1: first input "")
					| "[" [if (cells/1 = null) thru "]" | none]
					| "]" [
					   pos: if (cells/1 <> null)
					   (pos: find/reverse pos #"[") :pos
					   | none
					  ]
					| skip
				]
			]
			probe length? cells
			head output
		]
	]
	--assert all [
		not error? res: try [
			bf {
				++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.
				>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
			}
		]
		res = "Hello World!^/"
	]
===end-group===


===start-group=== "Other parse issues"

--test-- "issue-215"
;@@ https://github.com/Oldes/Rebol-issues/issues/215
;-- This functionality was removed!
;	--assert ["a" "b" "c"] = parse     "a/b/c" #"/"
;	--assert ["a" "b" "c"] = parse     "a/b/c"  "/"
;	--assert ["a" "b" "c"] = parse/all "a/b/c" #"/"
;	--assert ["a" "b" "c"] = parse/all "a/b/c"  "/"
;@@ https://github.com/Oldes/Rebol-issues/issues/2046
	--assert ["a" "b" "c"] = split     "a/b/c" #"/"
	--assert ["a" "b" "c"] = split     "a/b/c"  "/"
	--assert all [error? e: try [parse "a b c" none] e/id = 'expect-arg]
	--assert all [error? e: try [parse "a/b/c" #"/"] e/id = 'expect-arg]

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
	--assert     parse "a b c" rls
	--assert     parse "a b c" rla
	--assert not parse "a b"   rls
	--assert not parse "a b"   rla


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
	--assert parse "^(80)" [any-char]

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

if not error? try [str: to string! #{A032}][
	--test-- "parse with uni-replacement-char"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2476
	;- expected no crashes...
	--assert parse str [str]
	--assert parse str [thru ["2"]]
	--assert parse str [to [ "2"] 1 skip]
	--assert parse str [to [#"2"] 1 skip]
	--assert not parse "a" [to [str]]
	--assert not parse str [#"a"]
	c: first str
	--assert not parse "a" [c]
	--assert not parse [#"a"][c]
	; and also not parse related:
	--assert not equal? [#"a"] reduce [c]
]

--test-- "invalid rule error message"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1273
	--assert all [error? e: try [parse "abc" [huh "b"]] e/id = 'parse-rule e/arg1 = 'huh]
	;@@ https://github.com/Oldes/Rebol-issues/issues/2364
	--assert all [
		data: "aaabbb"
		pos: head data
		error? e: try [parse data [some "a" copy var :pos]]
		e/id = 'parse-rule
		e/arg1 = quote :pos
	]
--test-- "evaluation in optional rule"
	;@@ https://gitter.im/red/bugs?at=638e27b34cb5585f9666500d
	x: false 
	--assert all [
		not parse [1] [opt (x: true)]
		x
	]

--test-- "empty string rule"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1880
	--assert not parse "ab" ["" to end] ;-- not like in Red!

===end-group===

~~~end-file~~~
