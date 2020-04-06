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
		


===end-group===

~~~end-file~~~
