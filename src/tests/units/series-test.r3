Rebol [
	Title:   "Rebol series test script"
	Author:  "Oldes"
	File: 	 %series-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Series"

===start-group=== "Merging series"
--test-- "JOIN"
	--assert "ab"     == join  'a  'b
	--assert "ab"     == join  "a" "b"
	--assert %ab      == join  %a  "b"
	--assert "ab"     == join #"a" "b"
	--assert <ab>     == join  <a> "b"
	--assert "ab3"    == join  'a  ['b  3]
	--assert "ab3"    == join  "a" ["b" 3]
	--assert %ab3     == join  %a  ["b" 3]
	--assert "ab3"    == join #"a" ["b" 3]
	--assert <ab3>    == join  <a> ["b" 3]
	--assert "anone"  == join  "a" none
	--assert %anone   == join  %a  none
	--assert "anone"  == join #"a" none
	--assert error? try [join  "a" #[unset]]
	--assert error? try [join  %a  #[unset]]
	--assert error? try [join #"a" #[unset]]

;@@ https://github.com/Oldes/Rebol-issues/issues/2558
--test-- "AJOIN"
	--assert "ab3"    == ajoin [ 'a  'b  3]
	--assert "ab3"    == ajoin [ "a" "b" 3]
	--assert %ab3     == ajoin [ %a  "b" 3]
	--assert "ab3"    == ajoin [#"a" "b" 3]
	--assert "<a>b3"  == ajoin [ <a> "b" 3] ;; by design not a tag!
	--assert "a3"     == ajoin [ "a" #[none]  3]
	--assert %a3      == ajoin [ %a  #[none]  3]
	--assert "a3"     == ajoin [#"a" #[none]  3]
	--assert "a3"     == ajoin [ "a" #[unset] 3]
	--assert %a3      == ajoin [ %a  #[unset] 3]
	--assert "a3"     == ajoin [#"a" #[unset] 3]
	;; when first value is not a string, result is always string
	--assert "a3"     == ajoin [#[none]  "a"  3]
	--assert "a3"     == ajoin [#[none]  %a   3]
	--assert "a3"     == ajoin [#[none] #"a"  3]
	;; nested ajoin
	--assert "1234"   == ajoin [1 2 ajoin [3 4]]

--test-- "AJOIN/all"
	--assert "ab3"     == ajoin/all [ 'a  'b   3]
	--assert "ab3"     == ajoin/all [ "a" "b"  3]
	--assert %ab3      == ajoin/all [ %a  "b"  3]
	--assert "ab3"     == ajoin/all [#"a" "b"  3]
	--assert "anone3"  == ajoin/all [ "a" #[none]  3]
	--assert %anone3   == ajoin/all [ %a  #[none]  3]
	--assert "anone3"  == ajoin/all [#"a" #[none]  3]
	--assert "a3"      == ajoin/all [ "a" #[unset] 3]
	--assert %a3       == ajoin/all [ %a  #[unset] 3]
	--assert "a3"      == ajoin/all [#"a" #[unset] 3]
	;; when first value is not a string, result is always string
	--assert "nonea3"  == ajoin/all [#[none]  "a"  3]
	--assert "nonea3"  == ajoin/all [#[none]  %a   3]
	--assert "nonea3"  == ajoin/all [#[none] #"a"  3]

--test-- "AJOIN/with"
	--assert "a/b/3"   == ajoin/with [ 'a  'b       3] #"/"
	--assert "a/b/3"   == ajoin/with [ "a" "b"      3] #"/"
	--assert %a/b/3    == ajoin/with [ %a  "b"      3] #"/"
	--assert "a/b/3"   == ajoin/with [#"a" "b"      3] #"/"
	--assert "<a>/b/3" == ajoin/with [ <a> "b"      3] #"/" ;; by design not a tag!
	--assert "a/3"     == ajoin/with [ "a" #[none]  3] #"/"
	--assert %a/3      == ajoin/with [ %a  #[none]  3] #"/"
	--assert "a/3"     == ajoin/with [#"a" #[none]  3] #"/"
	--assert "a/3"     == ajoin/with [ "a" #[unset] 3] #"/"
	--assert %a/3      == ajoin/with [ %a  #[unset] 3] #"/"
	--assert "a/3"     == ajoin/with [#"a" #[unset] 3] #"/"

--test-- "AJOIN/all/with"
	--assert "a/b/3"    == ajoin/all/with [ 'a  'b       3] #"/"
	--assert "a/b/3"    == ajoin/all/with [ "a" "b"      3] #"/"
	--assert %a/b/3     == ajoin/all/with [ %a  "b"      3] #"/"
	--assert "a/b/3"    == ajoin/all/with [#"a" "b"      3] #"/"
	--assert "a/none/3" == ajoin/all/with [ "a" #[none]  3] #"/"
	--assert %a/none/3  == ajoin/all/with [ %a  #[none]  3] #"/"
	--assert "a/none/3" == ajoin/all/with [#"a" #[none]  3] #"/"
	--assert "a//3"     == ajoin/all/with [ "a" #[unset] 3] #"/"
	--assert %a//3      == ajoin/all/with [ %a  #[unset] 3] #"/"
	--assert "a//3"     == ajoin/all/with [#"a" #[unset] 3] #"/"

--test-- "FORM"
	--assert "a b 3"    == form [ 'a  'b       3]
	--assert "a b 3"    == form [ "a" "b"      3]
	--assert "a b 3"    == form [ %a  "b"      3]
	--assert "a b 3"    == form [#"a" "b"      3]
	--assert "<a> b 3"  == form [ <a> "b"      3]
	--assert "a none 3" == form [ "a" #[none]  3]
	--assert "a none 3" == form [ %a  #[none]  3]
	--assert "a none 3" == form [#"a" #[none]  3]
	--assert "a  3"     == form [ "a" #[unset] 3]
	--assert "a  3"     == form [ %a  #[unset] 3]
	--assert "a  3"     == form [#"a" #[unset] 3]
	;@@ https://github.com/Oldes/Rebol-issues/issues/2560
	--assert "  1  2"   == form ["" "" 1 "" 2]
	--assert "  1  2"   == form [#[unset] #[unset] 1 #[unset] 2]
===end-group===



===start-group=== "FIND & SELECT"

--test-- "SELECT or FIND NONE! anything == none - #473"
	--assert none? find none 1
	--assert none? select none 1

--test-- "FIND"
	--assert none? find/part [x] 'x 0
	--assert equal? [x] find/part [x] 'x 1
	--assert equal? [x] find/reverse tail [x] 'x
	--assert equal? [x y] find/match [x y] 'x
	--assert equal? [y] find/match/tail [x y] 'x
	--assert equal? [x] find/last [x] 'x
	--assert equal? [x] find/last [x x x] 'x

--test-- "FIND string! integer!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/237
	--assert "23" = find "123" 2

--test-- "FIND string! binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1159
	--assert "F00D"  = find "id: F00D" #{F00D}
	--assert "F00D"  = find "id: F00D" #{f00d}

--test-- "FIND binary! binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1161
	--assert #{0001} = find/match #{0001} #{00}
	--assert #{01}   = find/match/tail #{0001} #{00}
	--assert #{02}   = find #{000102} #{02}
	--assert #{}     = find/tail #{000102} #{02}
	--assert     none? find/match #{0001} #{01}
	--assert     none? find/match/tail #{0001} #{01}
	--assert     none? find #{000102} #{03}
	--assert     none? find/tail #{000102} #{03}

--test-- "FIND binary! char!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1161
	--assert tail? find/tail #{0063} #"c"
	--assert tail? find/tail #{0063} #"^(63)"
	--assert tail? find/tail #{00FF} #"^(ff)"
	--assert none? find/tail #{0063} #"C"
	--assert none? find/tail #{0063} #"^(700)"

--test-- "FIND binary! integer!"
	--assert tail? find/tail #{0063} 99
	--assert error? try [find/tail #{0063} 700]

--test-- "FIND string! tag!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1160
	--assert "<a>"  = find "<a>" <a>
	--assert "b"    = find/tail "<a>b" <a>
	--assert "<a>3" = find/last "1<a>2<a>3" <a>
	--assert "<a>b" = find/match "<a>b" <a>
	--assert "b"    = find/match/tail "<a>b" <a>
	--assert "<a>b" = find/match next "a<a>b" <a>
	--assert "<a>b" = find/reverse tail "a<a>b" <a>
	--assert none?    find/skip "a<a>b" <a> 2
	--assert "<a>b" = find/skip "aa<a>b" <a> 2
	--assert "<A>"  = find/case "<a><A>" <A>
	--assert "<a href=''>" = find "foo<a href=''>" <a href=''>

--test-- "FIND %file %file"
	;@@ https://github.com/Oldes/Rebol-issues/issues/624
	--assert %file = find %file %file

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
	--assert "abc"   = find/any/match "abc" "a?c"
	--assert "abcdef"= find/any/match "abcdef" "a*e?"
	--assert ""      = find/any/match/tail "abc" "a?c"
	--assert ""      = find/any/match/tail "abcdef" "a*e?"
	--assert "bcd"   = find/any/reverse tail "abcdabcd" "?c"
	--assert "d"     = find/any/reverse/tail tail "abcdabcd" "?c"
	--assert "d"     = find/any/reverse/tail tail "abcdabcd" "bc"
	--assert %abcabc  = find/any %abcabc %*bc
	--assert %abxabc  = find/any %abxabc %*bc
	--assert %abcabc  = find/any %abcabc %ab*
	--assert %cxbc    = find/any %abcxbc %c*bc
	--assert %cxbc    = find/any %abcxbc %c?bc
	--assert none?      find/any %abxabc %c*bc
	--assert none?      find/any %abcxxbc %c?bc
	--assert %cxxbc   = find/any %abcxxbc %c??bc
	--assert %cxxbcx  = find/any %abcxxbcx %c??bc
	--assert %x       = find/any/tail %abcxxbcx %c??bc
	--assert %abc     = find/any/tail %abcabc %*bc
	--assert %""      = find/any/tail %abxabc %*bc
	--assert "abxcd"  = find/any "abxcd" "ab*cd"
	--assert "abxxcd" = find/any "abxxcd" "ab*cd"
	--assert none?      find/any "abxcx" "ab*cd"
	--assert "abxcx"  = find/any "abxcx" "ab*c?"
	--assert "abxcxe" = find/any "abxcxe" "ab*c?e"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2522
	--assert %A.csv   = find/any %A.csv %A*.csv
	--assert %A..csv  = find/any %A..csv %A*.csv
	--assert %A..csv  = find/any %xA..csv %A*.csv
	--assert none?      find/any/match %xA..csv %A*.csv
	;@@ https://github.com/Oldes/Rebol-issues/issues/2528
	--assert %AA.BB.csv = find/any %AA.BB.csv %A*.csv
	--assert %BB.csv    = find/any %AA.BB.csv %B*.csv
	--assert %AA.BB.csv = find/any %AA.BB.csv %A*.*.csv
	;@@ https://github.com/Oldes/Rebol-issues/issues/2529
	--assert %ab        = find/any %ab %*b
	--assert %ab        = find/any/match %ab %*b
	--assert %a1a2      = find/any %a1a2 %*a2
	--assert %a1a2      = find/any/match %a1a2 %*a2
	--assert %x         = find/any/tail %abx %*b
	--assert %x         = find/any/tail %a1a2x %*a2


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
	--assert "ažc"   = find/any/match "ažc" "a?c"
	--assert "ažcdef"= find/any/match "ažcdef" "a*e?"
	--assert ""      = find/any/match/tail "ažc" "a?c"
	--assert ""      = find/any/match/tail "ažcdef" "a*e?"
	--assert "žcd"   = find/any/reverse tail "ažcdažcd" "?c"
	--assert "d"     = find/any/reverse/tail tail "ažcdažcd" "?c"
	--assert "d"     = find/any/reverse/tail tail "ažcdažcd" "žc"

--test-- "FIND/SAME in block!"
;@@ https://github.com/Oldes/Rebol-issues/issues/1720
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
	--assert 1 = index? find/same [1 1.0] 1
	--assert 2 = index? find/same [1 1.0] 1.0
	--assert 2 = index? find/same [1.0 1] 1
	--assert 1 = index? find/same [1.0 1] 1.0

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

--test-- "FIND/REVERSE/MATCH"
;@@ https://github.com/Oldes/Rebol-issues/issues/2328
	--assert none? find/reverse/match tail "abc" "abc"
	--assert none? find/reverse/match tail "abc" "cba"

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

--test-- https://github.com/Oldes/Rebol-issues/issues/1611
	--assert none? try [index? none]
--test-- https://github.com/Oldes/Rebol-issues/issues/1626
	--assert none? try [length? none]
--test-- https://github.com/Oldes/Rebol-issues/issues/473
	--assert none? try [select none 1]
	--assert none? try [  find none 1]

--test-- "SELECT"
	--assert 2 = select/part [1 2 1 3 1 2] 1 2
	--assert none? select/part [1 2 1 3 1 2] 2 2
	--assert 3 = select/part (skip [1 2 1 3 1 2] 2) 1 2
	;@@ https://github.com/Oldes/Rebol-issues/issues/1936
	;@@ https://github.com/Oldes/Rebol-issues/issues/686
	--assert 9 = select [1 2 3 4 5 6 6 6 6 6 1 2 3 4 5 6 7 8 9 0 1] [6 7 8] 

--test-- "SELECT/skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/101
	--assert none? select/skip [1 2 3 4 5 6] 5 3
	;@@ https://github.com/Oldes/Rebol-issues/issues/734
	--assert all [
		error? e: try [select/skip [1 2 3 4 5 6] 5 -4]
		e/id = 'out-of-range
	]
	;@@ https://github.com/Oldes/Rebol-issues/issues/735
	--assert all [
		error? e: try [find/skip [1 2 3 4 5 6] 5 -4]
		e/id = 'out-of-range
	]
	;@@ https://github.com/Oldes/Rebol-issues/issues/730
	tbl: [a a x  b b y  c c z]
	--assert 'c = select/skip tbl 'c 3
	--assert 'y = select/skip next tbl 'b 3
	--assert 'y = select/skip tbl [b b] 3
	tbl: [[a a] x  [b b] y  [c c] z]
	--assert 'y = select/skip/only tbl [b b] 2
	--assert 'y = select/only tbl [b b] ; this one is faster than above!

--test-- "SELECT/skip/last"
	;@@ https://github.com/Oldes/Rebol-issues/issues/616
	--assert 'b = select/skip [a b a c] 'a 2
	--assert 'c = select/skip/last [a b a c] 'a 2

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

--test-- "SUPPLEMENT"
	;@@ https://github.com/Oldes/Rebol-wishes/issues/11
	b: []
	--assert [%a]         = supplement b %a
	--assert [%a %b]      = supplement b %b
	--assert [%a %b]      = supplement b %a
	--assert [%a %b #"a"] = supplement b #"a"
	--assert [%a %b #"a"] = supplement b #"A" ; case-insensitive
	--assert [%a %b #"a" #"A"] = supplement/case b #"A"

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


===start-group=== "REDUCE"
	--test-- "reduce block!"
		--assert [2 6] == reduce [1 + 1 3 + 3]
		--assert all [
			[x] == reduce/into [1 + 1 3 + 3] b: [x]
			b = [2 6 x]
		]
		--assert all [
			tail? reduce/into ['a 1 + 1 3 + 3] p: make path! 3
			p = 'a/2/6
		]
	--test-- "reduce paren!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2060
		--assert 2 = reduce (1 + 1)
		--assert quote (2 6) == reduce quote (1 + 1 3 + 3)
		--assert all [
			[x] == reduce/into quote (1 + 1 3 + 3) b: [x]
			b = [2 6 x]
		]
		--assert all [
			tail? reduce/into quote ('a 1 + 1 3 + 3) p: make path! 3
			p = 'a/2/6
		]
	--test-- "reduce/only"
		words: [a b] set words [1 2]
		not-words: [a]
		--assert [1 2] = reduce words
		--assert [a 2] = reduce/only words not-words
		;@@ https://github.com/Oldes/Rebol-issues/issues/1771
		--assert all [error? e: try [reduce/only [a no-such-word] []] e/id = 'no-value]
===end-group===


===start-group=== "TRIM"
	--test-- "trim string!"
		str1: " a b c "
		str2: " ^(A0) ^-a b  ^- c  ^(2000) "
		mstr: {   a ^-1^/    ab2^-  ^/  ac3  ^/  ^/^/}
		--assert "a b c"  = trim copy str1
		--assert "a b c"  = trim/head/tail copy str1
		--assert "a b c " = trim/head copy str1
		--assert " a b c" = trim/tail copy str1
	;	--assert "a b  ^- c" = trim copy str2 ;- not like Red!
		--assert "a ^-1^/ab2^/ac3^/" = trim copy mstr
		--assert "a1ab2ac3" = trim/all { a ^-1^/ ab2^- ^/ ac3 ^/ ^/^/}
		--assert "    ^-1^/    b2^-  ^/  c3  ^/  ^/^/" = trim/with copy mstr #"a"
		--assert "    ^-1^/    b2^-  ^/  c3  ^/  ^/^/" = trim/with copy mstr 97
	--test-- "trim binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1482
		bin: #{0011001100}
		--assert #{110011}   = trim           copy bin
		--assert #{110011}   = trim/head/tail copy bin
		--assert #{11001100} = trim/head      copy bin
		--assert #{00110011} = trim/tail      copy bin
		--assert #{1111}     = trim/all       copy bin
		--assert #{000000}   = trim/all/with  copy bin #{11}
		--assert #{} = trim      #{0000}
		--assert #{} = trim/tail #{0000}
		--assert #{} = trim/head #{0000}
		--assert #{2061626320} = trim/head/tail to-binary " abc "
	--test-- "trim binary! incompatible"
		--assert all [error? e: try [trim/auto  #{00}] e/id = 'bad-refines]
		--assert all [error? e: try [trim/lines #{00}] e/id = 'bad-refines]
		--assert all [error? e: try [trim/head/all #{00}] e/id = 'bad-refines]
		--assert all [error? e: try [trim/tail/all #{00}] e/id = 'bad-refines]
		--assert all [error? e: try [trim/tail/with #{00} 0] e/id = 'bad-refines]
		--assert all [error? e: try [trim/head/with #{00} 0] e/id = 'bad-refines]
	--test-- "trim binary! with index > 1"
		bin: #{0000110000}
		--assert #{00001100} = head trim/tail at copy bin 5
		--assert #{00110000} = head trim/head at copy bin 2
		--assert #{0011}     = head trim/all  at copy bin 2
	--test-- "trim block!"
		blk: [#[none] 1 #[none] 2 #[none]]
		;@@ https://github.com/Oldes/Rebol-issues/issues/825
		--assert [1 #[none] 2 #[none]] = trim/head copy blk
		--assert [#[none] 1 #[none] 2] = trim/tail copy blk
		;@@ https://github.com/Oldes/Rebol-issues/issues/2482
		--assert [1 #[none] 2] = trim     copy blk
		--assert [1 2]         = trim/all copy blk
		--assert all [error? e: try [trim/head/all []] e/id = 'bad-refines]
		--assert all [error? e: try [trim/tail/all []] e/id = 'bad-refines]

===end-group===


===start-group=== "TRUNCATE"
--test-- "TRUNCATE"
	--assert "23"  = truncate next "123"
	--assert [2 3] = truncate next [1 2 3]
	--assert "2"   = truncate/part next "123" 1
	--assert [2]   = truncate/part next [1 2 3] 1
	--assert "23"  = head truncate next "123"
	--assert [2 3] = head truncate next [1 2 3]
	--assert "2"   = head truncate/part next "123" 1
	--assert [2]   = head truncate/part next [1 2 3] 1
--test-- "TRANSCODE truncate"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2571
	bin: truncate/part to-binary "2x" 1  ;== #{32}
	--assert all [
		1 = length? bin
		[2] = try [transcode bin]
	]
===end-group===

===start-group=== "REPLACE string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/54
	--test-- "issue-54"
		--assert "ABCDE456" = replace "123456" "123" "ABCDE"
		--assert "1!!2!!3"  = replace/all "1 2 3" " " "!!"

	--test-- "overlapping replace"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1162
		s: copy "abcde"
		--assert "bcdee" = replace/all s copy/part s 4 skip s 1
		
===end-group===

===start-group=== "REPLACE binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1499
	--test-- "issue-1499"
		--assert #{6163} = replace #{616263} #{6263} #{63}
===end-group===

===start-group=== "PATH"
	--test-- "get on path"
	;@@ https://github.com/Oldes/Rebol-issues/issues/248
		--assert tuple? get 'system/version
===end-group===

===start-group=== "SET-PATH"
	--test-- "set-path missing value"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2312
		data: copy [a 10 b 20]
		data/a: 30
		--assert data = [a 30 b 20] ; this one is ok
		; but rest are errors by design
		--assert all [
			error? e: try [data/c: 30]
			e/id = 'invalid-path
		]
		--assert all [
			data: [1 c]
			error? e: try [data/c: 30]
			e/id = 'invalid-path
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

	--test-- "overlapping change"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1162
		s: copy "12345"
		--assert     "5" = change at s 1 skip s 1 ;change returns just past the change
		--assert "23455" = s

===end-group===

===start-group=== "APPEND block!"
	--test-- "self-append"
		;@@ https://github.com/Oldes/Rebol-issues/issues/814
		a: copy [1]
		--assert [1 1] = append a a
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

===start-group=== "INSERT ref!"
	--test-- "INSERT ref! char!"
		--assert @a = head insert @ #"a"
		--assert @← = head insert @ #"^(2190)" ; wide char
	--test-- "INSERT ref! string!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/700
		--assert @ = i: @
		--assert @ = insert i "abc"
		--assert i = @abc
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
	--test-- "APPEND binary! block!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1452
		--assert #{00010361} = append #{} [#{00} #{01} 3 #"a"]
		--assert   error? try [append #{} [300]]

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
	--test-- "INSERT binary! block!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1452
		--assert #{00010361} = head insert #{} [#{00} #{01} 3 #"a"]
		--assert   error? try [insert #{} [300]]
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

	--test-- "CHANGE binary! tuple!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2576
		--assert all [
			#{00} = change b: #{00000000} 1.2.3
			b = #{01020300}
		]
		--assert all [
			tail? change b: #{00000000} 1.2.3.4.5.6
			b = #{010203040506}
		]
		--assert all [
			#{CCDD} = change/part b: #{AABBCCDD} 1.2.3.4 2
			b = #{01020304CCDD}
		]
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

	--test-- "take/part string! with negative part"
	;@@ https://github.com/red/red/issues/4078
	s: "123" --assert ""    == take/part      s    -1
	s: "123" --assert "1"   == take/part skip s 1  -1
	s: "123" --assert "2"   == take/part skip s 2  -1
	s: "123" --assert "3"   == take/part skip s 3  -1
	s: "123" --assert "1"   == take/part skip s 1  -2
	s: "123" --assert "12"  == take/part skip s 2  -2
	s: "123" --assert "23"  == take/part skip s 3  -2
	s: "123" --assert "1"   == take/part skip s 1  -3
	s: "123" --assert "12"  == take/part skip s 2  -3
	s: "123" --assert "123" == take/part skip s 3  -3
	s: "123" --assert "1"   == take/part      s    skip s 1
	s: "123" --assert "1"   == take/part skip s 1       s
	s: "123" --assert "2"   == take/part skip s 1  skip s 2
	s: "123" --assert "2"   == take/part skip s 2  skip s 1
	s: "123" --assert "3"   == take/part skip s 2  skip s 3
	s: "123" --assert "3"   == take/part skip s 3  skip s 2
	s: "123" --assert "12"  == take/part      s    skip s 2
	s: "123" --assert "12"  == take/part skip s 2       s
	s: "123" --assert "23"  == take/part skip s 1  skip s 3
	s: "123" --assert "23"  == take/part skip s 3  skip s 1
	s: "123" --assert "123" == take/part      s    skip s 3
	s: "123" --assert "123" == take/part skip s 3       s

	--test-- "take/part any-block!"
		--assert [1 2] = take/part [1 2 3 4] 2
		;@@ https://github.com/Oldes/Rebol-issues/issues/2174
		--assert "(1 2)" = mold take/part quote (1 2 3 4) 2
		--assert "a/b"   = mold take/part quote a/b/c 2
		--assert "'a/b"  = mold take/part quote 'a/b/c 2
		--assert ":a/b"  = mold take/part quote :a/b/c 2
		--assert "a/b:"  = mold take/part quote a/b/c: 2

	--test-- "take/part block! with negative part"
	;@@ https://github.com/red/red/issues/4078
	s: [1 2 3] --assert [ ]     == take/part      s    -1
	s: [1 2 3] --assert [1]     == take/part skip s 1  -1
	s: [1 2 3] --assert [2]     == take/part skip s 2  -1
	s: [1 2 3] --assert [3]     == take/part skip s 3  -1
	s: [1 2 3] --assert [1]     == take/part skip s 1  -2
	s: [1 2 3] --assert [1 2]   == take/part skip s 2  -2
	s: [1 2 3] --assert [2 3]   == take/part skip s 3  -2
	s: [1 2 3] --assert [1]     == take/part skip s 1  -3
	s: [1 2 3] --assert [1 2]   == take/part skip s 2  -3
	s: [1 2 3] --assert [1 2 3] == take/part skip s 3  -3
	s: [1 2 3] --assert [1]     == take/part      s    skip s 1
	s: [1 2 3] --assert [1]     == take/part skip s 1       s
	s: [1 2 3] --assert [2]     == take/part skip s 1  skip s 2
	s: [1 2 3] --assert [2]     == take/part skip s 2  skip s 1
	s: [1 2 3] --assert [3]     == take/part skip s 2  skip s 3
	s: [1 2 3] --assert [3]     == take/part skip s 3  skip s 2
	s: [1 2 3] --assert [1 2]   == take/part      s    skip s 2
	s: [1 2 3] --assert [1 2]   == take/part skip s 2       s
	s: [1 2 3] --assert [2 3]   == take/part skip s 1  skip s 3
	s: [1 2 3] --assert [2 3]   == take/part skip s 3  skip s 1
	s: [1 2 3] --assert [1 2 3] == take/part      s    skip s 3
	s: [1 2 3] --assert [1 2 3] == take/part skip s 3       s

	--test-- "take/last"
	;@@ https://github.com/Oldes/Rebol-issues/issues/177
	--assert #"c" = take/last str: "abc"
	--assert "ab" = str
	--assert 3    = take/last blk: [1 2 3]
	--assert [1 2] = blk
	;@@ https://github.com/Oldes/Rebol-issues/issues/2542
	--assert none? take/last tail "123"
	--assert none? take/last tail [1 2 3]
	--assert "123"   = take/last/part tail "123" -3
	--assert ""      = take/last/part tail "123"  3
	--assert [1 2 3] = take/last/part tail [1 2 3] -3
	--assert []      = take/last/part tail [1 2 3]  3

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


===start-group=== "EXTEND"
	--test-- "extend object!"
		--assert all [1 == extend  o: object[] 'a 1  o = #[object! [a: 1]]]
	--test-- "extend block!"
		--assert all [1 == extend  b: [] 'a 1  b = [a: 1]]
	--test-- "extend paren!"
		--assert all [1 == extend  b: quote () 'a 1  b = quote (a: 1)]
	--test-- "extend map!"
		--assert all [1 == extend  m: #() 'a 1  m = #(a: 1)]
===end-group===


===start-group=== "MOVE"
	--test-- "move/skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1629
	--assert [b 3 4 a 1 2] = head move/skip at [a 1 2 b 3 4] 4 -1 3
	--assert [b 3 4 a 1 2] = head move/to/skip at [a 1 2 b 3 4] 4 1 3
===end-group===

===start-group=== "FIND-MAX / FIND-MIN"
	;@@ https://github.com/Oldes/Rebol-issues/issues/754
	;@@ https://github.com/Oldes/Rebol-issues/issues/755
	b: [1 2 3 -1]
	--test-- "FIND-MAX block!" --assert  3 = first find-max b
	--test-- "FIND-MIN block!" --assert -1 = first find-min b
	b: [1 a 2 b 3 c -1 d]
	--test-- "FIND-MAX/skip block!"
		--assert  3 = first find-max/skip b 2
		;@@ https://github.com/Oldes/Rebol-issues/issues/738
		--assert all [
			error? e: try [find-max/skip b 0]
			e/id = 'out-of-range
		]
		--assert all [
			error? e: try [find-max/skip b -2]
			e/id = 'out-of-range
		]
	--test-- "FIND-MIN/skip block!"
		--assert -1 = first find-min/skip b 2
		;@@ https://github.com/Oldes/Rebol-issues/issues/739
		--assert all [
			error? e: try [find-min/skip b 0]
			e/id = 'out-of-range
		]
		--assert all [
			error? e: try [find-min/skip b -2]
			e/id = 'out-of-range
		]

===end-group===

===start-group=== "++ & --"
	;@@ https://github.com/Oldes/Rebol-issues/issues/554
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
	--test-- "++ & -- past end"
	;@@ https://github.com/Oldes/Rebol-issues/issues/409
		a: [1]
		--assert [1] = ++ a
		--assert [ ] = a
		--assert [ ] = ++ a
		--assert [ ] = -- a
		--assert [1] = a
		a: "A"
		--assert "A" = -- a
		--assert "A" = a
		--assert "A" = ++ a
		--assert ""  = a
		--assert ""  = -- a
		--assert "A" = a
===end-group===


===start-group=== "INDEXZ?"
	--test-- "indexz? on string"
		s: "abc"
		--assert all [
			0 = indexz? s
			1 = indexz? next s
			3 = indexz? tail s
			0 = indexz? atz s -1
			0 = indexz? atz s 0
			2 = indexz? atz s 2
			3 = indexz? atz s 6
		]
	--test-- "indexz? on binary"
		s: to binary! "abc"
		--assert all [
			0 = indexz? s
			1 = indexz? next s
			3 = indexz? tail s
			0 = indexz? atz s -1
			0 = indexz? atz s 0
			2 = indexz? atz s 2
			3 = indexz? atz s 6
		]
	--test-- "indexz? on block"
		s: [1 2 3]
		--assert all [
			0 = indexz? s
			1 = indexz? next s
			3 = indexz? tail s
			0 = indexz? atz s -1
			0 = indexz? atz s 0
			2 = indexz? atz s 2
			3 = indexz? atz s 6
		]
	--test-- "indexz? on vector"
		s: #[u16! 3]
		--assert all [
			0 = indexz? s
			1 = indexz? next s
			3 = indexz? tail s
			0 = indexz? atz s -1
			0 = indexz? atz s 0
			2 = indexz? atz s 2
			3 = indexz? atz s 6
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

===start-group=== "PICKZ"
	;@@ https://github.com/Oldes/Rebol-issues/issues/613
	--test-- "PICKZ of block!"
	b: [1 2 3]
	--assert all [
		none? pickz b -1
		1   = pickz b 0
		2   = pickz b 1
		3   = pickz b 2
		none? pickz b 3
	]
	b: skip b 2
	--assert all [
		none? pickz b -3
		1   = pickz b -2
		2   = pickz b -1
		3   = pickz b 0
		none? pickz b 1
		none? pickz b 2
	]
	--assert all [
		none? pickz tail b 0
		3   = pickz tail b -1
	]

	--test-- "PICKZ of string!"
	s: "123"
	--assert all [
		none? pickz s -1
		#"1"= pickz s 0
		#"2"= pickz s 1
		#"3"= pickz s 2
		none? pickz s 3
	]
	s: skip s 2
	--assert all [
		#"1"= pickz s -2
		#"2"= pickz s -1
		#"3"= pickz s 0
		none? pickz s 1
		none? pickz s 2	
	]
	--assert all [
		none? pickz tail s 0
		#"3"= pickz tail s -1
	]

===end-group===

===start-group=== "POKE"
	--test-- "poke into block"
		b: [1 2 3]
		--assert all [
			error? try [poke b 0 0]
			0 =         poke b 1 0
			error? try [poke b 4 0]
			b = [0 2 3]
		]
	--test-- "poke into string"
		s: "abc"
		--assert all [
			error? try [poke s -1 #"x"]
			error? try [poke s  0 #"x"]
			#"x" =      poke s  1 #"x"
			error? try [poke s  4 #"x"]
			s = "xbc"
		]
	--test-- "poke into binary"
		s: to binary! "abc"
		--assert all [
			error? try [poke s -1 #"x"]
			error? try [poke s  0 #"x"]
			#"x" =      poke s  1 #"x"
			error? try [poke s  4 #"x"]
			"xbc" = to string! s
		]
	--test-- "poke unset value"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1815
		--assert all [
			b: [1 2 3] 
			not error? try [poke b 2 #[unset]]
			unset? pick b 2
			unset? b/2
		]
	--test-- "out of range"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1057
		a: #{0102}
		--assert 3 = poke a 1 3
		--assert a = #{0302}
		--assert all [error? e: try [poke a 1 300] e/id = 'out-of-range]
		--assert 4 = a/1: 4
		--assert a = #{0402}
		--assert all [error? e: try [a/1: 400] e/id = 'out-of-range]
		--assert #{02} = change a 5
		--assert a = #{0502}
		--assert all [error? e: try [change a 500] e/id = 'out-of-range]
===end-group===

===start-group=== "POKEZ"
	;@@ https://github.com/Oldes/Rebol-issues/issues/613
	--test-- "pokez into block"
		b: [1 2 3]
		--assert all [
			error? try [pokez b -1 0]
			0 =         pokez b  0 0
			0 =         pokez b  1 0
			error? try [pokez b  4 0]
			b = [0 0 3]
		]
	--test-- "pokez into string"
		s: "abc"
		--assert all [
			error? try [pokez s -1 #"x"]
			#"x" =      pokez s  0 #"x"
			#"x" =      pokez s  1 #"x"
			error? try [pokez s  4 #"x"]
			s = "xxc"
		]
	--test-- "pokez into binary"
		s: to binary! "abc"
		--assert all [
			error? try [pokez s -1 #"x"]
			#"x" =      pokez s  0 #"x"
			#"x" =      pokez s  1 #"x"
			error? try [pokez s  4 #"x"]
			"xxc" = to string! s
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
	--test-- "insert/part"
	;@@ https://github.com/Oldes/Rebol-issues/issues/856
		a: make block! 10
		b: at [1 2 3 4 5 6 7 8 9] 5
		--assert tail? insert/part a b 2
		--assert a = [5 6]
		insert/part clear a b 2147483647
		--assert a = [5 6 7 8 9]
		insert/part clear a b -2
		--assert a = [3 4]
		insert/part clear a b -2147483647
		--assert a = [1 2 3 4]
		--assert all [error? e: try [insert/part clear a b  2147483648] e/id = 'out-of-range]
		--assert all [error? e: try [insert/part clear a b -2147483649] e/id = 'out-of-range]



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

	--test-- "remove/key any-block!"
		;@@ https://github.com/Oldes/Rebol-wishes/issues/20
		b: [a b b c]
		--assert [a b b c] = remove/key b 'c ; no change, c is value, not a key
		--assert [a b b c] = remove/key b 'B ; no change, B is not b
		--assert [a b    ] = remove/key b 'b
		b: quote (a b b c)
		--assert quote (a b b c) = remove/key b 'c ; no change, c is value, not a key
		--assert quote (a b b c) = remove/key b 'B ; no change, B is not b
		--assert quote (a b    ) = remove/key b 'b
		b: 'a/b/b/c
		--assert 'a/b/b/c = remove/key b 'c ; no change, c is value, not a key
		--assert 'a/b/b/c = remove/key b 'B ; no change, B is not b
		--assert 'a/b     = remove/key b 'b
	--test-- "remove/key string!"
		--assert all [
			error? e: try [remove/key "abcd" #"a"]
			e/id = 'feature-na
		]

===end-group===


===start-group=== "Series with index past its tail"
;@@ https://github.com/Oldes/Rebol-issues/issues/2543
--test-- "any-string"
	src: %ABC
	dir: tail src
	--assert 4 = index? dir
	--assert %ABC = copy/part dir -3
	--assert empty? clear src
	--assert 4 = index? dir
	--assert %"" = copy/part dir -3
--test-- "any-block"
	src: [A B C]
	dir: tail src
	--assert 4 = index? dir
	--assert [A B C] = copy/part dir -3
	--assert empty? clear src
	--assert 4 = index? dir
	--assert [] = copy/part dir -3

--test-- "Red's test (strings)"
;@@ https://github.com/red/red/issues/3369
	test: does [a: copy "12345678" b: skip a 2 c: skip a 6 remove/part a 4]
	test
	--assert all [a = "5678" b = "78" c = ""]
	test change c 1
	--assert all [a = "56781" b = "781" c = ""]
	test clear c
	--assert all [a = "5678" b = "78" c = ""]
	test remove c
	--assert all [a = "5678" b = "78" c = ""]
	test change/part c 99 -1
	--assert all [a = "56799" b = "799" c = ""]
	test insert c 1
	--assert all [a = "56781" b = "781" c = ""]
	test
	--assert none? last c
	--assert none? take/last c
--test-- "Red's test (blocks)"
;@@ https://github.com/red/red/issues/3369
	test: does [a: copy [1 2 3 4 5 6 7 8] b: skip a 2 c: skip a 6 remove/part a 4]
	test
	--assert all [a = [5 6 7 8] b = [7 8] c = []]
	test change c 1
	--assert all [a = [5 6 7 8 1] b = [7 8 1] c = []]
	test clear c
	--assert all [a = [5 6 7 8] b = [7 8] c = []]
	test remove c
	--assert all [a = [5 6 7 8] b = [7 8] c = []]
	test change/part c 99 -1
	--assert all [a = [5 6 7 99] b = [7 99] c = []]
	test insert c 1
	--assert all [a = [5 6 7 8 1] b = [7 8 1] c = []]
	test
	--assert none? last c
	--assert none? take/last c

===end-group===


===start-group=== "ANY-OF & ALL-OF"
;@@ https://github.com/Oldes/Rebol-issues/issues/637
--test-- "any-of"
	--assert 4 = any-of x [-1 4 10] [x > 0]
	--assert [10 8] = any-of [x y] [1 4 10 8 5 -3] [(x - 2) = y]
--test-- "all-of"
	--assert none? all-of x [33 -1 24] [x > 0]
	--assert all-of [x y] [1 2 3 4] [x < y]

===end-group===


===start-group=== "PAST?"
	--test-- "past? block!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/417
		a: [1 2 3 4]
		--assert [4] = b: skip a 3
		--assert [4] = remove/part a 3
		--assert [ ] = b ;@@ https://github.com/Oldes/Rebol-issues/issues/2439
		--assert empty? b
		--assert  tail? b
		--assert  past? b
	--test-- "past? string!"
		a: "1234"
		--assert "4" = b: skip a 3
		--assert "4" = remove/part a 3
		--assert  "" = b
		--assert empty? b
		--assert  tail? b
		--assert  past? b

	--test-- "past? is not tail?"
		b: tail [1 2 3]
		--assert     tail? b
		--assert not past? b
===end-group===

===start-group=== "SINGLE?"
	--test-- "single? block"
		;@@ https://github.com/Oldes/Rebol-issues/issues/875
		--assert single? [a]
		--assert single? next [a b]
		--assert not single? [a b]
		--assert not single? []
	--test-- "single? string!"
		--assert single? "a"
		--assert single? next "ab"
		--assert not single? "ab"
		--assert not single? ""
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
	--assert error? try [sort/compare [1 2 #[unset]] :>]

--test-- "SORT with unset!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1124
	--assert [#[unset] 2 3] = sort reduce [2 #[unset] 3 ]

--test-- "SORT/reverse"
	;@@ https://github.com/Oldes/Rebol-issues/issues/128
	--assert [9 3 2 1] = sort/reverse [9 1 2 3] 

--test-- "SORT block of strings"
	;@@ https://github.com/Oldes/Rebol-issues/issues/681
	--assert ["Fred" "fred" "FRED"] == sort      ["Fred" "fred" "FRED"]
	--assert ["fred" "Fred" "FRED"] == sort      ["fred" "Fred" "FRED"]
	--assert ["FRED" "Fred" "fred"] == sort/case ["Fred" "fred" "FRED"]

--test-- "SORT with NaN"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2493
	;@@ https://github.com/Oldes/Rebol-issues/issues/2494
	--assert [-1.#INF -1.0 0 1.0 1.#INF 1.#NaN 1.#NaN] = sort [1.#inf -1.0 1.#nan 1.0 -1.#inf 0 1.#NAN]
	--assert [-1.#INF -1.0 0 1.0 1.#INF 1.#NaN 1.#NaN] = sort [1.#inf -1.0 1.#nan 1.0 -1.#inf 1.#NAN 0]
	--assert [-1.#INF -1.0 0 1.0 1.#INF 1.#NaN 1.#NaN] = sort [1.#inf -1.0 1.#nan 1.0 1.#NAN -1.#inf 0]
	--assert [-1.#INF -1.0 0 1.0 1.#INF 1.#NaN 1.#NaN] = sort [1.#inf -1.0 1.#nan 1.#NAN 1.0 -1.#inf 0]
	--assert [-1.#INF -1.0 0 1.0 1.#INF 1.#NaN 1.#NaN] = sort [1.#inf 1.#NAN -1.0 1.#nan 1.0 -1.#inf 0]
	--assert [-1.#INF -1.0 0 1.0 1.#INF 1.#NaN 1.#NaN] = sort [1.#NAN 1.#inf -1.0 1.#nan 1.0 -1.#inf 0]
	;reversed
	--assert [1.#NaN 1.#NaN 1.#INF 1.0 0 -1.0 -1.#INF] = sort/reverse [1.#inf -1.0 1.#nan 1.0 -1.#inf 0 1.#NAN]
	--assert [1.#NaN 1.#NaN 1.#INF 1.0 0 -1.0 -1.#INF] = sort/reverse [1.#inf -1.0 1.#nan 1.0 -1.#inf 1.#NAN 0]
	--assert [1.#NaN 1.#NaN 1.#INF 1.0 0 -1.0 -1.#INF] = sort/reverse [1.#inf -1.0 1.#nan 1.0 1.#NAN -1.#inf 0]
	--assert [1.#NaN 1.#NaN 1.#INF 1.0 0 -1.0 -1.#INF] = sort/reverse [1.#inf -1.0 1.#nan 1.#NAN 1.0 -1.#inf 0]
	--assert [1.#NaN 1.#NaN 1.#INF 1.0 0 -1.0 -1.#INF] = sort/reverse [1.#inf 1.#NAN -1.0 1.#nan 1.0 -1.#inf 0]
	--assert [1.#NaN 1.#NaN 1.#INF 1.0 0 -1.0 -1.#INF] = sort/reverse [1.#NAN 1.#inf -1.0 1.#nan 1.0 -1.#inf 0]

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
	--test-- "random on path"
	;@@ https://github.com/Oldes/Rebol-issues/issues/912
		; not supported..
		--assert all [error? e: try [random 'a/b/c] e/id = 'cannot-use]

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
--test-- "FOREACH result"
	--assert #"3" = foreach x "123" [x]
	--assert 3 = foreach x [1 2 3] [x]
	--assert unset? foreach x [1 2 3] [if x = 2 [break]]
	--assert 4 = foreach x [1 2 3] [if x = 2 [break/return 4]]
--test-- "FOREACH []"
	;@@ https://github.com/Oldes/Rebol-issues/issues/540
	--assert all [
		error? e: try [foreach [][][]]
		e/id = 'invalid-arg
	]
--test-- "FOREACH [k v] object!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/542
	o: object [a: 1 b: 2] x: 0
	--assert 3 = foreach [k v] o [x: x + v]
	--assert 6 = foreach [k] o [x: x + o/:k]
	--assert 9 = foreach k o [x: x + o/:k]
	--assert all [
		error? e: try [foreach [k v b] o []]
		e/id = 'invalid-arg
	]
--test-- "FOREACH [ref: k v] object!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/723
	o: object [a: 1 b: 2] x: 0 ref: 'foo
	--assert 3 = foreach [ref: k v] o [if ref = o [x: x + v]]
	--assert 6 = foreach [ref: k] o [if ref = o [x: x + o/:k]]
	--assert ref = 'foo
--test-- "FOREACH [k v] map!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/722
	m: map [a: 1 b: 2] x: 0
	--assert 3 = foreach [k v] m [x: x + v]
	--assert 6 = foreach [k] m [x: x + m/:k]
	--assert 9 = foreach k m [x: x + m/:k]
	--assert 9 = foreach [k v] m [if v = 1 [put m 'c -3] x: x + v]
	--assert all [
		error? e: try [foreach [k v b] m []]
		e/id = 'invalid-arg
	]
--test-- "FOREACH [ref: k v] map!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/723
	m: map [a: 1 b: 2] x: 0 ref: 'foo
	--assert 3 = foreach [ref: k v] m [if ref = m [x: x + v]]
	--assert 6 = foreach [ref: k] m [if ref = m [x: x + m/:k]]
	--assert ref = 'foo

--test-- "FOREACH [ref:] series!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1751
	;@@ https://github.com/Oldes/Rebol-issues/issues/2530
	code: [if 2 = index? ref [break/return ref]]
	foreach [result values][
		[2]   [1 2]
		(2)   (1 2)
		b/c   a/b/c
		"bc"  "abc"
		%bc   %abc
		#{02} #{0102}
	][
		--assert result == foreach [ref:] :values :code
	]

===end-group===

===start-group=== "MAP-EACH"

--test-- "MAP-EACH []"
	;@@ https://github.com/Oldes/Rebol-issues/issues/541
	--assert all [
		error? e: try [map-each [][a b c][]]
		e/id = 'invalid-arg
	]

===end-group===

===start-group=== "REMOVE-EACH"
--test-- "remove-each result"
	;@@ https://github.com/Oldes/Rebol-issues/issues/931
	b: [a 1 b 2]
	--assert [b 2] = remove-each [k v] b [v < 2]
	--assert [b 2] = b

	s: next [1 2 3 4]
	--assert [3 4] = remove-each n s [n < 3]
	--assert [1 3 4] = head s

	;@@ https://github.com/Oldes/Rebol-issues/issues/806
	--assert #{0303} = remove-each v #{03010203} [v < 3]
	--assert [3 3] = to block! remove-each v #[u16! [3 1 2 3]] [v < 3]

--test-- "remove-each/count result"
	b: [a 1 b 2]
	--assert 2 = remove-each/count [k v] b [v < 2]
	--assert b = [b 2]

	s: next [1 2 3 4]
	--assert 1 = remove-each/count n s [n < 3]
	--assert [1 3 4] = head s

--test-- "break in remove-each"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2192
	--assert [2 3 4] = remove-each n s: [1 2 3 4] [if n = 2 [break] true]
	--assert s = [2 3 4]
	--assert 1 = remove-each/count n s: [1 2 3 4] [if n = 2 [break] true]
	--assert s = [2 3 4]
--test-- "break/return in remove-each"
	--assert 'x = remove-each n s: [1 2 3 4] [if n = 2 [break/return 'x] true]
	--assert s = [2 3 4]
	--assert 'x = remove-each/count n s: [1 2 3 4] [if n = 2 [break/return 'x] true]
	--assert s = [2 3 4]

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
		3160989 = checksum str 'crc24
	]
--test-- "issue-2186 read UCS16-BE"
	bin: read %units/files/issue-2186-UTF16-BE.txt
	--assert all [
		string? try [str: to-string bin]
		3160989 = checksum str 'crc24
	]
--test-- "issue-2186 read UCS32-LE"
	bin: read %units/files/issue-2186-UTF32-LE.txt
	--assert all [
		string? try [str: to-string bin]
		3160989 = checksum str 'crc24
	]
--test-- "issue-2186 read UCS32-BE"
	bin: read %units/files/issue-2186-UTF32-BE.txt
	--assert all [
		string? try [str: to-string bin]
		3160989 = checksum str 'crc24
	]
;- read/string converts CRLF to LF, so the checksum is different
--test-- "issue-2186 read/string UCS16-LE"
	--assert all [
		string? try [str: read/string %units/files/issue-2186-UTF16-LE.txt]
		11709824 = checksum str 'crc24
	]
--test-- "issue-2186 read/string UCS16-BE"
	--assert all [
		string? try [str: read/string %units/files/issue-2186-UTF16-BE.txt]
		11709824 = checksum str 'crc24
	]
--test-- "issue-2186 read/string UCS32-LE"
	--assert all [
		string? try [str: read/string %units/files/issue-2186-UTF32-LE.txt]
		11709824 = checksum str 'crc24
	]
--test-- "issue-2186 read/string UCS32-BE"
	--assert all [
		string? try [str: read/string %units/files/issue-2186-UTF32-BE.txt]
		11709824 = checksum str 'crc24
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
	--assert 13806406 = checksum str: to-string bin 'crc24 ; does not normalize CRLF
	--assert  5367801 = checksum deline str 'crc24
	--assert  5367801 = checksum read/string %units/files/quickbrown.bin 'crc24 ;converts CRLF to LF

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

--test-- "ICONV from UTF-8"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2475
	--assert "š" = iconv #{C5A1} 'utf8
	--assert "š" = iconv #{C5A1} 'utf-8
	--assert "š" = iconv #{C5A1} 'UTF8
	--assert "š" = iconv #{C5A1} 'UTF-8
	--assert "š" = iconv #{C5A1} 'CP65001
	--assert "š" = iconv #{C5A1} 65001

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

	--assert "áaá" = dehex "áa%C3%A1"
	--assert "aá"  = dehex next "áa%C3%A1"
	--assert "aa"  = dehex next "áaa"

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
	--assert       "šik" = dehex enhex "šik"
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

--test-- "ENHEX/uri"
	--assert "a%20b%2B" = enhex "a b+"
	--assert "a+b%2B" = enhex/uri "a b+"
	--assert "a%20%C3%A1%2B" = enhex "a á+"
	--assert "a+%C3%A1%2B" = enhex/uri "a á+"
	; quoted-printable:
	--assert "a=20b_" = enhex/escape "a b_" #"="
	--assert "a_b=5F" = enhex/uri/escape "a b_" #"="
	--assert "a=20=C3=A1_" = enhex/escape "a á_" #"="
	--assert "a_=C3=A1=5F" = enhex/escape/uri "a á_" #"="

--test-- "DEHEX/uri"
	--assert "a+b+" = dehex "a+b%2B"
	--assert "a b+" = dehex/uri "a+b%2B"
	; quoted-printable:
	--assert "a_b_" = dehex/escape"a_b=5F" #"="
	--assert "a b_" = dehex/uri/escape"a_b=5F" #"="
	; to get propper UTF8 results, we must use binary input (for now?)
	--assert "a á+" = dehex "a%20%C3%A1%2B"
	--assert "a á+" = dehex/uri "a+%C3%A1%2B"
	--assert "a á_" = dehex/escape "a=20=C3=A1_" #"="
	--assert "a á_" = dehex/escape/uri "a_=C3=A1=5F" #"="

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
--test-- "ENLINE block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/647
	--assert all [
		error? e: try [enline ["a"]]
		e/id = 'not-done
	]
	
===end-group===


===start-group=== "AS coercion"
;@@ https://github.com/Oldes/Rebol-issues/issues/546
;@@ https://github.com/Oldes/Rebol-issues/issues/2264
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

--test-- "CRLF inside tag"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1233
	; CRLF is not automatically converted inside tags
	--assert #{410D0A} = to-binary        first transcode #{3C410D0A3E}
	--assert #{410A}   = to-binary deline first transcode #{3C410D0A3E}

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

--test-- "binary AND binary"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1013
	--assert #{0102} and #{00FF} == #{0002}
	--assert #{0102} and #{0300} == #{0100}
	--assert #{0101} and #{03}   == #{0101} ; the shorter arg is now used repeatadly!
--test-- "binary OR binary"
	--assert #{0102}  or #{00FF} == #{01FF}
	--assert #{0102}  or #{0300} == #{0302}
	--assert #{0101}  or #{03}   == #{0303} ; the shorter arg is now used repeatadly!

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

--test-- "copy/part limit"
	;@@ https://github.com/Oldes/Rebol-issues/issues/853
	--assert [1] = copy/part tail [1] -2147483647
	--assert [1] = copy/part tail [1] -2147483648
	--assert all [error? e: try [copy/part tail [1] -2147483649] e/id = 'out-of-range]


===end-group===


===start-group=== "SPLIT"

--test-- "split block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2051
	b: [a b c d e f]
	--assert [[a b c d e f]]              = split/parts b 1
	--assert [[a b c] [d e f]]            = split/parts b 2
	--assert [[a b] [c d] [e f]]          = split/parts b 3
	--assert [[a] [b] [c] [d e f]]        = split/parts b 4
	--assert [[a] [b] [c] [d] [e f]]      = split/parts b 5
	--assert [[a] [b] [c] [d] [e] [f]]    = split/parts b 6
	--assert [[a] [b] [c] [d] [e] [f] []] = split/parts b 7

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
	--assert ["12345678" "12345678"] = split/parts "1234567812345678" 2
	--assert ["12345" "67812" "345678"] = split/parts "1234567812345678" 3
	--assert ["123" "456" "781" "234" "5678"] = split/parts "1234567812345678" 5
	;@@ https://github.com/Oldes/Rebol-issues/issues/573
	--assert ["c" "c"]  = split "c c" " "
	--assert ["1,2"]    = split "1,2" " "
	--assert ["c" "c "] = split "c,c " ","
	;@@ https://github.com/Oldes/Rebol-issues/issues/1096
	--assert [""] = split/parts "" 1
	--assert ["" ""] = split/parts "" 2
	--assert ["" "" ""] = split/parts "" 3
	--assert ["x" ""] = split/parts "x" 2

--test-- "split gregg 1"
	;@@ https://gist.github.com/greggirwin/66d7c6892fc310097cd91ab354189542
	--assert (split "1234567812345678" 4)       = ["1234" "5678" "1234" "5678"]
	--assert (split "1234567812345678" 3)       = ["123" "456" "781" "234" "567" "8"]
	--assert (split "1234567812345678" 5)       = ["12345" "67812" "34567" "8"]
	--assert (split/parts [1 2 3 4 5 6] 2)      = [[1 2 3] [4 5 6]]
	--assert (split/parts "1234567812345678" 2) = ["12345678" "12345678"]
	--assert (split/parts "1234567812345678" 3) = ["12345" "67812" "345678"]
	--assert (split/parts "1234567812345678" 5) = ["123" "456" "781" "234" "5678"]

--test-- "split gregg 2"
	; Dlm longer than series"
	--assert (split/parts "123" 6)   =     ["1" "2" "3" "" "" ""] ;or ["1" "2" "3"]
	--assert (split/parts [1 2 3] 6) =     [[1] [2] [3] [] [] []] ;or [1 2 3]

--test-- "split gregg 3"
	--assert (split [1 2 3 4 5 6] [2 1 3])                = [[1 2] [3] [4 5 6]]
	--assert (split "1234567812345678" [4 4 2 2 1 1 1 1]) = ["1234" "5678" "12" "34" "5" "6" "7" "8"]
	--assert (split first [(1 2 3 4 5 6 7 8 9)] 3)        = [(1 2 3) (4 5 6) (7 8 9)]
	--assert (split #{0102030405060708090A} [4 3 1 2])    = [#{01020304} #{050607} #{08} #{090A}]
	--assert (split [1 2 3 4 5 6] [2 1])                  = [[1 2] [3]]
	--assert (split [1 2 3 4 5 6] [2 1 3 5])              = [[1 2] [3] [4 5 6] []]
	--assert (split [1 2 3 4 5 6] [2 1 6])                = [[1 2] [3] [4 5 6]]

	; Old design for negative skip vals
	; --assert (split [1 2 3 4 5 6] [3 2 2 -2 2 -4 3]]    [[1 2 3] [4 5] [6] [5 6] [3 4 5]]
	; New design for negative skip vals
	--assert (split [1 2 3 4 5 6] [2 -2 2])               = [[1 2] [5 6]]

--test-- "split gregg 4"
	--assert (split "abc,de,fghi,jk" #",")                = ["abc" "de" "fghi" "jk"]
	--assert (split "abc<br>de<br>fghi<br>jk" <br>)       = ["abc" "de" "fghi" "jk"]

	--assert (split "a.b.c" ".")           = ["a" "b" "c"]
	--assert (split "c c" " ")             = ["c" "c"]
	--assert (split "1,2,3" " ")           = ["1,2,3"]
	--assert (split "1,2,3" ",")           = ["1" "2" "3"]
	--assert (split "1,2,3," ",")          = ["1" "2" "3" ""]
	--assert (split "1,2,3," charset ",.") = ["1" "2" "3" ""]
	--assert (split "1.2,3." charset ",.") = ["1" "2" "3" ""]

	--assert (split "-a-a" ["a"])    = ["-" "-"]
	--assert (split "-a-a'" ["a"])   = ["-" "-" "'"]

--test-- "split gregg 5"
	--assert (split "abc|de/fghi:jk" charset "|/:")                   = ["abc" "de" "fghi" "jk"]
	--assert (split "abc^M^Jde^Mfghi^Jjk" [crlf | #"^M" | newline])   = ["abc" "de" "fghi" "jk"]
	--assert (split "abc     de fghi  jk" [some #" "])                = ["abc" "de" "fghi" "jk"]

--test-- "split gregg 6"
	--assert (split [1 2 3 4 5 6] :even?)  = [[2 4 6] [1 3 5]]
	--assert (split [1 2 3 4 5 6] :odd?)   = [[1 3 5] [2 4 6]]
	--assert (split [1 2.3 /a word "str" #iss x: :y] :refinement?) = [[/a] [1 2.3 word "str" #iss x: :y]]
	--assert (split [1 2.3 /a word "str" #iss x: :y] :number?)     = [[1 2.3] [/a word "str" #iss x: :y]]
	--assert (split [1 2.3 /a word "str" #iss x: :y] :any-word?)   = [[/a word #iss x: :y] [1 2.3 "str"]]

--test-- "split gregg 7"
	--assert (split/at [1 2.3 /a word "str" #iss x: :y] 4) =	[[1 2.3 /a word] ["str" #iss x: :y]]
	;!! Splitting /at with a non-integer excludes the delimiter from the result
	--assert (split/at [1 2.3 /a word "str" #iss x: :y] "str") =	[[1 2.3 /a word] [#iss x: :y]]
	--assert (split/at [1 2.3 /a word "str" #iss x: :y] 'word) =	[[1 2.3 /a] ["str" #iss x: :y]]

--test-- "split using charset!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/690
	--assert (split "This! is a. test? to see " charset "!?.") = ["This" " is a" " test" " to see "]

--test-- "split/at"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2490
	--assert (split/at "a:b"   #":") = ["a" "b"]
	--assert (split/at "a:b:c" #":") = ["a" "b:c"]
	--assert (split/at "a"     #":") = ["a"]
	--assert (split/at "a:"    #":") = ["a" ""]
	--assert (split/at ":b"    #":") = ["" "b"]
	--assert (split/at [1 a 2]     'a) = [[1] [2]]
	--assert (split/at [1 a 2 3]   'a) = [[1] [2 3]]
	--assert (split/at [1 a 2 a 3] 'a) = [[1] [2 a 3]]
	--assert (split/at [1 2]       'a) = [[1 2]]


===end-group===


===start-group=== "SPLIT-LINES"
--test-- "split-lines string!"
	--assert ["a" "b" "c"]  = split-lines "a^/b^M^/c"
===end-group===


===start-group=== "SPLIT-PATH"

--test-- "split-path file!"
	--assert [%./ %dir]  = split-path %dir
	--assert [%./ %dir/] = split-path %dir/
	--assert [%dir/ %file.txt] = split-path %dir/file.txt
	;@@ https://github.com/Oldes/Rebol-issues/issues/2474
	--assert [%/ %""] = split-path %/
	--assert [%./ %""] = split-path %./
	--assert [%../ %""] = split-path %../
--test-- "split-path url!"
	--assert [http://foo.net/ %aa.txt] = split-path http://foo.net/aa.txt
	--assert [http:// %foo.net/] = split-path http://foo.net/ ;@@ could be better result!
--test-- "split-path string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1366
	--assert [%./ %dir]  = split-path "dir"
	--assert [%./ %dir/] = split-path "dir/"
	--assert ["dir/" %file.txt] = split-path "dir/file.txt"

===end-group===



===start-group=== "UNION"

--test-- "union on 2 strings"
	;@@ https://github.com/Oldes/Rebol-issues/issues/400
	--assert "123" = union "12" "13"
--test-- "union with none and unset"
	--assert [#[none] #[unset]] = union [#[none]] [#[unset]]

--test-- "union/skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2520
	--assert (union      [a 1 b 2] [a 1 b 2]          ) == [a 1 b 2]
	--assert (union/skip [b 1 b 2] [b 1 b 2] 2        ) == [b 1]
	--assert (union/skip [b 1 b 2] [b 2 b 2] 2        ) == [b 1]
	--assert (union/skip [b 2 b 2] [b 2 b 2] 2        ) == [b 2]
	--assert (union/skip [a 1 b 2] [a 1 b 2] 2        ) == [a 1 b 2]
	--assert (union/skip ["a" 1 "b" 2] ["a" 1 "b" 2] 2) == ["a" 1 "b" 2]

--test-- "union/skip 2"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1963
	--assert (union [x x 1 2 1 3] [1 4]) == [x 1 2 3 4]
	--assert (union [1 2 1 3] [1 4]    ) == [1 2 3 4]
	--assert (union/skip [x x 1 2 1 3] [1 4] 2 ) = [x x 1 2] ; like in R2 and Red
	--assert (union/skip [x x 1 2 1 2] [1 4] 2 ) = [x x 1 2] ; like in R2 and Red
	--assert (union/skip [1 2 1 3] [2 4] 2     ) = [1 2 2 4] ; like in R2 and Red

--test-- "unique/skip 3"
	;@@ https://github.com/Oldes/Rebol-issues/issues/726
	--assert (unique/skip [1 2 3 4 5 6 1 2 3 4]     2) == [1 2 3 4 5 6] ; like in R2 and Red
	--assert (unique/skip [1 2 3 4 5 6 1 2 3 4 5 6] 2) == [1 2 3 4 5 6] ; like in R2 and Red
	--assert (unique/skip [1 2 3 4 5 6 1 2 3 4 5 6] 3) == [1 2 3 4 5 6] ; like in R2 and Red

--test-- "union/skip with negative skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/736
	--assert all [
		error? e: try [union/skip [2 1][2 1] -2]
		e/id = 'out-of-range
	]
--test-- "union - first-wins"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1984
	--assert 1 = get first union reduce [in construct [a: 1] 'a] reduce [in construct [a: 2] 'a]

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
	--assert [#[unset] #[none]] = unique reduce [unset 'a unset 'a none none]

===end-group===

===start-group=== "INTERSECT"
--test-- "intersect"
	--assert [#[none] 1 #[unset]] = intersect [#[none] 1 #[unset]] [#[none] #[unset] 1]
	--assert [] = intersect [#[none]] [1 #[unset]]

===end-group===

===start-group=== "EXTRACT"
--test-- "extract with unset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1540
	--assert [5 1 #[unset]] = extract [5 3 1 #[unset] #[unset] #[unset]] 2

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
	;-- not allowed anymore!
	;@@ https://github.com/Oldes/Rebol-issues/issues/1978
	bin1: #{010203}
	bin2: #{010203010203}
	--assert all [error? e: try [bin1 = unique bin2] e/id = 'expect-arg]
	--assert all [error? e: try [bin1 = union  bin1 bin2 ] e/id = 'expect-arg]
	append bin2 bin3: #{0405}
	--assert all [error? e: try [bin1 = intersect  bin2 bin1] e/id = 'expect-arg]
	--assert all [error? e: try [bin3 = difference bin1 bin2] e/id = 'expect-arg]
	--assert all [error? e: try [bin3 = exclude bin2 bin1] e/id = 'expect-arg]
	--assert all [error? e: try [empty? exclude bin1 bin2] e/id = 'expect-arg]

===end-group===


===start-group=== "Bitwise operations on binary"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1013
	bin1: #{DEADBEAFF00D}
	bin2: #{BEAF}
	--test-- "AND on binary"
	--assert (bin1 AND bin2) == #{9EADBEAFB00D}
	--assert (bin2 AND bin1) == #{9EADBEAFB00D}
	--test-- "OR on binary"
	--assert (bin1 OR bin2)  == #{FEAFBEAFFEAF}
	--assert (bin2 OR bin1)  == #{FEAFBEAFFEAF}
	--test-- "XOR on binary"
	--assert (bin1 XOR bin2) == #{600200004EA2}
	--assert (bin2 XOR bin1) == #{600200004EA2}
	--assert (#{600200004EA2} XOR bin2) == bin1
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
--test-- "new-line with negative skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/737
	--assert all [error? e: try [new-line/skip [1 2] true  0] e/id = 'out-of-range]
	--assert all [error? e: try [new-line/skip [1 2] true -2] e/id = 'out-of-range]

===end-group===

===start-group=== "ARRAY"
--test-- "array"
	--assert [#[none] #[none]] = array 2
--test-- "array/initial"
	--assert [0 0] = array/initial 2 0
	;@@ https://github.com/Oldes/Rebol-issues/issues/360
	--assert [["" ""] ["" ""]] = array/initial [2 2] ""
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
	--assert (reword/escape to file! "ba" [a 1 b 2] none)
		== %21   ; /escape none like /escape ""

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
;	--assert (reword/escape "a :a /a #a" [a 1 :a 2 /a 3 #a 4] none)
;		== "1 2 3 4"  ; But otherwise let word types be distinct

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


===start-group=== "COMBINE"
;@@ https://github.com/Oldes/Rebol-wishes/issues/19
--test-- "combine to string"
	--assert "abc" = combine [a b c]
	--assert "abc" = combine [a #[none] b () c #[unset]]
	--assert "a|b|c" = combine/with [a #[none] b () c #[unset]] #"|"
	--assert "abcghi" = combine [{abc} (if false {def}) {ghi}]
	--assert "abcghi" = combine reduce [{abc} if false {def} {ghi}]
	--assert "a, b, c" = combine/with [a b c] ", "
	--assert "x, a, b, c" = combine/with/into [a b c] ", " "x"

--test-- "combine to block"
	--assert [a c]  = combine/into [a (if/only false [b]) c] []
	--assert [a b c] = combine/into [a (if/only true [b]) c] []
	--assert [x a b c] = combine/into [a b c] [x]
	--assert [x -- a -- b -- c] = combine/into/with [a [b c]] [x] '--

--test-- "combine to file"
	--assert %a/1/c = combine/into/with [#"a" 1 "c"] %"" #"/"

--test-- "combine to path"
	--assert 'a/b/c = combine/into [a b #[none] c] make path! 3

--test-- "combine to tag"
	url: http://rebol.com
	--assert <a href=http://rebol.com> = combine/into [{a href=} :url] make tag! 10

--test-- "combine/only"
	--assert [1 [a b] 2] = combine/only/into [1 [a b] 2] []
	--assert "1, [a b], 2" = combine/only/with [1 [a b] 2] ", "

--test-- "combine/ignore"
	--assert "ab" = combine/ignore [1 a b 1.0] number!
	--assert [a b] = combine/ignore/into [1 a b 1.0] number! []

--test-- "combine with get-word"
	--assert "<span>one</span>^/<span>1 < 2</span>" = combine [
		<span> "one" </span> :LF
		(if/only 1 < 2 [<span> "1 < 2" </span>])
		(if/only 1 > 2 [<span> "1 > 2" </span>])
	]

===end-group===


===start-group=== "COLLECT"
--test-- "collect unset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/880
	--assert unset? first collect [keep #[unset]]
	--assert unset? first head insert copy [] #[unset]

===end-group===

===start-group=== "MOVE"
--test-- "move/skip"
	;@@ https://github.com/Oldes/Rebol-issues/issues/740
	--assert all [error? e: try [move/skip [1 2 3] 2 0] e/id = 'out-of-range]

===end-group===


===start-group=== "SWAP-ENDIAN"
--test-- "swap-endian/width 2"
	--assert #{FF00FF00FF00} = swap-endian #{00FF00FF00FF}  
	--assert #{FF00FF00AA}   = swap-endian #{00FF00FFAA} ; the last not padded byte is ignored
--test-- "swap-endian/width 4"
	--assert #{4433221188776655} = swap-endian/width #{1122334455667788} 4  
	--assert #{44332211AAAA}     = swap-endian/width #{11223344AAAA} 4
--test-- "swap-endian/width 8"
	--assert #{7766554433221100FFEEDDCCBBAA9988} = swap-endian/width #{00112233445566778899AABBCCDDEEFF} 8 
	--assert #{7766554433221100AAAA}             = swap-endian/width #{0011223344556677AAAA} 8
--test-- "swap-endian/part"
	--assert #{FF00FF001122} = swap-endian/part #{00FF00FF1122} 4
	--assert #{FF00FF001122} = swap-endian/part #{00FF00FF1122} 5
	--assert #{FF00FF002211} = swap-endian/part #{00FF00FF1122} 6
	--assert #{4433221155667788} = swap-endian/width/part #{1122334455667788} 4 4
	--assert #{77665544332211008899AABBCCDDEEFF} = swap-endian/width/part #{00112233445566778899AABBCCDDEEFF} 8 8
===end-group===

;-- VECTOR related tests moved to %vector-test.r3

===start-group=== "RECYCLE"
--test-- "recycle"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1128
	recycle                 ;; force GC
	count: stats            ;; store current memory usage
	x: make string! 5000000 ;; create large series
	--assert all [
		stats >= (count + 5000000) ;; check that memory usage increased
		none? x: none              ;; unreference the series
		recycle                    ;; force GC
		(stats - count) < 2000     ;; check if memory usage decreased
	]
===end-group===


===start-group=== "ALTER"
--test-- "alter/case"
	;@@ https://github.com/Oldes/Rebol-issues/issues/408
	b: copy []
	--assert all [
		    alter b "a"
		not alter b "A"
		empty? b
	]
	--assert all [
		alter/case b "a"
		alter/case b "A"
		b == ["a" "A"]
	]
===end-group===

~~~end-file~~~