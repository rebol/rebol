Rebol [
	Title:   "Rebol3 word test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %word-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "WORD!"

===start-group=== "word"
	--test-- "Length? any-word!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1740
	;@@ https://github.com/Oldes/Rebol-issues/issues/2224
	--assert 1 = length? #a
	--assert 1 = length? 'a
	--assert 1 = length? quote 'a
	--assert 2 = length? #ša
	--assert 2 = length? quote 'ša
	--assert 1 = length? to-word to-string to-char 126
	--assert 1 = length? to-word to-string to-char 128

	--test-- "Word from tag"
	;@@ https://github.com/Oldes/Rebol-wishes/issues/2
	--assert 'a = to word! <a>
	--assert error? try [to word! <a b>]

	--test-- "invalid construction"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1167
	--assert all [error? e: try [to-word "a,"]  e/id = 'invalid-chars]
	--assert all [error? e: try [to-word "a;"]  e/id = 'invalid-chars]
	--assert all [error? e: try [to-word "a["]  e/id = 'invalid-chars]
	--assert all [error? e: try [to-word "a[]"] e/id = 'invalid-chars]
	;@@ https://github.com/Oldes/Rebol-issues/issues/330
	--assert all [error? e: try [to-word "a b"] e/id = 'invalid-chars]

	--test-- "word from /1"
	;@@ https://github.com/Oldes/Rebol-issues/issues/733
	; allowed so far..
	--assert word? try [to word! /1]
	;TODO: write mold test... should be molded to: #[word! "1"]

	--test-- "to word! string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2444
	find-nonloadable-words: function [][
		nlw: copy []
		for n 0 255 1 [
			w: copy []
			append w rejoin [""  to char! n "x"] ;; test as leading letter
			append w rejoin ["x" to char! n    ] ;; test as trailing letter
			append w rejoin ["x" to char! n "x"] ;; test as mid letter
			foreach c w [
				if not error? try [to word! c][ ;; test only those that can be to-worded
					s: join trim/tail c ": 999" ;; trimmed for "x^-" and "x " inputs (allowed)
					if any [
						error? try [unset? load s] ;; Can we load it?
						error? try [unset?   do s] ;; Can we do the assign x: 999 ?
						unset? do s ;; Did we get 999 if we did?
						999 <> do s
					][
						append nlw c ;; none of the above: it's not a serialisable word
					]
				]
			]
		]
		nlw
	]
	--assert empty? find-nonloadable-words


===end-group===

===start-group=== "word compare"
	--test-- "lesser-or-equal?"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2251
	--assert not lesser-or-equal? 'C 'a
	--assert     lesser-or-equal? 'c 'C
	--assert not lesser-or-equal? 'c 'a

	--test-- "equal?"
	--assert not equal? 'a 'b
	--assert     equal? 'a 'A
	--assert 'a = 'A

===end-group===

===start-group=== "word issues"
	--test-- "issue-368"
	;@@ https://github.com/Oldes/Rebol-issues/issues/368
	--assert (type? :set) = (type? :SET)
	--assert (type? :print) = (type? :PRINT)
	--assert 'set = 'SET
	--assert 'print = 'PRINT

	--test-- "to-word char!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1840
	--assert word? try [to-word #"d"]
	--assert word? try [to-word  "d"]

	--test-- "set issue! & refinement!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1663
	--assert all [
		error? e: try [set #issue 9999]
		e/id = 'expect-arg
	]
	--assert all [
		error? e: try [set /ref-word 9999]
		e/id = 'expect-arg
	]

===end-group===

~~~end-file~~~