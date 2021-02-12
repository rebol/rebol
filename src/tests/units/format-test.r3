Rebol [
	Title:   "Rebol3 file test script"
	Author:  "Oldes"
	File: 	 %format-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

secure [%/ allow]

~~~start-file~~~ "FORMAT tests"

===start-group=== "FORMAT"
--test-- "issue-532"
	;@@ https://github.com/Oldes/Rebol-issues/issues/532
	str: format [8 -8] ["this will overrun" 123]
	--assert all [
		str = "this wil     123"
		16 = length? str
	]

===end-group===

~~~end-file~~~