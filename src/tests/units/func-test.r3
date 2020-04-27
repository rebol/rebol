Rebol [
	Title:   "Rebol function related test script"
	Author:  "Oldes"
	File: 	 %func-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Function"

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
	--assert unset! = f #[unset!]

--test-- "issue-196"
;@@ https://github.com/Oldes/Rebol-issues/issues/196
	--assert do func [a] [bind? 'a] 1 ;-no crash

===end-group===

~~~end-file~~~
