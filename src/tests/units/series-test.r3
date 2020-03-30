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

--test-- "FIND/PART"
;@@ https://github.com/rebol/rebol-issues/issues/2329
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

--test-- https://github.com/rebol/rebol-issues/issues/66
	--assert none? find/skip [1 2 3 4 5 6] 2 3

--test-- https://github.com/rebol/rebol-issues/issues/88
	--assert "c" = find "abc" charset ["c"]
	--assert none? find/part "ab" "b" 1

--test-- https://github.com/rebol/rebol-issues/issues/2324
	str: "1.1.1"
	--assert "1.1.1" == find/part str "1." 2
	str: skip str 2
	--assert "1.1" == find str "1."
	--assert "1.1" == find/part str "1." 2

--test-- "SELECT"
	--assert 2 = select/part [1 2 1 3 1 2] 1 2
	--assert none? select/part [1 2 1 3 1 2] 2 2
	--assert 3 = select/part (skip [1 2 1 3 1 2] 2) 1 2

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

===end-group===

===start-group=== "REVERSE"
	--test-- "issue-19"
		--assert "4321" = sort/reverse "1234"
		--assert "4321" = sort/reverse/reverse "1234"
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
	--assert error? err: try [ put v 'a 2 ]
	--assert 'protected = err/id

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
		;@@ https://github.com/rebol/rebol-issues/issues/146
		b: append [] 0
		repeat n 10 [ append b n remove b]
		--assert 1 = length? b
		--assert 10 = first b

	--test-- "issue-2397"
		;@@ https://github.com/rebol/rebol-issues/issues/2397
		b: make binary! 40000 insert/dup b 0 40000
		remove/part b to integer! #{8000}
		--assert 7232 = length? b

===end-group===

===start-group=== "SORT"

--test-- "SORT/compare"
	;@@ https://github.com/rebol/rebol-issues/issues/720
	--assert [3 2 1] = sort/compare [1 2 3] func [a b] [a > b]
	;@@ https://github.com/rebol/rebol-issues/issues/2376
	--assert [1 3 10] = sort/compare [1 10 3] func[x y][case [x > y [1] x < y [-1] true [0]]]

--test-- "SORT/skip/compare"
	;@@ https://github.com/rebol/rebol-issues/issues/1152
	--assert ["A" "a"] = sort/compare ["A" "a"] func [a b] [a < b]
	--assert ["a" "A"] = sort/compare ["a" "A"] func [a b] [a < b]
	--assert ["A" "a"] = sort/compare ["A" "a"] func [a b] [a <= b]
	--assert ["a" "A"] = sort/compare ["a" "A"] func [a b] [a <= b]
	--assert [1 9 1 5 1 7] = sort/skip/compare [1 9 1 5 1 7] 2 1

--test-- "SORT with invalid compare function"
	;@@ https://github.com/rebol/rebol-issues/issues/1766
	--assert error? try [sort/compare [1 2 3]  func [/local loc-1 loc-2][local < loc-1] ]
	;@@ https://github.com/rebol/rebol-issues/issues/1516
	--assert error? try [sort/compare [1 2 #[unset!]] :>]


===end-group===


===start-group=== "REWORD"

--test-- "reword/escape"
	;@@ https://github.com/rebol/rebol-issues/issues/2333
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

;@@ https://github.com/rebol/rebol-issues/issues/2331
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
	;@@ https://github.com/rebol/rebol-issues/issues/2336
	--assert "^M"   = to-string to-binary "^M"
	--assert "^/"   = to-string to-binary "^/"
	--assert "^M^/" = to-string to-binary "^M^/"
	--assert "^/^M" = to-string to-binary "^/^M"

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
	;@@ https://github.com/rebol/rebol-issues/issues/1986
	--assert "řek" = to-string dehex to-binary "%c5%99ek"

	--assert "%3x " = dehex "%3x%20"
	--assert "++"   = dehex "%2b%2b"
	--assert 127 = to-integer first dehex "%7F"

--test-- "ENHEX"
	--assert "%C2%A3"   = enhex "£"
	--assert "a%20b%5C" = enhex "a b\"
	--assert "%C5%A1ik" = enhex "šik"
	--assert "%22%25-.%3C%3E%5C%1F%60%7B%7C%7D~" = enhex {"%-.<>\^_`{|}~}
	; --assert %%C5%A1ik  = enhex %šik ;<-- this does not work yet!
	--assert "šik" = to-string dehex enhex to-binary "šik"
	--assert    "%7F" = enhex to-string #{7F}
	--assert "%C2%80" = enhex to-string #{80}
	--assert "%C2%81" = enhex to-string #{81}
	--assert "%E5%85%83" = enhex {元}


===end-group===

===start-group=== "AS coercion"

--test-- "AS datatype! any-string!"
	s: "hell"
	--assert file?  f: as file!  s
	--assert email? e: as email! s
	--assert url?   u: as url!   s
	--assert tag?   t: as tag!   s
	append s #"o"
	--assert f = %hello
	--assert e = to-email %hello
	--assert u = #[url! "hello"]
	--assert t = <hello>

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

--test-- "AS with protect"
	b: protect [a b]
	--assert path? try [p: as path! b]
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

--test-- "TAKE binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/963
	--assert 32 = take #{20}
	--assert  4 = take #{04}

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
===end-group===


===start-group=== "BLOCK"

--test-- "path in block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/26
	b: [b 1]
	--assert 1 = b/b

===end-group===


===start-group=== "UNION"

--test-- "union on 2 strings"
	;@@ https://github.com/Oldes/Rebol-issues/issues/400
	--assert "123" = union "12" "13"

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

===end-group===


;-- VECTOR related tests moved to %vector-test.r3

~~~end-file~~~
