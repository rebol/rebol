Rebol [
	Title:   "Rebol gob! test script"
	Author:  "Peter W A Wood, Oldes"
	File: 	 %gob-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "gob!"



===start-group=== "INDEX? / INDEXZ?"
	g: make gob! [] loop 2 [append g make gob! []]
	--test-- "index? gob!"
		--assert 1   = index? g
		--assert 1   = index? back g
		--assert 2   = index? next g
		--assert 3   = index? tail g
		--assert 2   = index? at g 2
	--test-- "indexz? gob!"
		--assert 0   = indexz? g
		--assert 0   = indexz? back g
		--assert 1   = indexz? next g
		--assert 2   = indexz? tail g
		--assert 1   = indexz? atz g 1
===end-group===

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

	--test-- "gob require set-word!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/976
		--assert all [
			error? err: try [make gob! [color 127.0.127]]
			err/id = 'expect-val
			err/arg1 = set-word!
		]

	--test-- "gob parent changes"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1671
		g: make gob! []
		append g g2: make gob! []
		--assert g2/parent = g
		change g g3: make gob! []
		--assert g3/parent = g
		poke g 1 g4: make gob! []
		--assert g4/parent = g

	--test-- "gob/flags"
	;@@ https://github.com/Oldes/Rebol-issues/issues/332
		g: make gob! []
		g/flags: [resize]
		g/flags: [popup]
		--assert g/flags = [popup]

	--test-- "issue-1714"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1714
		g: make gob! []
		g/color: red
		g/color: none
		--assert not error? try [mold g] ; no crash

	--test-- "issue-1617"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1617
		--assert all [
			error? e: try [make gob! [data:]]
			e/id = 'need-value
		]
		--assert all [
			error? e: try [make gob! [data: size: 10x10]]
			e/id = 'need-value
		]
		--assert all [
			error? e: try [make gob! append [data:] make unset! none]
			e/id = 'need-value
		]

===end-group===

~~~end-file~~~

