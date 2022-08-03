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
	;@@ https://github.com/Oldes/Rebol-issues/issues/688
		--assert all [
			block? code: transcode to binary! "1 + 1"
			code = [1 + 1 #{}]
			#{31202B2031} = head last code
		]
		--assert all [
			block? code: transcode "1 + 1"
			code = [1 + 1 ""]
			"1 + 1" = head last code
		]
	--test-- "transcode/next"
	;@@ https://github.com/Oldes/Rebol-issues/issues/535
		--assert [1 " + 1"]      = transcode/next "1 + 1"
		--assert [1 #{202B2031}] = transcode/next to binary! "1 + 1"
		--assert [[1 + 1] ""]    = transcode/next "[1 + 1]"
		--assert [[1 + 1] #{}]   = transcode/next to binary! "[1 + 1]"

===end-group===

===start-group=== "Invalid construction"
	--test-- "Invalid MAP"
		--assert error? err: try [load {#(x)}]
		--assert err/id = 'invalid-arg
		--assert error? err: try [load {#(x}]
		--assert all [err/id = 'missing err/arg1 = "end-of-script" err/arg2 = ")"]

	--test-- "Invalid word"
	;@@ https://github.com/Oldes/Rebol-issues/issues/22
		--assert error? try [load "':a"]
		--assert error? try [load "':a:"]

	--test-- "Invalid issue!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/122
		--assert error? try [load "1234#"]
	;@@ https://github.com/oldes/rebol-issues/issues/2297
		--assert error? try [load {#}] ; originally (in R3-alpha) it was same like #[none]

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
		--assert all [error? e: try [load {1#[logic! 1]}] e/id = 'invalid]
		--assert all [error? e: try [load {a#[logic! 1]}] e/id = 'invalid]

	--test-- "Invalid char"
		--assert all [error? e: try [load {2#"a"}] e/id = 'invalid]

	--test-- "Invalid path construction"
	;@@ https://github.com/Oldes/Rebol-issues/issues/863
		--assert all [error? e: try [load {#[path! [0]]}] e/id = 'malconstruct]

	--test-- "Invalid file"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1415
		--assert all [error? e: try [load {%^^}] e/id = 'invalid]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1442
		--assert all [error? e: try [load {%a^^b}] e/id = 'invalid]
		--assert all [error? e: try [load {%a^^ }] e/id = 'invalid]

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


===end-group===


===start-group=== "Integer"
	--test-- "-0"
		--assert 0 = load "-0" ;@@ https://github.com/Oldes/Rebol-issues/issues/33

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
	--test-- "sign-before-pound-4"	--assert  [- #[none]] = try [load {-#[none]}]
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
		--assert "ab" = load {#[string! "ab"]}
		--assert  "b" = load {#[string! "ab" 2]}
		--assert %ab  = load {#[file! "ab"]}
		--assert  %b  = load {#[file! "ab" 2]}
		--assert struct? load {#[struct! []]}
		;@@ https://github.com/Oldes/Rebol-issues/issues/1034
		--assert error? try [load {#[string! "ab" 2 x]}]
		--assert error? try [load {#[file! "ab" x]}]
		--assert error? try [load {#[file! "ab" 2 x]}]
		--assert error? try [load {#[string! "ab" x]}]
		--assert error? try [load {#[string! "ab" 2 x]}]
		--assert error? try [load {#[file! "ab" x]}]
		--assert error? try [load {#[file! "ab" 2 x]}]
	--test-- {object!}
		;@@ https://github.com/Oldes/Rebol-issues/issues/864
		--assert block? try [transcode      "#[object! [a: 1 b: 2]]"]
		--assert block? try [transcode/only "#[object! [a: 1 b: 2]]"]
	--test-- {function!}
		;@@ https://github.com/Oldes/Rebol-issues/issues/1114
		--assert function? first transcode {#[function! [[a [series!]][print a]]]}

	--test-- {datatype!}
		;@@ https://github.com/Oldes/Rebol-issues/issues/2508
		--assert datatype? first transcode/only {#[unset!]}
		--assert datatype? first transcode/only {#[none!]}
		--assert datatype? first transcode/only {#[logic!]}
		--assert datatype? first transcode/only {#[integer!]}
		--assert datatype? first transcode/only {#[decimal!]}
		--assert datatype? first transcode/only {#[percent!]}
		--assert datatype? first transcode/only {#[money!]}
		--assert datatype? first transcode/only {#[char!]}
		--assert datatype? first transcode/only {#[pair!]}
		--assert datatype? first transcode/only {#[tuple!]}
		--assert datatype? first transcode/only {#[time!]}
		--assert datatype? first transcode/only {#[date!]}
		--assert datatype? first transcode/only {#[binary!]}
		--assert datatype? first transcode/only {#[string!]}
		--assert datatype? first transcode/only {#[file!]}
		--assert datatype? first transcode/only {#[email!]}
		--assert datatype? first transcode/only {#[ref!]}
		--assert datatype? first transcode/only {#[url!]}
		--assert datatype? first transcode/only {#[tag!]}
		--assert datatype? first transcode/only {#[bitset!]}
		--assert datatype? first transcode/only {#[image!]}
		--assert datatype? first transcode/only {#[vector!]}
		--assert datatype? first transcode/only {#[block!]}
		--assert datatype? first transcode/only {#[paren!]}
		--assert datatype? first transcode/only {#[path!]}
		--assert datatype? first transcode/only {#[set-path!]}
		--assert datatype? first transcode/only {#[get-path!]}
		--assert datatype? first transcode/only {#[lit-path!]}
		--assert datatype? first transcode/only {#[map!]}
		--assert datatype? first transcode/only {#[datatype!]}
		--assert datatype? first transcode/only {#[typeset!]}
		--assert datatype? first transcode/only {#[word!]}
		--assert datatype? first transcode/only {#[set-word!]}
		--assert datatype? first transcode/only {#[get-word!]}
		--assert datatype? first transcode/only {#[lit-word!]}
		--assert datatype? first transcode/only {#[refinement!]}
		--assert datatype? first transcode/only {#[issue!]}
		--assert datatype? first transcode/only {#[native!]}
		--assert datatype? first transcode/only {#[action!]}
		--assert datatype? first transcode/only {#[rebcode!]}
		--assert datatype? first transcode/only {#[command!]}
		--assert datatype? first transcode/only {#[op!]}
		--assert datatype? first transcode/only {#[closure!]}
		--assert datatype? first transcode/only {#[function!]}
		--assert datatype? first transcode/only {#[frame!]}
		--assert datatype? first transcode/only {#[object!]}
		--assert datatype? first transcode/only {#[module!]}
		--assert datatype? first transcode/only {#[error!]}
		--assert datatype? first transcode/only {#[task!]}
		--assert datatype? first transcode/only {#[port!]}
		--assert datatype? first transcode/only {#[gob!]}
		--assert datatype? first transcode/only {#[event!]}
		--assert datatype? first transcode/only {#[handle!]}
		--assert datatype? first transcode/only {#[struct!]}
		--assert datatype? first transcode/only {#[library!]}
		--assert datatype? first transcode/only {#[utype!]}
 	--test-- {direct values}
 		--assert logic? first transcode/only {#[true]}
 		--assert logic? first transcode/only {#[false]}
 		--assert none?  first transcode/only {#[none]}
 		--assert unset? first transcode/only {#[unset]}

===end-group===

===start-group=== "BINARY"
	--test-- {binary! with spaces}
		--assert #{00}   = first transcode/only " #{0 0}"
		--assert #{00}   = first transcode/only "2#{0000 00 00}"
		--assert #{00}   = first transcode/only "2#{0000^/0000}"
		--assert #{00}   = first transcode/only "2#{0000^M0000}"
		--assert #{01}   = first transcode/only "2#{0000^-0001}"
		--assert #{02}   = first transcode/only "2#{0000^ 0010}"
		--assert #{0001} = first transcode/only "16#{00 01}"
		--assert #{0001} = first transcode/only "64#{AA E=}"

	--test-- {binary! with comments inside}
	;@@ https://github.com/Oldes/Rebol-wishes/issues/23
		--assert #{00}   = first transcode/only/error "#{;XXX^/00}"
		--assert #{00}   = first transcode/only/error "#{00;XXX^/}"
		--assert #{0002} = first transcode/only/error "#{00;XXX^/02}"
		--assert #{0002} = first transcode/only/error "#{00;XXX^M02}" ;CR is also comment stopper
	--test-- {binary! with other valid escapes}
		--assert #{0003} = first transcode/only/error "#{^(30)^(30)03}"
	--test-- {binary! with unicode char} ; is handled early
		--assert error? first transcode/only/error "#{0č}"
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
		--assert try/except [
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