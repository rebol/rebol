Rebol [
	Title:   "Rebol MOLD test script"
	Author:  "bitbegin"
	File: 	 %mold-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

#include  %../../../quick-test/quick-test.red

~~~start-file~~~ "MOLD"

===start-group=== "string-basic"

	--test-- "mold-string-baseic-1"
		a: ""				;-- literal: ""
		b: {""}
		--assert b = mold a

	--test-- "mold-string-baseic-2"
		a: {}				;-- literal: ""
		b: {""}
		--assert b = mold a

	--test-- "mold-string-baseic-3"
		a: "{}"				;-- literal: "{}"
		b: {"{}"}
		--assert b = mold a

	--test-- "mold-string-baseic-4"
		a: {""}				;-- literal: {""}
		b: {{""}}
		--assert b = mold a

	--test-- "mold-string-baseic-5"
		a: "{"				;-- literal: "{"
		b: {"^{"}
		--assert b = mold a

	--test-- "mold-string-baseic-6"
		a: "}"				;-- literal: "}"
		b: {"^}"}
		--assert b = mold a

	--test-- "mold-string-baseic-7"
		a: {"}				;-- literal: {"}
		b: {{"}}
		--assert b = mold a

	--test-- "mold-string-baseic-8"
		a: {""}				;-- literal: {""}
		b: {{""}}
		--assert b = mold a

	--test-- "mold-string-baseic-9"
		a: "}{"				;-- literal: "}{"
		b: {"^}^{"}
		--assert b = mold a

	--test-- "mold-string-baseic-10"
		a: "^""				;-- literal: {"}
		b: {{"}}
		--assert b = mold a

	--test-- "mold-string-baseic-11"
		a: "^"{"			;-- literal: {"^{}
		b: {^{"^^^{^}}
		--assert b = mold a

	--test-- "mold-string-baseic-12"
		a: "^"{}"			;-- literal: {"{}}
		b: {{"{}}}
		--assert b = mold a

	--test-- "mold-string-baseic-13"
		a: "^"}{"			;-- literal: {"^}^{}
		b: {{"^^}^^{}}
		--assert b = mold a

	--test-- "mold-string-baseic-14"
		a: {^{}				;-- literal: "{"
		b: {"^{"}
		--assert b = mold a

	--test-- "mold-string-baseic-15"
		a: {^{"}			;-- literal: {^{"}
		b: {^{^^^{"^}}
		--assert b = mold a

	--test-- "mold-string-baseic-16"
		a: "{{{"			;-- literal: "{{{"
		b: {"^{^{^{"}
		--assert b = mold a

	--test-- "mold-string-baseic-17"
		a: "}}}"			;-- literal: "}}}"
		b: {"^}^}^}"}
		--assert b = mold a

	--test-- "mold-string-baseic-18"
		a: "{{{}}}}"		;-- literal: "{{{}}}}"
		b: {"{{{}}}^}"}
		--assert b = mold a

	--test-- "mold-string-baseic-19"
		a: "}{}"			;-- literal: "}{}"
		b: {"^}{}"}
		--assert b = mold a

	--test-- "mold-string-baseic-20"
		a: "}{{}"			;-- literal: "}{{}"
		b: {"^}^{^{^}"}
		--assert b = mold a

	--test-- "mold-string-baseic-21"
		a: "}{{}}"			;-- literal: "}{{}}"
		b: {"^}{{}}"}
		--assert b = mold a

	--test-- "mold-string-baseic-22"
		a: "{}{"			;-- literal: "{}{"
		b: {"{}^{"}
		--assert b = mold a

	--test-- "mold-string-baseic-23"
		a: "{}{}{"			;-- literal: "{}{"
		b: {"{}{}^{"}
		--assert b = mold a

	--test-- "mold issue #1486"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1486
		--assert {"^^(1E)"} = mold "^(1E)" ; double ^ because it's a string in a string

===end-group=== 

===start-group=== "mold string!"
	
	--test-- "mold-string-1"
		a: "abc"			;-- literal: "abc"
		b: {"abc"}
		--assert b = mold a

	--test-- "mold-string-2"
		a: "a^"bc"			;-- literal: {a"bc}
		b: {{a"bc}}
		--assert b = mold a

	--test-- "mold-string-3"
		a: "a{bc"			;-- literal: "a{bc"
		b: {"a^{bc"}
		--assert b = mold a

	--test-- "mold-string-4"
		a: "a}{bc"			;-- literal: "a}{bc"
		b: {"a^}^{bc"}
		--assert b = mold a

	--test-- "mold-string-5"
		a: "a}{bc"			;-- literal: "a}{bc"
		b: {{"a^^}^^{bc"}}
		--assert b = mold mold a

	--test-- "mold-string-6"
		a: "a^"b^"c"		;-- literal: {a"b"c}
		b: {{a"b"c}}
		--assert b = mold a

	--test-- "mold-string-7"
		a: "a{}bc"			;-- literal: "a{}bc"
		b: {"a{}bc"}
		--assert b = mold a

	--test-- "mold/part string!"
		--assert     {"a} = mold/part "abcd" 2
		--assert       "" = mold/part "abcd" 0
		--assert       "" = mold/part "abcd" -2
		--assert {"abcd"} = mold/part "abcd" 10

===end-group=== 

===start-group=== "mold url!"
	
	--test-- "mold url"
		--assert "ftp://"  = mold ftp://
		--assert "ftp://š" = mold ftp://š
		--assert "ftp://+" = mold ftp://+
		--assert "ftp://+" = mold ftp://%2b
		--assert "ftp://+" = mold ftp://%2B
		--assert "ftp://%20" = mold ftp://%20
	--test-- "mold append url"
		--assert "ftp://a" = mold append ftp:// #"a"
		--assert "ftp://a" = mold append ftp://  "a"
		--assert "ftp://š" = mold append ftp://  "š"
		--assert "ftp://+" = mold append ftp://  "+"
		--assert "ftp://%2528" = mold append ftp:// "%28"
		--assert "ftp://%28" = dehex mold append ftp:// "%28"

===end-group=== 


===start-group=== "mold-all"
	
	--test-- "mold-true" --assert "true" = mold true

	--test-- "mold-all-true" --assert "#[true]" = mold/all true

	--test-- "mold-false" --assert "false" = mold false

	--test-- "mold-all-false" --assert "#[false]" = mold/all false

	--test-- "mold-none" --assert "none" = mold none

	--test-- "mold-all-none" --assert "#[none]" = mold/all none

	--test-- "mold-block" --assert "[true false none]" = mold [#[true] #[false] #[none]]

	--test-- "mold-all-block"
		--assert "[#[true] #[false] #[none]]" = mold/all [#[true] #[false] #[none]]

	--test-- "mold/all block at tail"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1192
		--assert "#[path! [p p] 3]" = mold/all next next 'p/p
		--assert "#[block! [a b] 3]" = mold/all next next [a b]

===end-group=== 


===start-group=== "mold object!"

	--test-- "mold/flat object!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2357
		o: make object! [
			a: [ 1 2 
				 3 4
			]
		]
		--assert "make object! [a: [1 2 3 4]]" = mold/flat o

	--test-- "mold system"
		--assert "make" = mold/part system 4
		--assert (mold/part system 12) = (copy/part mold system 12)

===end-group===

===start-group=== "mold event!"

	--test-- "mold/flat event!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2387
		--assert "make event! [type: 'lookup]" = mold/flat make event! [type: 'lookup]

	--test-- "issues/2362"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2362
		p: make object! [x: "foo"]
		e: make event! [type: 'lookup port: p]
		--assert (mold/flat e) = {make event! [type: 'lookup port: make object! [x: "foo"]]}
		e: make event! [type: 'move offset: 10x20]
		--assert (mold/flat e) = {make event! [type: 'move offset: 10x20]}
		
		; only code or offset may be used!
		e: make event! [type: 'move code: 100 offset: 10x20]
		--assert (mold/flat e) = {make event! [type: 'move offset: 10x20]}
		e: make event! [type: 'move offset: 10x20 code: 100]
		--assert (mold/flat e) = {make event! [type: 'move code: 100]}

===end-group===

===start-group=== "mold block!"
	b: [
		1 2
		3 4
	]
	c: reduce [
		"A"
		b
	]
	--test-- "mold/flat block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2388
		--assert (mold/flat b) = "[1 2 3 4]" 
		--assert (mold/flat c) = {["A" [1 2 3 4]]}

	--test-- "mold/flat/all block!"
		--assert (mold/flat/all c) = {["A" [1 2 3 4]]}
		--assert (mold/flat/all next c) = {#[block! ["A" [1 2 3 4]] 2]}

	--test-- "mold/only"
	;@@ https://github.com/Oldes/Rebol-issues/issues/732
		--assert "1 2^/3 4" = mold/only b

	--test-- "mold/part block!"
		--assert "[1 2" = mold/part/flat b 4
		--assert "[1 2 3 4]" = mold/part/flat b 100

	--test-- "issue-1439"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1439
		--assert "1^/2^/3" = mold/only load "[1^/2^/3^/]"
		--assert "1 2^/3^/4" = mold/only load "[1 2^/3^/4^/]"
		--assert "1 2^/3^/4^/5" = mold/only load "[1 2^/3^/4^/5]"

	--test-- "issue-2279"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2279
		--assert "[[[^/    1^/]]]" = mold load {[[[^/1^/]]]}

===end-group===

===start-group=== "mold map!"
	--test-- "mold map!"
		m: make map! [
			a: 1
			b: 2
			c: [
				3 4
			]
		]

		--assert (mold m) = {#(
    a: 1
    b: 2
    c: [
        3 4
    ]
)}

	--test-- "mold/flat map!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2401
		--assert       "#(a: 1 b: 2 c: [3 4])"  = mold/flat m
		--assert "#[map! [a: 1 b: 2 c: [3 4]]]" = mold/flat/all m

	--test-- "mold with recursive value"
		m/c: m
		--assert "#(a: 1 b: 2 c: #(...))" = mold/flat m

	--test-- "mold #()"
		;@@ https://github.com/Oldes/Rebol-issues/issues/725
		--assert "#()" = mold #()
		--assert "#[map! []]" = mold/all #()

===end-group===

===start-group=== "mold binary!"
	bb: system/options/binary-base ;store original value
	bin: #{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}
	--test-- "mold/flat binary!"
		system/options/binary-base: 2
		--assert (mold/flat bin) = {2#{11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111}}
		system/options/binary-base: 16
		--assert (mold/flat bin) = {#{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}}
		system/options/binary-base: 64
		--assert (mold/flat bin) = {64#{/////////////////////////////////////////////w==}}

	--test-- "mold/part binary!"
		bin: to-binary make image! 2 * 1920x1080
		system/options/binary-base: 2
		--assert "2#{11111" = mold/part bin 8
		system/options/binary-base: 16
		--assert "#{FFFFFF" = mold/part bin 8
		system/options/binary-base: 64
		--assert "64#{////" = mold/part bin 8

	; restore options
	bin: none
	system/options/binary-base: bb
===end-group===

===start-group=== "form binary!"
	;-- form on binary! removes decoration..
	;@@ https://github.com/Oldes/Rebol-issues/issues/2413
	;@@ https://github.com/Oldes/Rebol-issues/issues/1999
	--test-- "form binary!"
		--assert "DEADBEEF" = form #{DEADBEEF}
		--assert "DEADBEEF" = append "" #{DEADBEEF}
		--assert     "BEEF" = form skip #{DEADBEEF} 2

===end-group===

===start-group=== "mold email!"
	--test-- "issue-159"
	;@@ https://github.com/Oldes/Rebol-issues/issues/159
		--assert "a@b" = mold a@b
	--test-- "issue-2406"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2406
		--assert "a@šiška" = mold a@šiška
		--assert "a@š" = mold a@%C5%A1
===end-group===

===start-group=== "mold ref!"
	--test-- "mold ref"
		--assert "@name"  = mold @name
		--assert "@ame"   = mold next @name
		--assert "@šiška" = mold @šiška
		--assert {#[ref! "a@"]} = mold to ref! "a@"
		--assert {#[ref! "a^^/b"]} = mold append @a "^/b"
	--test-- "form ref"
		--assert "name"  = form @name
		--assert "ame"   = form next @name
		--assert "šiška" = form @šiška
		--assert {a@}    = form to ref! "a@"
		--assert {a^/b}  = form append @a "^/b"

===end-group===

===start-group=== "mold image!"

	--test-- "mold empty image"
		--assert "make image! [0x0 #{}]"  = mold make image! 0x0
		--assert "make image! [10x0 #{}]" = mold make image! 10x0
		--assert "make image! [0x10 #{}]" = mold make image! 0x10

	--test-- "mold small image"
	;@@ https://github.com/Oldes/Rebol3/issues/13
		--assert (mold make image! 2x2) = {make image! [2x2 #{FFFFFFFFFFFFFFFFFFFFFFFF}]}

	--test-- "mold/flat image!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2389
		--assert (mold/flat make image! 8x1) = {make image! [8x1 #{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}]}
		--assert (mold/flat make image! 8x2) = {make image! [8x2 #{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}]}
		--assert (mold/flat make image! [1x1 0.0.0.66]) = {make image! [1x1 #{000000} #{42}]}

	--test-- "mold/flat/all image!"
		--assert (mold/all/flat make image! 8x1) = {#[image! 8x1 #{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}]}
		--assert (mold/all/flat next make image! 8x1) = {#[image! 8x1 #{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF} 2]}

	--test-- "mold/part image!"
		img: make image! 2 * 1920x1080
		--assert "make image! [3840x2160 #{FFFFF" = mold/part img 30
		--assert "#[image! 3840x2160 #{FFFFFFFFF" = mold/part/all img 30

===end-group===

===start-group=== "mold closure!"

	--test-- "mold/flat closure!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/23
		x: closure [a] [print a]
		--assert "make closure! [[a /local][print a]]" = mold/flat :x

===end-group===

===start-group=== "mold gob!"

	--test-- "mold gob!"
		--assert "make gob! [offset: 0x0 size: 100x100]" = mold make gob! []

	--test-- "mold/all gob!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/989
		--assert "#[gob! [offset: 0x0 size: 100x100]]" = mold/all make gob! []

===end-group===

===start-group=== "mold unset!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/567
	--test-- "mold unset!"
		--assert   "unset!"  = mold ()
		--assert "#[unset!]" = mold/all ()
	--test-- "form unset!"
		--assert "" = form ()

===end-group===


===start-group=== "mold/all"
	--test-- "mold/all datatype!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/219
		--assert "#[datatype! integer!]" = mold/all integer!

	--test-- "mold/all decimal!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1633
		--assert "0.10000000000000001" = mold/all 0.1
		--assert "0.29999999999999999" = mold/all 0.3

===end-group===

===start-group=== "form error!"
	--test-- "form error!"
		; no ANSI escape sequence!
		--assert parse (form try [1 / 0]) [
			{** Math error: attempt to divide by zero^/}
			{** Where: / try} thru #"^/"
			{** Near: / 0^/}
		]

===end-group===


===start-group=== "mold/form path!"
	--test-- "mold/form path!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/488
		--assert "a/b/c" = mold 'a/b/c
		--assert "a/b/c" = form 'a/b/c

	--test-- "to-path from block with newlines"
	;@@ https://github.com/Oldes/Rebol-issues/issues/493
		--assert "a/b/c" = mold to-path [
		    a
		    b
		    c
		]

===end-group===

~~~end-file~~~
