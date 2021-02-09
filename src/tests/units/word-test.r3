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