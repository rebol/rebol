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
	;@@ https://github.com/rebol/rebol-issues/issues/2358
	
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

~~~end-file~~~
