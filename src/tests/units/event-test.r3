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
	;@@ https://github.com/Oldes/Rebol-issues/issues/2484
	--assert all [event? e2: make e [offset: 0x0] e2/offset = 0x0]

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
--test-- "key event"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1526
	--assert all [
		event? e: try [make event! [type: 'key key: #"A"]]
		e/key == #"A"
		none? e/offset
	]
	--assert all [
		event? e: try [make event! [key: #"B"]]
		e/key == #"B"
		e/type = 'key ; added automaticaly if no type is specified
	]
	--assert all [
		event? e: try [make event! [type: 'custom key: #"C"]]
		none? e/key ; only key and key-up types will provide it
		e/type = 'custom
		e/code = 67
	]
	--assert all [
		event? e: try [make event! [type: 'key-up key: #"C"]]
		e/key == #"C"
		e/type = 'key-up
		e/code = 67
	]

--test-- "custom event"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1821
	--assert all [
		event? e: try [make event! [type: 'custom code: 1]]
		e/type = 'custom
		e/code = 1
	]
	if system/ports/event [
		; using port in the custom event
		--assert all [
			event? e: try [make event! [type: 'custom code: 2 port: system/ports/event]]
			e/type = 'custom
			e/code = 2
			e/port = system/ports/event
		]
	]

===end-group===

~~~end-file~~~