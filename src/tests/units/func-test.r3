Rebol [
	Title:   "Rebol function related test script"
	Author:  "Oldes"
	File: 	 %func-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Function"

===start-group=== "body-of"

--test-- "body-of NATIVE or ACTION"
	;@@ https://github.com/rebol/rebol-issues/issues/1577
	; body-of NATIVE or ACTION should return NONE 
	--assert none? body-of :equal?
	--assert none? body-of :add
	--assert none? body-of :+
	--assert none? body-of :append

--test-- "body-of FUNCTION"
	fce: func[a [integer!]][probe a]
	--assert [probe a] = body-of :fce

--test-- "invalid MAKE"
	;@@ https://github.com/rebol/rebol-issues/issues/1052
	--assert error? try [make :read [[][]]]
	--assert error? try [make action! [[][]]]
	--assert error? try [make native! [[][]]]
	--assert error? try [make op! [[][]]]

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

===end-group===

~~~end-file~~~
