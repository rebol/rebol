Rebol [
	Title:   "Rebol3 percent test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %decimal-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]


~~~start-file~~~ "percent"

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
