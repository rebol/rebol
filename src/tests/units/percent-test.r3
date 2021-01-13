Rebol [
	Title:   "Rebol3 percent test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %percent-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]


~~~start-file~~~ "percent"
===start-group=== "make percent!"
	--test-- "make percent from integer"
		--assert 100% = make percent! 1
	--test-- "make percent from decimal"
		--assert 50% = make percent! 0.5
	--test-- "make percent from block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1504
		--assert 1e18% = make percent! [1 18]

===start-group=== "to conversion"
	;@@ https://github.com/Oldes/Rebol-issues/issues/137
	--test-- "to percent!"
		--assert (to percent! 1   ) = 100%
		--assert (to percent! 1.0 ) = 100%
	--test-- "to decimal!"
		--assert (to decimal! 100%) = 1.0
		--assert (to decimal! 1%  ) = 0.01
	--test-- "to integer!"
		--assert (to integer! 100%) = 1
		--assert (to integer! 1%  ) = 0

===end-group===

===start-group=== "form/mold"
	--test-- "form"
		--assert    "0%" = form   0%
		--assert    "1%" = form   1%
		--assert   "10%" = form  10%
		--assert  "0.1%" = form 0.1%
		--assert  "100%" = form 100%

		--assert   "-0%" = form   -0%
		--assert   "-1%" = form   -1%
		--assert  "-10%" = form  -10%
		--assert "-0.1%" = form -0.1%
		--assert "-100%" = form -100%

	--test-- "mold"
		--assert    "0%" = mold   0%
		--assert    "1%" = mold   1%
		--assert   "10%" = mold  10%
		--assert  "0.1%" = mold 0.1%
		--assert  "100%" = mold 100%

		--assert   "-0%" = mold   -0%
		--assert   "-1%" = mold   -1%
		--assert  "-10%" = mold  -10%
		--assert "-0.1%" = mold -0.1%
		--assert "-100%" = mold -100%

	--test-- "mold/all"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1633
		--assert same? 9.9999999999999926e154% load mold/all 9.9999999999999926e154%
		
===end-group===

===start-group=== "percent issues"
	--test-- "issue-227"
	;@@ https://github.com/Oldes/Rebol-issues/issues/227
		--assert number? 1%

	--test-- "issue-262"
	;@@ https://github.com/Oldes/Rebol-issues/issues/262
		--assert 100% = try [load {1E+2%}]

	--test-- "large percent"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1475
		x: 30000000000000000000000000000000000%
		--assert "3e34%" = mold x
		--assert "3.0000000000000003e34%" = mold/all x
		--assert x = load mold x
		--assert x = load mold/all x

===end-group===


	
~~~end-file~~~
