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


===end-group===

===start-group=== "APPEND on map!"
	--test-- "appen on map with /part and /dup"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1158
	m: make map! [a 1]
	--assert 1 = length? m
	--assert 1 = length? append/part m [b 2] 0
	--assert 2 = length? append/part m [b 2] 2
	--assert 2 = length? append/dup  m [c 3] 0
	--assert 3 = length? append/dup  m [c 3] 1
	--assert 4 = length? append/dup  m [d 4] 2
	--assert 5 = length? append/part m [e 5 f 6] 1
	--assert [1 2 3 4 5] = values-of m

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
		--assert error? try [make map! [[a] 1]]

	;@@ https://github.com/Oldes/Rebol-issues/issues/1872
	--test-- "map-issue-1872"
		m: map [a: 42]
		protect m
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
	m: make map! b: [a 1 b: 2 :c 3 'd 4 #e 5 /f 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11]
	--test-- "body of map"
		--assert  [a  1 b: 2 :c 3 'd 4 #e 5 /f 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11] = b
		--assert  [a: 1 b: 2 c: 3 d: 4 e: 5 f: 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11] = body-of m
		--assert  [a: 1 b: 2 c: 3 d: 4 e: 5 f: 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11] = reflect m 'body

	--test-- "values of map"
		--assert  [1 2 3 4 5 6 7 8 9 10 11] = values-of m
		--assert  [1 2 3 4 5 6 7 8 9 10 11] = reflect m 'values

	--test-- "words of map"
		--assert  [a b c d e f "a" <b> 9 #"c" a@b] = words-of m
		--assert  [a b c d e f "a" <b> 9 #"c" a@b] = reflect m 'words

===end-group===

~~~end-file~~~