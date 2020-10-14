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

--test-- "event! offset out of range"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1635
	--assert all [
		error? e: try [make event! [offset: 32768x0]]
		e/id = 'out-of-range
	]
	--assert all [
		error? e: try [make event! [offset: 65536x1]]
		e/id = 'out-of-range
	]
	--assert all [
		event? e: try [make event! [offset: 1x2]]
		e/offset = 1x2
	]

===end-group===

~~~end-file~~~