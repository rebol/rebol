Rebol [
	Title:   "Rebol3 time test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %time-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]



~~~start-file~~~ "TIME"

===start-group=== "time"
	--test-- "round time"
	;@@ https://github.com/Oldes/Rebol-issues/issues/162
		--assert 1:30 = round/to 1:32:0 0:10
		--assert 2:00 = round/to 1:32:0 1:00
		--assert 0:00:01 = round/to 0:0:1.4 0:0:1
		--assert 12:35:23 = round/to 12:34:56 0:1:1
		--red-- --assert 12:35:23 = round/to 12:34:56 to integer! 0:1:1
		--red-- --assert 12:35:23 = round/to 12:34:56 to decimal! 0:1:1
		;Red also supports this (don't know why):
		; --assert 12:34:56.1 = round/to 12:34:56 0.3
		; --assert 12:34:56.1 = round/to 12:34:56 30%

	--test-- "compare with 0"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2463
		--assert 0 < 0:0:0.1
		--assert 0:0:0.1 > 0
		--assert 0.0 < 0:0:0.1
		--assert 0:0:0.1 > 0.0
		--assert 0% < 0:0:0.1
		--assert 0:0:0.1 > 0%

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

	--test-- "issue-149"
	;@@ https://github.com/Oldes/Rebol-issues/issues/149
		--assert same?  5:01:40  100 + 5:00
		--assert same? -4:58:20  100 - 5:00
		--assert same? -4:58:20  100 + -5:00

	--test-- "issue-150"
	;@@ https://github.com/Oldes/Rebol-issues/issues/150
		--assert error? try [0:0:0 /  0] ;- no crash
		--assert error? try [0:0:0 // 0] ;- no crash

	--test-- "issue-263"
	;@@ https://github.com/Oldes/Rebol-issues/issues/263
		--assert time? t: try [load "596523:00"]
		--assert t = load "596522:60"

	--test-- "issue-277"
	;@@ https://github.com/Oldes/Rebol-issues/issues/277
		--assert error? e: try [-1.0 + -596523:14:07.999999999]
		--assert e/id = 'type-limit

	--test-- "issue-289"
	;@@ https://github.com/Oldes/Rebol-issues/issues/289
		t: 225:00 --assert 224:59:59 = (t - 1)
		t: 226:00 --assert 225:59:59 = (t - 1)

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

	--test-- "issue-1391"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1391
		--assert 5:0 = (50% * 10:0:0)
		--assert error? try [50% + 10:0]
		--assert error? try [50% - 10:0]
		--assert error? try [50% / 10:0]

	--test-- "issue-972"
	;@@ https://github.com/Oldes/Rebol-issues/issues/972
		--assert 0:00:00.123456789 = 0:00:00.1234567892
		--assert 0:00:00.12345679  = 0:00:00.1234567895
		--assert 0:00:00.123456789 = 0:00:00.123456789456

	--test-- "divide by tuple"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1196
		--assert all [error? e: try [0:0:01 / 1.1.1] e/id = 'not-related]
		--assert all [error? e: try [1.1.1 / 0:0:01] e/id = 'not-related]

===end-group===

===start-group=== "random"
	--test-- "random time!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/911
		t: random 0:0:0.10
		--assert all [t > 0:0 t < 0:0:0.10]
		t: random 1:0:0
		--assert all [t > 0:0 t < 1:0:0]


===end-group===



;===start-group=== "time protect"
;	;@@ https://github.com/Oldes/Rebol-issues/issues/2416
;	--test-- "time protect 1"
;		t: now/time protect 't
;		--assert error? e: try [t/hour: 0]
;		--assert e/id = 'locked-word
;	--test-- "time protect 2"
;		protect/words o: object [t: now/time]
;		--assert error? e: try [o/t/hour: 0]
;		--assert e/id = 'locked-word
;	--test-- "time protect 3"
;		o: object [t: 1 protect/words o: object [t: now/time]]
;		--assert protected? 'o/o/t
;		--assert protected? 'o/o/t/hour        ;@@ <--- fails!
;		--assert error? e: try [o/o/t/hour: 0] ;@@ <--- fails!
;===end-group===

~~~end-file~~~
