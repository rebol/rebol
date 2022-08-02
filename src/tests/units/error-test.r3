Rebol [
	Title:   "Rebol error test script"
	Author:  "Oldes"
	File: 	 %error-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "ERROR"

===start-group=== "TRY"
	--test-- "basic TRY"
	--assert 2 = try [1 + 1]
	--assert 2 = try first [(1 + 1)]
===end-group===


===start-group=== "make error!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/835
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
	;@@ https://github.com/Oldes/Rebol-issues/issues/993
	--assert all [
		error? e: make e "message"
		e/type = 'User
		e/arg1 = "message"
	]

	--test-- "make invalid error!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1002
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

	;@@ https://github.com/Oldes/Rebol-issues/issues/1003
	; it is not posible to make `Throw` type errors!
	--assert all [
		error? e: try [mold/all/flat make error! [type: 'Throw id: 'halt]]
		e/id = 'invalid-arg
	]
	
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
		--assert assert/type [x #[string!]]

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
		;@@ https://github.com/Oldes/Rebol-issues/issues/889
		;@@ https://github.com/Oldes/Rebol-issues/issues/1001
		--assert object? o: to-object try [1 / 0]
		         o/code: 1
		--assert error? e: to-error o
		--assert e/code = 400

	--test-- "protected catalog"
		;@@ https://github.com/Oldes/Rebol-issues/issues/840
		--assert all [error? e: try [system/catalog/errors: none] e/id = 'locked-word]
		--assert all [error? e: try [system/catalog/errors/Math: none] e/id = 'locked-word]

===end-group===

~~~end-file~~~
