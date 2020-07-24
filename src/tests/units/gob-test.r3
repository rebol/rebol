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

	--test-- "reverse gob"
	;@@ https://github.com/Oldes/Rebol-issues/issues/211
	a: make gob! [size: 1x1]
	b: make gob! [size: 2x2]
	repend c: make gob! [] [a b] 
	--assert c/1 = a
	--assert c/2 = b
	--assert gob? reverse c
	--assert c/1 = b
	--assert c/2 = a

	--test-- "simple paths inside GOB"
	;@@ https://github.com/Oldes/Rebol-issues/issues/165
	try [
		wh: 100x100
		g1: make gob! [size: wh]
		g2: make gob! [size: g1/size]
	]
	--assert g1/size = wh
	--assert g2/size = wh

	--test-- "gob hidden flag"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1579
		g: make gob! [flags: [hidden]]
		--assert g/flags = [hidden]

===end-group===

~~~end-file~~~

