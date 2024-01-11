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
		
	--test-- "to char! string"
		;@@ https://github.com/Oldes/Rebol-issues/issues/465
		--assert #"x" = try [to char! "x" ]
		--assert #"x" = try [to char! "xy"]

===end-group===


===start-group=== "make/to date"
	--test-- "make/to date! integer!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2456
		--assert 20-Sep-2021/12:46:41 = make date! 1632142001
		--assert 20-Sep-2021/12:46:41 =   to date! 1632142001
		--assert 20-Sep-2021/12:46:42 = make date! 1632142002
		--assert 20-Sep-2021/12:46:42 =   to date! 1632142002
		--assert 20-Sep-2021/10:58:32 = make date! 1632135512
		--assert 20-Sep-2021/10:58:32 =   to date! 1632135512
	--test-- "make/to date! decimal!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2551
		--assert 17-Nov-1858/00:00:00 = make date! 0.0
		--assert 17-Nov-1858/00:00:00 =   to date! 0.0
		--assert 01-Jan-1900/00:00:00 = make date! 15020.0
		--assert 01-Jan-1900/00:00:00 =   to date! 15020.0
		--assert 02-May-2003/12:00:00 = make date! 52761.5
		--assert 02-May-2003/12:00:00 =   to date! 52761.5
		--assert 10-Jun-2023/01:30:00 = make date! 60105.0625
		--assert 10-Jun-2023/01:30:00 =   to date! 60105.0625
		--assert 10-Jun-2023/19:30:00 = make date! 60105.8125
		--assert 10-Jun-2023/19:30:00 =   to date! 60105.8125
		--assert 01-Jan-2023/12:00:00 = make date! 59945.5
		--assert 01-Jan-2023/12:00:00 =   to date! 59945.5
		--assert 01-Jan-2023/19:30:00 = make date! 59945.8125
		--assert 01-Jan-2023/19:30:00 =   to date! 59945.8125
		--assert 01-Jan-2023/01:30:00 = make date! 59945.0625
		--assert 01-Jan-2023/01:30:00 =   to date! 59945.0625
		--assert 31-Aug-2132/00:00:00 = make date! 99999.0
		--assert 31-Aug-2132/00:00:00 =   to date! 99999.0
		--assert 01-Sep-2132/00:00:00 = make date! 100000.0
		--assert 01-Sep-2132/00:00:00 =   to date! 100000.0
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

	--test-- "make/to integer! date!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2456
		--assert 1632142001 = make integer! 20-Sep-2021/12:46:41.3
		--assert 1632142001 =   to integer! 20-Sep-2021/12:46:41.3
		--assert 1632142002 = make integer! 20-Sep-2021/12:46:41.7
		--assert 1632142002 =   to integer! 20-Sep-2021/12:46:41.7
		--assert 1632135512 = make integer! 20-Sep-2021/12:58:32+2:00
		--assert 1632135512 =   to integer! 20-Sep-2021/12:58:32+2:00
		;@@ https://github.com/Oldes/Rebol-issues/issues/2550
		--assert 1685750400 == to integer! 3-Jun-2023
		--assert 1685750400 == to integer! 3-Jun-2023/0:00
		--assert 1685750400 == make integer! 3-Jun-2023
		--assert 1685750400 == make integer! 3-Jun-2023/0:00

	--test-- "make/to integer! string!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2164
		--assert 9223372036854775807 = to integer! "9223372036854775807"
		--assert 9223372036854775807 = to integer! "9'223'372'036'854'775'807"
		--assert all [error? e: try [to integer! "9'223'372'036'854'775'808"] e/id = 'bad-make-arg]
	 	;@@ https://github.com/Oldes/Rebol-issues/issues/2504
	 	--assert 0 = to integer! "0"
	 	--assert 0 = to integer! "00"
	 	--assert 0 = to integer! "00000000000"
	 	--assert 0 = to integer! "-0"
	 	--assert 0 = to integer! "-00"
	 	--assert 0 = to integer! "-00000000000"
	 	--assert 0 = to integer! "+0"
	 	--assert 0 = to integer! "+00"
	 	--assert 0 = to integer! "+00000000000"
	 	--assert 0 = to integer! "000'000'000"

	 --test-- "make/to integer! string! (known issue)"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2099
	 	--assert 302961000000 = to integer! "3.02961E+11"

	--test-- "make/to integer! 1.#NaN and 1.#INF"
		--assert error? try [to integer! 1.#NaN]
		--assert error? try [to integer! 1.#INF]
		--assert error? try [to integer! -1.#INF]
		--assert error? try [make integer! 1.#NaN]
		--assert error? try [make integer! 1.#INF]
		--assert error? try [make integer! -1.#INF]

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
	--test-- "make decimal from string"
		;@@ https://github.com/Oldes/Rebol-issues/issues/956
		ch: copy ""
		repeat x 255 [unless error? try [to-decimal join "1" to-char x] [append ch to-char x]]
		--assert ch = "^- ',.0123456789Ee"

	
	--test-- "make/to decimal! date!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2551
		--assert 0.0            = make decimal! 17-Nov-1858/00:00:00
		--assert 0.0            =   to decimal! 17-Nov-1858/00:00:00
		--assert 15020.0        = make decimal! 01-Jan-1900/00:00:00
		--assert 15020.0        =   to decimal! 01-Jan-1900/00:00:00
		--assert 52761.5        = make decimal! 02-May-2003/12:00:00
		--assert 52761.5        =   to decimal! 02-May-2003/12:00:00
		--assert 60105.0625     = make decimal! 10-Jun-2023/01:30:00
		--assert 60105.0625     =   to decimal! 10-Jun-2023/01:30:00
		--assert 60105.8125     = make decimal! 10-Jun-2023/19:30:00
		--assert 60105.8125     =   to decimal! 10-Jun-2023/19:30:00
		--assert 59945.5        = make decimal! 01-Jan-2023/12:00:00
		--assert 59945.5        =   to decimal! 01-Jan-2023/12:00:00
		--assert 59945.8125     = make decimal! 01-Jan-2023/19:30:00
		--assert 59945.8125     =   to decimal! 01-Jan-2023/19:30:00
		--assert 59945.0625     = make decimal! 01-Jan-2023/01:30:00
		--assert 59945.0625     =   to decimal! 01-Jan-2023/01:30:00
		--assert 99999.0        = make decimal! 31-Aug-2132/00:00:00
		--assert 99999.0        =   to decimal! 31-Aug-2132/00:00:00
		--assert 100000.0       = make decimal! 01-Sep-2132/00:00:00
		--assert 100000.0       =   to decimal! 01-Sep-2132/00:00:00
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
	--test-- "make money from string"
		;@@ https://github.com/Oldes/Rebol-issues/issues/957
		ch: copy ""
		repeat x 255 [unless error? try [to-money join "1" to-char x] [append ch to-char x]]
		--assert ch = "^- ',.0123456789Ee"
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
	--test-- "make percent from string"
		;@@ https://github.com/Oldes/Rebol-issues/issues/956
		ch: copy ""
		repeat x 255 [unless error? try [to-percent join "1" to-char x] [append ch to-char x]]
		--assert ch = "^- %',.0123456789Ee"
===end-group===


===start-group=== "make/to pair!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/925
	--test-- "make pair! .."
		--assert 1x1 = make pair! "1x1"
		--assert error? try [make pair! quote #[unset]   ] ; unset!
		--assert error? try [make pair! quote #[none]    ] ; none!
		--assert error? try [make pair! quote #[true]    ] ; logic!
		--assert 1x1  = try [make pair! quote 1          ] ; integer!
		--assert 0x0  = try [make pair! quote 0          ] ; integer!
		--assert 4x4  = try [make pair! quote 4          ] ; integer!
		--assert 4x4  = try [make pair! quote 4.0        ] ; decimal!
		--assert error? try [make pair! quote 4.0%       ] ; percent!
		--assert error? try [make pair! quote $4         ] ; money!
		--assert error? try [make pair! quote #"a"       ] ; char!
		--assert 2x2  = try [make pair! quote 2x2        ] ; pair!
		--assert error? try [make pair! quote 1.1.1      ] ; tuple!
		--assert error? try [make pair! quote 10:00      ] ; time!
		--assert error? try [make pair! quote 2000-01-01 ] ; date!
		--assert error? try [make pair! quote #{00}      ] ; binary!
		--assert error? try [make pair! quote #{312032}  ] ; binary!
		--assert error? try [make pair! quote #{317832}  ] ; binary!
		--assert error? try [make pair! quote ""         ] ; string!
		--assert error? try [make pair! quote "1 2"      ] ; string!
		--assert error? try [make pair! quote %file      ] ; file!
		--assert error? try [make pair! quote u@email    ] ; email!
		--assert error? try [make pair! quote @ref       ] ; ref!
		--assert error? try [make pair! quote http://aa  ] ; url!
		--assert error? try [make pair! quote <tag>      ] ; tag!
		--assert 1x2  = try [make pair! quote [1 2]      ] ; block!
		--assert error? try [make pair! quote (1 2)      ] ; paren!
		--assert error? try [make pair! quote a/b        ] ; path!
		--assert error? try [make pair! quote a/b:       ] ; set-path!
		--assert error? try [make pair! quote :a/b       ] ; get-path!
		--assert error? try [make pair! quote /ref       ] ; refinement!
		--assert error? try [make pair! quote #FF        ] ; issue!
		--assert error? try [make pair! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [make pair! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [make pair! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert error? try [make pair! quote #[object! [a: 1]] ] ; object!
		--assert error? try [make pair! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to pair! .."
		--assert 1x1 = to pair! "1x1"
		--assert error? try [to pair! quote #[unset]      ] ; unset!
		--assert error? try [to pair! quote #[none]       ] ; none!
		--assert error? try [to pair! quote #[true]       ] ; logic!
		--assert 1x1  = try [to pair! quote 1             ] ; integer!
		--assert 0x0  = try [to pair! quote 0             ] ; integer!
		--assert 4x4  = try [to pair! quote 4             ] ; integer!
		--assert 4x4  = try [to pair! quote 4.0           ] ; decimal!
		--assert error? try [to pair! quote 4.0%          ] ; percent!
		--assert error? try [to pair! quote $4            ] ; money!
		--assert error? try [to pair! quote #"a"          ] ; char!
		--assert 2x2  = try [to pair! quote 2x2           ] ; pair!
		--assert error? try [to pair! quote 1.1.1         ] ; tuple!
		--assert error? try [to pair! quote 10:00         ] ; time!
		--assert error? try [to pair! quote 2000-01-01    ] ; date!
		--assert error? try [to pair! quote #{00}         ] ; binary!
		--assert error? try [to pair! quote #{312032}     ] ; binary!
		--assert error? try [to pair! quote ""            ] ; string!
		--assert error? try [to pair! quote "1 2"         ] ; string!
		--assert error? try [to pair! quote %file         ] ; file!
		--assert error? try [to pair! quote u@email       ] ; email!
		--assert error? try [to pair! quote @ref          ] ; ref!
		--assert error? try [to pair! quote http://aa     ] ; url!
		--assert error? try [to pair! quote <tag>         ] ; tag!
		--assert 1x2  = try [to pair! quote [1 2]         ] ; block!
		--assert error? try [to pair! quote (1 2)         ] ; paren!
		--assert error? try [to pair! quote a/b           ] ; path!
		--assert error? try [to pair! quote a/b:          ] ; set-path!
		--assert error? try [to pair! quote :a/b          ] ; get-path!
		--assert error? try [to pair! quote /ref          ] ; refinement!
		--assert error? try [to pair! quote #FF           ] ; issue!
		--assert error? try [to pair! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [to pair! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [to pair! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert error? try [to pair! quote #[object! [a: 1]] ] ; object!
		--assert error? try [to pair! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to pair! string! (long)"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2202
		; using mold because of rounding error causing false comparison
		--assert "661.1194x510.1062" = mold to pair! "661.1194458007812x510.106201171875"

===end-group===


===start-group=== "make/to logic!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/763
	;@@ https://github.com/Oldes/Rebol-issues/issues/951
	;@@ https://github.com/Oldes/Rebol-issues/issues/2055
	--test-- "make logic! .."
		--assert #[true]  = try [make logic! quote #[unset] ] ; unset!
		--assert #[false]  = try [make logic! quote #[none] ] ; none!
		--assert #[true]  = try [make logic! quote #[true] ] ; logic!
		--assert #[true]  = try [make logic! quote 1 ] ; integer!
		--assert #[false]  = try [make logic! quote 0 ] ; integer!
		--assert #[true]  = try [make logic! quote 4 ] ; integer!
		--assert #[true]  = try [make logic! quote 4.0 ] ; decimal!
		--assert #[true]  = try [make logic! quote 4.0000000000000001% ] ; percent!
		--assert #[true]  = try [make logic! quote $4 ] ; money!
		--assert #[true]  = try [make logic! quote #"a" ] ; char!
		--assert #[true]  = try [make logic! quote 2x2 ] ; pair!
		--assert #[true]  = try [make logic! quote 1.1.1 ] ; tuple!
		--assert #[true]  = try [make logic! quote 10:00 ] ; time!
		--assert #[true]  = try [make logic! quote 2000-01-01 ] ; date!
		--assert #[true]  = try [make logic! quote #{00} ] ; binary!
		--assert #[true]  = try [make logic! quote #{312032} ] ; binary!
		--assert #[true]  = try [make logic! quote "" ] ; string!
		--assert #[true]  = try [make logic! quote "1 2" ] ; string!
		--assert #[true]  = try [make logic! quote %file ] ; file!
		--assert #[true]  = try [make logic! quote u@email ] ; email!
		--assert #[true]  = try [make logic! quote #[ref! "ref"] ] ; ref!
		--assert #[true]  = try [make logic! quote http://aa ] ; url!
		--assert #[true]  = try [make logic! quote <tag> ] ; tag!
		--assert #[true]  = try [make logic! quote [] ] ; empty block!
		--assert #[true]  = try [make logic! quote [1 2] ] ; block!
		--assert #[true]  = try [make logic! quote (1 2) ] ; paren!
		--assert #[true]  = try [make logic! quote a/b ] ; path!
		--assert #[true]  = try [make logic! quote a/b: ] ; set-path!
		--assert #[true]  = try [make logic! quote :a/b ] ; get-path!
		--assert #[true]  = try [make logic! quote /ref ] ; refinement!
		--assert #[true]  = try [make logic! quote #FF ] ; issue!
		--assert #[true]  = try [make logic! quote #[bitset! #{FF}] ] ; bitset!
		--assert #[true]  = try [make logic! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert #[true]  = try [make logic! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert #[true]  = try [make logic! quote #[object! [a: 1]] ] ; object!
		--assert #[true]  = try [make logic! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to logic! .."
		--assert #[true]  = try [to logic! quote #[unset] ] ; unset!
		--assert #[false]  = try [to logic! quote #[none] ] ; none!
		--assert #[true]  = try [to logic! quote #[true] ] ; logic!
		--assert #[true]  = try [to logic! quote 1 ] ; integer!
		--assert #[true]  = try [to logic! quote 0 ] ; integer!
		--assert #[true]  = try [to logic! quote 4 ] ; integer!
		--assert #[true]  = try [to logic! quote 4.0 ] ; decimal!
		--assert #[true]  = try [to logic! quote 4.0000000000000001% ] ; percent!
		--assert #[true]  = try [to logic! quote $4 ] ; money!
		--assert #[true]  = try [to logic! quote #"a" ] ; char!
		--assert #[true]  = try [to logic! quote 2x2 ] ; pair!
		--assert #[true]  = try [to logic! quote 1.1.1 ] ; tuple!
		--assert #[true]  = try [to logic! quote 10:00 ] ; time!
		--assert #[true]  = try [to logic! quote 2000-01-01 ] ; date!
		--assert #[true]  = try [to logic! quote #{00} ] ; binary!
		--assert #[true]  = try [to logic! quote #{312032} ] ; binary!
		--assert #[true]  = try [to logic! quote "" ] ; string!
		--assert #[true]  = try [to logic! quote "1 2" ] ; string!
		--assert #[true]  = try [to logic! quote %file ] ; file!
		--assert #[true]  = try [to logic! quote u@email ] ; email!
		--assert #[true]  = try [to logic! quote #[ref! "ref"] ] ; ref!
		--assert #[true]  = try [to logic! quote http://aa ] ; url!
		--assert #[true]  = try [to logic! quote <tag> ] ; tag!
		--assert #[true]  = try [to logic! quote [1 2] ] ; block!
		--assert #[true]  = try [to logic! quote (1 2) ] ; paren!
		--assert #[true]  = try [to logic! quote a/b ] ; path!
		--assert #[true]  = try [to logic! quote a/b: ] ; set-path!
		--assert #[true]  = try [to logic! quote :a/b ] ; get-path!
		--assert #[true]  = try [to logic! quote /ref ] ; refinement!
		--assert #[true]  = try [to logic! quote #FF ] ; issue!
		--assert #[true]  = try [to logic! quote #[bitset! #{FF}] ] ; bitset!
		--assert #[true]  = try [to logic! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert #[true]  = try [to logic! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert #[true]  = try [to logic! quote #[object! [a: 1]] ] ; object!
		--assert #[true]  = try [to logic! quote #[typeset! [integer! percent!]] ] ; typeset!
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
	--test-- "to binary! block!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1389
		--assert #{01}   = to binary! [1]
		--assert #{0102} = to binary! [1 2]
===end-group===


===start-group=== "make/to block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2056
	--test-- "make block! .."
		--assert  error?  try [make block! quote #[unset]   ] ; unset!
		--assert  error?  try [make block! quote #[none]    ] ; none!
		--assert  error?  try [make block! quote #[true]    ] ; logic!
		--assert  []    = try [make block! quote 4          ] ; integer!
		--assert  []    = try [make block! quote 4.0        ] ; decimal!
		--assert  error?  try [make block! quote 4%         ] ; percent!
		--assert  error?  try [make block! quote $4         ] ; money!
		--assert  error?  try [make block! quote #"a"       ] ; char!
		--assert  []    = try [make block! quote 2x2        ] ; pair! - preallocated for at least 4 values
		--assert  []    = try [make block! quote 0x2.2      ] ; pair! - preallocated for at least 2 values (1*2)
		--assert  []    = try [make block! quote 2x-2       ] ; pair! - preallocated for at least 2 values (2*1)
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
		--assert  [0 0] = try [make block! quote #[u32! 2 [0 0]]        ] ; vector!
		--assert [a: 1] = try [make block! quote #[object! [a: 1]]       ] ; object!
		--assert  error?  try [make block! #[typeset! [integer! percent!]] ] ; typeset!

	--test-- "to block! .."
		--assert [#[unset]]   = try [to block! quote #[unset]      ] ; unset!
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
		--assert [#[integer!] #[percent!]]   = try [to block! #[typeset! [integer! percent!]] ] ; typeset!
		;@@ https://github.com/Oldes/Rebol-issues/issues/2090
		--assert [a %b/c] = to block! 'a/%b/c
		--assert [a %b c] = to block! 'a/%"b"/c
===end-group===

===start-group=== "make/to path"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2451
	--test-- "make path! .."
		--assert error? try [make path! quote #[unset] ] ; unset!
		--assert error? try [make path! quote #[none] ] ; none!
		--assert error? try [make path! quote #[true] ] ; logic!
		--assert  path? try [make path! quote 1 ] ; integer!
		--assert  path? try [make path! quote 0 ] ; integer!
		--assert  path? try [make path! quote 4 ] ; integer!
		--assert  path? try [make path! quote 4.0 ] ; decimal!
		--assert error? try [make path! quote 4% ] ; percent!
		--assert error? try [make path! quote $4 ] ; money!
		--assert error? try [make path! quote #"a" ] ; char!
		--assert  path? try [make path! quote 2x2 ] ; pair!
		--assert error? try [make path! quote 1.1.1 ] ; tuple!
		--assert error? try [make path! quote 10:00 ] ; time!
		--assert error? try [make path! quote 2000-01-01 ] ; date!
		--assert  path? try [make path! quote #{00} ] ; binary!
		--assert  path? try [make path! quote #{312032} ] ; binary!
		--assert  path? try [make path! quote "" ] ; string!
		--assert  path? try [make path! quote "1 2" ] ; string!
		--assert error? try [make path! quote %file ] ; file!
		--assert error? try [make path! quote u@email ] ; email!
		--assert error? try [make path! quote #[ref! "ref"] ] ; ref!
		--assert error? try [make path! quote http://aa ] ; url!
		--assert error? try [make path! quote <tag> ] ; tag!
		--assert  path? try [make path! quote [1 2] ] ; block!
		--assert  path? try [make path! quote (1 2) ] ; paren!
		--assert  path? try [make path! quote a/b ] ; path!
		--assert  path? try [make path! quote a/b: ] ; set-path!
		--assert  path? try [make path! quote :a/b ] ; get-path!
		--assert error? try [make path! quote /ref ] ; refinement!
		--assert error? try [make path! quote #FF ] ; issue!
		--assert error? try [make path! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [make path! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert  path? try [make path! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert  path? try [make path! quote #[object! [a: 1]] ] ; object!
		--assert error? try [make path! quote #[typeset! [integer! percent!]] ] ; typeset!

	--test-- "to path! .."
		--assert  path? try [to path! quote #[unset] ] ; unset!
		--assert  path? try [to path! quote #[none] ] ; none!
		--assert  path? try [to path! quote #[true] ] ; logic!
		--assert  path? try [to path! quote 1 ] ; integer!
		--assert  path? try [to path! quote 0 ] ; integer!
		--assert  path? try [to path! quote 4 ] ; integer!
		--assert  path? try [to path! quote 4.0 ] ; decimal!
		--assert  path? try [to path! quote 4% ] ; percent!
		--assert  path? try [to path! quote $4 ] ; money!
		--assert  path? try [to path! quote #"a" ] ; char!
		--assert  path? try [to path! quote 2x2 ] ; pair!
		--assert  path? try [to path! quote 1.1.1 ] ; tuple!
		--assert  path? try [to path! quote 10:00 ] ; time!
		--assert  path? try [to path! quote 2000-01-01 ] ; date!
		--assert  path? try [to path! quote #{00} ] ; binary!
		--assert  path? try [to path! quote #{312032} ] ; binary!
		--assert  path? try [to path! quote "" ] ; string!
		--assert  path? try [to path! quote "1 2" ] ; string!
		--assert  path? try [to path! quote %file ] ; file!
		--assert  path? try [to path! quote u@email ] ; email!
		--assert  path? try [to path! quote #[ref! "ref"] ] ; ref!
		--assert  path? try [to path! quote http://aa ] ; url!
		--assert  path? try [to path! quote <tag> ] ; tag!
		--assert  path? try [to path! quote [1 2] ] ; block!
		--assert  path? try [to path! quote (1 2) ] ; paren!
		--assert  path? try [to path! quote a/b ] ; path!
		--assert  path? try [to path! quote a/b: ] ; set-path!
		--assert  path? try [to path! quote :a/b ] ; get-path!
		--assert  path? try [to path! quote /ref ] ; refinement!
		--assert  path? try [to path! quote #FF ] ; issue!
		--assert  path? try [to path! quote #[bitset! #{FF}] ] ; bitset!
		--assert  path? try [to path! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert  path? try [to path! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert  path? try [to path! quote #[object! [a: 1]] ] ; object!
		--assert  path? try [to path! quote #[typeset! [integer! percent!]] ] ; typeset!
===end-group===

===start-group=== "make/to map"
	;@@ https://github.com/Oldes/Rebol-issues/issues/982
	;@@ https://github.com/Oldes/Rebol-issues/issues/2451
	--test-- "make map! .."
		--assert error? try [make map! quote #[unset] ] ; unset!
		--assert error? try [make map! quote #[none] ] ; none!
		--assert error? try [make map! quote #[true] ] ; logic!
		--assert   map? try [make map! quote 1 ] ; integer!
		--assert   map? try [make map! quote 0 ] ; integer!
		--assert   map? try [make map! quote 4 ] ; integer!
		--assert   map? try [make map! quote 4.0 ] ; decimal!
		--assert error? try [make map! quote 4.0000000000000001% ] ; percent!
		--assert error? try [make map! quote $4 ] ; money!
		--assert error? try [make map! quote #"a" ] ; char!
		--assert error? try [make map! quote 2x2 ] ; pair!
		--assert error? try [make map! quote 1.1.1 ] ; tuple!
		--assert error? try [make map! quote 10:00 ] ; time!
		--assert error? try [make map! quote 2000-01-01 ] ; date!
		--assert error? try [make map! quote #{00} ] ; binary!
		--assert error? try [make map! quote #{312032} ] ; binary!
		--assert error? try [make map! quote "" ] ; string!
		--assert error? try [make map! quote "1 2" ] ; string!
		--assert error? try [make map! quote %file ] ; file!
		--assert error? try [make map! quote u@email ] ; email!
		--assert error? try [make map! quote #[ref! "ref"] ] ; ref!
		--assert error? try [make map! quote http://aa ] ; url!
		--assert error? try [make map! quote <tag> ] ; tag!
		--assert   map? try [make map! quote [1 2] ] ; block!
		--assert error? try [make map! quote (1 2) ] ; paren!
		--assert error? try [make map! quote a/b ] ; path!
		--assert error? try [make map! quote a/b: ] ; set-path!
		--assert error? try [make map! quote :a/b ] ; get-path!
		--assert error? try [make map! quote /ref ] ; refinement!
		--assert error? try [make map! quote #FF ] ; issue!
		--assert error? try [make map! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [make map! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [make map! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert   map? try [make map! quote #[object! [a: 1]] ] ; object!
		--assert error? try [make map! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to map! .."
		--assert error? try [to map! quote #[unset] ] ; unset!
		--assert error? try [to map! quote #[none] ] ; none!
		--assert error? try [to map! quote #[true] ] ; logic!
		--assert error? try [to map! quote 1 ] ; integer!
		--assert error? try [to map! quote 0 ] ; integer!
		--assert error? try [to map! quote 4 ] ; integer!
		--assert error? try [to map! quote 4.0 ] ; decimal!
		--assert error? try [to map! quote 4.0000000000000001% ] ; percent!
		--assert error? try [to map! quote $4 ] ; money!
		--assert error? try [to map! quote #"a" ] ; char!
		--assert error? try [to map! quote 2x2 ] ; pair!
		--assert error? try [to map! quote 1.1.1 ] ; tuple!
		--assert error? try [to map! quote 10:00 ] ; time!
		--assert error? try [to map! quote 2000-01-01 ] ; date!
		--assert error? try [to map! quote #{00} ] ; binary!
		--assert error? try [to map! quote #{312032} ] ; binary!
		--assert error? try [to map! quote "" ] ; string!
		--assert error? try [to map! quote "1 2" ] ; string!
		--assert error? try [to map! quote %file ] ; file!
		--assert error? try [to map! quote u@email ] ; email!
		--assert error? try [to map! quote #[ref! "ref"] ] ; ref!
		--assert error? try [to map! quote http://aa ] ; url!
		--assert error? try [to map! quote <tag> ] ; tag!
		--assert   map? try [to map! quote [1 2] ] ; block!
		--assert error? try [to map! quote (1 2) ] ; paren!
		--assert error? try [to map! quote a/b ] ; path!
		--assert error? try [to map! quote a/b: ] ; set-path!
		--assert error? try [to map! quote :a/b ] ; get-path!
		--assert error? try [to map! quote /ref ] ; refinement!
		--assert error? try [to map! quote #FF ] ; issue!
		--assert error? try [to map! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [to map! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [to map! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert   map? try [to map! quote #[object! [a: 1]] ] ; object!
		--assert error? try [to map! quote #[typeset! [integer! percent!]] ] ; typeset!
===end-group===

===start-group=== "make/to string"
	;@@  https://github.com/Oldes/Rebol-issues/issues/2073
	--test-- "make string! ..."
		--assert ""           = try [make string! quote #[unset]   ] ; unset!
		--assert error?         try [make string! quote #[none]    ] ; none!
		--assert "true"       = try [make string! quote #[true]    ] ; logic!
		--assert ""           = try [make string! quote 1          ] ; integer!
		--assert ""           = try [make string! quote 0          ] ; integer!
		--assert ""           = try [make string! quote 4          ] ; integer!
		--assert ""           = try [make string! quote 4.0        ] ; decimal!
		--assert "4%"         = try [make string! quote 4.0%       ] ; percent!
		--assert "$4"         = try [make string! quote $4         ] ; money!
		--assert "a"          = try [make string! quote #"a"       ] ; char!
		--assert "2x2"        = try [make string! quote 2x2        ] ; pair!
		--assert "1.1.1"      = try [make string! quote 1.1.1      ] ; tuple!
		--assert "10:00"      = try [make string! quote 10:00      ] ; time!
		--assert "1-Jan-2000" = try [make string! quote 2000-01-01 ] ; date!
		--assert "^@"         = try [make string! quote #{00}      ] ; binary!
		--assert "1 2"        = try [make string! quote #{312032}  ] ; binary!
		--assert ""           = try [make string! quote ""         ] ; string!
		--assert "1 2"        = try [make string! quote "1 2"      ] ; string!
		--assert "file"       = try [make string! quote %file      ] ; file!
		--assert "u@email"    = try [make string! quote u@email    ] ; email!
		--assert "ref"        = try [make string! quote @ref       ] ; ref!
		--assert "http://aa"  = try [make string! quote http://aa  ] ; url!
		--assert "tag"        = try [make string! quote <tag>      ] ; tag!
		--assert "12"         = try [make string! quote [1 2]      ] ; block!
		--assert "12"         = try [make string! quote (1 2)      ] ; paren!
		--assert "a"          = try [make string! quote a          ] ; word!
		--assert "a"          = try [make string! quote a:         ] ; set-word!
		--assert "a"          = try [make string! quote :a         ] ; get-word!
		--assert "a"          = try [make string! quote 'a         ] ; lit-word!
		--assert "a/b"        = try [make string! quote a/b        ] ; path!
		--assert "a/b"        = try [make string! quote a/b:       ] ; set-path!
		--assert "a/b"        = try [make string! quote :a/b       ] ; get-path!
		--assert "a/b"        = try [make string! quote 'a/b       ] ; lit-path!
		--assert "ref"        = try [make string! quote /ref       ] ; refinement!
		--assert "FF"         = try [make string! quote #FF        ] ; issue!
		--assert "0 0"        = try [make string! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert "a: 1"       = try [make string! quote #[object! [a: 1]] ] ; object!
		--assert "make bitset! #{FF}"  = try [make string! quote #[bitset! #{FF}] ] ; bitset!
		--assert "make image! [1x1 #{FFFFFF}]"  = try [make string! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert "integer! percent!"  = try [make string! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to string! ..."
		;@@ https://github.com/Oldes/Rebol-issues/issues/1404
		--assert ""           = try [to string! quote #[unset]   ] ; unset!
		--assert error?         try [to string! quote #[none]    ] ; none!
		--assert "true"       = try [to string! quote #[true]    ] ; logic!
		--assert "1"          = try [to string! quote 1          ] ; integer!
		--assert "0"          = try [to string! quote 0          ] ; integer!
		--assert "4"          = try [to string! quote 4          ] ; integer!
		--assert "4.0"        = try [to string! quote 4.0        ] ; decimal!
		--assert "4%"         = try [to string! quote 4.0%       ] ; percent!
		--assert "$4"         = try [to string! quote $4         ] ; money!
		--assert "a"          = try [to string! quote #"a"       ] ; char!
		--assert "2x2"        = try [to string! quote 2x2        ] ; pair!
		--assert "1.1.1"      = try [to string! quote 1.1.1      ] ; tuple!
		--assert "10:00"      = try [to string! quote 10:00      ] ; time!
		--assert "1-Jan-2000" = try [to string! quote 2000-01-01 ] ; date!
		--assert "^@"         = try [to string! quote #{00}      ] ; binary!
		--assert "1 2"        = try [to string! quote #{312032}  ] ; binary!
		--assert ""           = try [to string! quote ""         ] ; string!
		--assert "1 2"        = try [to string! quote "1 2"      ] ; string!
		--assert "file"       = try [to string! quote %file      ] ; file!
		--assert "u@email"    = try [to string! quote u@email    ] ; email!
		--assert "ref"        = try [to string! quote @ref       ] ; ref!
		--assert "http://aa"  = try [to string! quote http://aa  ] ; url!
		--assert "tag"        = try [to string! quote <tag>      ] ; tag!
		--assert "12"         = try [to string! quote [1 2]      ] ; block!
		--assert "12"         = try [to string! quote (1 2)      ] ; paren!
		--assert "a"          = try [to string! quote a          ] ; word!
		--assert "a"          = try [to string! quote a:         ] ; set-word!
		--assert "a"          = try [to string! quote :a         ] ; get-word!
		--assert "a"          = try [to string! quote 'a         ] ; lit-word!
		--assert "a/b"        = try [to string! quote a/b        ] ; path!
		--assert "a/b"        = try [to string! quote a/b:       ] ; set-path!
		--assert "a/b"        = try [to string! quote :a/b       ] ; get-path!
		--assert "a/b"        = try [to string! quote 'a/b       ] ; lit-path!
		--assert "ref"        = try [to string! quote /ref       ] ; refinement!
		--assert "FF"         = try [to string! quote #FF        ] ; issue!
		--assert "0 0"        = try [to string! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert "a: 1"       = try [to string! quote #[object! [a: 1]] ] ; object!
		--assert "make bitset! #{FF}"  = try [to string! quote #[bitset! #{FF}] ] ; bitset!
		--assert "make image! [1x1 #{FFFFFF}]"  = try [to string! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert "integer! percent!"  = try [to string! quote #[typeset! [integer! percent!]] ] ; typeset!

	--test-- "to string! with chars outside the BMP"
		;; current Rebol is able to use only 16bit Unicode..
		--assert #{EFBFBD} = to binary! to string! #{F09F989A}
		--assert #{EFBFBD} = to binary! to string! #{F09F989C}
===end-group===

===start-group=== "make/to tag"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1215
	--test-- "make tag! .."
		--assert #[tag! ""]  = try [make tag! quote #[unset] ] ; unset!
		--assert error? try [make tag! quote #[none] ] ; none!
		--assert <true>  = try [make tag! quote #[true] ] ; logic!
		--assert #[tag! ""]  = try [make tag! quote 1 ] ; integer!
		--assert #[tag! ""]  = try [make tag! quote 0 ] ; integer!
		--assert #[tag! ""]  = try [make tag! quote 4 ] ; integer!
		--assert #[tag! ""]  = try [make tag! quote 4.0 ] ; decimal!
		--assert <4%>  = try [make tag! quote 4% ] ; percent!
		--assert <$4>  = try [make tag! quote $4 ] ; money!
		--assert <a>  = try [make tag! quote #"a" ] ; char!
		--assert <2x2>  = try [make tag! quote 2x2 ] ; pair!
		--assert <1.1.1>  = try [make tag! quote 1.1.1 ] ; tuple!
		--assert <10:00>  = try [make tag! quote 10:00 ] ; time!
		--assert <1-Jan-2000>  = try [make tag! quote 2000-01-01 ] ; date!
		--assert #[tag! "^@"]  = try [make tag! quote #{00} ] ; binary!
		--assert <1 2>  = try [make tag! quote #{312032} ] ; binary!
		--assert #[tag! ""]  = try [make tag! quote "" ] ; string!
		--assert <1 2>  = try [make tag! quote "1 2" ] ; string!
		--assert <file>  = try [make tag! quote %file ] ; file!
		--assert <u@email>  = try [make tag! quote u@email ] ; email!
		--assert <ref>  = try [make tag! quote #[ref! "ref"] ] ; ref!
		--assert <http://aa>  = try [make tag! quote http://aa ] ; url!
		--assert <tag>  = try [make tag! quote <tag> ] ; tag!
		--assert <12>  = try [make tag! quote [1 2] ] ; block!
		--assert <12>  = try [make tag! quote (1 2) ] ; paren!
		--assert <a/b>  = try [make tag! quote a/b ] ; path!
		--assert <a/b>  = try [make tag! quote a/b: ] ; set-path!
		--assert <a/b>  = try [make tag! quote :a/b ] ; get-path!
		--assert <a/b>  = try [make tag! quote 'a/b ] ; lit-path!
		--assert <ref>  = try [make tag! quote /ref ] ; refinement!
		--assert <FF>   = try [make tag! quote #FF ] ; issue!
		--assert <make bitset! #{FF}>  = try [make tag! quote #[bitset! #{FF}] ] ; bitset!
		--assert <make image! [1x1 #{FFFFFF}]>  = try [make tag! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert <0 0>  = try [make tag! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert <a: 1>  = try [make tag! quote #[object! [a: 1]] ] ; object!
		--assert <integer! percent!>  = try [make tag! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to tag! .."
		--assert #[tag! ""] = try [to tag! quote #[unset] ] ; unset!
		--assert error? try [to tag! quote #[none] ] ; none!
		--assert <true>  = try [to tag! quote #[true] ] ; logic!
		--assert <1>  = try [to tag! quote 1 ] ; integer!
		--assert <0>  = try [to tag! quote 0 ] ; integer!
		--assert <4>  = try [to tag! quote 4 ] ; integer!
		--assert <4.0>  = try [to tag! quote 4.0 ] ; decimal!
		--assert <4%>  = try [to tag! quote 4% ] ; percent!
		--assert <$4>  = try [to tag! quote $4 ] ; money!
		--assert <a>  = try [to tag! quote #"a" ] ; char!
		--assert <2x2>  = try [to tag! quote 2x2 ] ; pair!
		--assert <1.1.1>  = try [to tag! quote 1.1.1 ] ; tuple!
		--assert <10:00>  = try [to tag! quote 10:00 ] ; time!
		--assert <1-Jan-2000>  = try [to tag! quote 2000-01-01 ] ; date!
		--assert #[tag! "^@"] = try [to tag! quote #{00} ] ; binary!
		--assert <1 2>  = try [to tag! quote #{312032} ] ; binary!
		--assert #[tag! ""] = try [to tag! quote "" ] ; string!
		--assert <1 2>  = try [to tag! quote "1 2" ] ; string!
		--assert <file>  = try [to tag! quote %file ] ; file!
		--assert <u@email>  = try [to tag! quote u@email ] ; email!
		--assert <ref>  = try [to tag! quote #[ref! "ref"] ] ; ref!
		--assert <http://aa>  = try [to tag! quote http://aa ] ; url!
		--assert <tag>  = try [to tag! quote <tag> ] ; tag!
		--assert <12>  = try [to tag! quote [1 2] ] ; block!
		--assert <12>  = try [to tag! quote (1 2) ] ; paren!
		--assert <a/b>  = try [to tag! quote a/b ] ; path!
		--assert <a/b>  = try [to tag! quote a/b: ] ; set-path!
		--assert <a/b>  = try [to tag! quote :a/b ] ; get-path!
		--assert <a/b>  = try [to tag! quote 'a/b ] ; lit-path!
		--assert <ref>  = try [to tag! quote /ref ] ; refinement!
		--assert <FF>   = try [to tag! quote #FF ] ; issue!
		--assert <make bitset! #{FF}>  = try [to tag! quote #[bitset! #{FF}] ] ; bitset!
		--assert <make image! [1x1 #{FFFFFF}]>  = try [to tag! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert <0 0>  = try [to tag! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert <a: 1>  = try [to tag! quote #[object! [a: 1]] ] ; object!
		--assert <integer! percent!>  = try [to tag! quote #[typeset! [integer! percent!]] ] ; typeset!
===end-group===

===start-group=== "make/to typeset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/987
	--test-- "make typeset! .."
		--assert error? try [make typeset! quote #[unset] ] ; unset!
		--assert error? try [make typeset! quote #[none] ] ; none!
		--assert error? try [make typeset! quote #[true] ] ; logic!
		--assert error? try [make typeset! quote 1 ] ; integer!
		--assert error? try [make typeset! quote 0 ] ; integer!
		--assert error? try [make typeset! quote 4 ] ; integer!
		--assert error? try [make typeset! quote 4.0 ] ; decimal!
		--assert error? try [make typeset! quote 4% ] ; percent!
		--assert error? try [make typeset! quote $4 ] ; money!
		--assert error? try [make typeset! quote #"a" ] ; char!
		--assert error? try [make typeset! quote 2x2 ] ; pair!
		--assert error? try [make typeset! quote 1.1.1 ] ; tuple!
		--assert error? try [make typeset! quote 10:00 ] ; time!
		--assert error? try [make typeset! quote 2000-01-01 ] ; date!
		--assert error? try [make typeset! quote #{00} ] ; binary!
		--assert error? try [make typeset! quote #{312032} ] ; binary!
		--assert error? try [make typeset! quote "" ] ; string!
		--assert error? try [make typeset! quote "1 2" ] ; string!
		--assert error? try [make typeset! quote %file ] ; file!
		--assert error? try [make typeset! quote u@email ] ; email!
		--assert error? try [make typeset! quote #[ref! "ref"] ] ; ref!
		--assert error? try [make typeset! quote http://aa ] ; url!
		--assert error? try [make typeset! quote <tag> ] ; tag!
		--assert error? try [make typeset! quote [1 2] ] ; block!
		--assert #[typeset! [block!]] = make typeset! [block!]
		--assert error? try [make typeset! quote (1 2) ] ; paren!
		--assert error? try [make typeset! quote a/b ] ; path!
		--assert error? try [make typeset! quote a/b: ] ; set-path!
		--assert error? try [make typeset! quote :a/b ] ; get-path!
		--assert error? try [make typeset! quote /ref ] ; refinement!
		--assert error? try [make typeset! quote #FF ] ; issue!
		--assert error? try [make typeset! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [make typeset! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [make typeset! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert error? try [make typeset! quote #[object! [a: 1]] ] ; object!
		--assert #[typeset! [integer! percent!]]  = try [make typeset! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to typeset! .."
		--assert error? try [to typeset! quote #[unset] ] ; unset!
		--assert error? try [to typeset! quote #[none] ] ; none!
		--assert error? try [to typeset! quote #[true] ] ; logic!
		--assert error? try [to typeset! quote 1 ] ; integer!
		--assert error? try [to typeset! quote 0 ] ; integer!
		--assert error? try [to typeset! quote 4 ] ; integer!
		--assert error? try [to typeset! quote 4.0 ] ; decimal!
		--assert error? try [to typeset! quote 4% ] ; percent!
		--assert error? try [to typeset! quote $4 ] ; money!
		--assert error? try [to typeset! quote #"a" ] ; char!
		--assert error? try [to typeset! quote 2x2 ] ; pair!
		--assert error? try [to typeset! quote 1.1.1 ] ; tuple!
		--assert error? try [to typeset! quote 10:00 ] ; time!
		--assert error? try [to typeset! quote 2000-01-01 ] ; date!
		--assert error? try [to typeset! quote #{00} ] ; binary!
		--assert error? try [to typeset! quote #{312032} ] ; binary!
		--assert error? try [to typeset! quote "" ] ; string!
		--assert error? try [to typeset! quote "1 2" ] ; string!
		--assert error? try [to typeset! quote %file ] ; file!
		--assert error? try [to typeset! quote u@email ] ; email!
		--assert error? try [to typeset! quote #[ref! "ref"] ] ; ref!
		--assert error? try [to typeset! quote http://aa ] ; url!
		--assert error? try [to typeset! quote <tag> ] ; tag!
		--assert error? try [to typeset! quote [1 2] ] ; block!
		--assert #[typeset! [block!]] = to typeset! [block!]
		--assert error? try [to typeset! quote (1 2) ] ; paren!
		--assert error? try [to typeset! quote a/b ] ; path!
		--assert error? try [to typeset! quote a/b: ] ; set-path!
		--assert error? try [to typeset! quote :a/b ] ; get-path!
		--assert error? try [to typeset! quote /ref ] ; refinement!
		--assert error? try [to typeset! quote #FF ] ; issue!
		--assert error? try [to typeset! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [to typeset! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [to typeset! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert error? try [to typeset! quote #[object! [a: 1]] ] ; object!
		--assert #[typeset! [integer! percent!]]  = try [to typeset! quote #[typeset! [integer! percent!]] ] ; typeset!
===end-group===

===start-group=== "make/to event"
	;@@ https://github.com/Oldes/Rebol-issues/issues/986
	--test-- "make event! .."
		--assert event? try [make event! make event! [type: 'connect]]
		--assert error? try [make event! quote #[unset] ] ; unset!
		--assert error? try [make event! quote #[none] ] ; none!
		--assert error? try [make event! quote #[true] ] ; logic!
		--assert error? try [make event! quote 1 ] ; integer!
		--assert error? try [make event! quote 0 ] ; integer!
		--assert error? try [make event! quote 4 ] ; integer!
		--assert error? try [make event! quote 4.0 ] ; decimal!
		--assert error? try [make event! quote 4.0000000000000001% ] ; percent!
		--assert error? try [make event! quote $4 ] ; money!
		--assert error? try [make event! quote #"a" ] ; char!
		--assert error? try [make event! quote 2x2 ] ; pair!
		--assert error? try [make event! quote 1.1.1 ] ; tuple!
		--assert error? try [make event! quote 10:00 ] ; time!
		--assert error? try [make event! quote 2000-01-01 ] ; date!
		--assert error? try [make event! quote #{00} ] ; binary!
		--assert error? try [make event! quote #{312032} ] ; binary!
		--assert error? try [make event! quote "" ] ; string!
		--assert error? try [make event! quote "1 2" ] ; string!
		--assert error? try [make event! quote %file ] ; file!
		--assert error? try [make event! quote u@email ] ; email!
		--assert error? try [make event! quote #[ref! "ref"] ] ; ref!
		--assert error? try [make event! quote http://aa ] ; url!
		--assert error? try [make event! quote <tag> ] ; tag!
		--assert error? try [make event! quote [1 2] ] ; block!
		--assert error? try [make event! quote (1 2) ] ; paren!
		--assert error? try [make event! quote a/b ] ; path!
		--assert error? try [make event! quote a/b: ] ; set-path!
		--assert error? try [make event! quote :a/b ] ; get-path!
		--assert error? try [make event! quote /ref ] ; refinement!
		--assert error? try [make event! quote #FF ] ; issue!
		--assert error? try [make event! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [make event! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [make event! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert error? try [make event! quote #[object! [a: 1]] ] ; object!
		--assert error? try [make event! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to event! .."
		--assert event? try [to event! to event! [type: 'connect]]
		--assert error? try [to event! quote #[unset] ] ; unset!
		--assert error? try [to event! quote #[none] ] ; none!
		--assert error? try [to event! quote #[true] ] ; logic!
		--assert error? try [to event! quote 1 ] ; integer!
		--assert error? try [to event! quote 0 ] ; integer!
		--assert error? try [to event! quote 4 ] ; integer!
		--assert error? try [to event! quote 4.0 ] ; decimal!
		--assert error? try [to event! quote 4.0000000000000001% ] ; percent!
		--assert error? try [to event! quote $4 ] ; money!
		--assert error? try [to event! quote #"a" ] ; char!
		--assert error? try [to event! quote 2x2 ] ; pair!
		--assert error? try [to event! quote 1.1.1 ] ; tuple!
		--assert error? try [to event! quote 10:00 ] ; time!
		--assert error? try [to event! quote 2000-01-01 ] ; date!
		--assert error? try [to event! quote #{00} ] ; binary!
		--assert error? try [to event! quote #{312032} ] ; binary!
		--assert error? try [to event! quote "" ] ; string!
		--assert error? try [to event! quote "1 2" ] ; string!
		--assert error? try [to event! quote %file ] ; file!
		--assert error? try [to event! quote u@email ] ; email!
		--assert error? try [to event! quote #[ref! "ref"] ] ; ref!
		--assert error? try [to event! quote http://aa ] ; url!
		--assert error? try [to event! quote <tag> ] ; tag!
		--assert error? try [to event! quote [1 2] ] ; block!
		--assert error? try [to event! quote (1 2) ] ; paren!
		--assert error? try [to event! quote a/b ] ; path!
		--assert error? try [to event! quote a/b: ] ; set-path!
		--assert error? try [to event! quote :a/b ] ; get-path!
		--assert error? try [to event! quote /ref ] ; refinement!
		--assert error? try [to event! quote #FF ] ; issue!
		--assert error? try [to event! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [to event! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [to event! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert error? try [to event! quote #[object! [a: 1]] ] ; object!
		--assert error? try [to event! quote #[typeset! [integer! percent!]] ] ; typeset!
===end-group===


===start-group=== "make/to word"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2492
	--test-- "make word! ..."
		--assert error? try [make word! quote #[unset] ] ; unset!
		--assert error? try [make word! quote #[none] ] ; none!
		--assert 'true = try [make word! quote #[true] ] ; logic!
		--assert error? try [make word! quote 1 ] ; integer!
		--assert error? try [make word! quote 0 ] ; integer!
		--assert error? try [make word! quote 4 ] ; integer!
		--assert error? try [make word! quote 4.0 ] ; decimal!
		--assert error? try [make word! quote 4% ] ; percent!
		--assert error? try [make word! quote $4 ] ; money!
		--assert 'a   = try [make word! quote #"a" ] ; char!
		--assert error? try [make word! quote 2x2 ] ; pair!
		--assert error? try [make word! quote 1.1.1 ] ; tuple!
		--assert error? try [make word! quote 10:00 ] ; time!
		--assert error? try [make word! quote 2000-01-01 ] ; date!
		--assert error? try [make word! quote #{00} ] ; binary!
		--assert error? try [make word! quote #{312032} ] ; binary!
		--assert error? try [make word! quote "" ] ; string!
		--assert error? try [make word! quote "1 2" ] ; string!
		--assert 'file = try [make word! quote %file ] ; file!
		--assert error? try [make word! quote u@email ] ; email!
		--assert 'foo = try [make word! to email! "foo" ] ; email!
		--assert 'ref = try [make word! quote #[ref! "ref"] ] ; ref!
		--assert error? try [make word! quote http://aa ] ; url!
		--assert 'foo = try [make word! to url! "foo" ] ; url
		--assert 'tag = try [make word! quote <tag> ] ; tag!
		--assert error? try [make word! quote [1 2] ] ; block!
		--assert error? try [make word! quote (1 2) ] ; paren!
		--assert error? try [make word! quote a/b ] ; path!
		--assert error? try [make word! quote a/b: ] ; set-path!
		--assert error? try [make word! quote :a/b ] ; get-path!
		--assert 'ref = try [make word! quote /ref ] ; refinement!
		--assert 'FF  = try [make word! quote #FF ] ; issue!
		--assert error? try [make word! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [make word! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [make word! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert error? try [make word! quote #[object! [a: 1]] ] ; object!
		--assert error? try [make word! quote #[typeset! [integer! percent!]] ] ; typeset!
	--test-- "to word! ..."
		--assert error? try [to word! quote #[unset] ] ; unset!
		--assert error? try [to word! quote #[none] ] ; none!
		--assert 'true = try [to word! quote #[true] ] ; logic!
		--assert error? try [to word! quote 1 ] ; integer!
		--assert error? try [to word! quote 0 ] ; integer!
		--assert error? try [to word! quote 4 ] ; integer!
		--assert error? try [to word! quote 4.0 ] ; decimal!
		--assert error? try [to word! quote 4.0000000000000001% ] ; percent!
		--assert error? try [to word! quote $4 ] ; money!
		--assert 'a   = try [to word! quote #"a" ] ; char!
		--assert error? try [to word! quote 2x2 ] ; pair!
		--assert error? try [to word! quote 1.1.1 ] ; tuple!
		--assert error? try [to word! quote 10:00 ] ; time!
		--assert error? try [to word! quote 2000-01-01 ] ; date!
		--assert error? try [to word! quote #{00} ] ; binary!
		--assert error? try [to word! quote #{312032} ] ; binary!
		--assert error? try [to word! quote "" ] ; string!
		--assert error? try [to word! quote "1 2" ] ; string!
		--assert 'file = try [to word! quote %file ] ; file!
		--assert error? try [to word! quote u@email ] ; email!
		--assert 'foo = try [to word! to email! "foo" ] ; email!
		--assert 'ref = try [to word! quote #[ref! "ref"] ] ; ref!
		--assert error? try [to word! quote http://aa ] ; url!
		--assert 'foo = try [to word! to url! "foo" ] ; url
		--assert 'tag = try [to word! quote <tag> ] ; tag!
		--assert error? try [to word! quote [1 2] ] ; block!
		--assert error? try [to word! quote (1 2) ] ; paren!
		--assert error? try [to word! quote a/b ] ; path!
		--assert error? try [to word! quote a/b: ] ; set-path!
		--assert error? try [to word! quote :a/b ] ; get-path!
		--assert 'ref = try [to word! quote /ref ] ; refinement!
		--assert 'FF  = try [to word! quote #FF ] ; issue!
		--assert error? try [to word! quote #[bitset! #{FF}] ] ; bitset!
		--assert error? try [to word! quote #[image! 1x1 #{FFFFFF}] ] ; image!
		--assert error? try [to word! quote #[vector! integer! 32 2 [0 0]] ] ; vector!
		--assert error? try [to word! quote #[object! [a: 1]] ] ; object!
		--assert error? try [to word! quote #[typeset! [integer! percent!]] ] ; typeset!
===end-group===


===start-group=== "make special"
	--test-- "make types from none!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1018
	;@@ https://github.com/Oldes/Rebol-issues/issues/1024
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
	--test-- "to types from none!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1019
		--assert error? try [to end! none]
		--assert not error? try [to unset! none]
		--assert not error? try [to none! none]
		--assert not error? try [to logic! none]
		--assert error? try [to integer! none]
		--assert error? try [to decimal! none]
		--assert error? try [to percent! none]
		--assert error? try [to money! none]
		--assert error? try [to char! none]
		--assert error? try [to pair! none]
		--assert error? try [to tuple! none]
		--assert error? try [to time! none]
		--assert error? try [to date! none]
		--assert error? try [to binary! none]
		--assert error? try [to string! none]
		--assert error? try [to file! none]
		--assert error? try [to email! none]
		--assert error? try [to ref! none]
		--assert error? try [to url! none]
		--assert error? try [to tag! none]
		--assert error? try [to bitset! none]
		--assert error? try [to image! none]
		--assert error? try [to vector! none]
		--assert [#[none]] = try [to block! none]
		--assert (quote (#[none])) = try [to paren! none]
		--assert     path? try [to path! none]
		--assert set-path? try [to set-path! none]
		--assert get-path? try [to get-path! none]
		--assert lit-path? try [to lit-path! none]
		--assert error? try [to map! none]
		--assert error? try [to datatype! none]
		--assert error? try [to typeset! none]
		--assert error? try [to word! none]
		--assert error? try [to set-word! none]
		--assert error? try [to get-word! none]
		--assert error? try [to lit-word! none]
		--assert error? try [to refinement! none]
		--assert error? try [to issue! none]
		--assert error? try [to native! none]
		--assert error? try [to action! none]
		--assert error? try [to rebcode! none]
		--assert error? try [to command! none]
		--assert error? try [to op! none]
		--assert error? try [to closure! none]
		--assert error? try [to function! none]
		--assert error? try [to frame! none]
		--assert error? try [to object! none]
		--assert error? try [to module! none]
		--assert error? try [to error! none]
		--assert error? try [to task! none]
		--assert error? try [to port! none]
		--assert error? try [to gob! none]
		--assert error? try [to event! none]
		--assert error? try [to handle! none]
		--assert error? try [to struct! none]
		--assert error? try [to library! none]
		--assert error? try [to utype! none]
===end-group===

~~~end-file~~~