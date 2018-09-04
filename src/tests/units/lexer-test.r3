Rebol [
	Title:   "Rebol lexer test script"
	Author:  "Olds"
	File: 	 %lexer-test.red
	Tabs:	 4
]

do %../quick-test.r3

~~~start-file~~~ "Lexer"

===start-group=== "SIGN before POUND char (#2319)"
	--test-- "sign-before-pound-1"	--assert  [- #"a"] = (load {-#"a"}) 
	--test-- "sign-before-pound-2"	--assert  [+ #"a"] = (load {+#"a"})
	--test-- "sign-before-pound-3"	--assert  [- #{00}]   = try [load {-#{00}}] 
	--test-- "sign-before-pound-4"	--assert  [- #[none]] = try [load {-#[none]}]
	--test-- "sign-before-pound-5"	--assert  word! = try [type? first load {+#"a"}] 
===end-group===

~~~end-file~~~