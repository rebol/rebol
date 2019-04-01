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


===start-group=== "DEHEX"

--test-- "DEHEX UTF-8 encoded data"
	;@@ https://github.com/rebol/rebol-issues/issues/1986
	--assert "řek" = to-string dehex to-binary "%c5%99ek"

===end-group===

;-- VECTOR related tests moved to %vector-test.r3

~~~end-file~~~
