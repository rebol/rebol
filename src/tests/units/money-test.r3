Rebol [
	Title:   "Rebol3 money test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %money-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]


~~~start-file~~~ "money"

===start-group=== "make money!"
	--test-- "make money! percent!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/971
		--assert $0   = try [make money! 0%]
		--assert $1   = try [make money! 100%]
		--assert $100 = try [make money! make percent! $100]

	--test-- "make money! binary!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1060
		--assert $15 = make money! #{00000000000000000000000F}
		--assert $15 =   to money! #{00000000000000000000000F}
		--assert $15 = make money! #{0F}
		--assert $15 =   to money! #{0F}
	;@@ https://github.com/Oldes/Rebol-issues/issues/946
		--assert #{0029B7D2DCC80CD2E3FFFFFF} = to-binary to-money #{0029B7D2DCC80CD2E3FFFFFF}
		--assert (to-money #{000100000000000000000000}) = ($1 + to-money #{0000FFFFFFFFFFFFFFFFFFFF})

	--test-- "make money! issue!" ; not supported by design
	;@@ https://github.com/Oldes/Rebol-issues/issues/1130
		--assert all [
			error? e: try [to money! #ff]
			e/id = 'bad-make-arg
		]

===end-group===


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
	;@@ https://github.com/Oldes/Rebol-issues/issues/765
	--assert $1.375 = round/to $1.333 $.125
	--assert $1.33  = round/to $1.333 $.01
	--assert     $1 = round/to $0.5   $1
	--assert     $0 = round/to $0.499 $1
	;@@ https://github.com/Oldes/Rebol-issues/issues/945
	--assert $0.9 = round/to $1 $0.9
	--assert $1.0 = round/to $1 $0.1

	--test-- "round/to (decimal)"
	;@@ https://github.com/Oldes/Rebol-issues/issues/936
	--assert 1.375 = round/to $1.333 .125
	--assert 1.33  = round/to $1.333 .01

	--test-- "round/to (integer)"
	;@@ https://github.com/Oldes/Rebol-issues/issues/936
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
	--assert $0.100000000 = ($.100000000 % $1)
	--assert $0.1000000000 = ($.1000000000 % $1)
	--assert $0.10000000000 = ($.10000000000 % $1)
	--assert $0.100000000000 = ($.100000000000 % $1)
	
===end-group===

===start-group=== "money issues"
	--test-- "issue-1441"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1441
	--assert error? try [load {1 + $}]

	--test-- "issue-492"
	;@@ https://github.com/Oldes/Rebol-issues/issues/492
	--assert $123  = round $123.123
	--assert 0.1231 = round/to to-money 0.123123 0.0001
	--assert 12.12 = round/to $12.1231 0.01
	
	--test-- "issue-116"
	;@@ https://github.com/Oldes/Rebol-issues/issues/116
	--assert  $8.00 == add      $4.00 $4.00
	--assert  $0.00 == subtract $4.00 $4.00
	--assert $16.00 == multiply $4.00 $4.00    ;@@ error?
	--assert  $1.00 == divide   $4.00 $4.00    ;@@ 1.0?

	--test-- "issue-236"
	;@@ https://github.com/Oldes/Rebol-issues/issues/236
		--assert 1.0 = to decimal! $1
	--test-- "issue-238"
	;@@ https://github.com/Oldes/Rebol-issues/issues/238
		--assert $1 = to money! $1

	--test-- "issue-253"
	;@@ https://github.com/Oldes/Rebol-issues/issues/253
		--assert "$1234"  = mold to money! 1234
		--assert "$987"   = mold m: to money! 987
		--assert "$11844" = mold to money! m * 12


	--test-- "issue-254"
	;@@ https://github.com/Oldes/Rebol-issues/issues/254
		m: $1 for i 1 508 1 [m: m * 2]
		--assert m = $83798799562141231872337704e127
		--assert error? try [m * 2]
		--assert error? try [m: to money! 2 ** 509]

	--test-- "issue-255"
	;@@ https://github.com/Oldes/Rebol-issues/issues/255
		--assert error? try [to-money #0]
		--assert error? try [pick $1 2]

	--test-- "issue-392"
	;@@ https://github.com/Oldes/Rebol-issues/issues/392
		--assert $1e-100 = ($1.00 / 1e100) ; no crash

	--test-- "issue-1205"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1205
		--assert error? try [$1 / #"a"]
		--assert error? try [$1 * 1.2.3.4]
		--assert error? try [$1 - 10:30]

	--test-- "issue-569"
	;@@ https://github.com/Oldes/Rebol-issues/issues/569
		--assert not strict-equal? $1 1

	--test-- "money! time! math"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2497
		--assert $7.5 = ($5 * 1:30:0)
		--assert error? try [$5 / 1:30:0]
		--assert error? try [$5 + 1:30:0]
		--assert error? try [$5 - 1:30:0]

===end-group===
	
~~~end-file~~~
