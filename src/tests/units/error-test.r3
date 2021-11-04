Rebol [
	Title:   "Rebol error test script"
	Author:  "Oldes"
	File: 	 %error-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "ERROR"

===start-group=== "make error!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1593
	--test-- "make error!"
	k: keys-of system/catalog/errors/Math
	--assert all [
		error? e: make error! [type: 'math id: 'positive]
		e/code = (system/catalog/errors/Math/code - 3 + index? find k 'positive)
		e/type = 'Math
		e/id   = 'positive
	]
	--assert all [
		error? e: make error! [code: 500 type: 'math id: 'overflow] ;- code value 500 ignored!
		e/code = (system/catalog/errors/Math/code - 3 + index? find k 'overflow) 
		e/type = 'Math
		e/id   = 'overflow
	]

	--test-- "make invalid error!"
	--assert all [ ;missing type and id
		error? e: try [make error! []]
		e/type = 'Internal
		e/id   = 'invalid-error
	]
	--assert all [ ;missing type and id
		error? e: try [make error! [code: 400]]
		e/type = 'Internal
		e/id   = 'invalid-error
	]
	--assert all [ ;missing id
		error? e: try [make error! [code: 500 type: 'math]]
		e/type = 'Internal
		e/id   = 'invalid-error
	]
	--assert all [
		error? e: try [make error! [type: 'math id: 'foo]]
		e/type = 'Script
		e/id   = 'invalid-arg
		e/arg1 = 'foo
	]
	--assert all [
		error? e: try [make error! [type: 'math id: 42]]
		e/type = 'Script
		e/id   = 'invalid-arg
		e/arg1 = 42
	]

	--assert all [
		error? e: try [make error! [type: 'foo id: 'overflow]]
		e/type = 'Script
		e/id   = 'invalid-arg
		e/arg1 = 'foo
	]
	;@@ https://github.com/Oldes/Rebol-issues/issues/975
	--assert all [error? e: try [make error! 1] e/id = 'invalid-arg]
	
===end-group===


===start-group=== "ASSERT"
	--test-- "valid assert"
		--assert assert [not none? 1]
		--assert all [
			error? e: try [assert [not none? none]]
			e/id   = 'assert-failed
			e/arg1 = [not none? none]
		]

	--test-- "assert with longer expression"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2231
		--assert all [
			error? e: try [assert [true 1 + 3 = 2 true]]
			e/id   = 'assert-failed
			e/arg1 = [true 1 + 3 = 2 true]
		]

	--test-- "valid assert/type"
		x: 1
		--assert assert/type [x  integer! ]
		--assert assert/type [x [integer!]]
		x: ""
		--assert all [
			error? e: try [assert/type [x integer!]]
			e/id   = 'wrong-type
			e/arg1 = 'x
		]
		--assert assert/type [x [integer! string!]]
		;@@ https://github.com/Oldes/Rebol-issues/issues/1364
		--assert assert/type [x #[typeset! [char! string!]]]
		--assert assert/type [x any-string!]
		--assert assert/type [x #[datatype! string!]]

	--test-- "invalid assert"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1363
	--assert all [
		error? e: try [assert/type [1 integer!]]
		e/id = 'invalid-arg
		e/arg1 = 1
	]

===end-group===

===start-group=== "Error issues"
	--test-- "issue-553"
		;@@ https://github.com/Oldes/Rebol-issues/issues/553
		--assert all [
			error? e: try [x: does [] length? :x]
			e/id = 'expect-arg
		]
		--assert all [
			error? e: try [x: does [] last :x]
			e/id = 'expect-arg
		]
	--test-- "object from error"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1001
		--assert object? o: to-object try [1 / 0]
		         o/code: 1
		--assert error? e: to-error o
		--assert e/code = 400
===end-group===

~~~end-file~~~
