Rebol [
	Title:   "Rebol3 event test script"
	Author:  "Oldes"
	File: 	 %event-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "EVENT"

===start-group=== "event"

--test-- "make event!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/132
	--assert  event? e: make event! [type: 'connect]
	--assert  'connect = e/type

===end-group===

~~~end-file~~~