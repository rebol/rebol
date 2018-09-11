Rebol [
	Title:   "Rebol lexer test script"
	Author:  "Olds"
	File: 	 %lexer-test.red
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Lexer"

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

~~~end-file~~~