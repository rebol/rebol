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
	
~~~end-file~~~
