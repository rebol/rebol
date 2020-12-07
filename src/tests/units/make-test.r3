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


===start-group=== "make special"
	--test-- "make types from none!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1041
		--assert error? try [make end! none]
		--assert not error? try [make unset! none]
		--assert not error? try [make none! none]
		--assert not error? try [make logic! none]
		--assert error? try [make integer! none]
		--assert error? try [make decimal! none]
		--assert error? try [make percent! none]
		--assert error? try [make money! none]
		--assert error? try [make char! none]
		--assert error? try [make pair! none]
		--assert error? try [make tuple! none]
		--assert error? try [make time! none]
		--assert error? try [make date! none]
		--assert error? try [make binary! none]
		--assert error? try [make string! none]
		--assert error? try [make file! none]
		--assert error? try [make email! none]
		--assert error? try [make ref! none]
		--assert error? try [make url! none]
		--assert error? try [make tag! none]
		--assert error? try [make bitset! none]
		--assert error? try [make image! none]
		--assert error? try [make vector! none]
		--assert error? try [make block! none]
		--assert error? try [make paren! none]
		--assert error? try [make path! none]
		--assert error? try [make set-path! none]
		--assert error? try [make get-path! none]
		--assert error? try [make lit-path! none]
		--assert error? try [make map! none]
		--assert error? try [make datatype! none]
		--assert error? try [make typeset! none]
		--assert error? try [make word! none]
		--assert error? try [make set-word! none]
		--assert error? try [make get-word! none]
		--assert error? try [make lit-word! none]
		--assert error? try [make refinement! none]
		--assert error? try [make issue! none]
		--assert error? try [make native! none]
		--assert error? try [make action! none]
		--assert error? try [make rebcode! none]
		--assert error? try [make command! none]
		--assert error? try [make op! none]
		--assert error? try [make closure! none]
		--assert error? try [make function! none]
		--assert error? try [make frame! none]
		--assert error? try [make object! none]
		--assert error? try [make module! none]
		--assert error? try [make error! none]
		--assert error? try [make task! none]
		--assert error? try [make port! none]
		--assert error? try [make gob! none]
		--assert error? try [make event! none]
		--assert error? try [make handle! none]
		--assert error? try [make struct! none]
		--assert error? try [make library! none]
		--assert error? try [make utype! none]


~~~end-file~~~