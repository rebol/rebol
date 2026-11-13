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

	--test-- "decimal! equal?/equiv?/same?"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1134
		--assert     equal? to decimal! #{3FD3333333333333} to decimal! #{3FD3333333333333}
		--assert     equiv? to decimal! #{3FD3333333333333} to decimal! #{3FD3333333333333}
		--assert      same? to decimal! #{3FD3333333333333} to decimal! #{3FD3333333333333}
		--assert     equal? to decimal! #{3FD3333333333333} to decimal! #{3FD3333333333334}
		--assert not equiv? to decimal! #{3FD3333333333333} to decimal! #{3FD3333333333334}
		--assert not  same? to decimal! #{3FD3333333333333} to decimal! #{3FD3333333333334}
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


===start-group=== "block!"
;@@ https://github.com/Oldes/Rebol-issues/issues/2501
;@@ https://github.com/Oldes/Rebol-issues/issues/2594
	--test-- "equal? block! with strings"
		--assert equal? ["a"] ["a"]
		--assert equal? ["a"] ["A"]
	--test-- "equal? block! with words"
		--assert equal? [a] [a]
		--assert equal? [a] [a:]
		--assert equal? [a] [:a]
		--assert equal? [a] ['a]
		--assert equal? [a] [/a]
	--test-- "equal? block! with numbers"
		--assert equal? [1] [1]
		--assert equal? [1] [1.0]
		--assert equal? [1] [100%]
		--assert equal? [1] [$1]
		--assert equal? [1.0] [1.0]
		--assert equal? [1.0] [1]
		--assert equal? [1.0] [100%]
		--assert equal? [1.0] [$1]
		--assert equal? [1%]   [1%]
		--assert equal? [100%] [1]
		--assert equal? [100%] [1.0]
		--assert equal? [100%] [$1]
		--assert equal? [$1] [$1]
		--assert equal? [$1] [1]
		--assert equal? [$1] [1.0]
		--assert equal? [$1] [100%]
	--test-- "equal? block! with blocks"
		--assert     equal? [[1]] [[1]]
		--assert not equal? [[1]] [(1)]


	--test-- "strict-equal? block! with strings"
		--assert     strict-equal? ["a"] ["a"]
		--assert not strict-equal? ["a"] ["A"]
	--test-- "strict-equal? block! with words"
		--assert     strict-equal? [a] [a]
		--assert not strict-equal? [a] [a:]
		--assert not strict-equal? [a] [:a]
		--assert not strict-equal? [a] ['a]
		--assert not strict-equal? [a] [/a]
	--test-- "strict-equal? block! with numbers"
		--assert     strict-equal? [1] [1]
		--assert not strict-equal? [1] [1.0]
		--assert not strict-equal? [1] [100%]
		--assert not strict-equal? [1] [$1]
		--assert     strict-equal? [1.0] [1.0]
		--assert not strict-equal? [1.0] [1]
		--assert not strict-equal? [1.0] [100%]
		--assert not strict-equal? [1.0] [$1]
		--assert     strict-equal? [1%]   [1%]
		--assert not strict-equal? [100%] [1]
		--assert not strict-equal? [100%] [1.0]
		--assert not strict-equal? [100%] [$1]
		--assert     strict-equal? [$1] [$1]
		--assert not strict-equal? [$1] [1]
		--assert not strict-equal? [$1] [1.0]
		--assert not strict-equal? [$1] [100%]
	--test-- "strict-equal? block! with blocks"
		--assert     strict-equal? [[1]] [[1]]
		--assert not strict-equal? [[1]] [(1)]

===end-group===


===start-group=== "object!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1133
	--test-- "equal? objects with numeric values"
		--assert equal? construct [c: 1] construct [c: 1]
		--assert equal? construct [c: 1] construct [c: 1.0]
		--assert equal? construct [c: 1] construct [c: $1]
		--assert equal? construct [c: 1] construct [c: 100%]
		--assert equal? construct [c: 1.0]  construct [c: 1]
		--assert equal? construct [c: $1]   construct [c: 1]
		--assert equal? construct [c: 100%] construct [c: 1]
	--test-- "strict-equal? objects with numeric values"
		--assert strict-equal? construct [c: 1] construct [c: 1]
		--assert not strict-equal? construct [c: 1] construct [c: 1.0]
		--assert not strict-equal? construct [c: 1] construct [c: $1]
		--assert not strict-equal? construct [c: 1] construct [c: 100%]
		--assert not strict-equal? construct [c: 1.0]  construct [c: 1]
		--assert not strict-equal? construct [c: $1]   construct [c: 1]
		--assert not strict-equal? construct [c: 100%] construct [c: 1]
===end-group===


;- tests from Red Language...
===start-group=== "prefix equal same datatype"
	--test-- "prefix-equal-same-datatype-1"		--assert equal? 0 0
	--test-- "prefix-equal-same-datatype-2"		--assert equal? 1 1
	--test-- "prefix-equal-same-datatype-3"		--assert equal? 0#FFFFFFFFFFFFFFFF -1
	--test-- "prefix-equal-same-datatype-4"		--assert equal? [] []
	--test-- "prefix-equal-same-datatype-5"		--assert equal? [a] [a]
	--test-- "prefix-equal-same-datatype-6"		--assert equal? [A] [a]
	--test-- "prefix-equal-same-datatype-7"		--assert equal? ['a] [a]
	--test-- "prefix-equal-same-datatype-8"		--assert equal? [a:] [a]
	--test-- "prefix-equal-same-datatype-9"		--assert equal? [:a] [a]
	--test-- "prefix-equal-same-datatype-10"	--assert equal? [:a] [a:]
	--test-- "prefix-equal-same-datatype-11"	--assert equal? [abcde] [abcde]
	--test-- "prefix-equal-same-datatype-12"	--assert equal? [a b c d] [a b c d]
	--test-- "prefix-equal-same-datatype-13"	--assert equal? [b c d] next [a b c d]
	--test-- "prefix-equal-same-datatype-14"	--assert equal? [b c d] (next [a b c d])
	--test-- "prefix-equal-same-datatype-15"	--assert equal? "a" "a"
	--test-- "prefix-equal-same-datatype-16"	--assert equal? "a" "A"
	--test-- "prefix-equal-same-datatype-17"	--assert equal? "abcdeè" "abcdeè"
	--test-- "prefix-equal-same-datatype-18"	--assert equal? (next "abcdeè") next "abcdeè"
	--test-- "prefix-equal-same-datatype-19"	--assert equal? (first "abcdeè") first "abcdeè"
	--test-- "prefix-equal-same-datatype-20"	--assert equal? (last "abcdeè") last "abcdeè"
	--test-- "prefix-equal-same-datatype-21"	--assert equal? "abcde^(2710)é" "abcde^(2710)é"
	--test-- "prefix-equal-same-datatype-22"	--assert equal? [d] back tail [a b c d]
	--test-- "prefix-equal-same-datatype-23"	--assert equal? "2345" next "12345"
	--test-- "prefix-equal-same-datatype-24"	--assert equal? #"z" #"z"
	--test-- "prefix-equal-same-datatype-25"    --assert equal? #"z" #"Z" ;@@ in Red this is not equal!
	--test-- "prefix-equal-same-datatype-25" --red-- --assert not equal? #"z" #"Z"
	--test-- "prefix-equal-same-datatype-26"	--assert not equal? #"e" #"è"
;	--test-- "prefix-equal-same-datatype-27"	--assert equal? #"^(010000)" #"^(010000)"
	--test-- "prefix-equal-same-datatype-28"	--assert equal? true true
	--test-- "prefix-equal-same-datatype-29"	--assert equal? false false
	--test-- "prefix-equal-same-datatype-30"	--assert not equal? false true
	--test-- "prefix-equal-same-datatype-31"	--assert not equal? true false
	--test-- "prefix-equal-same-datatype-32"	--assert equal? none none
	--test-- "prefix-equal-same-datatype-33"	--assert equal? 'a 'a
	--test-- "prefix-equal-same-datatype-34"	--assert equal? 'a 'A
	--test-- "prefix-equal-same-datatype-35"	--assert equal? (first [a]) first [a]
	--test-- "prefix-equal-same-datatype-36"	--assert equal? 'a first [A]
	--test-- "prefix-equal-same-datatype-37"	--assert equal? 'a first ['a]
	--test-- "prefix-equal-same-datatype-38"	--assert equal? 'a first [:a]
	--test-- "prefix-equal-same-datatype-39"	--assert equal? 'a first [a:]
	--test-- "prefix-equal-same-datatype-40"	--assert equal? (first [a:]) first [a:]
	--test-- "prefix-equal-same-datatype-41"	--assert equal? (first [:a]) first [:a]
	--test-- "prefix-equal-same-datatype-42"	--assert equal? [a b c d e] first [[a b c d e]]
	--test-- "prefix-equal-same-datatype-43" ea-result: 1 = 1      --assert ea-result = true
	--test-- "prefix-equal-same-datatype-44" ea-result: 1 = 0      --assert ea-result = false
	--test-- "prefix-equal-same-datatype-45" ea-result: equal? 1 1 --assert ea-result = true
	--test-- "prefix-equal-same-datatype-46" ea-result: equal? 1 0 --assert ea-result = false
===end-group===

===start-group=== "prefix strict-equal same datatype"
	--test-- "prefix-strict-equal-same-datatype-1"  --assert     strict-equal? 0 0
	--test-- "prefix-strict-equal-same-datatype-2"  --assert     strict-equal? 1 1
	--test-- "prefix-strict-equal-same-datatype-3"  --assert     strict-equal? 0#FFFFFFFFFFFFFFFF -1
	--test-- "prefix-strict-equal-same-datatype-4"  --assert     strict-equal? [] []
	--test-- "prefix-strict-equal-same-datatype-5"  --assert     strict-equal? [a] [a]
	--test-- "prefix-strict-equal-same-datatype-6"  --assert not strict-equal? [A] [a]
	--test-- "prefix-strict-equal-same-datatype-7"  --assert not strict-equal? ['a] [a]
	--test-- "prefix-strict-equal-same-datatype-8"  --assert not strict-equal? [a:] [a]
	--test-- "prefix-strict-equal-same-datatype-9"  --assert not strict-equal? [:a] [a]
	--test-- "prefix-strict-equal-same-datatype-10" --assert not strict-equal? [:a] [a:]
	--test-- "prefix-strict-equal-same-datatype-11" --assert     strict-equal? [abcde] [abcde]
	--test-- "prefix-strict-equal-same-datatype-12" --assert     strict-equal? [a b c d] [a b c d]
	--test-- "prefix-strict-equal-same-datatype-13" --assert     strict-equal? [b c d] next [a b c d]
	--test-- "prefix-strict-equal-same-datatype-14" --assert     strict-equal? [b c d] (next [a b c d])
	--test-- "prefix-strict-equal-same-datatype-15" --assert     strict-equal? "a" "a"
	--test-- "prefix-strict-equal-same-datatype-16" --assert not strict-equal? "a" "A"
	--test-- "prefix-strict-equal-same-datatype-17" --assert     strict-equal? "abcdeè" "abcdeè"
	--test-- "prefix-strict-equal-same-datatype-18" --assert     strict-equal? (next "abcdeè") next "abcdeè"
	--test-- "prefix-strict-equal-same-datatype-19" --assert     strict-equal? (first "abcdeè") first "abcdeè"
	--test-- "prefix-strict-equal-same-datatype-20" --assert     strict-equal? (last "abcdeè") last "abcdeè"
	--test-- "prefix-strict-equal-same-datatype-21" --assert     strict-equal? "abcde^(2710)é" "abcde^(2710)é"
	--test-- "prefix-strict-equal-same-datatype-22" --assert     strict-equal? [d] back tail [a b c d]
	--test-- "prefix-strict-equal-same-datatype-23" --assert     strict-equal? "2345" next "12345"
	--test-- "prefix-strict-equal-same-datatype-24" --assert     strict-equal? #"z" #"z"
	--test-- "prefix-strict-equal-same-datatype-25" --assert not strict-equal? #"z" #"Z"
	--test-- "prefix-strict-equal-same-datatype-26" --assert not strict-equal? #"e" #"è"
;	--test-- "prefix-strict-equal-same-datatype-27" --assert strict-equal? #"^(010000)" #"^(010000)"
	--test-- "prefix-strict-equal-same-datatype-28" --assert     strict-equal? true true
	--test-- "prefix-strict-equal-same-datatype-29" --assert     strict-equal? false false
	--test-- "prefix-strict-equal-same-datatype-30" --assert not strict-equal? false true
	--test-- "prefix-strict-equal-same-datatype-31" --assert not strict-equal? true false
	--test-- "prefix-strict-equal-same-datatype-32" --assert     strict-equal? none none
	--test-- "prefix-strict-equal-same-datatype-33" --assert     strict-equal? 'a 'a
	--test-- "prefix-strict-equal-same-datatype-34" --assert not strict-equal? 'a 'A
	--test-- "prefix-strict-equal-same-datatype-35" --assert     strict-equal? (first [a]) first [a]
	--test-- "prefix-strict-equal-same-datatype-36" --assert     strict-equal? 'a first [a]
	--test-- "prefix-strict-equal-same-datatype-37" --assert not strict-equal? 'a first ['a]
	--test-- "prefix-strict-equal-same-datatype-38" --assert not strict-equal? 'a first [:a]
	--test-- "prefix-strict-equal-same-datatype-39" --assert not strict-equal? 'a first [a:]
	--test-- "prefix-strict-equal-same-datatype-40" --assert     strict-equal? (first [a:]) first [a:]
	--test-- "prefix-strict-equal-same-datatype-41" --assert     strict-equal? (first [:a]) first [:a]
	--test-- "prefix-strict-equal-same-datatype-42" --assert     strict-equal? [a b c d e] first [[a b c d e]]
	--test-- "prefix-strict-equal-same-datatype-43" ea-result: 1 == 1      --assert ea-result = true
	--test-- "prefix-strict-equal-same-datatype-44" ea-result: 1 == 0      --assert ea-result = false
===end-group===

===start-group=== "prefix equal implcit cast"
	--test-- "prefix-equal-implcit-cast-1"		--assert equal? #"0" 48
	--test-- "prefix-equal-implcit-cast-2"		--assert equal? 48 #"0"
	--test-- "prefix-equal-implcit-cast-3"		--assert equal? #"^(2710)" 10000
;	--test-- "prefix-equal-implcit-cast-4"		--assert equal? #"^(010000)" 65536
	--test-- "prefix-equal-implcit-cast-5" ea-result: #"1" = 49 --assert ea-result = true
===end-group===

===start-group=== "prefix-greater-same-datatype"
	--test-- "prefix-greater-same-datatype-1"	--assert not greater? 0 0
	--test-- "prefix-greater-same-datatype-2"	--assert     greater? 1 0
	--test-- "prefix-greater-same-datatype-3"	--assert not greater? 1 1
	--test-- "prefix-greater-same-datatype-4"	--assert not greater? 0#FFFFFFFFFFFFFFFF -1
	--test-- "prefix-greater-same-datatype-5"	--assert     greater? -1 0#FFFFFFFFFFFFFFFE
	--test-- "prefix-greater-same-datatype-6"	--assert not greater? -2 0#FFFFFFFFFFFFFFFF
	--test-- "prefix-greater-same-datatype-7"	--assert not greater? "a" "a"
	--test-- "prefix-greater-same-datatype-8"	--assert     greater? "b" "a"
	--test-- "prefix-greater-same-datatype-9"	--assert     greater? "è" "f"
	--test-- "prefix-greater-same-datatype-10"	--assert not greater? "A" "a"
	--test-- "prefix-greater-same-datatype-11"	--assert not greater? "a" "A"
	--test-- "prefix-greater-same-datatype-12"	--assert not greater? "abcdeè" "abcdeè"
	--test-- "prefix-greater-same-datatype-13"	--assert not greater? (next "abcdeè") next "abcdeè"
	--test-- "prefix-greater-same-datatype-14"	--assert not greater? (first "abcdeè") first "abcdeè"
	--test-- "prefix-greater-same-datatype-15"	--assert not greater? (last "abcdeè") last "abcdeè"
	--test-- "prefix-greater-same-datatype-16"	--assert not greater? "abcde^(2710)é" "abcde^(2710)é"
	--test-- "prefix-greater-same-datatype-17"	--assert not greater? "2345" next "12345"
	--test-- "prefix-greater-same-datatype-18"	--assert not greater? #"z" #"z"
	--test-- "prefix-greater-same-datatype-19"	--assert     greater? #"z" #"Z"
	--test-- "prefix-greater-same-datatype-20"	--assert     greater? #"è" #"e"
;	--test-- "prefix-greater-same-datatype-21"	--assert not greater? #"^(010000)" #"^(010000)"
===end-group===


~~~end-file~~~
