Rebol [
	Title:   "Rebol3 decimal test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %decimal-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]


~~~start-file~~~ "decimal"

===start-group=== "max/min"
	--test-- "max"
		--assert  3.0 == max  3.0 1
		--assert  3.0 == max  3.0 1.0
		--assert  3.0 == max  3.0 $1
	--test-- "min"
		--assert -3.0 == min -3.0 2
		--assert -3.0 == min -3.0 1.0
		--assert -3.0 == min -3.0 $1
===end-group===

===start-group=== "round"
	--test-- "round"
	--assert  1.0 = round  1.4999
	--assert  2.0 = round  1.5
	--assert -2.0 = round -1.5

	--test-- "round/to (decimal)"
	--assert 1.375 = round/to 1.333 .125
	--assert 1.33  = round/to 1.333 .01

	--test-- "round/to (integer)"
	--assert     1 = round/to 0.5   1
	--assert     0 = round/to 0.499 1
	--assert integer? round/to 0.5  1

	--test-- "round/to (money)"
	--assert   $1 = round/to 0.5   $1
	--assert   $0 = round/to 0.499 $1
	--assert money? round/to 0.5   $1

	--test-- "round/down"
	--assert  1.0 = round/down  1.999
	--assert -1.0 = round/down -1.999

	--test-- "round/even"
	--assert  2.0 = round/even  1.5
	--assert -2.0 = round/even -1.5

	--test-- "round/half-down"
	--assert  1.0 = round/half-down  1.5
	--assert -1.0 = round/half-down -1.5

	--test-- "round/floor"
	--assert  1.0 = round/floor  1.999
	--assert -2.0 = round/floor -1.0000001

	--test-- "round/ceiling"
	--assert  2.0 = round/ceiling  1.0000001
	--assert -1.0 = round/ceiling -1.999

	--test-- "round/half-ceiling"
	--assert  2.0 = round/half-ceiling  1.5
	--assert -1.0 = round/half-ceiling -1.5

===end-group===

===start-group=== "to-degrees & to-radians"
;@@ https://github.com/Oldes/Rebol-issues/issues/2408
	--test-- "to-degrees to-radians"
		foreach [d r] [
			0   0.0
			30	0.5235987756
			45	0.7853981634
			60	1.0471975512
			90	1.5707963268
			120	2.0943951024
			135	2.3561944902
			150	2.6179938780
			180	3.1415926536
			270	4.7123889804
			360	6.2831853072
		][
			--assert r = round/to r: to-radians d 0.0000000001
			--assert d = to-degrees r
		]
===end-group===

===start-group=== "decimal issues"
	--test-- "issue-1753"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1753
	--assert "399405206703547.0" = mold tangent 89.99999999999986
	--assert 1.#INF = tangent 89.99999999999987

	--test-- "issue-95"
	;@@ https://github.com/Oldes/Rebol-issues/issues/95
	--assert 2.1 = (1 + 1.1)
	--assert 0.1 = (1.1 - 1)
	--assert 1.1 = (1 * 1.1)

	--test-- "issue-241"
	;@@ https://github.com/Oldes/Rebol-issues/issues/241
		--assert  0.0 == to decimal! #{0000000000000000}
		--assert  0.0 == to decimal! #{8000000000000000}
		--assert  1.#INF = to decimal! #{7FF0000000000000}
		--assert -1.#INF = to decimal! #{FFF0000000000000}
		--assert "1.#NaN" = mold to decimal! #{7FFFFFFFFFFFFFFF}
		
	--test-- "issue-267"
	;@@ https://github.com/Oldes/Rebol-issues/issues/267
		--assert 97.0 = try [1.0 * #"a"]

===end-group===
	
~~~end-file~~~
