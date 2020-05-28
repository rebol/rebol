Rebol [
	Title:   "Rebol3 known (not yet fixed) issues"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %word-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "KNOWN PROBLEMS!"

===start-group=== "MAP"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1872
	--test-- "map-issue-1872"
		m: map [a: 42]
		protect m
		--assert protected? 'm/a ;@@ <--- fails!

===end-group===


===start-group=== "SERIES"
--test-- "AS with protect"
	b: protect [a b]
	--assert path? try [p: as path! b]
	--assert protected? b
	--assert protected? p ;@@ <--- fails!
===end-group===


===start-group=== "PARSE"
--test-- "parse remove"
	--assert parse v: "yx" [some [remove #"y" | #"x"] ]
	--assert v = "x"
	--assert parse "yx" [copy v any [ remove #"y" | #"x" ] ]
	--assert v = "x"
===end-group===


===start-group=== "TIME"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2416
	--test-- "time protect 1"
		t: now/time protect 't
		--assert error? e: try [t/hour: 0]
		--assert e/id = 'locked-word
	--test-- "time protect 2"
		protect/words o: object [t: now/time]
		--assert error? e: try [o/t/hour: 0]
		--assert e/id = 'locked-word
	--test-- "time protect 3"
		o: object [t: 1 protect/words o: object [t: now/time]]
		--assert protected? 'o/o/t
		--assert protected? 'o/o/t/hour        ;@@ <--- fails!
		--assert error? e: try [o/o/t/hour: 0] ;@@ <--- fails!
===end-group===


===start-group=== "DATE"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2416
	--test-- "date protect 1"
		d: now protect 'd
		--assert error? e: try [d/year: 0]
		--assert e/id = 'locked-word
		unprotect 'd
	--test-- "date protect 2"
		protect/words o: object [d: now]
		--assert error? e: try [o/d/year: 0]
		--assert e/id = 'locked-word
	--test-- "date protect 3"
		o: object [d: 1 protect/words o: object [d: now]]
		--assert protected? 'o/o/d
		--assert protected? 'o/o/d/year        ;@@ <--- fails!
		--assert error? e: try [o/o/d/year: 0] ;@@ <--- fails!
===end-group===


===start-group=== "CRASH"
--test-- "issue-1865"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1865
	;--assert object? resolve/extend/only context [] context [a1: a2: a3: a4: a5: a6: none] [a] ;- no crash!
	;-- PROBLEM! above code sometimes causes failed bind table check (using: `evoke 3`)
	;-- probably related to the next test too!
	
--test-- "issue-2017"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2017
	--assert object? resolve/extend/only context [] context [a: none] [a] ;- no crash!

===end-group===

~~~end-file~~~