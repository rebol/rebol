Rebol [
	Title:   "Rebol object test script"
	Author:  "Oldes"
	File: 	 %object-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Object"

===start-group=== "Set OBJECT"

--test-- "set OBJECT OBJECT"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2358
	
	 def: object [int: ser: fce: none dec: 42.0]
	data: object [ser: "ah" foo: 'nothing int: 1 fce: does [int: int * 2] ]
	new: copy def
	set new data
	append new/ser "a"
	new/fce

	--assert [int ser fce dec] = words-of new ; only source keys were used
	--assert     2 = new/int  ; fce multiplied the int value
	--assert     1 = data/int ; data were not modified
	--assert  42.0 = new/dec  ; dec value was not modified
	--assert  "ah" = data/ser ; original serie not modified
	--assert "aha" = new/ser  ; only the new one

--test-- "set/only OBJECT OBJECT"
	o: object [a: b: c: none] set o object [a: 1 d: 3]
	--assert all [o/a = 1 none? o/b none? o/c]
	o: object [a: b: c: none] set/only o o2: object [a: 1 d: 3] o
	--assert all [object? o/a object? o/b object? o/c o/a = o/b  o/a = o2]
	; note that if unsed /only, the setter is not being copied
	o2/a: 23
	--assert o/a/a = 23


===end-group===

===start-group=== "EXTEND object"
	--test-- "extend object"
		obj: object []
		--assert 1 = extend obj 'a 1
		--assert 1 = obj/a
===end-group===

===start-group=== "APPEND on OBJECT"
	;@@ https://github.com/Oldes/Rebol-issues/issues/708
	--test-- "issue-708"
		o: object []
		append o 'x
		--assert none? o/x
		append o [y]
		--assert none? o/y
		append o [x: 1 y: 2]
		--assert o/x = 1
		--assert o/y = 2

	--test-- "append on protected object"
		o: object [a: 1]
		protect o
		--assert error? err: try [append o [a: 2]] 
		--assert err/id = 'protected
		unprotect o

	--test-- "issue-75"
	;@@ https://github.com/Oldes/Rebol-issues/issues/75
		o: make object! [a: 1]
		o2: make o [] ; no new words, so words block shared!
		append o2 [b 2]
		--assert [a b] = words-of o2
		--assert [a] = words-of o

===end-group===

===start-group=== "PROTECT object!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1014
	--test-- "protect object"
		; To prevent adding words to o, and prevent modification of words already in o: 
		o: protect object [a: object [b: 10]]
		--assert     error? try [extend o 'c 3]
		--assert     error? try [append o 'd]
		--assert     error? try [o/a: 0]
		--assert not error? try [o/a/b: 0] ;;@@ https://github.com/Oldes/Rebol-issues/issues/1170

	--test-- "protect/words object!"
		; To allow adding words to o, but prevent modification to words already in o:
		o: protect/words object [a: object [b: 10]]
		--assert not error? try [extend o 'c 3]
		--assert not error? try [append o 'd]
		--assert     error? try [o/a: 0]
		--assert not error? try [o/a/b: 0]

	--test-- "protect/deep object!"
		; To prevent adding words to o, modification of words already in o, or their contents:
		o: protect/deep object [a: object [b: 10]]
		--assert     error? try [extend o 'c 3]
		--assert     error? try [append o 'd]
		--assert     error? try [o/a: 0]
		--assert     error? try [o/a/b: 0]

	--test-- "protect/words/deep object!"
		; To allow adding words to o, but prevent modification of words already in o or their contents: 
		o: protect/words/deep object [a: object [b: 10]]
		--assert not error? try [extend o 'c 3]
		--assert not error? try [append o 'd]
		--assert     error? try [o/a: 0]
		--assert     error? try [o/a/b: 0]

===end-group===


===start-group=== "UNPROTECT object!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1015
	--test-- "unprotect object"
		; To allow adding words to o, and allow modification of words already in o, but not affect their values:
		o: unprotect protect/deep o: object [a: 10 b: [20]]
		--assert not error? try [extend o 'c 3]
		--assert not error? try [append o 'd]
		--assert not error? try [o/a: 0]
		--assert     error? try [append o/b 0]

	--test-- "unprotect/words object!"
		; To allow modification of words already in o, but not affect their values or the object itself: 
		o: unprotect/words protect/deep o: object [a: 10 b: [20]]
		--assert     error? try [extend o 'c 3]
		--assert     error? try [append o 'd]
		--assert not error? try [o/a: 0]
		--assert     error? try [append o/b 0]

	--test-- "unprotect/deep object!"
		; To allow adding words to o, and allow modification of words already in o or their values:
		o: unprotect/deep protect/deep o: object [a: 10 b: [20]]
		--assert not error? try [extend o 'c 3]
		--assert not error? try [append o 'd]
		--assert not error? try [o/a: 0]
		--assert not error? try [append o/b 0]

	--test-- "unprotect/words/deep object!"
		; To allow modification of words already in o and their contents, but not affect the object itself: 
		o: unprotect/words/deep protect/deep o: object [a: 10 b: [20]]
		--assert     error? try [extend o 'c 3]
		--assert     error? try [append o 'd]
		--assert not error? try [o/a: 0]
		--assert not error? try [append o/b 0]

===end-group===

~~~end-file~~~
