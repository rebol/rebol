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

===end-group===

===start-group=== "Email"
	--test-- "valid `emails`"
		--assert email? load {name@where}
		--assert email? load {@name} ;@@ https://github.com/Oldes/Rebol-issues/issues/1962
		--assert email? load {a@šiška}
		--assert email? load {@%C5%A1}

===end-group===

===start-group=== "Tag"
	--test-- "valid `tags`"
		--assert tag? load {<a '"'>} ;@@ https://github.com/Oldes/Rebol-issues/issues/1873

===end-group===

===start-group=== "Integer"
	--test-- "-0"
		--assert 0 = load "-0" ;@@ https://github.com/Oldes/Rebol-issues/issues/33

===end-group===

===start-group=== "SIGN before POUND char (issue #2319)"
;@@ https://github.com/rebol/rebol-issues/issues/2319
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


===start-group=== "Special tests"

	--test-- "NULLs inside loaded string"
	;@@ https://github.com/Oldes/Rebol3/commit/6f59240d7d4379a50fec29c4e74290ad61ba73ba
		out: ""
		--assert not error? set/any 'err try [
		;- using CALL as it could be reproduced only when the internal buffer is being extended durring load
			data: make string! 40000
			insert/dup data "ABCD" 10000

			any [
				exists? dir: join system/options/path %r3/src/tests/units/files/
				exists? dir: join system/options/path %../r3/src/tests/units/files/
				exists? dir: join system/options/path %../../src/tests/units/files/
			]
			save dir/tmp.data reduce [1 data]
			exe: rejoin [system/options/home last split-path system/options/boot]

			;@@ CALL seems not to work same on all OSes :-(
			either system/version/4 = 3 [
				call/wait/output probe rejoin [to-local-file exe { -s } to-local-file dir/bug-load-null.r3] out
			][	call/wait/output probe reduce [exe dir/bug-load-null.r3] out ]

			probe out
		]
		if error? err [ probe err ]
		--assert parse out ["Test OK" to end]
		error? try [ delete dir/tmp.data ]

===end-group===
~~~end-file~~~