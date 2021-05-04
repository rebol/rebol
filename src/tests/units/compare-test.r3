Rebol [
	Title:   "Rebol3 compare test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %compare-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "COMPARE"

===start-group=== "char!"
	--test-- "char! == ..."
		--assert #"a" == #"a"
		--assert not (#"a" == 97)

	--test-- "char! = ..."
		--assert #"a" = #"a"
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
	--test-- "integer! == ..."
		--assert 97 == 97
		--assert not (97 == 97.0)
		--assert not (97 == 9700%)
		--assert not (97 ==  #"a")
		--assert not (97 == 0:01:37)

	--test-- "integer! = ..."
		--assert 97 = 97
		--assert 97 = 97.0
		--assert 97 = 9700%
		--assert 97 =  #"a"
		--assert 97 = 0:01:37

	--test-- "integer! < ..."
		--assert 97 < 98
		--assert 97 < 97.1
		--assert 97 < 9701%
		--assert 97 <  #"b"
		--assert 97 < 0:01:38

	--test-- "integer! > ..."
		--assert 97 > 96
		--assert 97 > 96.0
		--assert 97 > 9600%
		--assert 98 > #"a"
		--assert 98 > 0:01:37

	--test-- "integer! invalid compare"
		--assert all [error? e: try [90 < "a" ] e/id = 'invalid-compare]
		--assert all [error? e: try [90 < 1x1 ] e/id = 'invalid-compare]
===end-group===


===start-group=== "decimal!"
	--test-- "decimal! == ..."
		--assert 97.0 == 97.0
		--assert not (97.0 == 97)
		--assert not (97.0 == 9700%)
		--assert not (97.0 ==  #"a")
		--assert not (97.0 == 0:01:37)
		--assert not same? 0.3 (0.1 + 0.1 + 0.1)

	--test-- "decimal! = ..."
		--assert 97.0 = 97
		--assert 97.0 = 97.0
		--assert 97.0 = 9700%
		--assert not (97.0 = #"a")
		--assert 97.0 = 0:01:37
		--assert equal? 0.3 (0.1 + 0.1 + 0.1)
		--assert equal? (0.1 + 0.1 + 0.1) 0.3
		--assert equal? (0.1 + 0.1 + 0.1) 0:0:0.3

	--test-- "decimal! < ..."
		--assert 97.0 < 98
		--assert 97.0 < 97.1
		--assert 97.0 < 9701%
		--assert 97.0 < 0:01:38

	--test-- "decimal! > ..."
		--assert 97.0 > 96
		--assert 97.0 > 96.0
		--assert 97.0 > 9600%
		--assert 98.0 > 0:01:37

	--test-- "decimal! invalid compare"
		--assert all [error? e: try [90.0 < "a" ] e/id = 'invalid-compare]
		--assert all [error? e: try [90.0 < 1x1 ] e/id = 'invalid-compare]
===end-group===


===start-group=== "time!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1103
	--test-- "time! == ..."
		--assert 0:0:1 == 0:0:1
		--assert not (0:0:1 == 1)
		--assert not (0:0:1 == 1.0)
		--assert not (0:0:1 == 100%)

	--test-- "time! = ..."
		--assert 0:0:1 = 0:0:1
		--assert 0:0:1 = 1
		--assert 0:0:1 = 1.0
		--assert 0:0:1.1 = 1.1
		--assert 0:0:1 = 100%
		--assert 0:0:0.3 = (0.1 + 0.1 + 0.1)
		--assert not equal? 0:0:1 $1

	--test-- "time! < ..."
		--assert 0:0:1 < 0:0:2
		--assert 0:0:1 < 2
		--assert 0:0:1 < 2.0
		--assert 0:0:1.1 < 1.2
		--assert 0:0:1 < 200%

	--test-- "time! > ..."
		--assert 0:0:2 > 0:0:1
		--assert 0:0:2 > 1
		--assert 0:0:2 > 1.0
		--assert 0:0:2.2 > 2.1
		--assert 0:0:2 > 100%

	--test-- "time! invalid compare"
		--assert all [error? e: try [0:0:0 < "a" ] e/id = 'invalid-compare]
		--assert all [error? e: try [0:0:0 < 1x1 ] e/id = 'invalid-compare]
===end-group===

~~~end-file~~~
