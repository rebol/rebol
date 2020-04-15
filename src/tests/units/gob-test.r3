Rebol [
	Title:   "Rebol gob! test script"
	Author:  "Peter W A Wood, Oldes"
	File: 	 %gob-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "gob!"

===start-group=== "gob issues"
	--test-- "issue-185"
	;@@ https://github.com/Oldes/Rebol-issues/issues/185
		--assert gob? a: make gob! [size: 1x1]
		--assert gob? b: make gob! [size: 2x2]
		--assert a = append a b
		--assert a = append a b
		--assert equal? mold a/pane "[make gob! [offset: 0x0 size: 2x2]]"

===end-group===

~~~end-file~~~

