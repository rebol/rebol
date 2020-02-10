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

===end-group===

===start-group=== "map issues"
	;@@ https://github.com/rebol/rebol-issues/issues/770
	--test-- "map-issue-770"
		--assert  map? m: make map! [a: 1]
		--assert  1 = m/a
		--assert  not empty? m
		--assert  empty? clear m

	;@@ https://github.com/rebol/rebol-issues/issues/598
	--test-- "map-issue-598"
		--assert error? try [make map! [[a] 1]]

	;@@ https://github.com/rebol/rebol-issues/issues/1872
	--test-- "map-issue-1872"
		m: map [a: 42]
		protect m
		--assert error? err: try [m/a: 0]
		--assert err/id = 'protected
		--assert 42 = try [m/a]
		unprotect m

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