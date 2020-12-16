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


===start-group=== "MAKE object"

	;@@ https://github.com/Oldes/Rebol-issues/issues/711
	--test-- "make object bounds-check v1"
		a: make object! [a: 1 b: func [c] [exit]]
		d: 0
		--assert error? try [make a [set 'd b]]
		--assert d = 0
		protect/hide in a 'a
		--assert error? try [make a [set 'd b]]
		--assert d = 0

	--test-- "make object bounds-check v2"
		a: make object! [b: func [c] [exit] a: 1]
		protect in a 'a
		--assert error? try [make a [set 'd b]]
		--assert d = 0

	--test-- "make object bounds-check v3"
		a: make object! [a: 1] b: func [c] [exit]
		protect in a 'a
		--assert error? try [make a [set 'd b]]
		--assert d = 0

	--test-- "issue-1863"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1863
		--assert all [
			o1: make object! [a: 1 f: does [a]]
			o2: make object! [a: 2]
			o3: make o1 o2
			2 == o3/f
		]
	--test-- "issue-1874"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1874
		a: make object! [b: []]
		c: make a []
		d: make a make object! []
		--assert not same? a/b c/b
		--assert not same? a/b d/b

	--test-- "issue-2049"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2049
		; when cloning an object, functions are cloned:
		o: make object! [n: 'o f: func [] [n]]
		--assert 'o = o/f
		p: make o [n: 'p]
		--assert 'p = p/f

		; while closures are ignored:
		o: make object! [n: 'o f: closure [] [n]]
		--assert 'o = o/f
		p: make o [n: 'p]
		--assert 'p = p/f

	--test-- "issue-2050"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2050
		o: make object! [n: 'o b: reduce [func [] [n]]]
		p: make o [n: 'p]
		--assert 'o = o/b/1

	--test-- "issue-2045-a"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2045
		a: 1
		f: func [] [a]
		g: :f
		o1: make object! [a: 2 g: :f]
		o2: make o1 [a: 3 g: :f]
		o3: make o1 [a: 4]
		--assert 1 = g
		--assert 1 = o1/g
		--assert 1 = o2/g
		--assert 1 = o3/g
	--test-- "issue-2045-b"
		a: 1
		b: [a]
		c: b
		o1: make object! [a: 2 c: b]
		o2: make o1 [a: 3 c: b]
		o3: make o1 [a: 4]
		--assert 1 = do c
		--assert 1 = do o1/c
		--assert 1 = do o2/c
		--assert 1 = do o3/c

	--test-- "issue-2118"
	;@@ https://github.com/oldes/rebol-issues/issues/2118
		--assert error? try [object [a: b:]]
		--assert all [
			not error? try [o: construct [a: b:]]
			none? o/a
			none? o/b
		]

	--test-- "construct"
	;@@ https://github.com/Oldes/Rebol-issues/issues/651
		--assert logic? get in construct [a: true] 'a
		--assert logic? get in construct [a: false] 'a
		--assert logic? get in construct [a: on] 'a
		--assert logic? get in construct [a: off] 'a
		--assert logic? get in construct [a: yes] 'a
		--assert logic? get in construct [a: no] 'a
		--assert none? get in construct [a: none] 'a
		--assert none? get/any in construct head insert tail [a:]() 'a
		--assert word? get in construct [a: b] 'a
		--assert word? get in construct [a: 'b] 'a
		--assert path? get in construct [a: b/c] 'a
		--assert path? get in construct [a: 'b/c] 'a

	--test-- "construct/only"
	;@@ https://github.com/Oldes/Rebol-issues/issues/687
		--assert word? get in construct/only [a: true] 'a
		--assert word? get in construct/only [a: false] 'a
		--assert word? get in construct/only [a: on] 'a
		--assert word? get in construct/only [a: off] 'a
		--assert word? get in construct/only [a: yes] 'a
		--assert word? get in construct/only [a: no] 'a
		--assert word? get in construct/only [a: none] 'a
		--assert unset? get/any in construct/only head insert tail [a:]() 'a
		--assert word? get in construct/only [a: b] 'a
		--assert lit-word? get in construct/only [a: 'b] 'a
		--assert path? get in construct/only [a: b/c] 'a
		--assert lit-path? get in construct/only [a: 'b/c] 'a

	--test-- "construct {a: 1}"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1554
		--assert ["1"] = values-of construct "a: 1"
		--assert ["1"] = values-of construct to-binary "a: 1"
		--assert ["1" "yes"] = values-of construct "a: 1^/b: yes"
		--assert ["1 b: yes"] = values-of construct "a: 1 b: yes" ; there is not the newline!

===end-group===


===start-group=== "Compare object"
	--test-- "issue-281"
	;@@ https://github.com/Oldes/Rebol-issues/issues/281
		a: context [b: "test" c: 123]
		b: context [b: "test" c: 123]
		--assert a = b
		--assert equal? a b
		--assert not same? a b
===end-group===

===start-group=== "Object actions"
	--test-- "empty?"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1669
	--assert empty? object []
	--assert empty? #[object! []]
	--test-- "length?"
	--assert 0 = length? object []
	--assert 0 = length? #[object! []]

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

	--test-- "issue-2076"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2076
		--assert object? o: bound? use [x] ['x]
		--assert object? append o 'self

===end-group===

===start-group=== "PROTECT object!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1014
	--test-- "protect object"
		; To prevent adding words to o, and prevent modification of words already in o: 
		o: protect object [a: object [b: 10]]
		--assert     protected?  o
		--assert not protected? 'o
		--assert     protected? 'o/a
		--assert not protected? 'o/a/b
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

	--test-- "unset with protected value"
		;@@ https://github.com/Oldes/Rebol-issues/issues/961
		o: protect/deep make object! [w: 0]
		--assert error? try [unset in o 'w]

	--test-- "protected inner object"
		o: object [a: 1 o: object [a: 2]]
		protect/words/deep 'o/o
		--assert not protected? 'o/a
		--assert     protected? 'o/o
		--assert     protected? 'o/o/a


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



===start-group=== "USE"
	--test-- "issue-197"
	;@@ https://github.com/Oldes/Rebol-issues/issues/197
	--assert object? o: use [a] [context? 'a]
	--assert [a] = words-of o

===end-group===


~~~end-file~~~
