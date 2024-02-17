Rebol [
	Title:   "Rebol lexer test script"
	Author:  "Oldes"
	File: 	 %lexer-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Lexer"

===start-group=== "TRANSCODE"
	--test-- "transcode basic"
	;@@ https://github.com/Oldes/Rebol-issues/issues/536
	;@@ https://github.com/Oldes/Rebol-issues/issues/688
	;@@ https://github.com/Oldes/Rebol-issues/issues/1329
		--assert [1 + 1] = transcode to binary! "1 + 1"
		--assert [1 + 1] = transcode "1 + 1"
		--assert []      = transcode ""

	--test-- "transcode/next"
	;@@ https://github.com/Oldes/Rebol-issues/issues/535
		--assert [1 " + 1"]      = transcode/next "1 + 1"
		--assert [1 #{202B2031}] = transcode/next to binary! "1 + 1"
		--assert [[1 + 1] ""]    = transcode/next "[1 + 1]"
		--assert [[1 + 1] #{}]   = transcode/next to binary! "[1 + 1]"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2511
		--assert all [error? e: try[transcode/next ""] e/id = 'past-end]
		--assert all [error? e: transcode/next/error "" e/id = 'past-end]

	--test-- "transcode/one"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1329
		--assert all [error? e: try[transcode/one ""] e/id = 'past-end]
		--assert all [error? e: transcode/one/error "" e/id = 'past-end]
		--assert unset?  transcode/one "#(unset)"
		--assert []    = transcode/one "[]"
		--assert 1     = transcode/one "1 2"
		--assert [1 2] = transcode/one "[1 2]"

	--test-- "transcode/line"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2302
		--assert all [error? e: try [transcode "1 1d"] e/near = "(line 1) 1 1d"]
		--assert all [error? e: try [transcode "1^/1d"] e/near = "(line 2) 1d"]
		--assert all [error? e: try [transcode/line "1 1d" 10] e/near = "(line 10) 1 1d"]
		--assert all [error? e: try [transcode/line "1^/1d" 10] e/near = "(line 11) 1d"]
		--assert all [
			code: "1^/2" line: 1
			set [value code line] transcode/next/line :code :line
			value = 1 line = 1
			set [value code line] transcode/next/line :code :line
			value = 2 line = 2
		]

	--test-- "transcode/error"
		--assert all [
			block? blk: transcode/error "1 2d"
			blk/1 = 1
			error? blk/2
			blk/2/id = 'invalid
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/1857
		--assert all [
			error? e: transcode/one/error "#(block! 1)"
			e/id = 'malconstruct
		]
		--assert all [
			error? e: transcode/one/error "#(block! [1d)"
			e/id = 'malconstruct
		]
		--assert all [
			error? e: transcode/one/error "#("
			e/id = 'missing
			e/arg1 = "end-of-script"
		]


===end-group===

===start-group=== "Invalid construction"
	--test-- "Invalid MAP"
		--assert error? err: try [load {#[x]}]
		--assert err/id = 'invalid-arg
		--assert error? err: try [load {#[x}]
		--assert all [err/id = 'missing err/arg1 = "end-of-script" err/arg2 = "]"]

	--test-- "Invalid word"
	;@@ https://github.com/Oldes/Rebol-issues/issues/22
		--assert error? try [load "':a"]
		--assert error? try [load "':a:"]

	--test-- "Invalid issue!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/122
		--assert error? try [load "1234#"]
	;@@ https://github.com/oldes/rebol-issues/issues/2297
		--assert error? try [load {#}] ; originally (in R3-alpha) it was same like #(none)

	--test-- "Multiple leading / in refinement!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1855
		--assert error? try [load {////a}]

	--test-- "Invalid money!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1441
		--assert all [
			error? e: try [load {$}]
			e/id = 'invalid
		]

	--test-- "Invalid path"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1319
		--assert all [
			error? e: try [load {a/b<}]
			e/id = 'invalid
			e/arg1 = "word"
		]
		--assert all [
			error? e: try [load {a/3<}]
			e/id = 'invalid
			e/arg1 = "integer"
		]

	--test-- "Invalid time"
	;@@ https://github.com/Oldes/Rebol-issues/issues/698
		--assert all [
			error? e: try [load {--1:23}]
			e/id = 'invalid
		]

	--test-- "Invalid date"
	;@@ https://github.com/Oldes/Rebol-issues/issues/698
		--assert all [error? e: try [load {19-Jan-2010<}] e/id = 'invalid]

	--test-- "Invalid % escape"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1443
		--assert all [error? e: try [load {a@%2h}] e/id = 'invalid]
		--assert all [error? e: try [load {%a%2h}] e/id = 'invalid]
		--assert all [error? e: try [load {url:a%2h}] e/id = 'invalid]

	--test-- "Invalid serialized value"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1429
		--assert all [error? e: try [load {1#(logic! 1)}] e/id = 'invalid]
		--assert all [error? e: try [load {a#(logic! 1)}] e/id = 'invalid]

	--test-- "Invalid char"
		--assert all [error? e: try [load {2#"a"}] e/id = 'invalid]

	--test-- "Invalid path construction"
	;@@ https://github.com/Oldes/Rebol-issues/issues/863
		--assert all [error? e: try [load {#(path! [0])}] e/id = 'malconstruct]

	--test-- "Invalid file"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1415
		--assert all [error? e: try [load {%^^}] e/id = 'invalid]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1442
		--assert all [error? e: try [load {%a^^b}] e/id = 'invalid]
		--assert all [error? e: try [load {%a^^ }] e/id = 'invalid]

===end-group===


===start-group=== "Raw string"
	--test-- "rawstring %{}%"
		--assert ""     == transcode/one "%{}%"
		--assert ""     == transcode/one "%%{}%%"
		--assert "a^^b" == transcode/one "%{a^^b}%"
		--assert "}"    == transcode/one "%{}}%"
		--assert "{"    == transcode/one "%{{}%"
		--assert " %{^}% "  == transcode/one "%%{ %{^}% }%%"
	--test-- "rawstring %{}% multiline"
		--assert "^/"   == transcode/one rejoin ["%{" LF "}%"]
		--assert "^M^/" == transcode/one rejoin ["%{" CR LF "}%"]

===end-group===


===start-group=== "Special % word"
	--test-- "valid % word cases"
		--assert word? try [load {%}]
		--assert word? try [load {% }]
		--assert word? try [load {%^-}]
		--assert word? try [first load {[%]}]
	--test-- "valid % lit-word cases"
		--assert lit-word? try [load {'%}]
		--assert lit-word? try [load {'% }]
		--assert lit-word? try [load {'%^-}]
		--assert lit-word? try [first load {['%]}]
	--test-- "valid % get-word cases"
		--assert get-word? try [load {:%}]
		--assert get-word? try [load {:% }]
		--assert get-word? try [load {:%^-}]
		--assert get-word? try [first load {[:%]}]
	--test-- "invalid % lit-word cases"
		--assert all [error? e: try [load {'%""}] e/id = 'invalid e/arg1 = "word-lit"]
		--assert all [error? e: try [load {'%/}]  e/id = 'invalid e/arg1 = "word-lit"]
	--test-- "invalid % get-word cases"
		--assert all [error? e: try [load {:%""}] e/id = 'invalid e/arg1 = "word-get"]
		--assert all [error? e: try [load {:%/}]  e/id = 'invalid e/arg1 = "word-get"]
	--test-- "% used in object"
		--assert all [
			not error? try [o: make object! [%: 1]]
			1 = o/%
		]

===end-group===

===start-group=== "Special arrow-like words"
;@@ https://github.com/Oldes/Rebol-issues/issues/1302
;@@ https://github.com/Oldes/Rebol-issues/issues/1318
;@@ https://github.com/Oldes/Rebol-issues/issues/1342
;@@ https://github.com/Oldes/Rebol-issues/issues/1461
;@@ https://github.com/Oldes/Rebol-issues/issues/1478

	--test-- "valid arrow-like words"
		--assert word? try [load {<-->}]
		--assert word? try [load {<==>}]
		--assert word? try [load {<-==->}]
		--assert word? try [load {<~~~>}]

	--test-- "valid left-arrow-like words"
		--assert word? try [load {<<}]
		--assert word? try [load {<<<}]
		--assert word? try [load {<<<<}]
		--assert word? try [load {<<==}]
		--assert word? try [load {<===}]
		--assert word? try [load {<---}]
		--assert word? try [load {<~~~}]
		--assert all [block? b: try [load {<<<""}] parse b [word! string!]]

	--test-- "valid right-arrow-like words"
		--assert word? try [load {>>}]
		--assert word? try [load {>>>}]
		--assert word? try [load {>>>>}]
		--assert word? try [load {==>>}]
		--assert word? try [load {===>}] 
		--assert word? try [load {--->}]
		--assert word? try [load {~~~>}] 
		--assert all [block? b: try [load {>>>""}] parse b [word! string!]]

	--test-- "invalid cases"
		--assert error? try [load {a<}]
		--assert error? try [load {a>}]
		--assert error? try [load {a-->}]

	--test-- "special cases"
		--assert all [block? b: try [load {a<--}] parse b [word! word!]]
		--assert all [block? b: try [load {a<a>}] parse b [word! tag!]]

	--test-- "valid arrow-like lit-words"
		--assert lit-word? try [load {'<>}]
		--assert lit-word? try [load {'<-->}]
		--assert lit-word? try [load {'<==>}]
		--assert lit-word? try [load {'<-==->}]
		--assert lit-word? try [load {'<~~~>}]

	--test-- "valid left-arrow-like lit-words"
		--assert lit-word? try [load {'<} ]
		--assert lit-word? try [load {'<<}]
		--assert lit-word? try [load {'<=}]
		--assert lit-word? try [load {'<<<}]
		--assert lit-word? try [load {'<<<<}]
		--assert all [block? b: try [load {'<<<""}] parse b [lit-word! string!]]

	--test-- "valid right-arrow-like lit-words"
		--assert lit-word? try [load {'>} ]
		--assert lit-word? try [load {'>>}]
		--assert lit-word? try [load {'>=}]
		--assert lit-word? try [load {'>>>}]
		--assert lit-word? try [load {'>>>>}]
		--assert lit-word? try [load {'==>>}]
		--assert lit-word? try [load {'===>}]
		--assert lit-word? try [load {'--->}]
		--assert lit-word? try [load {'~~~>}]
		--assert all [block? b: try [load {'>>>""}] parse b [lit-word! string!]]

	--test-- "valid arrow-like get-words"
		--assert get-word? try [load {:<-->}]
		--assert get-word? try [load {:<==>}]
		--assert get-word? try [load {:<-==->}]
		--assert get-word? try [load {:<~~~>}]

	--test-- "valid left-arrow-like get-words"
		--assert get-word? try [load {:<<}]
		--assert get-word? try [load {:<<<}]
		--assert get-word? try [load {:<<<<}]
		--assert all [block? b: try [load {:<<<""}] parse b [get-word! string!]]

	--test-- "valid right-arrow-like get-words"
		--assert get-word? try [load {:>>}]
		--assert get-word? try [load {:>>>}]
		--assert get-word? try [load {:>>>>}]
		--assert get-word? try [load {:==>>}]
		--assert get-word? try [load {:===>}]
		--assert get-word? try [load {:--->}]
		--assert get-word? try [load {:~~~>}]
		--assert all [block? b: try [load {:>>>""}] parse b [get-word! string!]]

	--test-- "valid arrow-like set-words"
		--assert set-word? try [load {<-->:}]
		--assert set-word? try [load {<==>:}]
		--assert set-word? try [load {<-==->:}]
		--assert set-word? try [load {<~~~>:}]

	--test-- "valid left-arrow-like set-words"
		--assert set-word? try [load {<<:}]
		--assert set-word? try [load {<<<:}]
		--assert set-word? try [load {<<<<:}]
		--assert all [block? b: try [load {<<<:""}] parse b [set-word! string!]]

	--test-- "valid right-arrow-like set-words"
		--assert set-word? try [load {>>:}]
		--assert set-word? try [load {>>>:}]
		--assert set-word? try [load {>>>>:}]
		--assert set-word? try [load {==>>:}]
		--assert set-word? try [load {===>:}]
		--assert set-word? try [load {--->:}]
		--assert set-word? try [load {~~~>:}]
		--assert all [block? b: try [load {>>>:""}] parse b [set-word! string!]]

	--test-- "valid arrow-like refinements"
		--assert refinement? try [load {/<-->}]
		--assert refinement? try [load {/<==>}]
		--assert refinement? try [load {/<-==->}]
		--assert refinement? try [load {/<~~~>}]

	--test-- "valid left-arrow-like refinements"
		--assert refinement? try [load {/<<}]
		--assert refinement? try [load {/<<<}]
		--assert refinement? try [load {/<<<<}]
		--assert all [block? b: try [load {/<<<""}] parse b [refinement! string!]]

	--test-- "valid right-arrow-like refinements"
		--assert refinement? try [load {/>>}]
		--assert refinement? try [load {/>>>}]
		--assert refinement? try [load {/>>>>}]
		--assert refinement? try [load {/==>>}]
		--assert refinement? try [load {/===>}]
		--assert refinement? try [load {/--->}]
		--assert refinement? try [load {/~~~>}]
		--assert all [block? b: try [load {/>>>""}] parse b [refinement! string!]]

===end-group===

===start-group=== "Special slash words"
;@@ https://github.com/Oldes/Rebol-issues/issues/1477
	--test-- "valid slash words"
		--assert word? try [load {/}]
		--assert word? try [load {//}]
		--assert word? try [load {///}]
	--test-- "valid slash lit-words"
		--assert lit-word? try [load {'/}]
		--assert lit-word? try [load {'//}]
		--assert lit-word? try [load {'///}]
	--test-- "valid slash get-words"
		--assert get-word? try [load {:/}]
		--assert get-word? try [load {://}]
		--assert get-word? try [load {:///}]
	--test-- "valid slash set-words"
		--assert set-word? try [load {/:}]
		--assert set-word? try [load {//:}]
		--assert set-word? try [load {///:}]
===end-group===


===start-group=== "Email"
	--test-- "valid `emails`"
		--assert email? load {name@where}
		--assert email? load {a@šiška}

===end-group===

===start-group=== "Money"
	--test-- "space requirement"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1445
		;@@ https://github.com/Oldes/Rebol-issues/issues/1472
		--assert all [error? e: try [load {$1/$2}] e/id = 'invalid e/arg2 = "$1/"]
		--assert all [error? e: try [load {$1*$2}] e/id = 'invalid e/arg2 = "$1*$2"]
		--assert all [error? e: try [load {$1+$2}] e/id = 'invalid e/arg2 = "$1+$2"]
		--assert all [error? e: try [load {$1-$2}] e/id = 'invalid e/arg2 = "$1-$2"]
===end-group===

===start-group=== "Ref"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1962
	--test-- "valid `ref!`"
		--assert ref? load {@name}
		--assert ref? load {@123}
		--assert ref? load {@1x2}
		--assert ref? load {@šiška}
		--assert ref? load {@%C5%A1}

	;@@ https://github.com/Oldes/Rebol-issues/issues/2437
	--test-- "invalid `ref!`"
		--assert error? try [load {'2nd}]
		--assert error? try [load {':foo}]
		--assert error? try [load {'@foo}]

===end-group===

===start-group=== "Get-word"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2437
	--test-- "invalid `get-word!`"
		--assert error? try [load {:2nd}]
		--assert error? try [load {::foo}]
		--assert error? try [load {:@foo}]

===end-group===

===start-group=== "Refinement"
	;@@ https://github.com/Oldes/Rebol-issues/issues/980
	;@@ https://github.com/Oldes/Rebol-issues/issues/1856
	--test-- "valid `refinement!`"
		--assert refinement? try [load {/foo}]
		--assert refinement? try [load {/+}]
		--assert refinement? try [load {/!}]
		--assert refinement? try [load {/111}]
		--assert refinement? try [load {/+1}]
		--assert refinement? load "/+123"
		--assert refinement? load "/-"
		--assert refinement? load "/."
		--assert refinement? load "/.123"

===end-group===

===start-group=== "Tag"
	--test-- "valid `tags`"
		--assert tag? load {<a '"'>} ;@@ https://github.com/Oldes/Rebol-issues/issues/1873

	--test-- "issue-1919"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1919
		--assert all  [
			block? b: try [load "<a<b b>>"]
			b/1 = <a<b b>
			b/2 = '>
		]
	--test-- "issue-1317"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1317
		--assert '<  = transcode/one/error "<]>"
		--assert '<  = transcode/one/error "<)>"
		--assert <a> = transcode/one/error "<a>"
		--assert [<] = transcode/one/error "[<]"
		--assert block? try [load "[(<)]"]
		--assert error? try [load "<)>"]
		--assert error? try [load "<]>"]

===end-group===


===start-group=== "Integer"
	--test-- "-0"
		--assert 0 = load "-0" ;@@ https://github.com/Oldes/Rebol-issues/issues/33

===end-group===

===start-group=== "Integer (bit/octal/decimal/hexadecimal)"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1781
	;@@ https://github.com/Oldes/Rebol-issues/issues/2197
	--test-- "base2"
		--assert  1 = transcode/one "2#01"
		--assert  1 = transcode/one "2#01[]"
		--assert  1 = transcode/one "2#01{}"
		--assert  3 = transcode/one "2#11"
		--assert  3 = transcode/one "2#011"
		--assert  3 = transcode/one "2#000011"
		--assert -1 = transcode/one "2#1111111111111111111111111111111111111111111111111111111111111111"
		--assert error? transcode/one/error "2#12"
		--assert error? transcode/one/error "2#11111111111111111111111111111111111111111111111111111111111111111"
		--assert error? transcode/one/error "-2#11"
	--test-- "base8"
		--assert 666 = transcode/one "8#1232"
		--assert 502 = transcode/one "8#766"
		--assert  -1 = transcode/one "8#7777777777777777777777"
		--assert error? transcode/one/error "8#88"
		--assert error? transcode/one/error "8#77777777777777777777777"
		--assert error? transcode/one/error "-8#123"
	--test-- "base10"
		--assert 123 = transcode/one "10#123"
		--assert 999999999999999999 = transcode/one "10#999999999999999999"
		--assert error? transcode/one/error "10#9999999999999999999"
		--assert error? transcode/one/error "10#1A2"
		--assert error? transcode/one/error "-10#123"
	--test-- "base16"
		--assert  15 = transcode/one "0#F"
		--assert  15 = transcode/one "0#0F"
		--assert 255 = transcode/one "0#FF"
		--assert  -1 = transcode/one "0#FFFFFFFFFFFFFFFF"
		--assert error? transcode/one/error "0#XA"
		--assert error? transcode/one/error "0#FFFFFFFFFFFFFFFFF"
		--assert error? transcode/one/error "-0#FF"
		--assert  15 = transcode/one "16#F"
		--assert  15 = transcode/one "16#0F"
		--assert 255 = transcode/one "16#FF"
		--assert  -1 = transcode/one "16#FFFFFFFFFFFFFFFF"
		--assert error? transcode/one/error "16#XA"
		--assert error? transcode/one/error "16#FFFFFFFFFFFFFFFFF"
		--assert error? transcode/one/error "-16#FF"



===end-group===


===start-group=== "Issue"
	--test-- {###}
	;@@ https://github.com/Oldes/Rebol-issues/issues/2583
		--assert "###" = try [mold transcode/one {###}]
		--assert "#a#" = try [mold transcode/one {#a#}]
		--assert "#ab" = try [mold transcode/one {#ab}]

===end-group===


===start-group=== "Char"
	--test-- {#"^(1)"}
		--assert #"^A" = load {#"^(1)"} ;@@ https://github.com/Oldes/Rebol-issues/issues/52
		--assert #"^A" = load {#"^(01)"}

===end-group===


===start-group=== "SIGN before POUND char (issue #2319)"
;@@ https://github.com/Oldes/Rebol-issues/issues/2319
	--test-- "sign-before-pound-1"	--assert  [- #"a"] = (load {-#"a"})
	--test-- "sign-before-pound-2"	--assert  [+ #"a"] = (load {+#"a"})
	--test-- "sign-before-pound-3"	--assert  [- #{00}]   = try [load {-#{00}}]
	--test-- "sign-before-pound-4"	--assert  [- #(none)] = try [load {-#(none)}]
	--test-- "sign-before-pound-5"	--assert  word! = try [type? first load {+#"a"}]
	--test-- "sign-before-pound-6"	--assert  [- #hhh] = try [load {-#hhh}]
	;above is now consistent with:
	--test-- "sign-before-string"	--assert  [- "a"] = (load {-"a"})
	--test-- "sign-before-block"	--assert  [- []] = (load {-[]})
	;and can be used correctly in charsets
	--test-- "lexer-charset-with-tight-range"
		--assert "make bitset! #{0000000000000000000000007FFFFFE0}" = mold charset [#"a"-#"z"] ;this failed before fix of #2319
		--assert "make bitset! #{0000000000000000000000007FFFFFE0}" = mold charset [#"a" - #"z"]

===end-group===

===start-group=== "Construction syntax"
	--test-- {any-string!}
		--assert "ab"  = load {#(string! "ab")}
		--assert  "b"  = load {#(string! "ab" 2)}
		--assert %ab   = load {#(file! "ab")}
		--assert  %b   = load {#(file! "ab" 2)}
		--assert struct? load {#(struct! [a [uint8!]])}
		;@@ https://github.com/Oldes/Rebol-issues/issues/1034
		--assert error? try [load {#(string! "ab" 2 x)}]
		--assert error? try [load {#(file! "ab" x)}]
		--assert error? try [load {#(file! "ab" 2 x)}]
		--assert error? try [load {#(string! "ab" x)}]
		--assert error? try [load {#(string! "ab" 2 x)}]
		--assert error? try [load {#(file! "ab" x)}]
		--assert error? try [load {#(file! "ab" 2 x)}]
	--test-- {object!}
		;@@ https://github.com/Oldes/Rebol-issues/issues/864
		--assert block?  try [transcode      "#(object! [a: 1 b: 2])"]
		--assert block?  try [transcode/only "#(object! [a: 1 b: 2])"]
		--assert object? try [transcode/one  "#(object! [a: 1 b: 2])"]
	--test-- {function!}
		;@@ https://github.com/Oldes/Rebol-issues/issues/1114
		--assert function? transcode/one {#(function! [[a [series!]][print a]])}

	--test-- {datatype!}
		;@@ https://github.com/Oldes/Rebol-issues/issues/2508
		--assert datatype? transcode/one {#(unset!)}
		--assert datatype? transcode/one {#(none!)}
		--assert datatype? transcode/one {#(logic!)}
		--assert datatype? transcode/one {#(integer!)}
		--assert datatype? transcode/one {#(decimal!)}
		--assert datatype? transcode/one {#(percent!)}
		--assert datatype? transcode/one {#(money!)}
		--assert datatype? transcode/one {#(char!)}
		--assert datatype? transcode/one {#(pair!)}
		--assert datatype? transcode/one {#(tuple!)}
		--assert datatype? transcode/one {#(time!)}
		--assert datatype? transcode/one {#(date!)}
		--assert datatype? transcode/one {#(binary!)}
		--assert datatype? transcode/one {#(string!)}
		--assert datatype? transcode/one {#(file!)}
		--assert datatype? transcode/one {#(email!)}
		--assert datatype? transcode/one {#(ref!)}
		--assert datatype? transcode/one {#(url!)}
		--assert datatype? transcode/one {#(tag!)}
		--assert datatype? transcode/one {#(bitset!)}
		--assert datatype? transcode/one {#(image!)}
		--assert datatype? transcode/one {#(vector!)}
		--assert datatype? transcode/one {#(block!)}
		--assert datatype? transcode/one {#(paren!)}
		--assert datatype? transcode/one {#(path!)}
		--assert datatype? transcode/one {#(set-path!)}
		--assert datatype? transcode/one {#(get-path!)}
		--assert datatype? transcode/one {#(lit-path!)}
		--assert datatype? transcode/one {#(map!)}
		--assert datatype? transcode/one {#(datatype!)}
		--assert datatype? transcode/one {#(typeset!)}
		--assert datatype? transcode/one {#(word!)}
		--assert datatype? transcode/one {#(set-word!)}
		--assert datatype? transcode/one {#(get-word!)}
		--assert datatype? transcode/one {#(lit-word!)}
		--assert datatype? transcode/one {#(refinement!)}
		--assert datatype? transcode/one {#(issue!)}
		--assert datatype? transcode/one {#(native!)}
		--assert datatype? transcode/one {#(action!)}
		--assert datatype? transcode/one {#(rebcode!)}
		--assert datatype? transcode/one {#(command!)}
		--assert datatype? transcode/one {#(op!)}
		--assert datatype? transcode/one {#(closure!)}
		--assert datatype? transcode/one {#(function!)}
		--assert datatype? transcode/one {#(frame!)}
		--assert datatype? transcode/one {#(object!)}
		--assert datatype? transcode/one {#(module!)}
		--assert datatype? transcode/one {#(error!)}
		--assert datatype? transcode/one {#(task!)}
		--assert datatype? transcode/one {#(port!)}
		--assert datatype? transcode/one {#(gob!)}
		--assert datatype? transcode/one {#(event!)}
		--assert datatype? transcode/one {#(handle!)}
		--assert datatype? transcode/one {#(struct!)}
		--assert datatype? transcode/one {#(library!)}
		--assert datatype? transcode/one {#(utype!)}
 	--test-- {direct values}
 		--assert logic? transcode/one {#(true)}
 		--assert logic? transcode/one {#(false)}
 		--assert none?  transcode/one {#(none)}
 		--assert unset? transcode/one {#(unset)}

===end-group===

===start-group=== "BINARY"
	--test-- {binary! with spaces}
		--assert #{00}   = transcode/one " #{0 0}"
		--assert #{00}   = transcode/one "2#{0000 00 00}"
		--assert #{00}   = transcode/one "2#{0000^/0000}"
		--assert #{00}   = transcode/one "2#{0000^M0000}"
		--assert #{01}   = transcode/one "2#{0000^-0001}"
		--assert #{02}   = transcode/one "2#{0000^ 0010}"
		--assert #{0001} = transcode/one "16#{00 01}"
		--assert #{0001} = transcode/one "64#{AA E=}"

	--test-- {binary! with comments inside}
	;@@ https://github.com/Oldes/Rebol-wishes/issues/23
		--assert #{00}   = transcode/one/error "#{;XXX^/00}"
		--assert #{00}   = transcode/one/error "#{00;XXX^/}"
		--assert #{0002} = transcode/one/error "#{00;XXX^/02}"
		--assert #{0002} = transcode/one/error "#{00;XXX^M02}" ;CR is also comment stopper

		--assert error? transcode/one/error "#{0}"

	--test-- {binary! with other valid escapes}
		--assert #{0003} = transcode/one/error "#{^(30)^(30)03}"
	--test-- {binary! with unicode char} ; is handled early
		--assert error? first transcode/only/error "#{0č}"
		--assert error? transcode/one/error "#{0č}"
	--test-- "Invalid binary"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1431
		--assert all [error? e: try [load {000016#{FF}}] e/id = 'invalid e/arg1 = "integer"]
		--assert all [error? e: try [load {+2#{}}] e/id = 'invalid e/arg1 = "integer"]

===end-group===


===start-group=== "Special tests"
;if "true" <> get-env "CONTINUOUS_INTEGRATION" [
	;- don't do this test on Travis CI
	;- it passes in my local tests, but fails on Travis because unknown reason
	;- WHY?

	--test-- "NULLs inside loaded string"
	;@@ https://github.com/Oldes/Rebol3/commit/6f59240d7d4379a50fec29c4e74290ad61ba73ba
		out: ""
		--assert try/with [
		;- using CALL as it could be reproduced only when the internal buffer is being extended durring load
			data: make string! 40000
			insert/dup data "ABCD" 10000

			dir: clean-path %units/files/
			save dir/tmp.data reduce [1 data]
			exe: system/options/boot
			;@@ CALL seems not to work same on all OSes :-(
			either system/platform = 'Windows [
				call/wait/output rejoin [to-local-file exe { -s } to-local-file dir/bug-load-null.r3] out
			][	call/wait/output reduce [exe "-s" dir/bug-load-null.r3] out ]

			;probe out
			parse out [thru "Test OK" to end]
		][
			probe system/state/last-error
			false
		]
		error? try [ delete dir/tmp.data ]
;]
===end-group===
~~~end-file~~~