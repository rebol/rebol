Rebol [
	Title:   "Rebol3 task test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %task-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "TASK!"

===start-group=== "task"
	if any [
		not error? test-task: try [make task! []] ; task may be disabled
		test-task/id <> 'feature-na
	][
		--test-- "empty task"
		--assert task? do test-task

		--test-- "issues-47"
		;@@ https://github.com/Oldes/Rebol-issues/issues/47
		--assert  task? do make task! [1 / 0] ; trying to evaluate a task should not crash!

		--test-- "issues-204"
		;@@ https://github.com/Oldes/Rebol-issues/issues/204
		--assert string? mold test-task
		--assert string? append "" test-task
	]

===end-group===

~~~end-file~~~