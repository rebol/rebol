Rebol [
	Title:   "Rebol series test script"
	Author:  "Oldes"
	File: 	 %series-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Series"

===start-group=== "FIND & SELECT"

--test-- "SELECT or FIND NONE! anything == none - #473"
	--assert none? find none 1
	--assert none? select none 1

--test-- "FIND"
	--assert none? find/part [x] 'x 0
	--assert equal? [x] find/part [x] 'x 1
	--assert equal? [x] find/reverse tail [x] 'x
	--assert equal? [y] find/match [x y] 'x
	--assert equal? [x] find/last [x] 'x
	--assert equal? [x] find/last [x x x] 'x

--test-- "FIND string! integer!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/237
	--assert "23" = find "123" 2

--test-- "FIND/ANY on string"
	--assert "abcd"  = find/any "abcd" "abc"
	--assert "abcd"  = find/any "abcd" "*c"
	--assert "abcd"  = find/any "abcd" "*c*"
	--assert "abcd"  = find/any "abcd" "*c?"
	--assert "abcd"  = find/any "abcd" "a*d"
	--assert "bcd"   = find/any "abcd" "b*d"
	--assert  none?    find/any "abcd" "b*dx"
	--assert "bcde"  = find/any "abcde" "b?d"
	--assert "bcde"  = find/any "abcde" "b??e"
	--assert "bcdef" = find/any "abcdef" "b*?*e"
	--assert ""      = find/any/tail  "abcd" "*d"
	--assert ""      = find/any/tail  "abcd" "c*"
	--assert "ef"    = find/any/tail  "abcdef" "b*d"
	--assert ""      = find/any/match "abc" "a?c"
	--assert ""      = find/any/match "abcdef" "a*e?"
	--assert "bcd"   = find/any/reverse tail "abcdabcd" "?c"
	--assert "d"     = find/any/reverse/tail tail "abcdabcd" "?c"
	--assert "d"     = find/any/reverse/tail tail "abcdabcd" "bc"

--test-- "FIND/ANY on string (unicode)"
	--assert "ažcd"  = find/any "ažcd" "ažc"
	--assert "ažcd"  = find/any "ažcd" "*c"
	--assert "ažcd"  = find/any "ažcd" "*c*"
	--assert "ažcd"  = find/any "ažcd" "*c?"
	--assert "ažcd"  = find/any "ažcd" "a*d"
	--assert "žcd"   = find/any "ažcd" "ž*d"
	--assert  none?    find/any "ažcd" "ž*dx"
	--assert "žcde"  = find/any "ažcde" "ž?d"
	--assert "žcde"  = find/any "ažcde" "ž??e"
	--assert "žcdef" = find/any "ažcdef" "ž*?*e"
	--assert ""      = find/any/tail  "ažcd" "*d"
	--assert "ef"    = find/any/tail  "ažcdef" "ž*d"
	--assert ""      = find/any/match "ažc" "a?c"
	--assert ""      = find/any/match "ažcdef" "a*e?"
	--assert "žcd"   = find/any/reverse tail "ažcdažcd" "?c"
	--assert "d"     = find/any/reverse/tail tail "ažcdažcd" "?c"
	--assert "d"     = find/any/reverse/tail tail "ažcdažcd" "žc"

--test-- "FIND/SAME in block!"
	a: "a"
	obj1: context [a: 1 b: 2]
	obj2: context [a: 1 b: 2]
	b: reduce  ["a" obj1 a obj2]
	--assert 1 = index? find b :a
	--assert 3 = index? find/same b :a
	--assert 2 = index? find/same b :obj1
	--assert 4 = index? find/same b :obj2
	--assert 2 = index? find b :obj1
	--assert 2 = index? find b :obj2
	--assert 2 = index? find/case b :obj2 ;/case is not /same in this case
	b: [a a: 'a :a]
	--assert 1 = index? find/same b b/1
	--assert 2 = index? find/same b b/2
	--assert 3 = index? find/same b b/3
	--assert 4 = index? find/same b b/4
	--assert 1 = index? find b b/1
	--assert 1 = index? find b b/2
	--assert 1 = index? find b b/3

--test-- "FIND/SAME in string!"
	--assert "AbcdAe" = find/same "aAbcdAe" "A"
	--assert "Ae" = find/same/last "aAbcdAe" "A"

--test-- "FIND/LAST/CASE in string!"
;@@ https://github.com/Oldes/Rebol-issues/issues/1495
	--assert none?   find/case "Abc" "a"
	--assert "Abc" = find/case "Abc" "A"
	--assert none?   find/case/last "Abc" "a"
	--assert "Abc" = find/case/last "Abc" "A"

--test-- "FIND/PART"
;@@ https://github.com/Oldes/Rebol-issues/issues/2329
;@@ need to decide, which result is correct
;	--assert none? find/part "abcd" "bc" 2
;	--assert none? find/part/any "abcd" "*c" 2

--test-- "FIND char in string"
	str: "a,b"
	--assert ",b" = find str #","
	--assert ",b" = find/reverse tail str #","
	--assert "b"  = find/tail str #","
	--assert "b"  = find/tail/reverse tail str #","
	--assert "b"  = find str #"b"
	--assert "b"  = find str #"B"
	--assert "b"  = find/case str #"b"
	--assert none?  find/case str #"B"
	;@@ https://github.com/Oldes/Rebol-issues/issues/118
	--assert "abc" = find/last "abcabcabc" #"a"

--test-- "FIND with negative skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2291
	--assert none? find/skip "acdcde" "cd" -3
	--assert "cde" = find/skip/reverse tail "acd000cde" "cd" -3
	--assert "cde111" = find/skip/reverse tail "acd000cde111" "cd" -3
	; I'm not sure with the next one, but I will keep it:
	--assert "cde" = find/reverse/skip tail "acde" make bitset! #"c" -2

--test-- https://github.com/Oldes/Rebol-issues/issues/66
	--assert none? find/skip [1 2 3 4 5 6] 2 3

--test-- https://github.com/Oldes/Rebol-issues/issues/88
	--assert "c" = find "abc" charset ["c"]
	--assert none? find/part "ab" "b" 1

--test-- https://github.com/Oldes/Rebol-issues/issues/2324
	str: "1.1.1"
	--assert "1.1.1" == find/part str "1." 2
	str: skip str 2
	--assert "1.1" == find str "1."
	--assert "1.1" == find/part str "1." 2

--test-- "SELECT"
	--assert 2 = select/part [1 2 1 3 1 2] 1 2
	--assert none? select/part [1 2 1 3 1 2] 2 2
	--assert 3 = select/part (skip [1 2 1 3 1 2] 2) 1 2
	;@@ https://github.com/Oldes/Rebol-issues/issues/1936
	--assert 9 = select [1 2 3 4 5 6 6 6 6 6 1 2 3 4 5 6 7 8 9 0 1] [6 7 8] 

--test-- "SELECT/skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/101
	--assert none? select/skip [1 2 3 4 5 6] 5 3

--test-- "SELECT on string"
	--assert #"e" = select     "abcde" "bcd"
	--assert #"e" = select/any "abcde" "b?d"
	--assert #"e" = select/any "abcde" "*d"
	--assert #"e" = select/any "abcde" "*?d"
	--assert none?  select     "abcde" "cde"
	--assert none?  select/any "abcde" "c*"
	--assert #"e" = select/any/with "abcde" "a##d" "@#"
	--assert #"e" = select/any/with "abcde" "a@d" "@#"
	--assert #"2" = select/last "ab1ab2" "ab"
	--assert #"2" = select/last/any "ab1ab2" "?b"
	--assert #"2" = select/last/any "ab1ab2" "ab"
	--assert #"b" = select/same "aAbcdAe" "A"
	--assert #"e" = select/same/last "aAbcdAe" "A"

===end-group===

===start-group=== "PATH notation"
	--test-- "issue-64"
	;@@ https://github.com/Oldes/Rebol-issues/issues/64
		b: [[1 2] [3 4]]
		i: 1
		--assert [1 2] = b/:i
		--assert 1 = b/1/:i
		b/:i: "foo"
		--assert "foo" = b/1
===end-group===


===start-group=== "REVERSE"
	--test-- "issue-19"
		--assert "4321" = sort/reverse "1234"
		--assert "4321" = sort/reverse/reverse "1234"
===end-group===


===start-group=== "REPLACE string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/54
	--test-- "issue-54"
		--assert "ABCDE456" = replace "123456" "123" "ABCDE"
		--assert "1!!2!!3"  = replace/all "1 2 3" " " "!!"
===end-group===

===start-group=== "REPLACE binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1499
	--test-- "issue-1499"
		--assert #{6163} = replace #{616263} #{6263} #{63}
===end-group===

===start-group=== "POKE"
	--test-- "poke unset value"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1815
		--assert all [
			b: [1 2 3] 
			not error? try [poke b 2 #[unset!]]
			unset? pick b 2
			unset? b/2
		]
===end-group===

===start-group=== "CHANGE string!"
	--test-- "change/part"
		;@@ https://github.com/Oldes/Rebol-issues/issues/55
		--assert "456" = change/part "123456" "99" 3
		;@@ https://github.com/Oldes/Rebol-issues/issues/257
		--assert "123cd" = head change/part "abcd" "123" 2
		;@@ https://github.com/Oldes/Rebol-issues/issues/490
		--assert "REBOL Rules" = head change/part data: "C# Rules" "REBOL" 2

	--test-- "change/dup"
		;@@ https://github.com/Oldes/Rebol-issues/issues/383
		mem: make string! 5 loop 10 [change/dup mem "x" 5] mem
		--assert mem = "xxxxx"

	--test-- "issue-404"
		;@@ https://github.com/Oldes/Rebol-issues/issues/404
		str: "abc"
		--assert #"Á" = str/1: str/1 + 96
		--assert "Ábc" = str

===end-group===


===start-group=== "APPEND string!"
	--test-- "APPEND string! char!"
		--assert "a" = append "" #"a"
		--assert "←" = append "" #"^(2190)" ; wide char
===end-group===

===start-group=== "APPEND ref!"
	--test-- "APPEND ref! char!"
		--assert @a = append @ #"a"
		--assert @← = append @ #"^(2190)" ; wide char
===end-group===


;@@ https://github.com/Oldes/Rebol-issues/issues/1791
===start-group=== "APPEND binary!"
	--test-- "APPEND binary! binary!"
		--assert #{0102}     = append #{01} #{02}
		--assert #{0102}     = append next #{01} #{02}
	--test-- "APPEND binary! string!"
		--assert #{0001}     = append #{00} "^(01)"
		--assert #{0001}     = append next #{00} "^(01)"
		--assert #{00E28690} = append #{00} "^(2190)"
		--assert #{00E28690} = append next #{00} "^(2190)"
	--test-- "APPEND binary! file!"
		--assert #{616263}   = append #{} %abc
		--assert #{C3A162}   = append #{} %áb
	--test-- "APPEND binary! char!"
		--assert #{0001}     = append #{00} #"^(01)"
		--assert #{00E28690} = append #{00} #"^(2190)"
	--test-- "APPEND/part binary!"
		--assert #{01} = append/part #{} #{0102} 1
		--assert #{01} = append/part #{} "^(01)^(02)" 1
		--assert #{E2} = append/part #{} "^(2190)" 1 ;-- by design!
	--test-- "APPEND to same value"
		;@@ https://github.com/Oldes/Rebol-issues/issues/226
		a: "x" b: #{FF}
		--assert "xx" = append a a
		--assert #{FFFF} = append b b
===end-group===

===start-group=== "INSERT binary!"
	--test-- "INSERT binary! binary!"
		--assert #{0201}     = head insert #{01} #{02}
		--assert #{0102}     = head insert next #{01} #{02}
	--test-- "INSERT binary! string!"
		--assert #{0100}     = head insert #{00} "^(01)"
		--assert #{0001}     = head insert next #{00} "^(01)"
		--assert #{E2869000} = head insert #{00} "^(2190)"
		--assert #{00E28690} = head insert next #{00} "^(2190)"
	--test-- "INSERT binary! file!"
		--assert #{61626300} = head insert #{00} %abc
		--assert #{C3A16200} = head insert #{00} %áb
	--test-- "INSERT binary! char!"
		--assert #{0100}     = head insert #{00} #"^(01)"
		--assert #{E2869000} = head insert #{00} #"^(2190)"
	--test-- "INSERT/part binary!"
		--assert #{0100} = head insert/part #{00} #{0102} 1
		--assert #{0100} = head insert/part #{00} "^(01)^(02)" 1
		--assert #{E200} = head insert/part #{00} "^(2190)" 1 ;-- by design!
===end-group===

===start-group=== "CHANGE binary!"
	--test-- "CHANGE binary! string!"
		--assert #{E188B4} = head change #{} "^(1234)"
		--assert #{E188B4} = head change #{00} "^(1234)"
		--assert #{E188B4} = head change #{0000} "^(1234)"
		--assert #{E188B403} = head change/part #{010203} "^(1234)" 2
===end-group===

===start-group=== "TAKE"
	--test-- "take string!"
	s: "a"
	--assert #"a" = take s
	--assert none?  take s

	--test-- "take/part string!"
	s: "123456"
	--assert "12" = take/part s 2
	--assert "34" = take/part skip s 2 -5 ;@@ https://github.com/Oldes/Rebol-issues/issues/373
	--assert "56" = take/part s 10

	--test-- "take/part any-block!"
		--assert [1 2] = take/part [1 2 3 4] 2
		;@@ https://github.com/Oldes/Rebol-issues/issues/2174
		--assert "(1 2)" = mold take/part quote (1 2 3 4) 2
		--assert "a/b"   = mold take/part quote a/b/c 2
		--assert "'a/b"  = mold take/part quote 'a/b/c 2
		--assert ":a/b"  = mold take/part quote :a/b/c 2
		--assert "a/b:"  = mold take/part quote a/b/c: 2

	--test-- "take/last"
	;@@ https://github.com/Oldes/Rebol-issues/issues/177
	--assert #"c" = take/last str: "abc"
	--assert "ab" = str
	--assert 3    = take/last blk: [1 2 3]
	--assert [1 2] = blk

	;@@ https://github.com/Oldes/Rebol-issues/issues/171
	--test-- "take/deep block!"
		a: [1 [2] "3"] b: reduce [a] c: take b
		--assert same? a c
		a: [1 [2] "3"] b: reduce [a] c: take/deep b
		--assert not same? a c
		--assert not same? a/2 c/2
		--assert not same? a/3 c/3
		--assert [2 3] = append c/2 3
		--assert [2] = a/2
	--test-- "take/deep block with string!"
		a: "1" b: reduce [a 2] c: take b
		--assert same? a c
		a: "1" b: reduce [a 2] c: take/deep b
		--assert "1" = a
		--assert "1" = c
		--assert not same? a c
		--assert "12" = append c 2
		--assert "1"  = a
	--test-- "take/deep block with object!"
		a: object [] b: reduce [a 2] c: take b
		--assert same? a c
		a: object [] b: reduce [a 2] c: take/deep b
		--assert same? a c ; object are not copied
	--test-- "take/deep/part block!"
		a: [1 [2] "3"] b: reduce [a] c: take/part b 1
		--assert same? a c/1
		--assert [] = b
		a: [1 [2] "3"] b: reduce [a] c: take/deep/part b 1
		--assert not same? a c/1
		--assert not same? a/2 c/1/2
		--assert not same? a/3 c/1/3
		--assert [2 3] = append c/1/2 3
		--assert "34"  = append c/1/3 4
		--assert "3"   = a/3
	--test-- "take/deep/part block with string!"
		a: "1" b: reduce [a 2] c: take/part b 1
		--assert same? a c/1
		--assert [2] = b
		a: "1" b: reduce [a 2] c: take/deep/part b 1
		--assert [2] = b
		--assert "1" = a
		--assert "1" = c/1
		--assert not same? a c/1
		--assert "12" = append c/1 2
		--assert "1"  = a

	--test-- "take block!"
	s: [1]
	--assert 1   =  take s
	--assert none?  take s

	--test-- "take/part block!"
	s: [1 2 3 4 5 6]
	--assert [1 2] = take/part s 2
	--assert [3 4] = take/part skip s 2 -5 ;@@ https://github.com/Oldes/Rebol-issues/issues/373
	--assert [5 6] = take/part s 10

	;@@ https://github.com/Oldes/Rebol-issues/issues/141
	s: [1 2 3 4]
	--assert [1]   = take/part s 1
	--assert [2 3] = take/part s 2
	--assert [4]   = take/part s 3
	--assert []    = take/part s 1


	--test-- "take binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/963
	--assert 32 = take #{20}
	--assert  4 = take #{04}

	--test-- "take/part binary!"
	s: #{010203040506}
	--assert #{0102} = take/part s 2
	--assert #{0304} = take/part skip s 2 -5 ;@@ https://github.com/Oldes/Rebol-issues/issues/373
	--assert #{0506} = take/part s 10

===end-group===


===start-group=== "++ & --"
	--test-- "++ & -- on block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/340
		a: [ 1 2 ]
		--assert all [
			[2] = (++ a ++ a)
			a = []
			3 = index? a
			[] = (-- a)
			2 = index? a
		]
		a: [ 1 2 ]
		--assert all [
			[1 2] = (-- a)
			[1 2] = a
			1 = index? a
			[1 2] = (++ a)
			[  2] = a
			2 = index? a
		]
		
===end-group===


===start-group=== "PICK"
	;@@ https://github.com/Oldes/Rebol-issues/issues/608
	;@@ https://github.com/Oldes/Rebol-issues/issues/857
	--test-- "PICK of block!"
	b: [1 2 3]
	--assert all [
		1   = pick b 1
		2   = pick b 2
		none? pick b -1
		none? pick b 0
		none? pick b 10
	]
	b: skip b 2
	--assert all [
		none? pick b 2
		3   = pick b 1
		none? pick b 0
		2   = pick b -1
		1   = pick b -2
	]
	--assert all [
		none? pick tail b 0
		3   = pick tail b -1
	]

	--test-- "PICK of string!"
	s: "123"
	--assert all [
		#"1"= pick s 1
		#"2"= pick s 2
		none? pick s -1
		none? pick s 0
		none? pick s 10
	]
	s: skip s 2
	--assert all [
		none? pick s 2
		#"3"= pick s 1
		none? pick s 0
		#"2"= pick s -1
		#"1"= pick s -2
	]
	--assert all [
		none? pick tail s 0
		#"3"= pick tail s -1
	]

===end-group===


===start-group=== "PUT"
	--test-- "PUT into BLOCK"
	v: [a 1 b 2 c]
	--assert 3 = put v 'a 3
	--assert 4 = put v 'b 4
	--assert 5 = put v 'c 5
	--assert 6 = put v 'd 6
	--assert v = [a 3 b 4 c 5 d 6]

	--test-- "PUT into PAREN"
	v: quote (a 1 b 2 c)
	--assert 3 = put v 'a 3
	--assert 4 = put v 'b 4
	--assert 5 = put v 'c 5
	--assert 6 = put v 'd 6
	--assert v = quote (a 3 b 4 c 5 d 6)

	--test-- "PUT into PATH"
	v: to path! [a 1 b 2 c]
	--assert 3 = put v 'a 3
	--assert 4 = put v 'b 4
	--assert 5 = put v 'c 5
	--assert 6 = put v 'd 6
	--assert v = 'a/3/b/4/c/5/d/6

	--test-- "PUT/CASE words"
	v: [a 1 b 2]
	--assert 3 = put v 'a 3
	--assert 4 = put/case v quote :a 4
	--assert 5 = put/case v quote 'b 5
	--assert v = [a 3 b 2 :a 4 'b 5]

	--test-- "PUT/CASE strings"
	v: ["a" 1 "b" 2]
	--assert 3 = put v "a" 3
	--assert 4 = put/case v "A" 4
	--assert 5 = put/case v "B" 5
	--assert v = ["a" 3 "b" 2 "A" 4 "B" 5]

	--test-- "PUT on protected block"
	v: protect [a 1]
	--assert protected? v
	--assert error? err: try [ put v 'a 2 ]
	--assert 'protected = err/id

===end-group===

===start-group=== "INSERT"
	--test-- "insert/dup"
		x: copy [] insert/dup x 5 3
		--assert x = [5 5 5]
	;@@ https://github.com/Oldes/Rebol-issues/issues/200
		x: copy [] insert/dup x 5 3.9
		--assert 3 = length? x
		x: copy [] insert/dup x 5 -1
		--assert 0 = length? x

===end-group===

===start-group=== "REMOVE"
	--test-- "remove-blk-1"
		a: [1 2 3]
		--assert [2 3] = remove a
		--assert [2 3] = a

	--test-- "remove-blk-2"
		a: [1 2 3]
		--assert [3] = remove next a
		--assert [1 3] = a

	--test-- "remove-blk-3"
		--assert tail? head remove []

	--test-- "remove-blk-4"
		a: [1 2 3]
		--assert [3] = remove/part a 2
		--assert [3] = a

	--test-- "remove-blk-5"
		a: [1 2 3]
		--assert [1 2 3] = remove/part a a

	--test-- "remove-blk-6"
		a: [1 2 3]
		--assert [2 3] = remove/part a next a
		--assert [2 3] = a
	
	--test-- "remove-blk-7"
		a: [1 2 3]
		--assert [1 2 3] =  remove/part a 0

;	--test-- "remove-blk-8"
;		blk: [a 1 b 2 c 3]
;		--assert [a 1 c 3] =  remove/key blk 'b
;
;	--test-- "remove-blk-9"
;		blk: [a 1 1 b 2 2 c 3 3]
;		--assert [a 1 1 c 3 3] =  remove/key/part blk 'b 2
;
;	--test-- "remove-hash-1"
;		hs-remove-1: make hash! [a 2 3]
;		--assert (make hash! [2 3]) = remove hs-remove-1
;		--assert none? hs-remove-1/a
;
;	--test-- "remove-hash-2"
;		hs-remove-1: make hash! [a 2 3]
;		remove next hs-remove-1
;		--assert 3 = hs-remove-1/a
;
;	--test-- "remove-hash-3"
;		--assert tail? head remove make hash! []
;
;	--test-- "remove-hash-4"
;		hs-remove-1: make hash! [a b c 2]
;		remove/part hs-remove-1 2
;		--assert 2 = hs-remove-1/c
;
;	--test-- "remove-hash-5"
;		hs-remove-1: make hash! [a b c 2]
;		remove/part next hs-remove-1 2
;		--assert 2 = hs-remove-1/a
;		--assert none? hs-remove-1/b
;		--assert none? hs-remove-1/c
;
;	--test-- "remove-hash-6"
;		hs: make hash! [a 1 b 2 c 3]
;		--assert (make hash! [a 1 c 3]) =  remove/key hs 'b
;
;	--test-- "remove-hash-7"
;		hs: make hash! [a 1 1 b 2 2 c 3 3]
;		--assert (make hash! [a 1 1 c 3 3]) =  remove/key/part hs 'b 2

	--test-- "remove-str-1"
		a: "123"
		--assert "23" = remove a
		--assert "23" = a

	--test-- "remove-str-2"
		a: "123"
		--assert "3" = remove next a
		--assert "13" = a

	--test-- "remove-str-3"
		--assert tail? head remove ""

	--test-- "remove-str-4"
		a: "123"
		--assert "3" = remove/part a 2
		--assert "3" = a

	--test-- "remove-str-5"
		a: "123"
		--assert "123" = remove/part a a

	--test-- "remove-str-6"
		a: "123"
		--assert "23"= remove/part a next a
		--assert "23" = a
	
	--test-- "remove-str-7"
		a: "123"
		--assert "123" = remove/part a 0
	
	--test-- "remove-str-8"
		a: "str123"
		--assert "" = remove back tail a
		--assert "str12" = head a

	--test-- "remove-bin-1"
		b: #{00010203}
		--assert #{010203} = remove b
	--test-- "remove-bin-2"
		--assert #{0203} = remove next #{00010203}
	--test-- "remove-bin-3"
		--assert #{000203} = head remove next #{00010203}
	--test-- "remove-bin-4"
		--assert #{0003} = head remove/part next #{00010203} 2

	--test-- "issue-146"
		;@@ https://github.com/Oldes/Rebol-issues/issues/146
		b: append [] 0
		repeat n 10 [ append b n remove b]
		--assert 1 = length? b
		--assert 10 = first b

	--test-- "issue-2397"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2397
		b: make binary! 40000 insert/dup b 0 40000
		remove/part b to integer! #{8000}
		--assert 7232 = length? b

===end-group===

===start-group=== "SORT"

--test-- "sort block of words"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2218
	b: [A b C a B c]
	--assert [A a b B C c] == sort b
	--assert [A B C a b c] == sort/case b
	--assert [A a B b C c] == sort b

--test-- "sort block of datatypes"
	;@@ https://github.com/Oldes/Rebol-issues/issues/406
	--assert (reduce [integer! string!]) == (sort reduce [string! integer!])
	
--test-- "sort string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2170
	s: "ABCabcdefDEF"
	--assert "AaBbCcdDEefF" == sort s
	--assert "AaBbCcdDEefF" == sort s ; just to test if it stays same
	--assert "ABCDEFabcdef" == sort/case s
	--assert "AaBbCcDdEeFf" == sort s

--test-- "SORT/compare block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/720
	--assert [3 2 1] = sort/compare [1 2 3] func [a b] [a > b]
	;@@ https://github.com/Oldes/Rebol-issues/issues/2376
	--assert [1 3 10] = sort/compare [1 10 3] func[x y][case [x > y [1] x < y [-1] true [0]]]
	;@@ https://github.com/Oldes/Rebol-issues/issues/721
	--assert [4 3 2 1] = sort/compare [1 2 3 4] :greater?

--test-- "SORT/compare string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1100
	comp: func [a b] [a > b]
	--assert "zyxcba" == sort/compare "abczyx" :comp
	--assert %54321 == sort/compare %21543 :comp
	--assert #{050403020100} == sort/compare #{000102030405} :comp
	--assert "šřba" == sort/compare "ašbř" :comp

--test-- "SORT/skip/compare"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1152
	--assert ["A" "a"] = sort/compare ["A" "a"] func [a b] [a < b]
	--assert ["a" "A"] = sort/compare ["a" "A"] func [a b] [a < b]
	--assert ["A" "a"] = sort/compare ["A" "a"] func [a b] [a <= b]
	--assert ["a" "A"] = sort/compare ["a" "A"] func [a b] [a <= b]
	--assert [1 9 1 5 1 7] = sort/skip/compare [1 9 1 5 1 7] 2 1
	;@@ https://github.com/Oldes/Rebol-issues/issues/161
	--assert [3 2 1] = sort/compare [1 2 3] func [a b] [return a > b]

--test-- "SORT/skip with negative skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/736
	--assert all [
		error? e: try [sort/skip [2 1] -2]
		e/id = 'out-of-range
	]

--test-- "SORT with invalid compare function"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1766
	--assert error? try [sort/compare [1 2 3]  func [/local loc-1 loc-2][local < loc-1] ]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1516
	--assert error? try [sort/compare [1 2 #[unset!]] :>]

--test-- "SORT with unset!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1124
	--assert [#[unset!] 2 3] = sort reduce [2 #[unset!] 3 ]

--test-- "SORT/reverse"
	;@@ https://github.com/Oldes/Rebol-issues/issues/128
	--assert [9 3 2 1] = sort/reverse [9 1 2 3] 

--test-- "SORT block of strings"
	;@@ https://github.com/Oldes/Rebol-issues/issues/681
	--assert ["Fred" "fred" "FRED"] == sort      ["Fred" "fred" "FRED"]
	--assert ["fred" "Fred" "FRED"] == sort      ["fred" "Fred" "FRED"]
	--assert ["FRED" "Fred" "fred"] == sort/case ["Fred" "fred" "FRED"]

===end-group===


===start-group=== "UPPERCASE / LOWERCASE"
	--test-- "uppercase/part backwards"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1772
		--assert "abcDEFG" = head uppercase/part tail "abcdefg" -4
		--assert "abCDEFG" = head uppercase/part tail "abcdefg" -5
		--assert "aBCDEFG" = head uppercase/part tail "abcdefg" -6
		--assert "ABCDEFG" = head uppercase/part tail "abcdefg" -10
	--test-- "lowercase/part backwards"
		--assert "ABCdefg" = head lowercase/part tail "ABCDEFG" -4
		--assert "ABcdefg" = head lowercase/part tail "ABCDEFG" -5
		--assert "Abcdefg" = head lowercase/part tail "ABCDEFG" -6
		--assert "abcdefg" = head lowercase/part tail "ABCDEFG" -10

===end-group===



===start-group=== "RANDOM"
	--test-- "random/only string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1083
	    s: "0123456789" b: copy []
    	loop 100 [append b random/only s]
		--assert [#"0" #"1" #"2" #"3" #"4" #"5" #"6" #"7" #"8" #"9"] = sort unique b 

===end-group===


===start-group=== "PAD"
--test-- "pad"
	--assert "ab  " = pad "ab"  4
	--assert "  ab" = pad "ab" -4
	--assert "12  " = pad  12   4
	--assert "  12" = pad  12  -4
--test-- "pad/with"
	--assert "1200" = pad/with  12   4 #"0"
	--assert "0012" = pad/with  12  -4 #"0"
===end-group===


===start-group=== "REWORD"

--test-- "reword/escape"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2333
	subs: ["1" "foo" "10" "bar"]
	--assert "bar" = reword/escape "$<10>" subs ["$<" ">"]

===end-group===

===start-group=== "FORALL"

--test-- "Basic FORALL"
	;-- compatible with R2 & Red - the series is at its tail
	data: [1 2 3 4]
	--assert unset? forall data []
	--assert data = [1 2 3 4]
	data: next data
	--assert unset? forall data []
	--assert data = [2 3 4]
	--assert 4 = forall data [data/1]
	--assert data = [2 3 4]

;@@ https://github.com/Oldes/Rebol-issues/issues/2332
--test-- "Escaping from FORALL loop using THROW"
	data: [1 2 3 4]
	--assert "yes" = catch [forall data [if data/1 = 3 [throw "yes"]]]
	--assert [3 4] = data
--test-- "Escaping from FORALL loop using BREAK"
	data: [1 2 3 4]
	--assert unset? forall data [if data/1 = 3 [break]]
	--assert [3 4] = data
--test-- "Escaping from FORALL loop using BREAK/RETURN"
	data: [1 2 3 4]
	--assert forall data [if data/1 = 3 [break/return true]]
	--assert [3 4] = data
--test-- "Escaping from FORALL loop on error"
	data: [1 2 3 4]
	--assert error? try [forall data [if data/1 = 3 [do make error! "stopped"]]]
	--assert [3 4] = data

===end-group===

===start-group=== "FOREACH"

--test-- "FOREACH on string"
	;@@ https://github.com/Oldes/Rebol-issues/issues/15
	data: copy ""
	foreach x "123" [append data x]
	--assert "123" = data

===end-group===

===start-group=== "STRING conversion"

--test-- "issues/2336"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2336
	--assert "^M"   = to-string to-binary "^M"
	--assert "^/"   = to-string to-binary "^/"
	--assert "^M^/" = to-string to-binary "^M^/"
	--assert "^/^M" = to-string to-binary "^/^M"

--test-- "issue-2186"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2186
	--assert "äöü^/" = to-string #{FFFE0000E4000000F6000000FC0000000A000000}
	--assert "äöü^/" = to-string #{0000FEFF000000E4000000F6000000FC0000000A}

; additional tests which also contain CRLF
--test-- "issue-2186 read UCS16-LE"
	bin: read %units/files/issue-2186-UTF16-LE.txt
	--assert all [
		string? try [str: to-string bin]
		3160989 = checksum str
	]
--test-- "issue-2186 read UCS16-BE"
	bin: read %units/files/issue-2186-UTF16-BE.txt
	--assert all [
		string? try [str: to-string bin]
		3160989 = checksum str
	]
--test-- "issue-2186 read UCS32-LE"
	bin: read %units/files/issue-2186-UTF32-LE.txt
	--assert all [
		string? try [str: to-string bin]
		3160989 = checksum str
	]
--test-- "issue-2186 read UCS32-BE"
	bin: read %units/files/issue-2186-UTF32-BE.txt
	--assert all [
		string? try [str: to-string bin]
		3160989 = checksum str
	]
;- read/string converts CRLF to LF, so the checksum is different
--test-- "issue-2186 read/string UCS16-LE"
	--assert all [
		string? try [str: read/string %units/files/issue-2186-UTF16-LE.txt]
		11709824 = checksum str
	]
--test-- "issue-2186 read/string UCS16-BE"
	--assert all [
		string? try [str: read/string %units/files/issue-2186-UTF16-BE.txt]
		11709824 = checksum str
	]
--test-- "issue-2186 read/string UCS32-LE"
	--assert all [
		string? try [str: read/string %units/files/issue-2186-UTF32-LE.txt]
		11709824 = checksum str
	]
--test-- "issue-2186 read/string UCS32-BE"
	--assert all [
		string? try [str: read/string %units/files/issue-2186-UTF32-BE.txt]
		11709824 = checksum str
	]

--test-- "invalid UTF8 char"
;@@ https://github.com/Oldes/Rebol-issues/issues/1064
;@@ https://github.com/Oldes/Rebol-issues/issues/1216
	--assert 1 = length? str: to-string #{C2E0}
	--assert "^(FFFD)" = str
	--assert #{EFBFBD} = to-binary str
	--assert #{C2E0} = invalid-utf? #{C2E0}
	--assert #{C2E0} = invalid-utf? #{01C2E0}
	--assert 2 = index? invalid-utf? #{20C2E030}
	--assert 2 = index? invalid-utf? #{20C3}
	--assert none? invalid-utf? #{20C3A030}
	--assert #{EF} = invalid-utf? #{20EF}
	--assert #{EFBF} = invalid-utf? #{20EFBF}
	--assert "^(FFFD)" = to-string #{C3}
	--assert "^(FFFD)" = to-string #{EF}
	--assert "^(FFFD)" = to-string #{EFBF}
	--assert "^(FFFD)" = to-string #{EFBFBD}
	;- using quickbrown.bin instead of quickbrown.txt beacause GIT modifies CRLF to LF on posix
	--assert none? invalid-utf? bin: read %units/files/quickbrown.bin
	--assert 13806406 = checksum str: to-string bin ; does not normalize CRLF
	--assert 5367801 = checksum deline str
	--assert 5367801 = checksum read/string %units/files/quickbrown.bin ;converts CRLF to LF

--test-- "LOAD Unicode encoded text with BOM"
	--assert "Writer" = form load #{FEFF005700720069007400650072}     ;UTF-16BE
	--assert "Writer" = form load #{FFFE570072006900740065007200}     ;UTF-16LE
	--assert "ěšč"    = form load #{0000feff0000011b000001610000010d} ;UTF-32BE
	--assert "ěšč"    = form load #{fffe00001b010000610100000d010000} ;UTF-32LE
	--assert "esc"    = form load #{0000feff000000650000007300000063} ;UTF-32BE
	--assert "esc"    = form load #{fffe0000650000007300000063000000} ;UTF-32LE
	--assert [a b]    = load #{0000feff000000610000002000000062}
	--assert [a b]    = load #{fffe0000610000002000000062000000}
	;@@ https://github.com/Oldes/Rebol-issues/issues/2280
	--assert "äöü"    = form load #{EFBBBFC3A4C3B6C3BC}               ;UTF-8
	--assert "aou"    = form load #{EFBBBF616F75}                     ;UTF-8
	--assert "1 2"    = load #{EFBBBF2231203222}
	--assert "1 2"    = load #{2231203222}
	;@@ https://github.com/Oldes/Rebol-issues/issues/474
	write %temp #{EFBBBF612062}
	--assert [a b] = load %temp
	delete %temp

===end-group===

===start-group=== "ICONV"
;@@TODO: add some tests for exotic codepages?

--test-- "ICONV (conversion to string)"
	txt: "Přihlásit"
	--assert txt = iconv #{50F869686CE1736974} "ISO-8859-2"
	--assert txt = iconv #{50F869686CE1736974} 'ISO-8859-2
	--assert txt = iconv #{50F869686CE1736974} <ISO-8859-2>
	--assert txt = iconv #{50F869686CE1736974} 28592
	--assert txt = iconv #{50005901690068006C00E100730069007400} 1200
	;--assert txt = iconv #{FFFE50005901690068006C00E100730069007400} 'UTF16
	--assert (next txt) = iconv next #{50F869686CE1736974} 28592

--test-- "ICONV with empty imput"
	--assert "" = iconv #{} 28592
	--assert "" = iconv #{} 'utf8

--test-- "ICONV from UTF-16 without BOM"
	--assert "Writer" = iconv #{005700720069007400650072} 'UTF-16BE
	--assert "Writer" = iconv #{570072006900740065007200} 'UTF-16LE

--test-- "ICONV from UTF-16 with BOM"
	;@@ https://github.com/Oldes/Rebol3/issues/19
	--assert "^(FEFF)Writer" = iconv #{FEFF005700720069007400650072} 'UTF-16BE
	--assert "^(FEFF)Writer" = iconv #{FFFE570072006900740065007200} 'UTF-16LE
	--assert "Writer" = decode 'text #{FEFF005700720069007400650072}
	--assert "Writer" = decode 'text #{FFFE570072006900740065007200}

--test-- "ICONV from UTF-32 without BOM"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2426
	--assert "ěšč" = iconv #{0000011b000001610000010d} 'UTF-32BE
	--assert "ěšč" = iconv #{1b010000610100000d010000} 'UTF-32LE
	--assert  "šč" = iconv skip #{0000011b000001610000010d} 4 'UTF-32BE
	--assert  "šč" = iconv skip #{1b010000610100000d010000} 4 'UTF-32LE
--test-- "ICONV from UTF-32 without BOM (ascii content)"
	--assert "esc" = iconv #{000000650000007300000063} 'UTF-32BE
	--assert "esc" = iconv #{650000007300000063000000} 'UTF-32LE
	--assert  "sc" = iconv skip #{000000650000007300000063} 4 'UTF-32BE
	--assert  "sc" = iconv skip #{650000007300000063000000} 4 'UTF-32LE
--test-- "ICONV from UTF-32 with BOM"
	--assert "^(FEFF)ěšč" = iconv #{0000feff0000011b000001610000010d} 'UTF-32BE
	--assert "^(FEFF)ěšč" = iconv #{fffe00001b010000610100000d010000} 'UTF-32LE
--test-- "ICONV from UTF-32 to UTF-8"	
	--assert #{C49BC5A1C48D} = iconv/to #{1b010000610100000d010000} 'UTF-32LE 'UTF-8
	--assert #{C49BC5A1C48D} = iconv/to #{0000011b000001610000010d} 'UTF-32BE 'UTF-8

--test-- "ICONV/TO (conversion to different codepage - binary result)"
	bin: to binary! txt ; normaly conversion is done to UTF-8
	--assert bin = iconv/to #{50F869686CE1736974} "ISO-8859-2" "utf8"
	--assert bin = iconv/to #{50F869686CE1736974} 'ISO-8859-2  'utf8
	--assert bin = iconv/to #{50F869686CE1736974} <ISO-8859-2> <UTF-8>
	--assert bin = iconv/to #{50F869686CE1736974} 28592 65001

	--assert #{C5A1C3A96D} = iconv/to #{9AE96D} 1250 65001 ; this one internally uses preallocated series data
	--assert #{C5A1C3A96DC5A1C3A96D} = iconv/to #{9AE96D9AE96D} 1250 65001 ;this one internally extends series

--test-- "ICONV/TO (UTF-16 variants)"
	;- UTF-16 handling must be coded specially on Windows, so adding these tests here
	;- using UTF-16LE instead of just UTF-16 as iconv function on posix adds BOM if just UTF16 is used
	--assert #{50005901} = iconv/to  #{50F8} 28592 'UTF-16LE
	--assert #{5901} = iconv/to next #{50F8} 28592 'UTF-16LE
	--assert #{50005100} = iconv/to #{50005100} 'UTF-16LE 'UTF-16LE
	--assert #{00500051} = iconv/to #{00500051} 'UTF-16BE 'UTF-16BE

	--assert #{00500159} = iconv/to  #{50F8} 28592 'UTF-16BE
	--assert #{0159} = iconv/to next #{50F8} 28592 'UTF-16BE
	--assert #{00500051} = bin: iconv/to #{50005100} 'UTF-16LE 'UTF-16BE
	--assert #{50005100} = iconv/to bin 'UTF-16BE 'UTF-16LE
	--assert "PQ" = iconv bin 'UTF-16BE

--test-- "ICONV with nonsense codepages"
	--assert error? try [iconv #{30} 'foo]
	--assert error? try [iconv/to #{30} 'utf8 'foo]

--test-- "ICONV euro sign"
	--assert "€"   = iconv #{E282AC} 'utf8
	--assert #{80} = iconv/to #{E282AC} 'utf8 'cp1252
	--assert "€"   = iconv #{80} 'cp1252

===end-group===


===start-group=== "DEHEX / ENHEX"

--test-- "DEHEX UTF-8 encoded data"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1986
	--assert "řek" = to-string dehex to-binary "%c5%99ek"

	--assert "%3x " = dehex "%3x%20"
	--assert "++"   = dehex "%2b%2b"
	--assert 127 = to-integer first dehex "%7F"

--test-- "ENHEX"
	;@@ https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURIComponent
	--assert "%C2%A3"   = enhex "£"
	--assert "a%20b%5C" = enhex "a b\"
	--assert "%C5%A1ik" = enhex "šik"
	--assert "%D1%88%D0%B5%D0%BB%D0%BB%D1%8B" = enhex "шеллы"
	--assert "%22%23%24%25%26%2B%2C%2F%3A%3B%3D%3F%40%5B%5D%5C" = enhex {"#$%&+,/:;=?@[]\}
	--assert "%22#$%25&+,/:;=?@%5B%5D%5C" = form enhex as url! {"#$%&+,/:;=?@[]\}
	--assert ";,/?:@&=+$#" = form enhex as url! {;,/?:@&=+$#}
	--assert "%3B%2C%2F%3F%3A%40%26%3D%2B%24%23" = enhex {;,/?:@&=+$#}
	--assert "!'()*_.-~" = enhex {!'()*_.-~}
	--assert http://a?b=%25C5%25A1 = enhex http://a?b=š
	--assert "%C5%A1ik"  = to-string enhex %šik
	--assert       "šik" = to-string dehex enhex to-binary "šik"
	--assert       "%7F" = enhex to-string #{7F}
	--assert "%EF%BF%BD" = enhex to-string #{80} ; #{80} is not valid UTF-8!
	--assert "%EF%BF%BD" = enhex to-string #{81}
	--assert "%E5%85%83" = enhex {元}

--test-- "ENHEX/escape"
	--assert "(#23)"    = enhex/escape "(#)" #"#"
	--assert "(#C5#A1)" = enhex/escape "(š)" #"#"
--test-- "DEHEX/escape"
	--assert "C# #XX" = dehex/escape "C#23#20#XX" #"#"
	--assert "(š)"    = dehex/escape "#28š#29"    #"#"

--test-- "ENHEX/except"
	--assert "12%20%61%62" = enhex/except "12 ab" charset "12"
	--assert "12 %61%62"   = enhex/except "12 ab" charset "12 "


===end-group===

===start-group=== "DETAB / ENTAB"

--test-- "DETAB"
	--assert "    A^/    B" = detab      "^-A^/^-B"
	--assert "    A^/    Š" = detab      "^-A^/^-Š"
	--assert "  A^/  B"     = detab/size "^-A^/^-B" 2
	--assert "  A^/  Š"     = detab/size "^-A^/^-Š" 2
--test-- "ENTAB"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2216
	--assert "^-A^/^-B"     = entab      {    A^/    B}
	--assert "^-A^/^-Š"     = entab      {    A^/    Š}
	--assert "^-^-A^/^-^-B" = entab/size {    A^/    B} 2
	--assert "^-^-A^/^-^-Š" = entab/size {    A^/    Š} 2
	--assert "^-^-^- A"     = entab      {    ^-     A}

===end-group===


===start-group=== "DELINE / ENLINE"

--test-- "DELINE"
	--assert "^/^/" = deline "^M^/^/"
	--assert "a^/b^/" = deline "a^M^/b^M^/"
	--assert ["a" "b"] = deline/lines "a^M^/b^M^/"
	--assert ["" ""] = deline/lines "^M^/^M^/"
	;@@ https://github.com/Oldes/Rebol-issues/issues/648
	--assert ["a"] = deline/lines "a"
--test-- "DELINE modifies"
	str: "a^/b^M^/c"
	--assert str = deline str
	--assert str = "a^/b^/c"

--test-- "deline/lines issue 1794"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1794
	--assert 1 = length? deline/lines "Slovenščina"

--test-- "ENLINE"
	either 'Windows = system/platform [
		--assert "a^M^/b" = enline "a^/b"
	][
		--assert "a^/b" = enline "a^/b"
	]
--test-- "ENLINE modifies"
	str: "a^/b^M^/c"
	--assert str = enline str
	
===end-group===


===start-group=== "AS coercion"

--test-- "AS datatype! any-string!"
	s: "hell"
	--assert file?  f: as file!  s
	--assert email? e: as email! s
	--assert url?   u: as url!   s
	--assert tag?   t: as tag!   s
	--assert ref?   r: as ref!   s
	append s #"o"
	--assert f = %hello
	--assert e = to-email %hello
	--assert u = #[url! "hello"]
	--assert t = <hello>
	--assert r = @hello

--test-- "AS datatype! any-block!"
	b: [a b]
	--assert paren?    pa: as paren!    b
	--assert path?     p:  as path!     b
	--assert set-path? sp: as set-path! b
	--assert get-path? gp: as get-path! b
	--assert lit-path? lp: as lit-path! b
	append b 'c
	--assert pa = quote (a b c)
	--assert p  = quote a/b/c
	--assert sp = quote a/b/c:
	--assert gp = quote :a/b/c
	--assert lp = quote 'a/b/c

--test-- "AS example any-string!"
	s: "hell"
	--assert file?  f: as %file  s
	--assert email? e: as e@mail s
	--assert url?   u: as #[url! ""] s
	--assert tag?   t: as <tag>  s
	--assert ref?   r: as @ref   s

--test-- "AS with protect"
	b: protect [a b]
	--assert path? try [p: as path! b]
	--assert protected? b
	;--assert protected? p ;@@ <--- fails!
	--assert error? e: try [append b 'c]
	--assert e/id = 'protected
	--assert error? e: try [append p 'c]
	--assert e/id = 'protected

--test-- "AS coercion error"
	--assert error? e: try [as string! []]
	--assert e/id = 'not-same-class
	--assert error? e: try [as block! ""]
	--assert e/id = 'not-same-class

===end-group===

===start-group=== "TAG"

--test-- "Join tags"
	;@@ https://github.com/Oldes/Rebol-issues/issues/10
	--assert "<a<b b>>" = mold append <a> <b b>
	--assert "<a<b b>>" = mold join <a> <b b>

===end-group===


===start-group=== "BINARY"

--test-- "CHANGE binary! integer!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/966
	--assert #{00} = head change #{} 0
	--assert #{FF} = head change #{} 255
	--assert error? try [head change #{} 256]
	--assert error? try [head change #{} -1]

--test-- "APPEND binary! integer!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/965
	--assert #{00} = append #{} 0
	--assert #{FF} = append #{} 255
	--assert error? try [append #{} 256]
	--assert error? try [append #{} -1]

--test-- "REPLACE binary! integer! integer!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2412
	b: #{010201}
	--assert #{010301} = replace b 2 3
	--assert #{000300} = replace/all b 1 0
	--assert error? try [replace b 257 0]

--test-- "any-string? binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/820
	--assert not any-string? #{}

--test-- "binary not allowed with latin1? and ascii?"
	;@@ https://github.com/Oldes/Rebol-issues/issues/908
	--assert error? try [latin1? #{}]
	--assert error? try [ascii? #{}]

--test-- "copy/part binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/138
	b: make binary! 10
	--assert #{} = copy/part b 1
	--assert #{} = copy/part b 100
	b: #{01020304}
	--assert #{0102} = copy/part b 2
	--assert #{01020304} = copy/part b 100

--test-- "to-string to-binary"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1517
	--assert "^M" = to-string to-binary "^M"
	--assert "^/" = to-string to-binary "^/"
	--assert "^M^/"  = to-string to-binary "^M^/"
	--assert #{0D0A} = to-binary to-string #{0D0A}

===end-group===


===start-group=== "BLOCK"

--test-- "path in block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/26
	b: [b 1]
	--assert 1 = b/b

--test-- "random next block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1875
	--assert block?   random      next [1 2]
	--assert integer? random/only next [1 2]

===end-group===


===start-group=== "SPLIT"

--test-- "split block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2051
	b: [a b c d e f]
	--assert [[a b c d e f]]              = split/skip b 1
	--assert [[a b c] [d e f]]            = split/skip b 2
	--assert [[a b] [c d] [e f]]          = split/skip b 3
	--assert [[a] [b] [c] [d e f]]        = split/skip b 4
	--assert [[a] [b] [c] [d] [e f]]      = split/skip b 5
	--assert [[a] [b] [c] [d] [e] [f]]    = split/skip b 6
	--assert [[a] [b] [c] [d] [e] [f] []] = split/skip b 7

--test-- "split string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1886
	--assert ["a" "b" "c"]    = split "a.b.c"  "."
	--assert ["a" "b" "c" ""] = split "a.b.c." "."
	--assert ["1234" "5678" "1234" "5678"] = split "1234567812345678" 4
	--assert ["123" "456" "781" "234" "567" "8"] = split "1234567812345678" 3
	--assert ["abc" "de" "fghi" "jk"] = split "abc<br>de<br>fghi<br>jk" <br>
	--assert ["abc" "de" "fghi" "jk"] = split "abc|de/fghi:jk" charset "|/:"
	--assert ["abc" "de" "fghi" "jk"] = split "abc^M^Jde^Mfghi^Jjk" [crlf | #"^M" | newline]
	--assert ["abc" "de" "fghi" "jk"] = split "abc     de fghi  jk" [some #" "]
	--assert ["12345678" "12345678"] = split/skip "1234567812345678" 2
	--assert ["12345" "67812" "345678"] = split/skip "1234567812345678" 3
	--assert ["123" "456" "781" "234" "5678"] = split/skip "1234567812345678" 5
	;@@ https://github.com/Oldes/Rebol-issues/issues/573
	--assert ["c" "c"] = split "c c" " "
	--assert ["1,2"]   = split "1,2" " "
	--assert ["c" "c "] = split "c,c " ","

===end-group===


===start-group=== "UNION"

--test-- "union on 2 strings"
	;@@ https://github.com/Oldes/Rebol-issues/issues/400
	--assert "123" = union "12" "13"
--test-- "union with none and unset"
	--assert [#[none!] #[unset!]] = union [#[none!]] [#[unset!]]

--test-- "union/skip with negative skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/736
	--assert all [
		error? e: try [union/skip [2 1][2 1] -2]
		e/id = 'out-of-range
	]

===end-group===

===start-group=== "UNIQUE & DEDUPLICATE"

--test-- "unique on string"
	;@@ https://github.com/Oldes/Rebol-issues/issues/402
	--assert "123" = unique "123"
	--assert "123" = unique s: "123123"
	--assert s = "123123"                    ;- unique does not modify its input
	--assert "123" = deduplicate s: "123123" ;- while deduplicate does
	--assert "123" = s                       ;- https://github.com/Oldes/Rebol-issues/issues/1573
--test-- "unique/skip on string"
	--assert "abca"   = unique/skip "ababca" 2
	--assert "ababca" = unique/skip s: "ababcaaba" 3
	--assert s = "ababcaaba"
	--assert "ababca" = deduplicate/skip s: "ababcaaba" 3
	--assert "ababca" = s

--test-- "unique on block"
	--assert [1 2] = unique [1 2]
	--assert [1 2] = unique b: [1 2 2 1]
	--assert b = [1 2 2 1]                   ;- unique does not modify its input
	--assert [1 2] = deduplicate b: [1 2 2 1]
	--assert [1 2] = b                       ;- while deduplicate does
--test-- "unique/skip on block"
	--assert [1 2 3 4] = unique/skip b: [1 2 1 2 3 4] 2
	--assert b = [1 2 1 2 3 4]
	--assert [1 2 3 4] = deduplicate/skip b: [1 2 1 2 3 4] 2
	--assert [1 2 3 4] = b

--test-- "unique with unset and none"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1124
	;@@ https://github.com/Oldes/Rebol-issues/issues/1592
	--assert [#[unset!] #[none!]] = unique reduce [unset 'a unset 'a none none]

===end-group===

===start-group=== "INTERSECT"
--test-- "intersect"
	--assert [#[none] 1 #[unset!]] = intersect [#[none] 1 #[unset!]] [#[none] #[unset!] 1]
	--assert [] = intersect [#[none]] [1 #[unset!]]

===end-group===

===start-group=== "EXTRACT"
--test-- "extract with unset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1540
	--assert [5 1 #[unset!]] = extract [5 3 1 #[unset!] #[unset!] #[unset!]] 2

===end-group===

===start-group=== "More set operations"	
--test-- "unhashable types"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1765
	blk1: reduce ["a" [1 2 3] 'path/path func [][] charset ["a"]]
	blk2: append copy blk1 blk1
	--assert blk1 = unique blk2
	--assert blk1 = union blk1 blk2 
	append blk2 blk3: ["b" [3 4]]
	--assert blk1 = intersect  blk2 blk1
	--assert blk3 = difference blk1 blk2
	--assert blk3 = exclude blk2 blk1
	--assert empty? exclude blk1 blk2

--test-- "set ops on binary"
	;@@ https://github.com/Oldes/Rebol-issues/issues/837
	bin1: #{010203}
	bin2: #{010203010203}
	--assert bin1 = unique bin2
	--assert bin1 = union  bin1 bin2 
	append bin2 bin3: #{0405}
	--assert bin1 = intersect  bin2 bin1
	--assert bin3 = difference bin1 bin2
	--assert bin3 = exclude bin2 bin1
	--assert empty? exclude bin1 bin2

===end-group===

===start-group=== "TO-*"

--test-- "to-path"
	--assert (mold to-path [1 2 3]) = "1/2/3"
	--assert (mold to-path [1 none 3]) = "1/none/3"
	;@@ https://github.com/Oldes/Rebol-issues/issues/477
	--assert path? p: try [to-path b: [1 #[none] #[true] [] () #{}]]
	--assert integer? p/1
	--assert none?    p/2
	--assert true?    p/3
	--assert block?   p/4
	--assert paren?   p/5
	--assert binary?  p/6
	--assert b = to-block p

--test-- "to-string"
	;@@ https://github.com/Oldes/Rebol-issues/issues/207
	--assert "^@" = to-string to-char 0
	--assert "^@" = to-string #"^(00)"
	--assert "^@" = to-string #"^@"

--test-- "to-ref"
	--assert @123 = to-ref 123
	--assert @1.3 = to-ref 1.3
	--assert @1x3 = to-ref 1x3
	--assert @abc = to-ref "abc"
	--assert @ščř = to-ref "ščř"
	--assert @ščř = to-ref to-binary "ščř"
	--assert 2 = length? to-ref #{0102} ; should this be allowed?

===end-group===


===start-group=== "NEW-LINE"
--test-- "new-line paren!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1940
	foo: quote (a b c)
	--assert not new-line? next foo
	new-line next foo true
	--assert new-line? next foo

===end-group===

===start-group=== "ARRAY"
--test-- "array"
	--assert [#[none] #[none]] = array 2
--test-- "array/initial"
	--assert [0 0] = array/initial 2 0
--test-- "array/initial func"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2193
	--assert [10 9 8 7 6 5 4 3 2 1] = array/initial length: 10 func [] [-- length]
	--assert [["1 1" "1 2"] ["2 1" "2 2"]] = array/initial [2 2] func [x y] [ajoin [x " " y]]
===end-group===


===start-group=== "REWORD"
--test-- "reword"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1990

	--assert (reword/escape "ba" [a 1 b 2] none)
		== "21"  ; /escape none like /escape ""

	--assert (reword "$a$A$a" [a 1 A 2])
		== "222"  ; case-insensitive, last value wins
	--assert (reword/case "$a$A$a" [a 1 A 2])
		== "121"  ; case-sensitive, even with words

	--assert (reword/escape "<bang>" [bang "!"] ["<" ">"])
		== "!"  ; Leading and trailing delimiters considered
	--assert (reword/escape "<bang>bang>" [bang "!"] ["<" ">"])
		== "!bang>"  ; One-pass, continues after the replacement
	--assert (reword/escape "!bang;bang;" [bang "!"] ["!"])
		== "!;bang;"  ; No trailing delimiter specified

	--assert (reword "$a" ["a" ["A" "B" "C"]])
		== "C"  ; evaluate a block value as code by default
	--assert (reword/only "$a" ["a" ["A" "B" "C"]])
		== "ABC"  ; Just insert a block value with /only

	--assert (reword "$a$b$+" [a 1 + 1 b 2 + 2])
		== "24$+"  ; Value expressions evaluated, keys treated as literals
	--assert (reword/only "$a$b$+" [a 1 + 1 b 2 + 2])
		== "122"   ; With /only, treated as raw values, + keyword defined twice in this case
	--assert (reword "$a ($b)" ["a" does ["AAA"] "b" ()])
		== "AAA ($b)"  ; Evaluates function builders and parens too. Note that b was undefined because () evaluates to unset
	--assert (reword/only "$a ($b)" reduce ["a" does ["AAA"] "b" ["B" "B" "B"]])
		== "AAA (BBB)"  ; With /only of explicitly reduced spec, functions still called even though blocks are just inserted

	--assert (reword "$a" [a: 1])
		== "1"  ; It should be easy to switch from block to object specs
	--assert (reword "$a" context [a: 1])
		== "1"  ; ... like this, so we should special-case set-words
	--assert (reword "$a" ['a 1])
		== "1"  ; It should be easy to explicitly reduce the same spec
	--assert (reword "$a" reduce ['a 1])
		== "1"  ; ... like this, so we should special-case lit-words
	--assert (reword/escape "a :a /a #a" [a 1 :a 2 /a 3 #a 4] none)
		== "1 2 3 4"  ; But otherwise let word types be distinct

	--assert (reword to-binary "$a$A$a" [a 1 A 2])
		== #{010201}  ; binaries supported, note the case-sensitivity, same key rules, values inserted by binary rules
	--assert (tail? reword/into to-binary "$a$A$a" [a 1 A 2] #{})
		== true  ; /into option behaves the same
	--assert (head reword/into "$a$A$a" [a 1 A 2] #{})
		== #{020202}  ; string templates can insert into binaries, note the case-insensitivity
	--assert (head reword/case/into "$a$A$a" [a 1 A 2] #{})
		== #{010201}  ; ... and this time it's case-sensitive
	--assert (head reword/into to-binary "b$a$A$ac" [a 1 A 2] "")
		== "b121c"  ; Binary templates can insert into strings, using string insert rules, still case-sensitive

===end-group===

;-- VECTOR related tests moved to %vector-test.r3

~~~end-file~~~
