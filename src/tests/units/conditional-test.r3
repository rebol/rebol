Rebol [
	Title:   "Rebol conditional test script"
	Author:  "Olds"
	File: 	 %conditional-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Conditional"

===start-group=== "Dealing with unset! value in conditions"
	--test-- "any"
		--assert unset? any [() 2]
		--assert true?  any [() 2]
	--test-- "all"
		--assert true? all []
		--assert true? all [()]
		--assert 3 = all [() 1 2 3]
		--assert unset? all [1 ()]
		--assert 1 = if all [1 ()][1]
		--assert 1 = either all [()][1][2]
	--test-- "any and all"
		if any [
			all [false x: 1 ()]
			all [true  x: 2 ()]
		][ x: 2 * x]
		--assert x = 4


===end-group===

~~~end-file~~~