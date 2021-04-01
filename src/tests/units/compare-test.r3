Rebol [
	Title:   "Rebol3 compare test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %compare-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "COMPARE"

===start-group=== "char!"
	--test-- "char! = ..."
		--assert #"a" =  #"a"
		--assert #"a" = 97

	--test-- "char! <> ..."
		--assert #"a" <> #"b"
		--assert #"a" <> 98
		--assert #"a" <> 97.0
		--assert #"a" <> $97
		--assert #"a" <> 97%
		--assert #"a" <> "a"

	--test-- "char! < ..."
		--assert #"a" <  #"b"
		--assert #"a" < 98

	--test-- "char! > ..."
		--assert #"b" >  #"a"
		--assert #"a" > 96

	--test-- "char! invalid compare"
		--assert all [error? e: try [#"a" < 98.0] e/id = 'invalid-compare]
		--assert all [error? e: try [#"a" < $98 ] e/id = 'invalid-compare]
		--assert all [error? e: try [#"a" < 98% ] e/id = 'invalid-compare]
		--assert all [error? e: try [#"a" < "a" ] e/id = 'invalid-compare]
		--assert all [error? e: try [#"a" < 1x1 ] e/id = 'invalid-compare]

===end-group===

===start-group=== "integer!"
	--test-- "integer! = ..."
		--assert 97 = 97
		--assert 97 = 97.0
		--assert 97 = 9700%
		--assert 97 =  #"a"

	--test-- "integer! < ..."
		--assert 97 < 98
		--assert 97 < 97.1
		--assert 97 < 9701%
		--assert 97 <  #"b"

	--test-- "integer! > ..."
		--assert 97 > 96
		--assert 97 > 96.0
		--assert 97 > 9600%
		--assert 98 > #"a"

	--test-- "integer! invalid compare"
		--assert all [error? e: try [90 < "a" ] e/id = 'invalid-compare]
		--assert all [error? e: try [90 < 1x1 ] e/id = 'invalid-compare]
===end-group===

~~~end-file~~~
