Rebol [
	Title:   "Rebol3 percent test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %percent-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]


~~~start-file~~~ "percent"

===start-group=== "to conversion"
	;@@ https://github.com/Oldes/Rebol-issues/issues/137
	--test-- "to percent!"
		--assert (to percent! 1   ) == 100%
		--assert (to percent! 1.0 ) == 100%
	--test-- "to decimal!"
		--assert (to decimal! 100%) == 1.0
		--assert (to decimal! 1%  ) == 0.01
	--test-- "to integer!"
		--assert (to integer! 100%) == 1
		--assert (to integer! 1%  ) == 0

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
===end-group===

	
~~~end-file~~~
