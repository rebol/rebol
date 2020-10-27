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
===end-group===


~~~end-file~~~
