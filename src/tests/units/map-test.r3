Rebol [
	Title:   "Rebol3 map! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %map-test.red
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "map"

===start-group=== "map issues"
	;@@ https://github.com/rebol/rebol-issues/issues/770
	--test-- "map-issue-770"
		m: make map! [a: 1]
		--assert  1 = m/a
		--assert  not empty? m
		--assert  empty? clear m
===end-group===

~~~end-file~~~