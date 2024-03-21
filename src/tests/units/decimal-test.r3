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
	;@@ https://github.com/Oldes/Rebol-issues/issues/507
	--assert "0.5" = mold round/to 0.5 0.1
	--assert "0.6" = mold round/to 0.6 0.1
	--assert "0.7" = mold round/to 0.7 0.1
	--assert "1.0" = mold round/to 1   0.1

	--test-- "round/to (integer)"
	;@@ https://github.com/Oldes/Rebol-issues/issues/936
	--assert     1 = round/to 0.5   1
	--assert     0 = round/to 0.499 1
	--assert integer? round/to 0.5  1
	--assert   1.0 = round/to 1 0.1

	--test-- "round/to (money)"
	--assert   $1 = round/to 0.5   $1
	--assert   $0 = round/to 0.499 $1
	--assert money? round/to 0.5   $1

	--test-- "round/to (percent)"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1028
	--assert 150% = round/to 1.45677 10%
	--assert 150% = round/to 145.6%  10%
	--assert 150% = to percent! round/to 1.45677 to decimal! 10%

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

	--test-- "round/even/to"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2324
	--assert 2 = round/even/to 1.555555 1
	--assert "1.55556" = mold round/even/to 1.555555 1E-5

===end-group===

===start-group=== "even? / odd?"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1775
	--test-- "even?"
	--assert even? 0.0
	--assert not even? 1.0
	--assert even? 2.0
	--assert not even? -1.0
	--assert even? -2.0
	--assert even? 1.7976931348623157e308
	--assert even? -1.7976931348623157e308

	--test-- "odd?"
	--assert not odd? 0.0
	--assert odd? 1.0
	--assert not odd? 2.0
	--assert odd? -1.0
	--assert not odd? -2.0
	--assert not odd? 1.7976931348623157e308
	--assert not odd? -1.7976931348623157e308
	--assert not odd? 1000000000.0
	--assert not odd? 10000000000.0
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


===start-group=== "square-root"
	--test-- "square-root 4"
		--assert 2.0 = square-root 4
	--test-- "square-root -1"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2431
		--assert "1.#NaN" = mold try [square-root -1]
	--test-- "sqrt"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2433
		--assert all [
			error? e: try [sqrt 4]
			e/id = 'expect-arg
			e/arg3 = integer!
		]
		--assert 2.0 = sqrt 4.0
===end-group===


===start-group=== "log"
	--test-- "log-* -1"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2431
		--assert "1.#NaN" = mold try [log-2  -1]
		--assert "1.#NaN" = mold try [log-e  -1]
		--assert "1.#NaN" = mold try [log-10 -1]
		
	--test-- "log-* 0"
		--assert "-1.#INF" = mold try [log-2  0]
		--assert "-1.#INF" = mold try [log-e  0]
		--assert "-1.#INF" = mold try [log-10 0]

	--test-- "log-2 32"
		--assert 5.0 = log-2 32
	--test-- "log-10 100"
		--assert 2.0 = log-10 100
	--test-- "log-e 123"
		--assert 4.812184355372417 = log-e 123

===end-group===

===start-group=== "random"
	--test-- "random decimal!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/935
	--assert 1.0 <> random 1.0

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
	;@@ https://github.com/Oldes/Rebol-issues/issues/1134
		--assert    0.0   = to decimal! #{0000000000000000}
		--assert    0.0  == to decimal! #{0000000000000000}
		--assert   -0.0   = to decimal! #{8000000000000000}
		--assert   -0.0  == to decimal! #{8000000000000000}
		--assert    0.0 !== to decimal! #{8000000000000000}
		--assert ! -0.0  != to decimal! #{8000000000000000}
		--assert  1.#INF  = to decimal! #{7FF0000000000000}
		--assert -1.#INF  = to decimal! #{FFF0000000000000}
		--assert not number? to decimal! #{7FFFFFFFFFFFFFFF}
		--assert "1.#NaN" = mold to decimal! #{7FFFFFFFFFFFFFFF}
		
	--test-- "issue-267"
	;@@ https://github.com/Oldes/Rebol-issues/issues/267
		--assert 97.0 = try [1.0 * #"a"]

	--test-- "issue-271"
	;@@ https://github.com/Oldes/Rebol-issues/issues/271
		--assert not same? 0.1 0.3 - 0.2

	--test-- "decimal construction"
	;@@  https://github.com/Oldes/Rebol-issues/issues/1034
		--assert 1.0 = #(decimal! 1)
		--assert error? try [load {#[decimal! 1 2]}]

	--test-- "decimal pick"
		b: [1 2]
		--assert 1 = b/1.0
		--assert 1 = b/1.1
		--assert 1 = b/1.6
		--assert 2 = b/2.0
		--assert 2 = b/2.1
		--assert 2 = b/2.6
		s: "12"
		--assert #"1" = s/1.0
		--assert #"1" = s/1.1
		--assert #"1" = s/1.6
		--assert #"2" = s/2.0
		--assert #"2" = s/2.1
		--assert #"2" = s/2.6
		p: 1x2
		--assert 1 = p/1.0
		--assert 1 = p/1.1
		--assert 1 = p/1.6
		--assert 2 = p/2.0
		--assert 2 = p/2.1
		--assert 2 = p/2.6
		t: 1.2.3
		--assert 1 = t/1.0
		--assert 1 = t/1.1
		--assert 1 = t/1.6
		--assert 2 = t/2.0
		--assert 2 = t/2.1
		--assert 2 = t/2.6

	--test-- "equality of numerical values of different types"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1046
		--assert 0 = 0%
		--assert 0 = $0
		--assert 0 = 0.0
		--assert 0% = 0
		--assert 0% = $0
		--assert 0% = 0.0
		--assert $0 = 0
		--assert $0 = 0%
		--assert $0 = 0.0
		--assert 0.0 = 0%
		--assert 0.0 = $0
		--assert 0.0 = 0
	;@@ https://github.com/Oldes/Rebol-issues/issues/952
		--assert not ($0 == 0)
		--assert not ($0.0 == 0)
		--assert not (0.0 == 0)

	--test-- "NaN equality"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1134
	;@@ https://github.com/Oldes/Rebol-issues/issues/2494
		--assert 1.#NaN = 1.#NaN
		--assert not 1.#NaN != 1.#NaN
		--assert not 1.#NaN == 1.#NaN
		--assert 1.#NaN !== 1.#NaN
		--assert same? 1.#NaN 1.#NaN
		--assert equal? 1.#NaN 1.#NaN
		--assert not equiv? 1.#NaN 1.#NaN
		--assert not strict-equal? 1.#NaN 1.#NaN
	--test-- "Equality between NaN and normal decimal"
		--assert not 1.#NaN  = 1.0
		--assert not 1.#NaN == 1.0
		--assert     1.#NaN != 1.0
		--assert     1.#NaN !== 1.0
		--assert not same?  1.#NaN 1.0
		--assert not equal? 1.#NaN 1.0
		--assert not equiv? 1.#NaN 1.0
		--assert not strict-equal? 1.#NaN 1.0

===end-group===

===start-group=== "trigonometric function"

	--test-- "cosine"
		--assert -1.0 = cosine/radians pi
		--assert  0.0 = cosine 90
		--assert  0.0 = cosine/radians pi / 2

	--test-- "sine"
		--assert  0.0 = sine/radians pi
		--assert  1.0 = sine 90

	--test-- "tangent"
		--assert  0.0 = tangent/radians 0
		--assert -1.0 = tangent 135

	--test-- "arcsine"
		--assertf~= -1.5707963267949 arcsine/radians -1 1E-13	
		--assert 90.0 = arcsine 1

	--test-- "arccosine"
		--assertf~= 1.5707963267949 arccosine/radians 0 1E-13
		--assert 90 = arccosine 0

	--test-- "arctangent"
		--assertf~= -0.785398163397448 arctangent/radians -1 1E-13
		--assert 45 = arctangent 1

	;@@ https://github.com/Oldes/Rebol-issues/issues/882
	--test-- "atan2"
		--assertf~=  3.1415926535898  atan2  0.0 -1.0 1E-13
		--assertf~= -1.5707963267949  atan2 -1.0  0.0 1E-13
		--assertf~= -0.78539816339745 atan2 -1.0  1.0 1E-13
		--assertf~= -0.78539816339745 atan2 -1.5  1.5 1E-13

	--test-- "arctangent2"
		--assertf~=  180.0 arctangent2   -1x0    1E-13
		--assertf~=  180.0 arctangent2 -1.0x0.0  1E-13
		--assertf~= -90.0  arctangent2    0x-1   1E-13
		--assertf~= -45.0  arctangent2    1x-1   1E-13
		--assertf~= -45.0  arctangent2  1.5x-1.5 1E-13

	--test-- "arctangent2/radians"
		--assertf~=  3.1415926535898  arctangent2/radians   -1x0    1E-13
		--assertf~=  3.1415926535898  arctangent2/radians -1.0x0.0  1E-13
		--assertf~= -1.5707963267949  arctangent2/radians    0x-1   1E-13
		--assertf~= -0.78539816339745 arctangent2/radians    1x-1   1E-13
		--assertf~= -0.78539816339745 arctangent2/radians  1.5x-1.5 1E-13

===end-group===

===start-group=== "modulo / remainder"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2450
	;@@ https://github.com/Oldes/Rebol-issues/issues/1332
	;@@ https://github.com/Oldes/Rebol-issues/issues/2311
	;@@ https://github.com/metaeducation/ren-c/issues/843
	;@@ https://github.com/red/red/issues/1515
	--test-- "remainder"
		b: copy [] for i -7 7 1 [append b i % 3] b
		--assert b = [-1 0 -2 -1 0 -2 -1 0 1 2 0 1 2 0 1]
		b: copy [] for i -7 7 1 [append b i % -3] b
		--assert b = [-1 0 -2 -1 0 -2 -1 0 1 2 0 1 2 0 1]
		--assert all [error? e: try [7 % 0] e/id = 'zero-divide]
		--assert 1.222090944E+33 % -2147483648.0 = 0.0
	--test-- "mod"
		b: copy [] for i -7 7 1 [append b mod i 3] b
		--assert b = [2 0 1 2 0 1 2 0 1 2 0 1 2 0 1]
		b: copy [] for i -7 7 1 [append b mod i -3] b
		--assert b = [-1 0 -2 -1 0 -2 -1 0 -2 -1 0 -2 -1 0 -2]
		--assert all [error? e: try [mod 7 0] e/id = 'zero-divide]
		--assert 0.25 = mod 562949953421311.25 1
		--assert 5.55111512312578e-17 = mod 0.1 + 0.1 + 0.1 0.3
		--assert -3 == mod -8 -5
		--assert -3.0 == mod -8.0 -5

	--test-- "modulo"
		b: copy [] for i -7 7 1 [append b i // 3] b
		--assert b = [2 0 1 2 0 1 2 0 1 2 0 1 2 0 1]
		b: copy [] for i -7 7 1 [append b i // -3] b
		--assert b = [-1 0 -2 -1 0 -2 -1 0 -2 -1 0 -2 -1 0 -2]
		--assert 0.0 = (1.222090944E+33 // -2147483648.0)
		--assert 0.0 = modulo 562949953421311.25 1
		--assert 0.0 = modulo  0.1 + 0.1 + 0.1 0.3
		--assert $0 == modulo $0.1 + $0.1 + $0.1 $0.3
		--assert $0 == modulo $0.3 $0.1 + $0.1 + $0.1
		--assert  0 == modulo 1 0.1
		--assert   -3 //  2   ==  1
		--assert    3 // -2   == -1
		--assert 1000 // #"a" == 30
		--assert #"a" // 3    == #"^A"
		--assert 10:0 // 3:0  == 1:0
		--assert not (100% // 3% == 1%)
		--assert     (100% // 3%  = 1%)
		--assert not ( 10% // 3% == 1%)
		--assert     ( 10% // 3%  = 1%)
		--assert  10  // 3%   == 0  ; because result A was integer, result is also integer!
		--assert 0.01 = round/to (10.0 // 3%) 0.00001

===end-group===

	
~~~end-file~~~
