Rebol [
	Title:   "Rebol lexer test script"
	Author:  "Oldes"
	File: 	 %lexer-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Lexer"

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

===start-group=== "Set-word"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2437
	--test-- "invalid `set-word!`"
		--assert error? try [load {:2nd}]
		--assert error? try [load {::foo}]
		--assert error? try [load {:@foo}]

===end-group===

===start-group=== "Refinement"
	;@@ https://github.com/Oldes/Rebol-issues/issues/980
	--test-- "valid `refinement!`"
		--assert refinement? try [load {/foo}]
		--assert refinement? try [load {/+}]
		--assert refinement? try [load {/!}]
		--assert refinement? try [load {/111}]
		--assert refinement? try [load {/+1}]
		

===end-group===

===start-group=== "Tag"
	--test-- "valid `tags`"
		--assert tag? load {<a '"'>} ;@@ https://github.com/Oldes/Rebol-issues/issues/1873

===end-group===

===start-group=== "Lit"
	--test-- "quote arrow-based words"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1461
		--assert lit-word? load {'<}
		--assert lit-word? load {'>}
		--assert lit-word? load {'<>}
		--assert lit-word? load {'<=}
		--assert lit-word? load {'>=}

===end-group===

===start-group=== "Integer"
	--test-- "-0"
		--assert 0 = load "-0" ;@@ https://github.com/Oldes/Rebol-issues/issues/33

===end-group===

===start-group=== "Refinement" 
	--test-- "/+"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1856
		--assert refinement? load "/+"
		--assert refinement? load "/+123"
		--assert refinement? load "/-"
		--assert refinement? load "/."
		--assert refinement? load "/.123"

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
		;@@ https://github.com/Oldes/Rebol-issues/issues/1034
		--assert error? try [load {#[string! "ab" 2 x]}]
		--assert error? try [load {#[file! "ab" x]}]
		--assert error? try [load {#[file! "ab" 2 x]}]
		--assert error? try [load {#[string! "ab" x]}]
		--assert error? try [load {#[string! "ab" 2 x]}]
		--assert error? try [load {#[file! "ab" x]}]
		--assert error? try [load {#[file! "ab" 2 x]}]

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

			any [
				exists? dir: join system/options/path %r3/src/tests/units/files/
				exists? dir: join system/options/path %../r3/src/tests/units/files/
				exists? dir: join system/options/path %../../src/tests/units/files/
			]
			probe dir: clean-path dir
			probe save %units/files/tmp.data reduce [1 data]
			probe exe: system/options/boot

			;@@ CALL seems not to work same on all OSes :-(
			either system/version/4 = 3 [
				call/wait/output probe rejoin [to-local-file exe { -s } to-local-file dir/bug-load-null.r3] out
			][	call/wait/output probe reduce [exe "-s" dir/bug-load-null.r3] out ]

			probe out
			parse out [thru "Test OK" to end]
		][
			probe system/state/last-error
			false
		]
		error? try [ delete dir/tmp.data ]
;]
===end-group===
~~~end-file~~~