Rebol [
	Title:   "Rebol3 integer test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %integer-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "integer"

===start-group=== "max/min"
	--test-- "max"
		--assert  3 == max  3 1
		--assert  3 == max  3 1.0
		--assert  3 == max  3 $1
	--test-- "min"
		--assert -3 == min -3 2
		--assert -3 == min -3 1.0
		--assert -3 == min -3 $1
===end-group===

===start-group=== "shift"
	--test-- "shift native"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1310
		m: to-integer #{8000 0000 0000 0000}
		--assert  0 = shift 0 0
		--assert  0 = shift 0 1
		--assert  0 = shift 0 63
		--assert  0 = shift 0 -1
		--assert  1 = shift 1 0
		--assert  2 = shift 1 1
		--assert  error? try [shift 1 63]
		--assert  error? try [shift 1 64]
		--assert  0 = shift 1 -1
		--assert -1 = shift m -63
		--assert -1 = shift m -64
		--assert  m = shift/logical 1 63 ; same as above
		--assert  1 = shift/logical m -63
		--assert  0 = shift/logical m -64

	--test-- "shift-op-left"
		--assert 2 << 3 = 16
		--assert 1 <<  0 = 1
		--assert 1 <<  1 = 2
		--assert 1 << 64 = 1
		--assert 1 << 65 = 2
		--assert #{8000000000000000} = to-binary ((to-integer #{4000000000000000}) << 1)
	--test-- "shift-op-right"
		--assert 1024 >> 1 = 512
		--assert 2 >> 0 = 2
		--assert 2 >> 1 = 1
		--assert 2 >> 2 = 0
		--assert 2 >> 3 = 0

	--test-- "shift overflow"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2067
		--assert -9223372036854775808 = shift/logical 1 63
		--assert error? try [shift 1 63] ; overflow
		--assert 0 = shift 1 -100

===end-group===


===start-group=== "gcd/lcm"
	--test-- "gcd"
		--assert  6 = gcd 54 24
		--assert  6 = gcd 24 54
		--assert  3 = gcd 0 3
		--assert  3 = gcd 3 0
		--assert  3 = gcd 21 -48
	--test-- "lcm"
		--assert 36 = lcm 12 18
		--assert 36 = lcm 18 12
		--assert  0 = lcm 0 1
		--assert  0 = lcm 1 0
		--assert  0 = lcm 0 0

===end-group===


===start-group=== "multiply"
	--test-- "0 * 1"
		i: 0
		j: 1
		--assert strict-equal? 0 0 * 1
		--assert strict-equal? 0 multiply 0 1
		--assert strict-equal? 0 i * j
		--assert strict-equal? 0 multiply i j

	--test-- "0 * -1"
		i: 0
		j: -1
		--assert strict-equal? 0 0 * -1
		--assert strict-equal? 0 multiply 0 -1
		--assert strict-equal? 0 i * j
		--assert strict-equal? 0 multiply i j

	--test-- "0 * -2147483648"
		i: 0
		j: -2147483648
		--assert strict-equal? 0 0 * -2147483648
		--assert strict-equal? 0 multiply 0 -2147483648
		--assert strict-equal? 0 i * j
		--assert strict-equal? 0 multiply i j

	--test-- "0 * 2147483647"
		i: 0
		j: 2147483647
		--assert strict-equal? 0 0 * 2147483647
		--assert strict-equal? 0 multiply 0 2147483647
		--assert strict-equal? 0 i * j
		--assert strict-equal? 0 multiply i j

	--test-- "0 * 65536"
		i: 0
		j: 65536
		--assert strict-equal? 0 0 * 65536
		--assert strict-equal? 0 multiply 0 65536
		--assert strict-equal? 0 i * j
		--assert strict-equal? 0 multiply i j

	--test-- "0 * 256"
		i: 0
		j: 256
		--assert strict-equal? 0 0 * 256
		--assert strict-equal? 0 multiply 0 256
		--assert strict-equal? 0 i * j
		--assert strict-equal? 0 multiply i j

	--test-- "0 * 16777216"
		i: 0
		j: 16777216
		--assert strict-equal? 0 0 * 16777216
		--assert strict-equal? 0 multiply 0 16777216
		--assert strict-equal? 0 i * j
		--assert strict-equal? 0 multiply i j

	--test-- "1 * -1"
		i: 1
		j: -1
		--assert strict-equal? -1 1 * -1
		--assert strict-equal? -1 multiply 1 -1
		--assert strict-equal? -1 i * j
		--assert strict-equal? -1 multiply i j

	--test-- "1 * -2147483648"
		i: 1
		j: -2147483648
		--assert strict-equal? -2147483648 1 * -2147483648
		--assert strict-equal? -2147483648 multiply 1 -2147483648
		--assert strict-equal? -2147483648 i * j
		--assert strict-equal? -2147483648 multiply i j

	--test-- "1 * 2147483647"
		i: 1
		j: 2147483647
		--assert strict-equal? 2147483647 1 * 2147483647
		--assert strict-equal? 2147483647 multiply 1 2147483647
		--assert strict-equal? 2147483647 i * j
		--assert strict-equal? 2147483647 multiply i j

	--test-- "1 * 65536"
		i: 1
		j: 65536
		--assert strict-equal? 65536 1 * 65536
		--assert strict-equal? 65536 multiply 1 65536
		--assert strict-equal? 65536 i * j
		--assert strict-equal? 65536 multiply i j

	--test-- "1 * 256"
		i: 1
		j: 256
		--assert strict-equal? 256 1 * 256
		--assert strict-equal? 256 multiply 1 256
		--assert strict-equal? 256 i * j
		--assert strict-equal? 256 multiply i j

	--test-- "1 * 16777216"
		i: 1
		j: 16777216
		--assert strict-equal? 16777216 1 * 16777216
		--assert strict-equal? 16777216 multiply 1 16777216
		--assert strict-equal? 16777216 i * j
		--assert strict-equal? 16777216 multiply i j

	--test-- "-1 * -2147483648"
		i: -1
		j: -2147483648
		--assert strict-equal? 2147483648 -1 * -2147483648
		--assert strict-equal? 2147483648 multiply -1 -2147483648
		--assert strict-equal? 2147483648 i * j
		--assert strict-equal? 2147483648 multiply i j

	--test-- "-1 * 2147483647"strict-equal? 
		i: -1
		j: 2147483647
		--assert strict-equal? -2147483647 -1 * 2147483647
		--assert strict-equal? -2147483647 multiply -1 2147483647
		--assert strict-equal? -2147483647 i * j
		--assert strict-equal? -2147483647 multiply i j

	--test-- "-1 * 65536"
		i: -1
		j: 65536
		--assert strict-equal? -65536 -1 * 65536
		--assert strict-equal? -65536 multiply -1 65536
		--assert strict-equal? -65536 i * j
		--assert strict-equal? -65536 multiply i j

	--test-- "-1 * 256"
		i: -1
		j: 256
		--assert strict-equal? -256 -1 * 256
		--assert strict-equal? -256 multiply -1 256
		--assert strict-equal? -256 i * j
		--assert strict-equal? -256 multiply i j

	--test-- "-1 * 16777216"
		i: -1
		j: 16777216
		--assert strict-equal? -16777216 -1 * 16777216
		--assert strict-equal? -16777216 multiply -1 16777216
		--assert strict-equal? -16777216 i * j
		--assert strict-equal? -16777216 multiply i j

	--test-- "-2147483648 * 2147483647"
		i: -2147483648
		j: 2147483647
		--assert strict-equal? -4611686016279904256 -2147483648 * 2147483647
		--assert strict-equal? -4611686016279904256 multiply -2147483648 2147483647
		--assert strict-equal? -4611686016279904256 i * j
		--assert strict-equal? -4611686016279904256 multiply i j

	--test-- "-2147483648 * 65536"
		i: -2147483648
		j: 65536
		--assert strict-equal? -140737488355328 -2147483648 * 65536
		--assert strict-equal? -140737488355328 multiply -2147483648 65536
		--assert strict-equal? -140737488355328 i * j
		--assert strict-equal? -140737488355328 multiply i j

	--test-- "-2147483648 * 256"
		i: -2147483648
		j: 256
		--assert strict-equal? -549755813888 -2147483648 * 256
		--assert strict-equal? -549755813888 multiply -2147483648 256
		--assert strict-equal? -549755813888 i * j
		--assert strict-equal? -549755813888 multiply i j

	--test-- "-2147483648 * 16777216"
		i: -2147483648
		j: 16777216
		--assert strict-equal? -36028797018963968 -2147483648 * 16777216
		--assert strict-equal? -36028797018963968 multiply -2147483648 16777216
		--assert strict-equal? -36028797018963968 i * j
		--assert strict-equal? -36028797018963968 multiply i j

	--test-- "2147483647 * 65536"
		i: 2147483647
		j: 65536
		--assert strict-equal? 140737488289792 2147483647 * 65536
		--assert strict-equal? 140737488289792 multiply 2147483647 65536
		--assert strict-equal? 140737488289792 i * j
		--assert strict-equal? 140737488289792 multiply i j

	--test-- "2147483647 * 256"
		i: 2147483647
		j: 256
		--assert strict-equal? 549755813632 2147483647 * 256
		--assert strict-equal? 549755813632 multiply 2147483647 256
		--assert strict-equal? 549755813632 i * j
		--assert strict-equal? 549755813632 multiply i j

	--test-- "2147483647 * 16777216"
		i: 2147483647
		j: 16777216
		--assert strict-equal? 36028797002186752 2147483647 * 16777216
		--assert strict-equal? 36028797002186752 multiply 2147483647 16777216
		--assert strict-equal? 36028797002186752 i * j
		--assert strict-equal? 36028797002186752 multiply i j

	--test-- "65536 * 256"
		i: 65536
		j: 256
		--assert strict-equal? 16777216 65536 * 256
		--assert strict-equal? 16777216 multiply 65536 256
		--assert strict-equal? 16777216 i * j
		--assert strict-equal? 16777216 multiply i j

	--test-- "65536 * 16777216"
		i: 65536
		j: 16777216
		--assert strict-equal? 1099511627776 65536 * 16777216
		--assert strict-equal? 1099511627776 multiply 65536 16777216
		--assert strict-equal? 1099511627776 i * j
		--assert strict-equal? 1099511627776 multiply i j

	--test-- "256 * 16777216"
		i: 256
		j: 16777216
		--assert strict-equal? 4294967296 256 * 16777216
		--assert strict-equal? 4294967296 multiply 256 16777216
		--assert strict-equal? 4294967296 i * j
		--assert strict-equal? 4294967296 multiply i j

===end-group===


===start-group=== "integer to binary conversion"
	--test-- "issue-550"
	;@@ https://github.com/Oldes/Rebol-issues/issues/550
		--assert #{0000000000000001} = to binary! 1
		--assert #{00000000000000FF} = to binary! 255
		--assert #{0000000000000100} = to binary! 256
		--assert #{FFFFFFFFFFFFFFFF} = to binary! -1
		--assert #{FFFFFFFFFFFFFFFE} = to binary! -2
===end-group===


===start-group=== "++ & --"
	--test-- "++ and -- integer!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/554
		a: 1
		--assert 1 = ++ a
		--assert 2 = -- a
		--assert 1 = a

===end-group===


===start-group=== "integer issues"
	--test-- "issue-502"
	;@@ https://github.com/Oldes/Rebol-issues/issues/502
		--assert error? e: try [set 1 1]
		--assert e/id = 'expect-arg

	--test-- "issue-126"
	;@@ https://github.com/Oldes/Rebol-issues/issues/126
		--assert error? try [to integer! "11111111111111111111111"]

	--test-- "issue-260"
	;@@ https://github.com/Oldes/Rebol-issues/issues/260
		--assert not same? 97 #"a"


	--test-- "issue-288"
	;@@ https://github.com/Oldes/Rebol-issues/issues/288
		--assert all [
			error? e: try [1 + "2"]
			e/id = 'expect-arg
		]

	--test-- "issue-569"
	;@@ https://github.com/Oldes/Rebol-issues/issues/569
		--assert not strict-equal? 1 $1
		
===end-group===


===start-group=== "RANDOM integer!"
	--test-- "Random range"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1053
		two-to-62: to integer! 2 ** 62
		a: (to integer! #{7ffffffffffffffe}) / 3
		b: two-to-62 - a
		count: 10000
		random/seed 0
		f: 0
		loop count [if (random a) <= b [f: f + 1]]
		--assert (b / a) = round/to (f / count) 0.1
===end-group===

~~~end-file~~~
