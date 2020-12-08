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


===start-group=== "make/to integer"
	--test-- "to integer! logic!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1018
		--assert all [
			error? e: try [to integer! true]
			e/id = 'bad-make-arg
		]
		--assert all [
			error? e: try [to integer! false]
			e/id = 'bad-make-arg
		]
	--test-- "make integer! logic!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1018
		--assert 1 = make integer! true
		--assert 0 = make integer! false
		--assert 1 = make 42 true
		--assert 0 = make 42 false
		--assert error? try [to integer! true]
		--assert error? try [to integer! false]

===end-group===

===start-group=== "make/to decimal!"
	--test-- "to decimal! issue!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1130
		--assert all [
			error? e: try [to decimal! #FF]
			e/id = 'bad-make-arg
		]
	--test-- "make/to decimal! logic!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1018
		--assert 1.0 = make decimal! true
		--assert 0.0 = make decimal! false
		--assert 1.0 = make 0.0 true
		--assert 0.0 = make 0.0 false
		--assert error? try [to decimal! true]
		--assert error? try [to decimal! false]

===end-group===


===start-group=== "make money!"
	--test-- "to money! issue!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1130
		--assert all [
			error? e: try [to money! #FF]
			e/id = 'bad-make-arg
		]
	--test-- "make/to money! logic!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1018
		--assert $1 = make money! true
		--assert $0 = make money! false
		--assert $1 = make $111 true
		--assert $0 = make $111 false
		--assert error? try [to money! true]
		--assert error? try [to money! false]
===end-group===


===start-group=== "make percent!"
	--test-- "to percent! issue!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1130
		--assert all [
			error? e: try [to percent! #FF]
			e/id = 'bad-make-arg
		]
	--test-- "make/to percent! logic!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1018
		--assert 100% = make percent! true
		--assert   0% = make percent! false
		--assert 100% = make 50% true
		--assert   0% = make 50% false
		--assert error? try [to percent! true]
		--assert error? try [to percent! false]
===end-group===

===start-group=== "make logic!"
	--test-- "make/to logic! integer!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2055
		--assert 100% = make percent! true
		--assert   0% = make percent! false
		--assert 100% = make 50% true
		--assert   0% = make 50% false
		--assert error? try [to percent! true]
		--assert error? try [to percent! false]
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


===start-group=== "make/to block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2056
	--test-- "make block! .."
		--assert  error?  try [make block! quote #[unset!]  ] ; unset!
		--assert  error?  try [make block! quote #[none]    ] ; none!
		--assert  error?  try [make block! quote #[true]    ] ; logic!
		--assert  []    = try [make block! quote 4          ] ; integer!
		--assert  []    = try [make block! quote 4.0        ] ; decimal!
		--assert  error?  try [make block! quote 4%         ] ; percent!
		--assert  error?  try [make block! quote $4         ] ; money!
		--assert  error?  try [make block! quote #"a"       ] ; char!
		--assert  error?  try [make block! quote 2x2        ] ; pair!
		--assert  error?  try [make block! quote 1.1.1      ] ; tuple!
		--assert  error?  try [make block! quote 10:00      ] ; time!
		--assert  error?  try [make block! quote 2000-01-01 ] ; date!
		--assert  []    = try [make block! quote #{00}      ] ; binary!
		--assert  [1 2] = try [make block! quote #{312032}  ] ; binary!
		--assert  []    = try [make block! quote ""         ] ; string!
		--assert  [1 2] = try [make block! quote "1 2"      ] ; string!
		--assert  error?  try [make block! quote %file      ] ; file!
		--assert  error?  try [make block! quote u@email    ] ; email!
		--assert  error?  try [make block! quote @ref       ] ; ref!
		--assert  error?  try [make block! quote http://aa  ] ; url!
		--assert  error?  try [make block! quote <tag>      ] ; tag!
		--assert  [1 2] = try [make block! quote [1 2]      ] ; block!
		--assert  [1 2] = try [make block! quote (1 2)      ] ; paren!
		--assert  [a b] = try [make block! quote a/b        ] ; path!
		--assert  [a b] = try [make block! quote a/b:       ] ; set-path!
		--assert  [a b] = try [make block! quote :a/b       ] ; get-path!
		--assert  error?  try [make block! quote /ref       ] ; refinement!
		--assert  error?  try [make block! quote #FF        ] ; issue!
		--assert  error?  try [make block! quote #[bitset! #{FF}]        ] ; bitset!
		--assert  error?  try [make block! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert  [0 0] = try [make block! quote #[ui32! 2 [0 0]]        ] ; vector!
		--assert [a: 1] = try [make block! quote #[object! [a: 1]]       ] ; object!
		--assert  error?  try [make block! #[typeset! [#[datatype! integer!] #[datatype! percent!]]] ] ; typeset!

	--test-- "to block! .."
		--assert [#[unset!]]  = try [to block! quote #[unset!]     ] ; unset!
		--assert [#[none]]    = try [to block! quote #[none]       ] ; none!
		--assert [#[true]]    = try [to block! quote #[true]       ] ; logic!
		--assert [4]          = try [to block! quote 4             ] ; integer!
		--assert [4.0]        = try [to block! quote 4.0           ] ; decimal!
		--assert [4%]         = try [to block! quote 4%            ] ; percent!
		--assert [$4]         = try [to block! quote $4            ] ; money!
		--assert [#"a"]       = try [to block! quote #"a"          ] ; char!
		--assert [2x2]        = try [to block! quote 2x2           ] ; pair!
		--assert [1.1.1]      = try [to block! quote 1.1.1         ] ; tuple!
		--assert [10:00]      = try [to block! quote 10:00         ] ; time!
		--assert [2000-01-01] = try [to block! quote 2000-01-01    ] ; date!
		--assert [#{00}]      = try [to block! quote #{00}         ] ; binary!
		--assert [#{312032}]  = try [to block! quote #{312032}     ] ; binary!
		--assert [""]         = try [to block! quote ""            ] ; string!
		--assert ["1 2"]      = try [to block! quote "1 2"         ] ; string!
		--assert [%file]      = try [to block! quote %file         ] ; file!
		--assert [u@email]    = try [to block! quote u@email       ] ; email!
		--assert [@ref]       = try [to block! quote @ref          ] ; ref!
		--assert [http://aa]  = try [to block! quote http://aa     ] ; url!
		--assert [<tag>]      = try [to block! quote <tag>         ] ; tag!
		--assert [1 2]        = try [to block! quote [1 2]         ] ; block!
		--assert [1 2]        = try [to block! quote (1 2)         ] ; paren!
		--assert [a b]        = try [to block! quote a/b           ] ; path!
		--assert [a b]        = try [to block! quote a/b:          ] ; set-path!
		--assert [a b]        = try [to block! quote :a/b          ] ; get-path!
		--assert [/ref]       = try [to block! quote /ref          ] ; refinement!
		--assert [#FF]        = try [to block! quote #FF           ] ; issue!
		--assert [0 0]        = try [to block! quote #[vector! integer! 32 2 [0 0]]        ] ; vector!
		--assert [a: 1]       = try [to block! quote #[object! [a: 1]]                     ] ; object!
		--assert [#[bitset! #{FF}]]         = try [to block! quote #[bitset! #{FF}]        ] ; bitset!
		--assert [#[image! 1x1 #{FFFFFF}]]  = try [to block! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert [#[datatype! integer!] #[datatype! percent!]]  = try [to block! #[typeset! [#[datatype! integer!] #[datatype! percent!]]] ] ; typeset!

===end-group===


===start-group=== "make special"
	--test-- "make types from none!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1018
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