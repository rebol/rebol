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

===start-group=== "MAKE"
	--test-- "to pair! string! (long)"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2202
		; this is ok:
		--assert "661.1194x510.1062" = mold to pair! "661.1194458007812x510.106201171875"
		; but this should be too:
		--assert  661.1194x510.1062  =      to pair! "661.1194458007812x510.106201171875"
===end-group===


===start-group=== "SERIES"
--test-- "AS with protect"
	b: protect [a b]
	--assert path? try [p: as path! b]
	--assert protected? b
	--assert protected? p ;@@ <--- fails!
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


===start-group=== "union"
		
	--test-- "u-1"
		c1: charset "0123456789"
		c2: charset [#"a" - #"z"]
		u: "make bitset! #{000000000000FFC0000000007FFFFFE0}"
		--assert u = mold union c1 c2
		--assert u = mold union c2 c1

	--test-- "u-2"
		nd: charset [not #"0" - #"9"]
		zero: charset #"0"
		nd-zero: union nd zero
		--assert not find nd #"0"
		--assert not find nd #"1"
		--assert find nd #"B"
		--assert find nd #"}"

	--test-- "u-3"
		--assert find zero #"0"
		--assert not find zero #"1"
		--assert not find zero #"B"
		--assert not find zero #"}"

	--test-- "u-4"
		--assert find nd-zero #"0"
		--assert not find nd-zero #"1"
		--assert find nd-zero #"B"
		--assert find nd-zero #"}"
	
===end-group===

===start-group=== "and"

	--test-- "and-1"
		c1: charset "b"
		c2: charset "1"
		u: "make bitset! #{00000000000000}"
		--assert u = mold c1 and c2
		--assert u = mold c2 and c1

	--test-- "and-2"
		c1: charset "b"
		c2: charset "1"
		c3: complement c1
		u: "make bitset! [not #{FFFFFFFFFFFFBF}]"
		--assert u = mold c3 and c2
		--assert u = mold c2 and c3
		u: "make bitset! [not #{FFFFFFFFFFFFFFFFFFFFFFFFFF}]"
		--assert u = mold c1 and c3
		c4: complement c2
		--assert "make bitset! #{FFFFFFFFFFFFBF}" = mold c3 and c4

===end-group===

===start-group=== "xor"

	--test-- "xor-1"
		c1: charset "b"
		c2: charset "1"
		u: "make bitset! #{00000000000040000000000020}"
		--assert u = mold c1 xor c2
		--assert u = mold c2 xor c1

	--test-- "xor-2"
		c1: charset "b"
		c2: charset "1"
		c3: complement c1
		u: "make bitset! [not bits #{00000000000040000000000020}]"
		--assert u = mold c3 xor c2
		--assert u = mold c2 xor c3
		u: "make bitset! [not bits #{00000000000000000000000000}]"
		--assert u = mold c1 xor c3
		c4: complement c2
		--assert "make bitset! #{00000000000040FFFFFFFFFFDF}" = mold c3 xor c4

===end-group===


===start-group=== "Valgrind issues"
;@@ these tests passes but Valgrind reports a memory leak
;@@ (I believe it is false report so it is moved here as I don't want to spend time on it now)
;@@ all of them are related to stack expansion
--test-- {compose large block}
	;@@ https://github.com/Oldes/Rebol-issues/issues/1906
	b: copy [] insert/dup b 1 32768
	--assert b = compose b ;- no crash
--test-- "stack expansion"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2161
	--assert 128000 = length? rejoin array/initial 128000 #{00}
--test-- "stack expansion 2"
	;@@ https://github.com/Oldes/Rebol-issues/issues/953
	a: copy [] loop 250000 [append a random/secure 1000]
	--assert 250000 = join [] a

===end-group===

~~~end-file~~~