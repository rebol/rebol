Rebol [
	Title:   "Rebol function related test script"
	Author:  "Oldes"
	File: 	 %func-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Function"

===start-group=== "Function refinements"
	fce: func[a [string!] /ref1 b [integer!] /ref2 :c 'd][
		reduce [a ref1 b ref2 c d]
	]
	--test-- "no refinements"
	--assert all [error? e: try [fce  ] e/id = 'no-arg]
	--assert all [error? e: try [fce 1] e/id = 'expect-arg]
	--assert (fce "a") == ["a" #[none] #[none] #[none] #[none] #[none]]

	--test-- "simple refinements"
	--assert all [error? e: try [fce/ref1 "a"   ] e/id = 'no-arg]
	--assert all [error? e: try [fce/ref1 "a" ""] e/id = 'expect-arg]
	--assert (fce/ref1 "a" 1)     == ["a" #[true] 1 #[none] #[none] #[none]]
	--assert (fce/ref1 "a" 1 + 1) == ["a" #[true] 2 #[none] #[none] #[none]]
	--assert (fce/ref1/ref2 "a" 1 x y)     == ["a" #[true] 1 #[true] x y]
	--assert (fce/ref2/ref1 "a" x y 1 + 1) == ["a" #[true] 2 #[true] x y]

	--test-- "dynamic refinements"
	ref1: yes --assert all [error? e: try [fce/:ref1 "a"   ] e/id = 'no-arg]
	ref1: off --assert all [error? e: try [fce/:ref1 "a"   ] e/id = 'no-arg]
	ref1: yes --assert all [error? e: try [fce/:ref1 "a" ""] e/id = 'expect-arg]
	ref1: off --assert (fce/:ref1 "a" "")    == ["a" #[none] #[none] #[none] #[none] #[none]]
	ref1: yes --assert (fce/:ref1 "a" 1)     == ["a" #[true]  1      #[none] #[none] #[none]]
	ref1: off --assert (fce/:ref1 "a" 1)     == ["a" #[none] #[none] #[none] #[none] #[none]]
	ref1: yes --assert all [(fce/:ref1 "a" x: 1 + 1) == ["a" #[true]  2      #[none] #[none] #[none]] x == 2]
	ref1: off --assert all [(fce/:ref1 "a" x: 1 + 1) == ["a" #[none] #[none] #[none] #[none] #[none]] x == 2]
	ref1: yes ref2: yes --assert (fce/:ref1/:ref2 "a" 1 + 1 x y) == ["a" #[true] 2 #[true] x y]
	ref1: yes ref2: yes --assert (fce/:ref2/:ref1 "a" x y 1 + 1) == ["a" #[true] 2 #[true] x y]
	ref1: yes ref2: off --assert (fce/:ref1/:ref2 "a" 1 + 1 x y) == ["a" #[true] 2 #[none] #[none] #[none]]
	ref1: yes ref2: off --assert (fce/:ref2/:ref1 "a" x y 1 + 1) == ["a" #[true] 2 #[none] #[none] #[none]]
	ref1: off ref2: yes --assert (fce/:ref1/:ref2 "a" 1 + 1 x y) == ["a" #[none] #[none] #[true] x y]
	ref1: off ref2: yes --assert (fce/:ref2/:ref1 "a" x y 1 + 1) == ["a" #[none] #[none] #[true] x y]
	ref1: off ref2: off --assert (fce/:ref1/:ref2 "a" 1 + 1 x y) == ["a" #[none] #[none] #[none] #[none] #[none]]
	ref1: off ref2: off --assert (fce/:ref2/:ref1 "a" x y 1 + 1) == ["a" #[none] #[none] #[none] #[none] #[none]]

===end-group===


===start-group=== "Apply"

--test-- "apply :do [:func]"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1950
	--assert 2 = try [apply :do [:add 1 1]]

--test-- "apply 'path/to/func []"
	;@@ https://github.com/Oldes/Rebol-issues/issues/44
	o: object [ f: func[/a][] ]
	--assert error? try [ apply 'o/f [true] ]

--test-- "apply with refinements"
	;@@ https://github.com/Oldes/Rebol-issues/issues/167
	f: func [a /b c] [reduce [a b c]]
	--assert [1 #[true] 3] = apply :f [1 2 3]

--test-- "apply/only"
	;@@ https://github.com/Oldes/Rebol-issues/issues/105
	f: func[a][a]
	--assert date?  apply :f [now]
	--assert 'now = apply/only :f [now]

--test-- "apply op!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/618
	--assert 3 = apply :add [1 2]
	--assert 3 = apply :+ [1 2]

===end-group===


===start-group=== "body-of"

--test-- "body-of NATIVE or ACTION"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1577
	; body-of NATIVE or ACTION should return NONE 
	--assert none? body-of :equal?
	--assert none? body-of :add
	--assert none? body-of :+
	--assert none? body-of :append

--test-- "body-of FUNCTION"
	fce: func[a [integer!]][probe a]
	--assert [probe a] = body-of :fce
	;@@ https://github.com/Oldes/Rebol-issues/issues/166
	fce: func[a][append "xx" s]
	clear second body-of :fce
	--assert [append "xx" s] = body-of :fce
	clear body-of :fce
	--assert [append "xx" s] = body-of :fce


--test-- "invalid MAKE"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1052
	--assert error? try [make :read [[][]]]
	--assert error? try [make action! [[][]]]
	--assert error? try [make native! [[][]]]
	;- op! requires at least 2 args
	--assert error? try [make op! [[][]]]
	--assert error? try [make op! [[a][]]]
	--assert error? try [make op! [[/local a b][]]]
	--assert error? try [make op! [[a /local b][]]]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1607
	--assert all [
		error? e: try [new-op: make := [* [value1 > value2]]]
		e/id = 'cannot-use
	]

===end-group===


===start-group=== "function's context?"
--test-- "wish-2440"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2440
	f: func[arg1 arg2][context? 'arg1]
	--assert same? :f f 1 2
	f: func[arg1 arg2][spec-of context? 'arg1]
	--assert [arg1 arg2] = f 1 2
	f: func[arg1 arg2 /local f2][
		f2: func[a][spec-of context? 'a]
		f2 arg1
	]
	--assert [a] = f 1 2
	
	;@@ https://github.com/Oldes/Rebol-issues/issues/886
	f: func[arg][p: 'arg]
	f 1 2
	; --assert none? context? p ; and no crash!
	; not using above, because tests are run from `do`, so the context is not none like in console!
	--assert same? :do context? p ; and no crash!
	c: closure[a][context? 'a]
	--assert all [object? o: c 1  o/a = 1]
===end-group===


===start-group=== "types-of"

--test-- "types-of FUNCTION"
	;@@ https://github.com/Oldes/Rebol-issues/issues/436
	f: func[/a b [integer!]][]
	--assert all [
		block? b: types-of :f
		b/1 = #[typeset! [none! logic!]]
		b/2 = #[typeset! [integer!]]
	]

===end-group===


===start-group=== "OP!"

--test-- "make op!"
	--assert op? +*: try [ make op! [[a b][a + (a * b)]] ]
	--assert 3 = (1 +* 2)
	--assert 6 = (2 +* 2)
--test-- "make op! with /local"
	c: 1
	--assert op? try [.: make op! [[a "val1" b "val2" /local c ][ c: none join a b ]]]
	--assert "a"."b" = "ab"
	--assert "a".["b" "c"] = "abc"
--test-- "body-of op!"
	--assert (body-of :+*) = [a + (a * b)]
	--assert (body-of :. ) = [c: none join a b]

--test-- "spec-of op!"
	--assert [a b]         = spec-of :+*
	--assert (spec-of :. ) = [a "val1" b "val2" /local c]

--test-- "make op! from action!"
	--assert all [
		op? op1: try [make op! :remainder]
		0 = (6 op1 3)
	]
--test-- "make op! from function!"
	fce: func[a b][a * b]
	--assert op? op2: make op! :fce
	--assert 6 = (2 op2 3)
	--assert 2 op2 3 = 6
	fce: func[a b c][a + b + c]
	--assert all [error? e: try [make op! :fce] e/id = 'bad-make-arg]
	fce: func[a][a]
	--assert all [error? e: try [make op! :fce] e/id = 'bad-make-arg]
	
===end-group===


===start-group=== "ZERO?"
	;@@ https://github.com/Oldes/Rebol-issues/issues/772
	;@@ https://github.com/Oldes/Rebol-issues/issues/1102
	--test-- "zero?"
	--assert all [
		zero? 0
		zero? 0.0
		zero? $0
		zero? 0%
		zero? #"^@"
		zero? 0:0:0
		zero? 0x0
	]
	--assert all [
		not zero? 10
		not zero? 10.0
		not zero? $10
		not zero? 10%
		not zero? 1:0:0
		not zero? 0x1
		not zero? 1x0
	]
	--assert all [
		not zero? "0"
		not zero? #"0"
		not zero? @0
		not zero? #0
		not zero? 1-1-2000
		not zero? ""
		not zero? []
		not zero? none
		not zero? false
	]

===end-group===


===start-group=== "COLLECT-WORDS"
	;@@ https://github.com/Oldes/Rebol-issues/issues/544
	blk: [a "b" c: 2 [:d e:]]
	--test-- "collect-words"
		--assert [a c] = collect-words blk
	--test-- "collect-words/deep"
		--assert [a c d e] = collect-words/deep blk
	--test-- "collect-words/set"
		--assert [c] = collect-words/set blk
		--assert [c e] = collect-words/set/deep blk
	--test-- "collect-words/ignore"
		--assert [a c] = collect-words/ignore blk none
		--assert [a] = collect-words/ignore blk [c]
		--assert [c] = collect-words/ignore blk object [a: 1]
	--test-- "collect-words/as"
	;@@ https://github.com/Oldes/Rebol-issues/issues/724
		--assert [:a :c] = collect-words/as blk get-word!
		--assert [a: c:] = collect-words/as blk set-word!
		--assert ['a 'c] = collect-words/as blk lit-word!
		--assert [/a /c] = collect-words/as blk refinement!
		--assert [#a #c] = collect-words/as blk issue!
		--assert all [error? e: try [collect-words/as blk string!] e/id = 'bad-func-arg]
===end-group===


===start-group=== "FUNCTION (funct)"
	--test-- "function/with"
		;@@ https://github.com/Oldes/Rebol-issues/issues/766
		fun: funct/with [i [integer!]][
			blk: [1 2 3]
			return pick data i
		][
			data: ["ab" "cd"]
		]
		--assert "ab" = fun 1
		;@@ https://github.com/Oldes/Rebol-issues/issues/900
		b: body-of :fun
		clear second b
		--assert [1 2 3] = second body-of :fun

	--test-- "function/with object and module"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2575
		o: object [a: 1 test: does [a * 10]]
		--assert all [
			function? try [fun: function/with [][test] o]
			10 = fun
		]
		--assert all [
			closure? try [fun: closure/with [][test] o]
			10 = fun
		]
		m: module [][a: 1 test: does [a * 20]]
		--assert all [
			function? try [fun: function/with [][test] m]
			20 = fun
		]
		--assert all [
			closure? try [fun: closure/with [][test] m]
			20 = fun
		]

===end-group===


===start-group=== "Other issues"

--test-- "issue-2025"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2025
	f: make function! reduce [[x /local x-v y-v] body: [
	    x-v: either error? try [get/any 'x] [
	        "x does not have a value"
	    ] [
	        rejoin ["x: " mold/all :x]
	    ]
	    y-v: either error? try [get/any 'y] [
	        "y does not have a value"
	    ] [
	        rejoin ["y: " mold/all :y]
	    ]
	    ;print [x-v y-v]
	]]
	g: make function! reduce [[y /local x-v y-v] body]

	--assert error? try [f 1]

--test-- "issue-2044"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2044
	body: [x + y]
    f: make function! reduce [[x] body]
    g: make function! reduce [[y] body]
    --assert error? try [f 1]

 --test-- "op! as a path"
 	;@@ https://github.com/Oldes/Rebol-issues/issues/1236
 	math: make object! [ plus: :+ ]
 	--assert error? try [1 math/plus 2]

 --test-- "issue-87"
 	;@@ https://github.com/Oldes/Rebol-issues/issues/87
 	f1: func [a][b]
 	--assert [a] = spec-of :f1
 	--assert [b] = body-of :f1

 	--assert function? f2: copy :f1
 	--assert [a] = spec-of :f2
 	--assert [b] = body-of :f2

 	--assert function? f2: make :f1 [] ; same as copy
 	--assert [a] = spec-of :f2
 	--assert [b] = body-of :f2

	--assert function? f2: make :f1 [ [x] ]
	--assert [x] = spec-of :f2
 	--assert [b] = body-of :f2

	--assert function? f2: make :f1 [ [x] [y] ]
	--assert [x] = spec-of :f2
 	--assert [y] = body-of :f2

 	--assert function? f2: make :f1 [ * [y] ]
	--assert [a] = spec-of :f2
 	--assert [y] = body-of :f2

 --test-- "body-of native!"
 	;@@ https://github.com/Oldes/Rebol-issues/issues/1578
 	--assert none? body-of :print

 --test-- "issue-168"
 	;@@ https://github.com/Oldes/Rebol-issues/issues/168
 	a: "foo"
 	f1: func [a][a + 1]
 	f2: make :f1 [[b][reduce [b a]]]
 	f3: make :f1 [[b /local a][reduce [b a]]]
 	--assert 2 = f1 1
 	--assert [2 "foo"] = f2 2
 	--assert [3 #[none]] = f3 3

 --test-- "unset as a function argument"
 ;@@ https://github.com/Oldes/Rebol-issues/issues/293
	f: func [v [unset!]] [type? v]
	--assert error? try [f make unset! none]
	f: func [v [any-type!]] [type? get/any 'v]
	--assert unset! = f make unset! none
	f: func [v [unset!]] [type? get/any 'v]
	--assert unset! = f #[unset]

--test-- "issue-196"
;@@ https://github.com/Oldes/Rebol-issues/issues/196
;@@ https://github.com/Oldes/Rebol-issues/issues/886
;@@ https://github.com/Oldes/Rebol-issues/issues/2440
	--assert function? do func [/local a] [context? 'a] ;-no crash
	--assert 1 = do has [arg] [1]
	--assert function? do has [arg] [context? 'arg] ;-no crash and recursion

--test-- "issue-216"
;@@ https://github.com/Oldes/Rebol-issues/issues/216
	f: func [a code] [do bind code 'a]
	--assert 1 = try [f 1 [a]]

--test-- "issue-217"
;@@ https://github.com/Oldes/Rebol-issues/issues/217
	f: func [c] [make function! reduce [copy [a] compose/deep [print a/1 (c)]]]
	f1: f [print 1]
	f2: f [print 2]
	--assert error? e: try [f1 1]
	--assert e/id = 'bad-path-type
--test-- "copy function"
;@@ https://github.com/Oldes/Rebol-issues/issues/2043
	f: func [] []
	--assert function? copy :f

--test-- "function rebinding (closure compatibility)"
;@@ https://github.com/Oldes/Rebol-issues/issues/2048
	; example of an R3 function with "special binding" of its body
	; create a function with "normally" bound body
	; and keep the original of its body for future use
	f: make function! reduce [[value] f-body: [value + value]]
	; some tests
	--assert 2 = f 1
	--assert 4 = f 2
	--assert 6 = f 3
	; adjust the binding
	value: 1
	change f-body 'value
	; some tests
	--assert 2 = f 1
	--assert 3 = f 2
	--assert 4 = f 3

	; example of an R3 closure with "special binding" of its body
	; create a "normal" closure
	; and keep the original of its body for future use
	f: make closure! reduce [[value] f-body: [value + value]]
	; some tests
	--assert 2 = f 1
	--assert 4 = f 2
	--assert 6 = f 3
	; adjust the binding
	value: 1
	change f-body 'value
	; some tests
	--assert 2 = f 1
	--assert 3 = f 2
	--assert 4 = f 3

--test-- "clos vs closure"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2002
	--assert empty? spec-of clos [] [a: 1]
	--assert [/local a] = spec-of closure [] [a: 1]

--test-- "closure's self"
	;@@ https://github.com/Oldes/Rebol-issues/issues/447
	slf: 'self 
	--assert do closure [x] [same? slf 'self] 1

--test-- "issue-1893"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1893
	word: do func [x] ['x] 1
	--assert same? word try [bind 'x word]

--test-- "issue-1047"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1047
	--assert all [
		error? e: try [f: func [a:][print a]]
		e/id = 'bad-func-def
	]

--test-- "issue-717"
	;@@ https://github.com/Oldes/Rebol-issues/issues/717
	--assert all [
		error? e: try [f: func [a /local b b ][]]
		e/id = 'dup-vars
		e/arg1 = 'b
	]
--test-- "PICK and the ordinals for reflection"
	;@@ https://github.com/Oldes/Rebol-issues/issues/660
	--assert all [error? e: try [first :append] e/id = 'cannot-use]

--test-- "issue-313"
	;@@ https://github.com/Oldes/Rebol-issues/issues/313
	obj: make object! [a: none f: func [v] [a: v]]
	abc: make obj []
	abc/f 3
	--assert abc/a = 3
	--assert obj/a = none

===end-group===

~~~end-file~~~
