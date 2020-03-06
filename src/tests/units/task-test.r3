Rebol [
	Title:   "Rebol3 task test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %task-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "TASK!"

===start-group=== "task"
	--test-- "issues-47"
	;@@ https://github.com/Oldes/Rebol-issues/issues/47
	--assert error? try [do make task! [2 / 0]] ; trying to evaluate a task should not crash!

===end-group===

~~~end-file~~~