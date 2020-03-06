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

===end-group===

~~~end-file~~~