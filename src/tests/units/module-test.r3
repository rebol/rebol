Rebol [
	Title:   "Rebol module! test script"
	Author:  "Peter W A Wood, Oldes"
	File: 	 %module-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "module!"

===start-group=== "module issues"
	--test-- "issue-1708"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1708
		--assert none? find system/modules module [] []


===end-group===

~~~end-file~~~

