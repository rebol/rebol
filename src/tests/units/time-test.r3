Rebol [
	Title:   "Rebol3 time test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %time-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]



~~~start-file~~~ "TIME"

===start-group=== "time"

	--test-- "issue-90"
	;@@ https://github.com/Oldes/Rebol-issues/issues/90
		--assert -23:00 = (1:0:0 - 24:00)
		--assert 0:0 = subtract 1:0:0  1:0:0
		--assert -2:01:02 = (3:0:0 - 5:1:2)

	--test-- "issue-108"
	;@@ https://github.com/Oldes/Rebol-issues/issues/108
		--assert "0:00:00.0001"   = mold 0:0:0.0001
		--assert "0:00:00.001"    = mold 0:0:0.0001 * 10
		--assert "0:00:00.01"     = mold 0:0:0.0001 * 100
		--assert "0:00:00.1"      = mold 0:0:0.0001 * 1000
		--assert "0:00:01"        = mold 0:0:0.0001 * 10000
		--assert "0:00:00.01"     = mold 0:0:0.1 / 10
		--assert "0:00:00.001"    = mold 0:0:0.1 / 100
		--assert "0:00:00.0001"   = mold 0:0:0.1 / 1000
		--assert "0:00:00.00001"  = mold 0:0:0.1 / 10000
		--assert "0:00:00.000001" = mold 0:0:0.1 / 100000
		--assert "0:00:01.0001"   = mold 0:0:1.0001
		--assert "0:00:01.001"    = mold 0:0:1.001
		--assert "0:00:01.01"     = mold 0:0:1.01
		--assert "0:00:01.1"      = mold 0:0:1.1

	--test-- "issue-150"
	;@@ https://github.com/Oldes/Rebol-issues/issues/150
		--assert error? try [0:0:0 /  0] ;- no crash
		--assert error? try [0:0:0 // 0] ;- no crash

	--test-- "issue-263"
	;@@ https://github.com/Oldes/Rebol-issues/issues/263
		--assert time? t: try [load "596523:00"]
		--assert t = load "596522:60"

	--test-- "issue-1032"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1032
		--assert -3723.0 = to-decimal make time! [-1 2 3]
		--assert -3723.4 = to-decimal make time! [-1 2 3.4]

	--test-- "issue-1033"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1033
		--assert "-1:02:03.45" = mold make time! [-1 2 3.45]
		
	--test-- "issue-958"
	;@@ https://github.com/Oldes/Rebol-issues/issues/958
		--assert error? try [make time! (2 ** 32) - 1]
		--assert error? try [make time! (2 ** 32) - 0.99]
		--assert error? try [make time! (2 ** 32) - 1.01]


===end-group===

~~~end-file~~~
