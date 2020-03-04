Rebol [
	Title:   "Rebol3 money test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %money-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]


~~~start-file~~~ "money"

===start-group=== "max/min"
	--test-- "max"
		--assert  $3 == max  $3 1
		--assert  $3 == max  $3 1.0
		--assert  $3 == max  $3 $1
	--test-- "min"
		--assert -$3 == min -$3 2
		--assert -$3 == min -$3 1.0
		--assert -$3 == min -$3 $1
===end-group===

===start-group=== "round"
	--test-- "round"
	--assert  $1 = round  $1.4999
	--assert  $2 = round  $1.5
	--assert -$2 = round -$1.5

	--test-- "round/to"
	--assert $1.375 = round/to $1.333 $.125
	--assert $1.33  = round/to $1.333 $.01
	--assert     $1 = round/to $0.5   $1
	--assert     $0 = round/to $0.499 $1

	--test-- "round/to (decimal)"
	--assert 1.375 = round/to $1.333 .125
	--assert 1.33  = round/to $1.333 .01

	--test-- "round/to (integer)"
	--assert     1 = round/to $0.5   1
	--assert     0 = round/to $0.499 1

	--test-- "round/down"
	--assert  $1 = round/down  $1.999
	--assert -$1 = round/down -$1.999

	--test-- "round/even"
	--assert  $2 = round/even  $1.5
	--assert -$2 = round/even -$1.5

	--test-- "round/half-down"
	--assert  $1 = round/half-down  $1.5
	--assert -$1 = round/half-down -$1.5

	--test-- "round/floor"
	--assert  $1 = round/floor  $1.999
	--assert -$2 = round/floor -$1.0000001

	--test-- "round/ceiling"
	--assert  $2 = round/ceiling  $1.0000001
	--assert -$1 = round/ceiling -$1.999

	--test-- "round/half-ceiling"
	--assert  $2 = round/half-ceiling  $1.5
	--assert -$1 = round/half-ceiling -$1.5

===end-group===

===start-group=== "reminder on money"
	--test-- "issue-401"
	;@@ https://github.com/Oldes/Rebol-issues/issues/401
	--assert $0.100000000 = $.100000000 // $1
	--assert $0.1000000000 = $.1000000000 // $1
	--assert $0.10000000000 = $.10000000000 // $1
	--assert $0.100000000000 = $.100000000000 // $1
	
===end-group===

===start-group=== "money issues"
	--test-- "issue-1441"
	;@@ https://github.com/rebol/rebol-issues/issues/1441
	--assert error? try [load {1 + $}]

	--test-- "issue-492"
	;@@ https://github.com/Oldes/Rebol-issues/issues/492
	--assert $123  = round $123.123
	--assert 0.1231 = round/to to-money 0.123123 0.0001
	--assert 12.12 = round/to $12.1231 0.01
	
===end-group===
	
~~~end-file~~~
