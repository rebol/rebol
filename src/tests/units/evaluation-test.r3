Rebol [
	Title:   "Rebol3 evaluation test script"
	Author:  "Nenad Rakocevic"
	File: 	 %evaluation-test.r3
	Tabs:	 4
	Rights:  "Copyright (C) 2011-2018 Red Foundation. All rights reserved."
	License: "BSD-3 - https://github.com/red/red/blob/origin/BSD-3-License.txt"
	Needs:   [%../quick-test-module.r3]
	Notes:   {Taken from Red-language and ported for use in Rebol3 by Oldes}
]

~~~start-file~~~ "evaluation"

===start-group=== "do"

	--test-- "do-1"
		--assert 123 = do [123]
		
	--test-- "do-2"
		--assert none = do [none]
		
	--test-- "do-3"
		--assert false = do [false]
		
	--test-- "do-4"
		--assert 'z = do ['z]
		
	--test-- "do-5"
		a: 123
		--assert 123 = do [a]
		
	--test-- "do-6"
		--assert 3 = do [1 + 2]
		
	--test-- "do-7"
		--assert 7 = do [1 + 2 3 + 4]
		
	--test-- "do-8"
		--assert 9 = do [1 + length? mold append [1] #"t"]
		
	--test-- "do-9"
		--assert word! = do [type? first [a]]

	--test-- "do-10"
		;@@ https://github.com/Oldes/Rebol-issues/issues/871
		--assert 1 = do "1"

	--test-- "do/next-1"
		code: [3 4 + 5 length? mold 8 + 9 append copy "hel" form 'lo]
		--assert 3 		 = do/next code 'code
		--assert 9 		 = do/next code 'code
		--assert 2 		 = do/next code 'code
		--assert "hello" = do/next code 'code
		--assert unset? do/next code 'code
		--assert unset? do/next code 'code
		--assert tail? code
		--assert (head code) = [3 4 + 5 length? mold 8 + 9 append copy "hel" form 'lo]

	--test-- "issue-661"
	;@@ https://github.com/Oldes/Rebol-issues/issues/661
		--assert [print 'hello] = do "[print 'hello]"

	--test-- "issue-662"
	;@@ https://github.com/Oldes/Rebol-issues/issues/662
		--assert not error? try [do "rebol [type: module] 1 + 1"]
		--assert not error? try [do "rebol [type: module] print {Hello issues/662}"]

	--test-- "issue-121"
	;@@ https://github.com/Oldes/Rebol-issues/issues/121
		--assert logic? do 'true

	--test-- "do/next"
		;@@ https://github.com/Oldes/Rebol-issues/issues/902
		--assert 1 = do/next {1 2} 'n
		;@@ https://github.com/Oldes/Rebol-issues/issues/901
		--assert n = [2]
		--assert 2 = do/next n 'n
		--assert n = []
		--assert unset? do/next n 'n
		--assert n = []
		;@@ https://github.com/Oldes/Rebol-issues/issues/1951
		--assert 2 = do/next 2 'n
		--assert n = none
		;@@ https://github.com/Oldes/Rebol-issues/issues/960
		--assert unset? do/next a: [] 'b
		--assert same? a b
		--assert 1 = index? b

	--test-- "issue-903"
	;@@ https://github.com/Oldes/Rebol-issues/issues/903
		--assert all [error? e: try [do "<> 0"]  e/id = 'missing-arg]
		--assert all [error? e: try [do next [1 <> 0]]  e/id = 'missing-arg]
		
===end-group===

===start-group=== "do script"
	--test-- "script with error"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2374
	;@@ https://github.com/Oldes/Rebol-issues/issues/2425
		dir: what-dir
		--assert error? try [do %units/files/error.r3]
		--assert dir = what-dir

	--test-- "script returning UNSET value"
		--assert unset? do %units/files/unset.r3
		--assert dir = what-dir

	--test-- "script with quit"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1734
	;@@ https://github.com/Oldes/Rebol-issues/issues/2250
		--assert unset? do %units/files/quit.r3
		--assert 42 = do %units/files/quit-return.r3

	--test-- "do needs"
	;@@ https://github.com/Oldes/Rebol-issues/issues/891
		--assert all [error? e: try [do "rebol[needs: 255.8.5]"] e/id = 'needs]

===end-group===

===start-group=== "do function"
	
	--test-- "do-func-1"
		df1-f: func[][1]
		--assert 1 = do [df1-f] 
		
	--test-- "do-func-2"
		df2-f: func[i][2 * i]
		--assert 4 = do [df2-f 2]
		
	--test-- "do-func-3"
		df3-f: func[][3]
		--assert 3 = do df3-f
		
	--test-- "do-func-4"
		df4-i: 4
		df4-f: [func[i][df4-i * i] 4]
		--assert 16 = do reduce df4-f
		
	--test-- "do-func-5"
		df5-f: func[i][5 * i]
		--assert 25 = do [df5-f 5]
		
	--test-- "do-func-6"
		df6-i: 6
		df6-f: func[i][df6-i * i]
		--assert 36 = do [df6-f 6]
		
===end-group=== 

===start-group=== "do object"

	--test-- "do-object-1"
		do1-blk: load {
			o: make object! [
				oo: make object! [
					ooo: make object! [
						a: 1
					]
				]
			]
		}
		do do1-blk
		--assert 1 == o/oo/ooo/a

===end-group===

===start-group=== "do word!"
;@@ https://github.com/Oldes/Rebol-issues/issues/1882
	--test-- "do-word-1"
		a: does ["OK"] b: 23
		--assert "OK" == do 'a
		--assert  23  == do 'b

	--test-- "do-word-2"
		o: make object! [a: does ["OK"] b: 23]
		--assert "OK" == do in o 'a
		--assert  23  == do in o 'b

	--test-- "do-word-3"
		b: [print "OK"]
		--assert block? do 'b

	--test-- "do set-word!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1883
		--assert all [error? e: try [do quote a:] e/id = 'invalid-arg]

===end-group===

===start-group=== "do path!"
;@@ https://github.com/Oldes/Rebol-issues/issues/1881
	--test-- "do-path-1"
		o: make object! [a: does ["OK"] b: 23]
		--assert "OK" == do 'o/a
		--assert  23  == do 'o/b
		--assert all [error? e: try [do 'o/x] e/id = 'invalid-path]
		
	--test-- "do-path-2"
		b: [["OK"]]
		--assert ["OK"] == do first [b/1]

	--test-- "do-path-3"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1434
		a: [[1 + 2]] b: [a a/1]
		--assert [[1 + 2]] = do first b
		--assert  [1 + 2]  = do second b
		b: ['a 'a/1]
		--assert word? do first b
		--assert path? do second b

	--test-- "do set-path!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1883
		--assert all [error? e: try [do quote a/1:] e/id = 'invalid-arg]

===end-group===


===start-group=== "with"
	--test-- "with"
		o: object [a: 1]
		--assert 2 = with o [2 * a]
		; test that return is not catched by `with`
		f: does [with o [return a] 2]
		--assert 1 = f
===end-group===


===start-group=== "reduce"

	--test-- "reduce-1"
		--assert [] = reduce []
		
	--test-- "reduce-2"
		--assert [] = do [reduce []]
		
	--test-- "reduce-3"
		--assert [123] = reduce [123]
		
	--test-- "reduce-4"
		--assert none = first reduce [none]
		
	--test-- "reduce-5"
		--assert false = first reduce [false]

	--test-- "reduce-6"
		--assert 'z = first reduce ['z]	
	
	--test-- "reduce-7"
		a: 123
		--assert [123 8 z] = reduce [a 3 + 5 'z]
	
	--test-- "reduce-8"
		blk: [a b c]
		--assert [a b c] = reduce/into [3 + 4 a] blk
		--assert blk = [7 123 a b c]
	
	--test-- "reduce-9"
		a: 123
		--assert [123 8 z] = do [reduce [a 3 + 5 'z]]
	
	--test-- "reduce-10"
		blk: [a b c]
		--assert [a b c] = do [reduce/into [3 + 4 a] blk]
		--assert blk = [7 123 a b c]

	--test-- "reduce-11"
		code: [1 + 3 a 'z append "hell" #"o"]
		--assert [4 123 z "hello"] = reduce code
	
	--test-- "reduce-11"
		code: [1 + 3 a 'z append "hell" #"o"]
		--assert [4 123 z "hello"] = do [reduce code]

	--test-- "reduce-12"
		--assert none = reduce none

	--test-- "reduce-13"
		--assert none = do [reduce none]
		
	--test-- "reduce-14"
		--assert [[]] = reduce [reduce []]
	
	--test-- "reduce-15"
		--assert [3 z] = reduce [
			1 + length? reduce [3 + 4 789] 'z
		]
	
	--test-- "reduce-16"
		--assert [[]] = do [reduce [reduce []]]
	
	--test-- "reduce-17"
		--assert [3 z] = do [
			reduce [
				1 + length? reduce [3 + 4 789] 'z
			]
		]
		
	--test-- "reduce-18"
		a: [3 + 4]
		--assert [7] = reduce a
		--assert [7] = do [reduce a]

	--test-- "reduce-19"
		b: next [1 2]
		--assert [2] = reduce/into [yes 3 4 5] b
		--assert [1 #(true) 3 4 5 2] = head b

	--test-- "reduce-20"
		b: 2
		--assert [2] = head reduce/into b []
		--assert ["a"] = head reduce/into "a" []

	--test-- "reduce/no-set"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2122
		--assert [x: 3] = reduce/no-set [x: 1 + 2]
		--assert [x/1: 3] = reduce/no-set [x/1: 1 + 2]

	--test-- "reduce/only"
		;@@ https://github.com/Oldes/Rebol-issues/issues/359
		--assert [1 2] = reduce/only [1 2] []
		--assert [1 2] = reduce/only [1 2] none
		--assert native? second reduce/only [1 now 2] none
		--assert word?   second reduce/only [1 now 2] [now]
		; it is not allowed to have unset reduced using reduce/only anymore!
		;@@ https://github.com/Oldes/Rebol-issues/issues/1771
		--assert all [error? e: try [ reduce/only [1 no-such-word 2] []   ] e/id = 'no-value]
		--assert all [error? e: try [ reduce/only [1 no-such-word 2] none ] e/id = 'no-value]
		; if the word is listed in the /only value, than it is ok:
		--assert [1 no-such-word 2] = reduce/only [1 no-such-word 2] [no-such-word]

	--test-- "reduce/into"
		;@@ https://github.com/Oldes/Rebol-issues/issues/506
		a: 1 b: 2 x: copy []
		--assert tail? reduce/into a x
		--assert x = [1]
		--assert [1] = reduce/into b x
		--assert x = [2 1]
		--assert [2 1] = reduce/into [a b] x
		--assert x = [1 2 2 1]
		--assert tail? reduce/into [b a] tail x
		--assert x = [1 2 2 1 2 1]

===end-group===

===start-group=== "compose"
	
	--test-- "compose-1"
	--assert  [] = compose []
	--assert  [] = compose/deep []
	--assert  [] = compose/deep/only []
	--assert  [] = do [compose []]
	--assert [] = do [compose/deep []]
	--assert [] = do [compose/deep/only []]
	
	--test-- "compose-2"
	--assert [1 [2] "3" a 'b c: :d] = compose [1 [2] "3" a 'b c: :d]
	--assert [1 [2] "3" a 'b c: :d] = do [compose [1 [2] "3" a 'b c: :d]]
	
	--test-- "compose-3"
	--assert [1] = compose [(1)]
	--assert [1] = do [compose [(1)]]
	
	--test-- "compose-4"
	--assert none == first compose [(none)]
	--assert none == first do [compose [(none)]]

	--test-- "compose-5"
	--assert true == first compose [(true)]
	--assert true == first do [compose [(true)]]
	
	--test-- "compose-6"
	--assert [3] = compose [(1 + 2)]
	--assert [3] = do [compose [(1 + 2)]]
	
	--test-- "compose-7"
	--assert [x 9 y] = compose [x (4 + 5) y]
	--assert [x 9 y] = do [compose [x (4 + 5) y]]
	
	--test-- "compose-8"
	--assert [] = compose [([])]
	--assert [] = do [compose [([])]]
	
	--test-- "compose-9"
	--assert [[]] = compose/only [([])]
	--assert [[]] = do [compose/only [([])]]
	
	--test-- "compose-10"
	--assert [1 2 3] = compose [([1 2 3])]
	--assert [1 2 3] = do [compose [([1 2 3])]]
	
	--test-- "compose-11"
	--assert [1 2 3] = compose [([1 2 3])]
	--assert [1 2 3] = do [compose [([1 2 3])]]
	
	--test-- "compose-12"
	--assert [[(5 + 6)]] = compose [[(5 + 6)]]
	--assert [[(5 + 6)]] = do [compose [[(5 + 6)]]]
	
	--test-- "compose-13"
	--assert [[1]] = compose/deep [[(7 - 6)]]
	--assert [[1]] = do [compose/deep [[(7 - 6)]]]
	
	--test-- "compose-14"
	--assert [[]] = compose/deep [[([])]]
	--assert [[]] = do [compose/deep [[([])]]]
	
	--test-- "compose-15"
	--assert [[[]]] = compose/deep/only [[([])]]
	--assert [[[]]] = do [compose/deep/only [[([])]]]
	
	--test-- "compose-16"
	--assert [[8] x [9] y] = compose/deep [[(2 + 6)] x [(4 + 5)] y]
	--assert [[8] x [9] y] = do [compose/deep [[(2 + 6)] x [(4 + 5)] y]]
	
	--test-- "compose-17"
	--assert [a 3 b 789 1 2 3] = compose [a (1 + 2) b () (print "") ([]) 789 ([1 2 3])]
	--assert [a 3 b 789 1 2 3] = compose [a (1 + 2) b () (print "") ([]) 789 ([1 2 3])]

	--test-- "compose-18"
	--assert [a 3 b [] 789 [1 2 3]] = compose/only [a (1 + 2) b () (print "") ([]) 789 ([1 2 3])]
	--assert [a 3 b [] 789 [1 2 3]] = compose/only [a (1 + 2) b () (print "") ([]) 789 ([1 2 3])]
	
	--test-- "compose-19"
	--assert [a [3] 8 b [2 3 [x "hello" x]]] = compose/deep [
		a [(1 + 2)] (9 - 1) b [
			2 3 [x (append "hell" #"o") x]
		]
	]
	--assert [a [3] 8 b [2 3 [x "hello" x]]] = do [		;; refinements not supported yet by DO
		compose/deep [
			a [(1 + 2)] (9 - 1) b [
				2 3 [x (append "hell" #"o") x]
			]
		]
	]

	--test-- "compose-20"
	a: [1 2 3]
	--assert [1 2 3] = compose/into [r (1 + 6)] a
	--assert a = [r 7 1 2 3]
	a: [1 2 3]
	--assert [1 2 3] = do [compose/into [r (1 + 6)] a]
	--assert a = [r 7 1 2 3]
	
	--test-- "compose-21"
	a: [(mold 2 + 3)]
	--assert ["5"] = compose a
	--assert ["5"] = do [compose a]

	--test-- "compose-22"
	b: next [1 2]
	--assert [2] = compose/into [no 7 8 9 (2 * 10) ([5 6])] b
	--assert [1 no 7 8 9 20 5 6 2] = head b

	--test-- "compose/into"
	;@@ https://github.com/Oldes/Rebol-issues/issues/506
	;@@ https://github.com/Oldes/Rebol-issues/issues/2062
	a: 1 b: 2 x: copy []
	--assert tail? compose/into "a" x
	--assert x = ["a"]
	--assert ["a"] = compose/into [a b] x
	--assert x = [a b "a"]
	--assert tail? compose/into [a (b)] tail x
	--assert x = [a b "a" a 2]
	
===end-group===

===start-group=== "unset value passing"

	--test-- "unset-1"
		--assert unset! = type? set/any 'xyz ()
		--assert unset! = type? get/any 'xyz
		--assert unset! = type? :xyz

	--test-- "unset-2"
		test-unset: has [zyx][
			--assert unset! = type? set/any 'zyx ()
			--assert unset! = type? get/any 'zyx
			--assert unset! = type? :zyx
		]
		test-unset

	--test-- "unset? if true []"
		--assert unset? if true []
	--test-- "unset? case [true []]"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2246
		--assert unset? case [true []]
		--assert unset? case/all [true [1] true []]

	--test-- "to-value"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2003
		--assert none? to-value #(unset)
		--assert integer? to-value 1

	--test-- "unset unbind 'x"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2219
		--assert error? try [unset unbind 'x]
		
===end-group===

===start-group=== "SET tests"

	--test-- "set-1"
		--assert 123 = set 'value 123
		--assert value = 123
		
	--test-- "set-2"
		--assert 456 = set [A B] 456
		--assert a = 456
		--assert b = 456
		
	--test-- "set-3"
		--assert [7 8] = set [A B] [7 8]
		--assert a = 7
		--assert b = 8
		
	--test-- "set-4"
		--assert [4 5] = set/only [A B] [4 5]
		--assert a = [4 5]
		--assert b = [4 5]
		
	--test-- "set-5"
		--assert [4 #(none)] = set [A B] reduce [4 none]
		--assert a = 4
		--assert b = none
		
	--test-- "set-6"
		b: 789
		--assert [4 #(none)] = set/some [A B] reduce [4 none]
		--assert a = 4
		--assert b = 789

	--test-- "set-7"
		obj: object [a: 1 b: 2]
		--assert [4 5] = set obj [4 5]
		--assert obj/a = 4
		--assert obj/b = 5

	--test-- "set-8"
		obj:  object [a: 3 b: 4]
		obj2: object [z: 0 a: 6 b: 7 c: 9]
		--assert obj2 = set obj obj2
		--assert "make object! [a: 6 b: 7]" = mold/flat obj
		--assert "make object! [z: 0 a: 6 b: 7 c: 9]" = mold/flat obj2
		
	--test-- "set-9"
		obj:  object [a: 3 b: 4]
		obj2: object [z: 0]
		--assert obj2 = set/only obj obj2
		--assert obj/a = obj2
		--assert obj/b = obj2
		
	--test-- "set-10"
		obj:  object [a: 3 b: 4]
		obj2: object [z: 0 a: none b: 7]
		--assert obj2 = set obj obj2
		--assert "make object! [a: none b: 7]" = mold/flat obj
		--assert "make object! [z: 0 a: none b: 7]" = mold/flat obj2

	--test-- "set-11"
		obj:  object [a: 3 b: 4]
		obj2: object [z: 0 a: none b: 7]
		--assert obj2 = set/some obj obj2
		--assert "make object! [a: 3 b: 7]" = mold/flat obj
		--assert "make object! [z: 0 a: none b: 7]" = mold/flat obj2

	--test-- "set-12"
		o1: object [a: 1 b: 2 ]
		o2: object [a: 10 b: 20 ] set/any 'o2/b () ;source has unset value
		--assert o2 = set o1 o2
		--assert all [o1/a = 10 o1/b = 2]

	--test-- "set-13"
		o1: object [a: 1 b: 2 ]
		o2: object [a: 10 b: 20 ] set/any 'o2/b () ;source has unset value
		--assert o2 = set/any o1 o2
		--assert all [o1/a = 10 unset? o1/b]

	--test-- "set-14"
		a: b: 1
		--assert error? try [ set [A B] reduce [4 ()] ]
		--assert all [a = 1 b = 1]

	--test-- "set-15"
		a: b: 1 d: reduce [4 ()] 
		--assert d = set/any [A B] d
		--assert all [a = 4 unset? :b]

	--test-- "Set - issue 2367"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2367
		--assert error? try [set #ab 1]
		--assert error? try [set #12 2]

	--test-- "set block with word types"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1745
		set [a 'b :c d:] [1 2 3 4]
		--assert a = 1
		--assert b = 2
		--assert c = 3
		--assert d = 4
		--assert error? try [set [/e][5]]
		--assert error? try [set [#f][6]]
		--assert error? try [set /a 1]

	--test-- "set any block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2488
		--assert all [
			[[][]] = set [a b] first [[[][]]]
			a = []
			b = []
		]
		--assert all [
			paren? set [a b] first [([][])]
			a = []
			b = []
		]
		--assert all [
			path? set [a b] first [x/y]
			a = 'x
			b = 'y
		]
		--assert all [
			[[][]] = set/only [a b] val: first [[[][]]]
			a = val
			b = val
		]
		--assert all [
			paren? set/only [a b] val: first [([][])]
			a = val
			b = val
		]
		--assert all [
			path? set/only [a b] val: first [x/y]
			a = val
			b = val
		]

	--test-- "set path"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2275
	;Parenthesized expressions on left for SET-PATH! evaluate after right hand expressions 
		obj: make object! [x: 10 y: 20]
		some-func: does [var: 'y]
		var: 'x
		obj/(var): (some-func 30)
		--assert all [obj/x = 10 obj/y = 30]
	--test-- "set path 2"
	;@@ https://github.com/Oldes/Rebol-issues/issues/396
		c: 0 b: [1 2 3]
		--assert 1 = b/(c: 2): c + 1


===end-group===

===start-group=== "STACK"
	--test-- "issue-1623"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1623
		--assert block? b: stack 0
		--assert 'stack = first b
		--assert 'stack = stack/word 0

===end-group===


===start-group=== "SWITCH"
	--test-- "issue-2242"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2242
		out: copy ""
		foo: does [
			switch/all 10 [
				10 [append out "Hello" return "Returning"]
				10 [append out "World"]
			]
			"End"
		]
		append out foo
		--assert "HelloReturning" = out
	--test-- "switch/case"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1773
		--assert 'upper = switch "a" ["A" ['upper] "a" ['lower]]
		--assert 'lower = switch/case "a" ["A" ['upper] "a" ['lower]]
		--assert 'upper = switch/case "A" ["A" ['upper] "a" ['lower]]
===end-group===


===start-group=== "BOOT"
	--test-- "issue-232"
	;@@ https://github.com/Oldes/Rebol-issues/issues/232
		--assert file? system/options/home
		--assert file? system/options/boot
		--assert any [none? system/options/script file? system/options/script]
===end-group===


===start-group=== "TRY"
	--test-- "try [do..]"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1361
		--assert error? try [do {"}]
		--assert error? try [do "1" do {"}]
	--test-- "try [catch..]"
	;@@ https://github.com/Oldes/Rebol-issues/issues/851
		--assert error? try [do [1] do make error! "try failure"]
		--assert error? try [do "1" do make error! "try failure"]
		--assert error? try [catch/quit [1] do make error! "Hello"]
		--assert error? try [try [catch/quit []] 1 / 0]

	;@@ https://github.com/Oldes/Rebol-issues/issues/822
	--test-- "try/with [1 / 0] block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2419
		system/state/last-error: none
		try/with [1 / 0][
			--assert error? system/state/last-error
			--assert system/state/last-error/id = 'zero-divide
		]
		; any TRY call resets system/state/last-error to none:
		--assert not error? try [1 + 1]
		--assert none? system/state/last-error
		; the last-error is stored also when /with is not used:
		--assert error? try [this-is-error]
		--assert error? system/state/last-error

	--test-- "try/with [1 / 0] function!"
		system/state/last-error: none
		--assert string? try/with [1 / 0] :mold
		--assert system/state/last-error/id = 'zero-divide

	--test-- "try/all"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1506
		--assert error? try/all [break]
		--assert error? try/all [continue]
		--assert error? try/all [exit]
		--assert error? try/all [return 1]
		--assert error? try/all [throw 1]

	--test-- "try/all/with block handler"
		handler: [system/state/last-error/id]
		--assert 'break    = try/all/with [break   ] :handler
		--assert 'continue = try/all/with [continue] :handler
		--assert 'return   = try/all/with [exit    ] :handler
		--assert 'return   = try/all/with [return 1] :handler
		--assert 'throw    = try/all/with [throw 1 ] :handler

	--test-- "try/all/with function handler"
		--assert 'break = try/all/with [break] func[e][e/id]

		--assert all [
			string? try/all/with [break] :mold
			system/state/last-error/id = 'break
			system/state/last-error/arg1 = none
		]
		--assert all [
			string? try/all/with [exit] :mold
			system/state/last-error/id = 'return
			system/state/last-error/arg1 = none
		]
		--assert all [
			string? try/all/with [throw 1] :mold
			system/state/last-error/id = 'throw
			system/state/last-error/arg1 = 1
		]
		--assert all [
			string? try/all/with [throw/name 1 'foo] :mold
			system/state/last-error/id = 'throw
			system/state/last-error/arg1 = 1
			system/state/last-error/arg2 = 'foo
		]

===end-group===


===start-group=== "attempt"
	--test-- "issue-41"
	;@@ https://github.com/Oldes/Rebol-issues/issues/41
		--assert none? attempt [2 / 0]

	--test-- "attempt/safer"
	;@@ https://github.com/Oldes/Rebol-issues/issues/583
	;@@ https://github.com/Oldes/Rebol-issues/issues/1506
		--assert none? attempt/safer [break    'not-reached]
		--assert none? attempt/safer [continue 'not-reached]
		--assert none? attempt/safer [exit     'not-reached]
		--assert none? attempt/safer [return 1 'not-reached]
		--assert none? attempt/safer [throw 1  'not-reached]
	--test-- "try/all [attempt]"
		--assert error? try/all [attempt [break    'not-reached]]
		--assert error? try/all [attempt [continue 'not-reached]]
		--assert error? try/all [attempt [exit     'not-reached]]
		--assert error? try/all [attempt [return 1 'not-reached]]
		--assert error? try/all [attempt [throw 1  'not-reached]]

===end-group===


===start-group=== "CATCH"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1518
	;@@ https://github.com/Oldes/Rebol-issues/issues/1520
	;@@ https://github.com/Oldes/Rebol-issues/issues/1734
	;@@ https://github.com/Oldes/Rebol-issues/issues/1742
	--test-- "catch/quit [halt]"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1742
		a: 0 catch/quit [++ a halt ++ a]
		--assert a == 1

	--test-- "catch/quit [quit]"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1734
		a: 0  
		--assert unset? catch/quit [++ a quit ++ a]
		--assert a == 1
		--assert 100 = catch/quit [++ a quit/return 100 ++ a]
		--assert a == 2
		--assert   0 = call/shell/wait append to-local-file system/options/boot { --do "quit"}
		--assert 100 = call/shell/wait append to-local-file system/options/boot { --do "quit/return 100"}
	--test-- "nested catch"
		a: 0
		--assert 'x = catch [++ a catch/quit [++ a quit a: 0] a: a * 2 throw 'x a: a * 100]
		--assert a == 4
		a: 0
		--assert 'x = catch [++ a catch/quit [++ a throw 'x a: 0] a: a * 2 quit 'x a: a * 100]
		--assert a == 2
		a: 0
		--assert unset? catch/quit [++ a a: a + catch [++ a throw 100 a: 0] a: a * 2 quit a: a * 100]
		--assert a == 202
		a: 0
		--assert unset? catch/quit [++ a a: a + catch [++ a quit a: 0] a: a * 2 throw 100 a: a * 100]
		--assert a == 2

	--test-- "catch/with block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1521
		--assert 10 = catch/quit/with [a: 1 quit a: 2][a: a * 10]
		--assert  a = 10

		--assert 40 = catch/quit/with [a: 1 quit/return 4 a: 2][system/state/last-result * 10]
		--assert  a = 1

		--assert 2 = catch/quit/with [a: 2][a: a * 10]
		--assert a = 2

		--assert 10 = catch/with [a: 1 throw 'x a: 2][a: a * 10]
		--assert a = 10

		--assert 30 = catch/with [a: 1 throw 3 a: 2][system/state/last-result * 10]  
		--assert a = 1

		--assert all [
			2 == catch/with [throw 1][2 * system/state/last-result]
			2 == system/state/last-result
		]
	--test-- "catch/with function!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1521
		on-catch: func[value name][
			if :name = 'foo    [return join "b" :value]
			if unset?   :value [return true]
			if integer? :value [return value * 10]
			mold value
		]

		--assert all [
			#(true) == catch/quit/with [a: 1 quit a: 2] :on-catch
			a = 1
		]

		--assert all [
			40 = catch/quit/with [a: 1 quit/return 4 a: 2] :on-catch
			a = 1
		]

		--assert all [
			2 = catch/quit/with [a: 2] :on-catch
			a = 2
		]

		--assert all [
			"x" = catch/with [a: 1 throw 'x a: 2] :on-catch
			'x  = system/state/last-result ;; or "x" ???
			a = 1
		]

		--assert all [
			30 = catch/with [a: 1 throw 3 a: 2] :on-catch
			a = 1
		]

		--assert all [
			"b3" = catch/all/with [a: 1 throw/name 3 'foo a: 2] :on-catch 
			a = 1
		]

	--test-- "catch/quit/name"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2549
		--assert all [
			unset? catch/quit/name [a: 1 quit a: 2] 'name
			a = 1
		]
		--assert all [
			0 = catch/quit/name [a: 1  throw/name 0 'name  a: 2] 'name
			a = 1
		]

	--test-- "nested catch"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1518
		--assert 1 = catch/name [a: 0 catch [++ a throw/name 1 'foo] a: a * 10] 'foo
		--assert 1 = a
		--assert 1 = catch/name [a: 0 catch/name [++ a throw/name 1 'foo] 'bar a: a * 10] 'foo
		--assert 1 = a
	--test-- "catch/all"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1520
		--assert 1 = catch/all [a: 0 throw/name 1 'foo a: a * 10]
		--assert 0 = a
		--assert 1 = catch/all [a: 0 catch [++ a throw/name 1 'foo] a: a * 10]
		--assert 1 = a
		--assert 1 = catch/all [a: 0 catch/name [++ a throw/name 1 'foo] 'bar a: a * 10]
		--assert 1 = a

	--test-- "throw from make error!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2244
		--assert error? catch [make error! [type: 'Access arg1: 10 + 20 id: 'Protocol]]
		--assert 30 = catch [make error! [type: 'Access arg1: throw 10 + 20 id: 'Protocol]]

	--test-- "throw from path evaluation"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2243
		foo: object [bar: 1]
		--assert "ok" = catch [foo/(throw "ok" 'bar)]
		--assert "ok" = catch [foo/(throw "ok" 'bar): 3]
		--assert "ok" = catch [foo/(throw "ok" 'bar)/xx]
		--assert "ok" = catch [foo/(throw "ok" 'bar)/xx: 3]

===end-group===


===start-group=== "FOR"
	--test-- "FOR boundary tests fail"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1136
		num: 0
		--assert all [
			error? e: try [
				for i 9223372036854775807 9223372036854775807 1 [
					num: num + 1
					either num > 1 [break/return false] [true]
				]
			]
			e/id = 'overflow
			num = 1
		]
		num: 0
		--assert all [
			error? e: try [
				for i -9223372036854775808 -9223372036854775808 -1 [
					num: num + 1
					either num > 1 [break/return false] [true]
				]
			]
			e/id = 'overflow
			num = 1
		]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1994
		num: 0
		--assert all [
			error? e: try [
				for i 9223372036854775807 9223372036854775807 9223372036854775807 [
				    num: num + 1
				    if num <> 1 [break/return false]
				    true
				]
			]
			e/id = 'overflow
			num == 1 ;- note that num was incremented!
		]
		--assert all [
			num: 0
			for i 1 1 1 [num: num + 1]
			num == 1 ;- also incremented
		]
		
	--test-- "FOR with series! start and number! end"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1601
		out: copy ""
		for x "abcde" 3 1 [append out x]
		--assert "abcdebcdecde" == out
		clear out
		for x "abcde" tail "ab" 1 [append out x]
		--assert "abcdebcdecde" == out

	--test-- "SELF inside a FOR block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1529
		--assert same? 'self for i 1 1 1 ['self]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1552
		--assert none? for self 1 1 1 [context? 'self]

===end-group===


===start-group=== "REPEAT"
	--test-- "repeat pair!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1995
		b: copy []
		repeat x 2x2 [append b x]
		--assert b = [1x1 2x1 1x2 2x2]
		--assert b = collect [repeat x 2x2 [keep x]]

	--test-- "SELF inside a FOR block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1529
		--assert same? 'self repeat n 1 ['self]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1552
		--assert none? repeat self 1 [context? 'self]

===end-group===


===start-group=== "BREAK"
	--test-- "break returns unset"
		--assert unset?   loop 1 [break 2]
	--test-- "break/return should return argument"
		--assert none?    loop 1 [break/return none 2]
		--assert false =? loop 1 [break/return false 2]
		--assert true  =? loop 1 [break/return true 2]
		--assert unset?   loop 1 [break/return () 2]
		--assert error?   loop 1 [break/return try [1 / 0] 2]
	--test-- "break - the `result` of break should not be assignable"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1515
		a: 1 loop 1 [a: break]
		--assert :a =? 1
		a: 1 loop 1 [set 'a break]
		--assert :a =? 1
		a: 1 loop 1 [set/any 'a break]
		--assert :a =? 1
		a: 1 loop 1 [a: break/return 2]
		--assert :a =? 1
		a: 1 loop 1 [set 'a break/return 2]
		--assert :a =? 1
		a: 1 loop 1 [set/any 'a break/return 2]
		--assert :a =? 1
	--test-- "break - the `result` of break should not be passable to functions"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1509
		a: 1 loop 1 [a: error? break] ; error? function takes 1 arg
		--assert :a =? 1
		a: 1 loop 1 [a: error? break/return 2]
		--assert :a =? 1
		a: 1 loop 1 [a: type? break] ; type? function takes 1-2 args
		--assert :a =? 1
		foo: func [x y] [9] a: 1 loop 1 [a: foo break 5] ; foo takes 2 args
		--assert :a =? 1
		foo: func [x y] [9] a: 1 loop 1 [a: foo 5 break]
		--assert :a =? 1
		foo: func [x y] [9] a: 1 loop 1 [a: foo break break]
		--assert :a =? 1
	--test-- "check that BREAK is evaluated (and not CONTINUE)"
		foo: func [x y] [] a: 1 loop 2 [a: a + 1 foo break continue a: a + 10]
		--assert :a =? 2
	--test-- "check that BREAK is not evaluated (but CONTINUE is)"
		foo: func [x y] [] a: 1 loop 2 [a: a + 1 foo continue break a: a + 10]
		--assert :a =? 3
	--test-- "issue-1535"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1535
		--assert (loop 1 [words-of  break] true)
		--assert (loop 1 [values-of break] true)
	--test-- "issue-1945"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1945
		--assert (loop 1 [spec-of   break] true)
	--test-- "the `result` of break should not be caught by try"
		a: 1 loop 1 [a: error? try [break]]
		--assert :a =? 1
	--test-- "reduce [break]"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1760
		a: 1 loop 1 [a: reduce [break]]
		--assert :a =? 1
		a: 1 loop 1 [a: compose [(break)]]
		--assert :a =? 1
	;@@ https://github.com/Oldes/Rebol-issues/issues/2060
		a: 1 loop 1 [a: reduce quote (break)]
		--assert :a =? 1

===end-group===

===start-group=== "RETURN"
	--test-- "return value"
	;@@ https://github.com/Oldes/Rebol-issues/issues/771
		--assert 1 == do does [type? return 1 2]

===end-group===


===start-group=== "WAIT"
	--test-- "wait -1"
	;@@ https://github.com/Oldes/Rebol-issues/issues/342
		--assert all [
			error? err: try [wait -1]
			err/id = 'out-of-range
		]

===end-group===


===start-group=== "delta-profile"

	--test-- "delta-profile []"
		;@@ https://github.com/Oldes/Rebol-issues/issues/574
		p: delta-profile []
		--assert 0 = p/evals
		--assert 0 = p/eval-natives
		--assert 0 = p/eval-functions
		--assert 0 = p/series-made
		--assert 0 = p/series-freed
		--assert 0 = p/series-expanded
		--assert 0 >= p/series-bytes ; may 
		--assert 0 = p/series-recycled
		--assert 0 = p/made-blocks
		--assert 0 = p/made-objects

	;@@ https://github.com/Oldes/Rebol-issues/issues/1606
	--test-- "dp [change binary! integer!]"
		b: #{000000}
		--assert 0 = select dp [change b 255] 'series-made
		--assert b = #{FF0000}
		--assert 0 = select dp [change/dup b 255 3] 'series-made
		--assert b = #{FFFFFF}
	--test-- "dp [change binary! char!]"
		b: #{000000}
		--assert 0 = select dp [change b #"a"] 'series-made
		--assert b = #{610000}
		--assert 0 = select dp [change/dup b #"a" 3] 'series-made
		--assert b = #{616161}
		--assert 0 = select dp [change/dup b #"á" 3] 'series-made
		--assert b = #{C3A1C3A1C3A1}
	--test-- "dp [change string! char!]"
		b: "xxx"
		--assert 0 = select dp [change b #"a"] 'series-made
		--assert b = "axx"
		--assert 0 = select dp [change/dup b #"a" 3] 'series-made
		--assert b = "aaa"

===end-group===


===start-group=== "Dynamic refinements"
	;@@ https://github.com/red/red/blob/c69d4763173/tests/source/units/evaluation-test.red#L1210
	dyn-ref-fun: func [i [integer!] b /ref c1 /ref2 /ref3 c3 c4][
	    reduce [i b ref c1 ref2 ref3 c3 c4]
	]

	--test-- "dyn-ref-1"
		only: yes
		repend/:only s: [] [1 + 2 3 * 4]
		--assert s == [[3 12]]
	
	--test-- "dyn-ref-2"
		only: no
		repend/:only s: [] [4 + 5 6 * 7]
		--assert s == [9 42]
	
	--test-- "dyn-ref-3"
		part: no length: 10 
		--assert "def" == find/:part "abcdef" "d" length
		
	--test-- "dyn-ref-4"
		part: yes length: 2
		--assert none? find/:part "abcdef" "d" length

	--test-- "dyn-ref-7"
		ref: yes
		--assert (dyn-ref-fun/:ref 10 * 9 "hello" 789)
			== [90 "hello" #(true) 789 #(none) #(none) #(none) #(none)]
		
	--test-- "dyn-ref-8"
		ref: no
		--assert (dyn-ref-fun/:ref 10 * 9 "hello" 789)
			== [90 "hello" #(none) #(none) #(none) #(none) #(none) #(none)]

	--test-- "dyn-ref-9"
		ref: ref2: yes
		--assert (dyn-ref-fun/:ref/:ref2 10 * 9 "hello" 789)
			== [90 "hello" #(true) 789 #(true) #(none) #(none) #(none)]		

	--test-- "dyn-ref-10"
		ref: no ref2: yes
		--assert (dyn-ref-fun/:ref/:ref2 10 * 9 "hello" 789)
			== [90 "hello" #(none) #(none) #(true) #(none) #(none) #(none)]

	--test-- "dyn-ref-11"
		ref: no ref2: ref3: yes
		--assert (dyn-ref-fun/:ref/:ref2/:ref3 10 * 9 "hello" 789 6 7)
			== [90 "hello" #(none) #(none) #(true) #(true) 6 7]
	
	--test-- "dyn-ref-12"		
		dyn-ref-12-obj: context [
		    foo: func [i [integer!] b /ref c1 /ref2 /ref3 c3 c4][
		    	reduce [i b ref c1 ref2 ref3 c3 c4]
		    ]
		    bar: func [/local ref][
		        ref: no
				--assert (foo/:ref 10 * 9 "hello" 789)
				== [90 "hello" #(none) #(none) #(none) #(none) #(none) #(none)]

		        ref: yes
				--assert (foo/:ref 10 * 9 "hello" 789)
				== [90 "hello" #(true) 789 #(none) #(none) #(none) #(none)]

		    ]
		]
		dyn-ref-12-obj/bar
===end-group===

~~~end-file~~~
