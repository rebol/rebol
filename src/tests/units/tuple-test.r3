Rebol [
	Title:   "Rebol3 tuple test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %tuple-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "TUPLE!"

===start-group=== "tuple"
	--test-- "tuple divide"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1974
	--assert (1.1.1 / 0.1)                   == 10.10.10
	--assert (1.1.1 / 0.625)                 == 2.2.2        ;because round 1 / 0.625 = 2.0 
	--assert (1.1.1 / 1.953125E-3)           == 255.255.255
	--assert (1.1.1 / -1.0)                  == 0.0.0
	--assert (1.1.1 / 4.656612873077393e-10) == 255.255.255

	--test-- "tuple multiply"
	--assert (1.1.1 * 2147483648.0)          == 255.255.255
	--assert (1.1.1 * 4.656612873077e+100)   == 255.255.255
	--assert (1.1.1 * 4656612873077)         == 255.255.255
	--assert (0.0.0 * 4656612873077)         == 0.0.0

	--test-- "to-tuple tuple!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/180
	--assert 1.1.1 = to-tuple 1.1.1

	--test-- "reverse tuple"
	;@@ https://github.com/Oldes/Rebol-issues/issues/211
	--assert 3.2.1 = reverse 1.2.3

	--test-- "reverse/part tuple"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1810
	--assert 3.2.1.4.5 = reverse/part 1.2.3.4.5 3

	--test-- "poke on tuple not supported"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1070
	t: 1.2.3.4
	--assert all [
		error? e: try [poke t 2 99]
		e/id = 'expect-arg
	]
	--assert all [
		99 = (t/2: 99)
		t = 1.99.3.4
	]

	--test-- "tuple shortening"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1639
	t: 1.2.3.4 t/4: none
	--assert t == 1.2.3
	--assert (t + 0.0.0.0) == 1.2.3.0
	
===end-group===

~~~end-file~~~