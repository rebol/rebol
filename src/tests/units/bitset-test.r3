Rebol [
	Title:   "Rebol3 bitset test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %bitset-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "bitset"

===start-group=== "zero? bitset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1353
	--test-- "zero? bitset"
		--assert zero? make bitset! #{}
		--assert zero? make bitset! #{00}
		--assert zero? make bitset! #{0000}
		--assert not zero? make bitset! #{10}
		--assert zero? clear make bitset! #{10}
		--assert zero? complement make bitset! #{FF}
===end-group===

===start-group=== "tail? bitset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/764
	--test-- "tail? bitset"
		--assert tail? make bitset! #{}
		--assert not tail? make bitset! #{00}
===end-group===

===start-group=== "empty? bitset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1353
	--test-- "empty? bitset"
		--assert empty? make bitset! #{}
		--assert not empty? make bitset! #{00}
===end-group===


===start-group=== "make bitset!"
	--test-- "basic make"
		--assert "make bitset! #{}"   = mold make bitset! 0
		--assert "make bitset! #{}"   = mold charset ""
		--assert "make bitset! #{}"   = mold charset []
		--assert "make bitset! #{00}" = mold make bitset! 1
		--assert "make bitset! #{80}" = mold charset #"^(00)"
		--assert "make bitset! #{40}" = mold charset #"^(01)"
		--assert "make bitset! #{000000000000FFC0}" = mold charset "0123456789"
		--assert "make bitset! #{F0}" = mold charset [0 1 2 3]
		--assert error? try [make bitset! [-1]]

	--test-- "make with ranges"	
		--assert "make bitset! #{FF800000FFFF8000048900007FFFFFE0}"
			 = mold charset [#"a" - #"z" 0 - 8 32 - 48 "HELLO"]

	--test-- "make bitset! from bitset!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2420
		--assert #[bitset! #{}] = b1: charset ""
		--assert #[bitset! #{}] = b2: make bitset! b1
		b2/1: true
		--assert #[bitset! #{}]   = b1
		--assert #[bitset! #{40}] = b2

	--test-- "charset"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1092
		--assert #[bitset! #{80}] = charset #"^@"
		--assert #[bitset! #{8000}] = charset/length #"^@" 16

===end-group===

===start-group=== "pick bitset!"
	--test-- "pick-1"
		bs: make bitset! [0 1 2 3]
		--assert 8 = length? bs
		--assert true  = pick bs 0
		--assert true  = pick bs 1
		--assert true  = pick bs 2
		--assert true  = pick bs 3
		--assert false = pick bs 4
		--assert false = pick bs 256
		--assert false = pick bs 257
		--assert false = pick bs 2147483647
		--assert error? try [false = pick bs -2147483648]  ;- not like Red!

	--test-- "pick-2"
		bs: make bitset! [0 1 2 3]
		--assert 8 = length? bs
		--assert true  = bs/0
		--assert true  = bs/1
		--assert true  = bs/2
		--assert true  = bs/3
		--assert false = bs/4
		--assert false = bs/256
		--assert false = bs/257
		--assert false = bs/2147483647
		--assert error? try [false = bs/-2147483648] ;- not like Red!

	--test-- "pick-3"
		bs: make bitset! [0 1 2 3]
		--assert 8 = length? bs
		--assert true  = pick bs #"^(00)"
		--assert true  = pick bs #"^(01)"
		--assert true  = pick bs #"^(02)"
		--assert true  = pick bs #"^(03)"
		--assert false = pick bs #"^(04)"
		--assert false = pick bs #"^(0100)"
		--assert false = pick bs #"^(0101)"

	--test-- "pick-4"
		bs: make bitset! [255 257]
		--assert 264 = length? bs
		--assert true  = pick bs 255
		--assert false = pick bs 256
		--assert true  = pick bs 257
		
	--test-- "pick-5"
		bs: make bitset! [255 256]
		--assert 264 = length? bs
		--assert true = pick bs 255
		--assert true = pick bs 256
		
	--test-- "basic-6"
		bs: make bitset! [65536]
		--assert 65544 = length? bs
		--assert true = pick bs 65536
	
	--test-- "pick-7"
		bs: make bitset! 9
		--assert 16 = length? bs
		bs/7: yes
		--assert bs/7 = true
		--assert bs/8 = false
		bs/8: yes
		--assert bs/8 = true
		--assert bs/9 = false
	
	--test-- "pick-8"
		bs: make bitset! 8
		--assert 8 = length? bs
		bs/7: yes
		--assert bs/7 = true
		--assert bs/8 = false
		bs/8: yes
		--assert 16 = length? bs
		--assert bs/8 = true
		--assert bs/9 = false

	--test-- "pick-9"
		ABC: charset [#"A" #"B" #"C"]
		--assert pick ABC "BCB"
		--assert not pick ABC "BCBX"

===end-group===

===start-group=== "modify"
	
	--test-- "poke-1"
		bs: make bitset! [0 1 2 3]
		poke bs 4 true
		--assert true = pick bs 0
		--assert true = pick bs 1
		--assert true = pick bs 2
		--assert true = pick bs 3
		--assert true = pick bs 4
		--assert false = pick bs 5

	--test-- "poke-2"
		bs: make bitset! [0 1 2 3]
		--assert true = pick bs 0
		poke bs 0 false
		--assert false = pick bs 0
		poke bs 0 true
		--assert true = pick bs 0
		poke bs 0 none
		--assert false = pick bs 0
		bs/0: yes
		--assert bs/0 = true
		bs/0: no
		--assert bs/0 = false
		bs/0: yes
		--assert bs/0 = true
		bs/0: none
		--assert bs/0 = false

	--test-- "append-1"
		bs: make bitset! 8
		--assert 8 = length? bs
		append bs ["hello" #"x" - #"z"]
		--assert "make bitset! #{000000000000000000000000048900E0}" = mold bs

	--test-- "insert-1"
		;@@ https://github.com/Oldes/Rebol-issues/issues/789
		bs: make bitset! 8
		--assert 8 = length? bs
		insert bs ["hello" #"x" - #"z"]
		--assert "make bitset! #{000000000000000000000000048900E0}" = mold bs

	--test-- "clear-1"
		clear bs
		--assert "make bitset! #{}" = mold bs

	--test-- "clear-2"
		bs: charset "^(00)^(01)^(02)^(03)^(04)^(05)^(06)^(07)"
		--assert 8 = length? bs
		--assert "make bitset! #{FF}" = mold bs
		clear bs
		--assert "make bitset! #{}" = mold bs

	--test-- "remove-1"
		;@@ https://github.com/Oldes/Rebol-wishes/issues/20
		bs: charset "012345789"
		--assert 64 = length? bs
		--assert "make bitset! #{000000000000FDC0}" = mold bs
		--assert "make bitset! #{0000000000007DC0}" = mold remove/key bs #"0"
		--assert "make bitset! #{0000000000003DC0}" = mold remove/key bs 49
		--assert "make bitset! #{0000000000000000}" = mold remove/key bs [#"2" - #"7" "8" #"9"]
	--test-- "remove/part invalid"
		--assert all [
			error? e: try [remove/part bs "01"]
			e/id = 'bad-refines
		]

===end-group===


===start-group=== "find"
	--test-- "find bitset! char!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2166
		ABC: charset [#"A" #"B" #"C"]
		--assert find ABC #"a"
		--assert find ABC #"A"
		--assert find/case ABC #"A"
		--assert not find/case ABC #"a"
		; finding integer value is always case-sensitive
		--assert find ABC to-integer #"A"
		--assert not find ABC to-integer #"a"
		; pick is always case-sensitive
		--assert pick ABC #"A"
		--assert not pick ABC #"a"

	--test-- "find bitset! string!"
		--assert find ABC "ABC"
		--assert find ABC "BAC"
		--assert find ABC "CA"
		--assert find/any ABC "XCA"
		--assert not find ABC "XCA"
		--assert not find ABC "abc" ;@@ should be searching of a string case-insensitive?

	--test-- "find bitset! block-of-integers"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1512
		bits1-3: make bitset! [1 - 3]
		--assert find bits1-3 [1]
		--assert find bits1-3 [1 2]
		--assert find bits1-3 [1 - 3]
		--assert find bits1-3 as paren! [1 - 3]
		--assert not find bits1-3 [4]
		--assert not find bits1-3 [1 2 4]
		--assert not find bits1-3 [1 - 4]
		--assert find/any bits1-3 [4 1]
		--assert find/any bits1-3 [1 2 4]
		--assert find/any bits1-3 [1 - 4]

	--test-- "find bitset! block-of-strings!"
		--assert find ABC ["AB"]
		--assert find ABC ["AB" "CB"]
		--assert not find ABC ["AB" "CBX"]

===end-group===


===start-group=== "complemented"
	
	--test-- "comp-1"	--assert "make bitset! [not bits #{}]"   = mold charset [not]
	--test-- "comp-2"	--assert "make bitset! [not bits #{80}]" = mold charset [not #"^(00)"]
	--test-- "comp-3"	--assert "make bitset! [not bits #{40}]" = mold charset [not #"^(01)"]
	--test-- "comp-4"	--assert "make bitset! [not bits #{000000000000FFC0}]" = mold charset [not "0123456789"]
	--test-- "comp-5"	--assert "make bitset! [not bits #{F0}]" = mold charset [not 0 1 2 3]

	--test-- "comp-6"
		bs: make bitset! 1
		--assert false = complement? bs
		--assert "make bitset! #{00}" = mold bs
		--assert 8 = length? bs
		bs: complement bs
		--assert true = complement? bs
		--assert 8 = length? bs
		--assert "make bitset! [not bits #{00}]" = mold bs

	--test-- "comp-7"
		bs: charset [not "hello123" #"a" - #"z"]
		--assert 128 = length? bs
		--assert "make bitset! [not bits #{0000000000007000000000007FFFFFE0}]" = mold bs

	--test-- "comp-8"
		bs: complement charset " "
		--assert 40 = length? bs
		--assert bs/31 = true
		--assert bs/32 = false
		--assert bs/33 = true
		--assert bs/200 = true

	--test-- "comp-9"
		bs/32: true
		--assert bs/32 = true
		--assert "make bitset! [not bits #{0000000000}]" = mold bs

	--test-- "comp-10"
		poke bs #" " none
		--assert bs/32 = false
		--assert "make bitset! [not bits #{0000000080}]" = mold bs

	--test-- "comp-11"
		clear bs
		--assert "make bitset! [not bits #{}]" = mold bs

	--test-- "comp-12"
		poke bs [32 - 40] none
		--assert "make bitset! [not bits #{00000000FF80}]" = mold bs
		poke bs [32 - 40] true
		--assert "make bitset! [not bits #{000000000000}]" = mold bs

	--test-- "issue-1541"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1541
		b: charset " "
		b/48: true
		--assert b = #[bitset! #{00000000800080}]
		b: complement charset " "
		b/48: none
		--assert b = make bitset! [not bits #{00000000800080}]
		b/48: true
		--assert b = make bitset! [not bits #{00000000800000}]

===end-group===

===start-group=== "bitset issues"

	--test-- "issue Red#3443"
		bs: make bitset! #{}
		n: 135 idx: 0
		until [
			bs/:idx: true
			idx: idx + 1
			idx > n
		]
		--assert "make bitset! #{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}" = mold bs


	--test-- "issue Red#3950"
		--assert (make bitset! [not bits #{000000000000000040}]) = do mold complement charset "A"

	--test-- "issue-209"
	;@@ https://github.com/Oldes/Rebol-issues/issues/209
		--assert (charset [#"^(80)"]) = make bitset! #{0000000000000000000000000000000080}

	--test-- "issue-1271"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1271
		chars: complement charset "ther "
		--assert "it goes" = find "there it goes" chars

	--test-- "issue-1283"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1283
		bits: make bitset!  [1]
		--assert find bits [1] ;- no crash

	--test-- "to-binary bitset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2436
		bits: make bitset!  [1]
		--assert #{40} = to binary! bits
		--assert #{BF} = to binary! complement bits
		--assert #{BF} = complement to binary! bits


===end-group===




~~~end-file~~~

