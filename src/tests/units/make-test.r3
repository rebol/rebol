Rebol [
	Title:   "Rebol make test script"
	Author:  "Oldes"
	File: 	 %make-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "MAKE"

===start-group=== "make char!"
	--test-- "make char! string!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1030
		--assert all [
			error? err: try [make char! ""]
			err/id = 'bad-make-arg
		]
	--test-- "make char! binary!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1045
		--assert #"1" = make char! #{3132}
		--assert #"š" = make char! to-binary "ša"
		--assert #"^@" = make char! #{00}
		--assert all [
			error? err: try [make char! #{A3}]
			err/id = 'bad-make-arg
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/1043
		--assert all [
			error? err: try [make char! #{}]
			err/id = 'bad-make-arg
		]
===end-group===

~~~end-file~~~