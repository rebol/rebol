Rebol [
	Title:   "Rebol3 map! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %map-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "map"

===start-group=== "MAP construction"
	--test-- "map []"
		m: map [a: 1 b: 2]
		--assert 1 = m/a

	--test-- "make map! []"
		m: make map! [a: 1 b: 2]
		--assert 1 = m/a
		--assert empty? make map! []

	--test-- "#[map! []]"
		m: #[map! [a: 1 b: 2]]
		--assert 2 = m/b
		--assert empty? #[map! []]

	--test-- "#()"
		m: #(a: 1 b: 2)
		--assert 2 = m/b
		--assert empty? #()

	--test-- "case sensitivity"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1153
		m: #(
			 <a> 1   <A> 2   <ab> 3   <Ab> 4
			 %a  5   %A  6   %ab  7   %Ab  8
			 "a" 9   "A" 10  "ab" 11  "Ab" 12
			  a  13   A  14   ab  15   Ab  16
			 @a  17  @A  18   @ab 19  @Ab  20
			#"a" 21 #"A" 22 #{61} 23 #{41} 24
		)
		--assert 24 = length? m
		--assert 1  = select m <A>
		--assert 5  = select m %A
		--assert 9  = select m "A"
		--assert 13 = select m 'A
		--assert 17 = select m @A
		--assert 21 = select m #"A"
		--assert 24 = select m #{41} ; always case sensitive
		--assert 24 = length? m
		--assert 2  = select/case m <A>
		--assert 6  = select/case m %A
		--assert 10 = select/case m "A"
		--assert 14 = select/case m 'A
		--assert 18 = select/case m @A
		--assert 22 = select/case m #"A"
		--assert 24 = select/case m #{41}
		--assert 1  = m/(<A>)
		--assert 5  = m/(%A)
		--assert 9  = m/("A")
		--assert 13 = m/('A)
		--assert 17 = m/(@A)
		--assert 21 = m/(#"A")
		;@@ https://github.com/Oldes/Rebol-issues/issues/471
		--assert none? select m none 


===end-group===

===start-group=== "APPEND on map!"
	--test-- "append/insert"
	;@@ https://github.com/Oldes/Rebol-issues/issues/789
	m: make map! []
	--assert 1 = length? append m [a 1]
	--assert 2 = length? insert m [b 2]
	--assert [1 2] = values-of m


	--test-- "appen on map with /part and /dup"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1158
	m: make map! [a 1]
	--assert 1 = length? m
	--assert 1 = length? append/part m [b 2] 0
	--assert 2 = length? append/part m [b 2] 2
	--assert 2 = length? append/dup  m [c 3] 0
	--assert 3 = length? append/dup  m [c 3] 1
	--assert 4 = length? append/dup  m [d 4] 2
	--assert 4 = length? append/part m [e 5 f 6] 1 ;no-op!
	--assert 5 = length? append/part m [e 5 f 6] 2
	--assert [1 2 3 4 5] = values-of m

	--test-- "append/part map! with odd part"
	--assert []     == body-of append/part make map! [] [a 1 b 2 c 3] 1
	--assert [a: 1] == body-of append/part make map! [] [a 1 b 2 c 3] 2
	--assert [a: 1] == body-of append/part make map! [] [a 1 b 2 c 3] 3
	--assert []     == body-of append/part make map! [] tail [a 1 b 2 c 3] -1
	--assert [c: 3] == body-of append/part make map! [] tail [a 1 b 2 c 3] -2
	--assert [2 c]  == body-of append/part make map! [] tail [a 1 b 2 c 3] -3 ;NOTE that it is not [c: 3] and its ok!

===end-group===


===start-group=== "find"

	--test-- "map-find-1"
		mf1-m: #(a: none b: 1 c: 2)
		--assert 'a		= find mf1-m 'a
		--assert 'b		= find mf1-m 'b
		--assert 'c		= find mf1-m 'c
		--assert none	= find mf1-m 'd
		
	--test-- "map-find-2"
		mf2-m: #(a: 1 b: 2 c: 3)
		mf2-m/a: 'none
		mf2-m/b: none
		--assert 'a = find mf2-m 'a
		--assert 'b = find mf2-m 'b
		--assert 'c = find mf2-m 'c

	--test-- "map-find-3"
		mf3-m: #(aB: 1 ab: 2 AB: 3)
		--assert 'ab = find/case mf3-m 'ab
		--assert none = find/case mf3-m 'Ab

	--test-- "map-find-4"
		mf4-m: make map! [b 1]
		--assert 'b	= find mf4-m first [b:]
		--assert 'b	= find mf4-m first [b:]
===end-group===


===start-group=== "map issues"
	;@@ https://github.com/Oldes/Rebol-issues/issues/699
	--test-- "map-issue-699"
		m: make map! []
		--assert 1 = m/(#{01}): 1
		--assert 1 = m/(#{01}): 1
		--assert 2 = m/("a"): 2
		--assert 2 = m/("a"): 2

	;@@ https://github.com/Oldes/Rebol-issues/issues/770
	--test-- "map-issue-770"
		--assert  map? m: make map! [a: 1]
		--assert  1 = m/a
		--assert  not empty? m
		--assert  empty? clear m

	;@@ https://github.com/Oldes/Rebol-issues/issues/598
	--test-- "map-issue-598"
		--assert all [
			map? m: try [make map! [[a] 1]]
			1 = select m [a]
			2 = poke m [b] 2
			1 = m/([a])
			2 = pick m [b]
		]
		;- note that keys are not implicitly protected!
		k: [c]
		poke m :k 3
		--assert [[a] [b] [c]] = keys-of m
		;- so it's possible to create a map with multiple same keys!
		append clear k 'b
		--assert [[a] [b] [b]] = keys-of m
		;- it's still possible to protect keys manually like:
		k: protect [d] clear m
		poke m :k 4
		--assert [[d]] = keys-of m
		--assert error? try [append clear k 'b]

	;@@ https://github.com/Oldes/Rebol-issues/issues/1872
	--test-- "map-issue-1872"
		m: map [a: 42]
		protect m
		--assert protected? m
		;--assert protected? 'm/a ;@@ <--- fails!
		--assert error? err: try [m/a: 0]
		--assert err/id = 'protected
		--assert 42 = try [m/a]
		unprotect m

	;@@ https://github.com/Oldes/Rebol-issues/issues/2293
	--test-- "map-issue-2293"
		m: make map! []
		m/b: copy [1 2]
		n: copy/deep m
		--assert not same? n/b m/b

	;@@ https://github.com/Oldes/Rebol-issues/issues/2220
	--test-- "map-issue-2220"
		; using at least 8 keys (initial map implemention was using internally block for small maps)
		keys: ["a" #{} #{00} a@b http://a http://b %file1 %file2]
		m: map collect [repeat i 8 [keep keys/:i keep i]]
		--assert keys = keys-of m
		repeat i 8 [
			--assert i = select m keys/:i
			append keys/:i #"X"
			--assert none? select m keys/:i
		]

	;@@ https://github.com/Oldes/Rebol-issues/issues/555
	--test-- "map-issue-555"
		a: make map! []
		--assert map? make map! a

	;@@ https://github.com/Oldes/Rebol-issues/issues/472
	--test-- "clear map"
		--assert empty? clear make map! [a: 1]

	;@@ https://github.com/Oldes/Rebol-issues/issues/1864
	--test-- "copy/deep map"
		m1: #(b: [1])
		m2: copy m1
		m3: copy/deep m1
		--assert     same? m1/b m2/b
		--assert not same? m1/b m3/b
		append m1/b 2
		--assert m1/b = [1 2]
		--assert m2/b = [1 2]
		--assert m3/b = [1]

	;@@ https://github.com/Oldes/Rebol-issues/issues/1315
	--test-- "map string sensitivity"
		m: make map! ["foo" 1 "FOO" 2 %foo 1 %FOO 2]
		--assert ["foo" "FOO" %foo %FOO] = keys-of m

	;@@ https://github.com/Oldes/Rebol-issues/issues/968
	;@@ https://github.com/Oldes/Rebol-issues/issues/969
	;@@ https://github.com/Oldes/Rebol-issues/issues/970
	--test-- "strict-equal? and strict-not-equal? on map"
		m: make map! []
		--assert same? m m
		--assert strict-equal? m m
		--assert not strict-equal? "" m
		--assert not strict-not-equal? m m
		--assert strict-not-equal? "" m
		;@@ https://github.com/Oldes/Rebol-issues/issues/905
		--assert m == m
		--assert not (m !== m)
		--assert "" !== m

===end-group===


===start-group=== "FORM map!"
	--test-- "form empty map"
		--assert "" = form #()
	--test-- "form single key map"
		--assert "a: 1" = form #(a: 1)
	--test-- "form map with multiple keys"
		--assert "a: 1^/b: 2" = form #(a: 1 b: 2) ; there should not be line break at tail
===end-group===


===start-group=== "PUT"
	--test-- "PUT into map"
		m: #(a: 42)
		--assert "foo" = put m 'b "foo"
		--assert "baz" = put m 'a "baz"
		--assert "foo" = m/b
		--assert "baz" = m/a

	--test-- "PUT into protected map"
		m: #(a: 42)
		protect m
		--assert error? err: try [put m 'b "foo"]
		--assert err/id = 'protected
		--assert error? err: try [put m 'a "baz"]
		--assert err/id = 'protected
		unprotect m

===end-group===

===start-group=== "reflection"
	;@@ https://github.com/Oldes/Rebol-issues/issues/597
	;@@ https://github.com/Oldes/Rebol-issues/issues/774
	m: make map! b: [a 1 b: 2 :c 3 'd 4 #e 5 /f 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11 3.14 12 1x0 13 $1 14]
	--test-- "body of map"
		--assert  [a  1 b: 2 :c 3 'd 4 #e 5 /f 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11 3.14 12 1x0 13 $1 14] = b
		--assert  [a: 1 b: 2 c: 3 d: 4 e: 5 f: 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11 3.14 12 1x0 13 $1 14] = body-of m
		--assert  [a: 1 b: 2 c: 3 d: 4 e: 5 f: 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11 3.14 12 1x0 13 $1 14] = reflect m 'body

	--test-- "values of map"
		--assert  [1 2 3 4 5 6 7 8 9 10 11 12 13 14] = values-of m
		--assert  [1 2 3 4 5 6 7 8 9 10 11 12 13 14] = reflect m 'values

	--test-- "keys of map"
		--assert  [a b c d e f "a" <b> 9 #"c" a@b 3.14 1x0 $1] = keys-of m
		--assert  [a b c d e f "a" <b> 9 #"c" a@b 3.14 1x0 $1] = reflect m 'words

	--test-- "spec of map"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1859
		--assert all [
			error? e: try [ spec-of #() ]
			e/id = 'expect-arg
			e/arg2 = 'value
			e/arg3 = map!
		]
	--test-- "path expression on map!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2421
		foreach k keys-of m [ 
			--assert not error? try [m/:k]
		]

	--test-- "block of map"
	;@@ https://github.com/Oldes/Rebol-wishes/issues/31
		--assert {a: 1^/b: 2^/c: 3^/d: 4^/e: 5^/f: 6^/"a" 7^/<b> 8^/9 9^/#"c" 10^/a@b 11^/3.14 12^/1x0 13^/$1 14}
		         = mold/only to block! m
		--assert {[^/    a: 1^/    b: 2^/]} = mold to block! #(a: 1 b: 2)

	--test-- "more keys.."
	;@@ https://github.com/Oldes/Rebol-issues/issues/1804
		m: make map! [1-1-2000 1 10:00 2 1.1.1 3 ]
		--assert [1-Jan-2000 10:00 1.1.1] = keys-of m

===end-group===

===start-group=== "MAP with NONE"
	;@@ https://github.com/Oldes/Rebol-wishes/issues/21
	--test-- "map with none"
		m: #(a: #[none] b: 1)
		m/b: none
		--assert [a b] = keys-of m
		--assert [#[none] #[none]] = values-of m
	--test-- "foreach on map with none"
		o: copy ""
		foreach [k v] m [append o k]
		--assert o = "ab"

	--test-- "remove from map"
	;@@ https://github.com/Oldes/Rebol-wishes/issues/20
		m: #("ab" 1 "AB" 2)
		--assert ["ab" 1 "AB" 2]  = to block! remove/key m "aB"
		--assert 2 = length? m
		--assert ["ab" 1       ]  = to block! remove/key m "AB"
		--assert 1 = length? m
		m: #(ab: 1 AB: 2)
		--assert [ab: 1 AB: 2]  = to block! remove/key m 'aB
		--assert 2 = length? m
		--assert [ab: 1      ]  = to block! remove/key m 'AB
		--assert 1 = length? m

===end-group===

===start-group=== "remove-each with map!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/806
	;@@ https://github.com/red/REP/issues/93
	--test-- "remove-each with map"
		m: #(a 1 "b" 2 c #[none] d: 3)
		--assert m = remove-each [k v] m [any [string? k none? v]]
		--assert [a d] = words-of m
	--test-- "remove-each/count with map"
		m: #(a 1 "b" 2 c #[none] d: 3)
		--assert 2 = remove-each/count [k v] m [any [string? k none? v]]
		--assert [a d] = words-of m

===end-group===


===start-group=== "set operations with map!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1984
	m1: #(a: 1 A: 2 "b" 3 "B" 4)
	m2: #(a: 2 c: 5 "b" 6 "D" 7)
	--test-- "difference with map"
		--assert (mold/flat difference m1 m1) = {#()}
		--assert (mold/flat difference m1 m2) = {#(c: 5 "D" 7)}
		--assert (mold/flat difference m2 m1) = {#(c: 5 "D" 7)}
		--assert (mold/flat difference m2 m2) = {#()}

		--assert (mold/flat difference/case m1 m1) = {#()}
		--assert (mold/flat difference/case m1 m2) = {#(A: 2 "B" 4 c: 5 "D" 7)}
		--assert (mold/flat difference/case m2 m1) = {#(c: 5 "D" 7 A: 2 "B" 4)}
		--assert (mold/flat difference/case m2 m2) = {#()}

	--test-- "exclude with map"
		--assert (mold/flat exclude m1 m1) = {#()}
		--assert (mold/flat exclude m1 m2) = {#()}
		--assert (mold/flat exclude m2 m1) = {#(c: 5 "D" 7)}
		--assert (mold/flat exclude m2 m2) = {#()}

		--assert (mold/flat exclude/case m1 m1) = {#()}
		--assert (mold/flat exclude/case m1 m2) = {#(A: 2 "B" 4)}
		--assert (mold/flat exclude/case m2 m1) = {#(c: 5 "D" 7)}
		--assert (mold/flat exclude/case m2 m2) = {#()}

	--test-- "intersect with map"
		--assert (mold/flat intersect m1 m1) = {#(a: 1 "b" 3)}
		--assert (mold/flat intersect m1 m2) = {#(a: 1 "b" 3)}
		--assert (mold/flat intersect m2 m1) = {#(a: 2 "b" 6)}
		--assert (mold/flat intersect m2 m2) = {#(a: 2 c: 5 "b" 6 "D" 7)}

		--assert (mold/flat intersect/case m1 m1) = {#(a: 1 A: 2 "b" 3 "B" 4)}
		--assert (mold/flat intersect/case m1 m2) = {#(a: 1 "b" 3)}
		--assert (mold/flat intersect/case m2 m1) = {#(a: 2 "b" 6)}
		--assert (mold/flat intersect/case m2 m2) = {#(a: 2 c: 5 "b" 6 "D" 7)}

	--test-- "union with map"
		--assert (mold/flat union m1 m1) = {#(a: 1 "b" 3)}
		--assert (mold/flat union m1 m2) = {#(a: 1 "b" 3 c: 5 "D" 7)}
		--assert (mold/flat union m2 m1) = {#(a: 2 c: 5 "b" 6 "D" 7)}
		--assert (mold/flat union m2 m2) = {#(a: 2 c: 5 "b" 6 "D" 7)}

		--assert (mold/flat union/case m1 m1) = {#(a: 1 A: 2 "b" 3 "B" 4)}
		--assert (mold/flat union/case m1 m2) = {#(a: 1 A: 2 "b" 3 "B" 4 c: 5 "D" 7)}
		--assert (mold/flat union/case m2 m1) = {#(a: 2 c: 5 "b" 6 "D" 7 A: 2 "B" 4)}
		--assert (mold/flat union/case m2 m2) = {#(a: 2 c: 5 "b" 6 "D" 7)}

	--test-- "unique with map"
		--assert (mold/flat unique m1) = {#(a: 1 "b" 3)}
		--assert (mold/flat unique/case m1) = {#(a: 1 A: 2 "b" 3 "B" 4)}


===end-group===

~~~end-file~~~