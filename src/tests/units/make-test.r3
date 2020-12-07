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
		;@@ https://github.com/Oldes/Rebol-issues/issues/1031
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
	--test-- "to char! 30"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1414
		--assert #"^(1E)" = to char! 30
		--assert #"^^" = to char! 94
		--assert 30 = to integer! #"^(1E)"
		--assert 94 = to integer! #"^^"

	--test-- "to char! issue!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1130
		--assert all [
			error? e: try [to char! #FF]
			e/id = 'bad-make-arg
		]

===end-group===


===start-group=== "make decimal!"
	--test-- "to decimal! issue!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1130
		--assert all [
			error? e: try [to decimal! #FF]
			e/id = 'bad-make-arg
		]
===end-group===


===start-group=== "make money!"
	--test-- "to money! issue!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1130
		--assert all [
			error? e: try [to money! #FF]
			e/id = 'bad-make-arg
		]
===end-group===


===start-group=== "make issue!"
	--test-- "make issue! char!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1106
	;	--assert #1 = make issue! #"1" <--- does not work!
		--assert #à = make issue! #"à"
===end-group===


===start-group=== "make binary!"
	--test-- "to binary! char!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1106
		--assert #{31}   = make binary! #"1"
		--assert #{C3A0} = make binary! #"à"
===end-group===


~~~end-file~~~