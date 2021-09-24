Rebol [
	Title:   "Rebol protect test script"
	Author:  "Oldes"
	File: 	 %protect-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Protect"

===start-group=== "Checks if protected data are really protected"
	bin: #{cafe}
	str:  "cafe"
	blk: ["cafe"]
	protect/values [bin str blk]

	is-protected-error?: function[code][
		true? all [
			error? err: try code
			err/id = 'protected
		]
	]
	is-locked-error?: function[code][
		true? all [
			error? err: try code
			err/id = 'locked-word
		]
	]
	is-invalid-path-error?: function[code][
		true? all [
			error? err: try code
			err/id = 'invalid-path
		]
	]

	--test-- "clear"   --assert is-protected-error? [clear bin]
	--test-- "append"  --assert is-protected-error? [append bin #{0bad}]
	--test-- "insert"  --assert is-protected-error? [insert bin #{0bad}]

	;@@ https://github.com/Oldes/Rebol-issues/issues/2321
	--test-- "encloak" --assert is-protected-error? [encloak bin "key"]
	--test-- "decloak" --assert is-protected-error? [decloak bin "key"]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1780
	;@@ https://github.com/Oldes/Rebol-issues/issues/2272
	--test-- "remove-each" --assert is-protected-error? [remove-each a bin [a < 3]]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1780
	--test-- "random block"  --assert is-protected-error? [random blk]
	--test-- "random string" --assert is-protected-error? [random str]
	--test-- "random binary" --assert is-protected-error? [random bin]

	--test-- "swap block"  --assert is-protected-error? [swap blk [0]]
	--test-- "swap string" --assert is-protected-error? [swap str "0bad"]
	--test-- "swap binary" --assert is-protected-error? [swap bin #{0bad}]

	;@@ https://github.com/Oldes/Rebol-issues/issues/2325
	str: protect "a^M^/b"
	--test-- "deline string"  --assert is-protected-error? [deline str]
	--test-- "enline string"  --assert is-protected-error? [enline str]
	;@@ https://github.com/Oldes/Rebol-issues/issues/694
	--test-- "uppercase string"  --assert is-protected-error? [uppercase str]
	--test-- "lowercase string"  --assert is-protected-error? [lowercase str]

	--test-- "delect"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1783
	dialect: context [default: [tuple!]]
	inp: [1.2.3]
	out: make block! 4
	protect out
	--assert is-protected-error? [delect dialect inp out]

	--test-- "protect bitset"
	;@@ https://github.com/Oldes/Rebol-issues/issues/977
		ws: protect charset "^- "
		--assert is-protected-error? [clear ws  ]
		--assert is-protected-error? [ws/1: true]

	--test-- "protect inside an object"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1193
		o: make object! [a: 1 f: does [a: 2] g: does [self/a: 3]]
		protect in o 'a
		--assert is-locked-error? [o/a: 4]
		--assert is-locked-error? [o/f]
		--assert is-locked-error? [o/g]
	;@@ https://github.com/Oldes/Rebol-issues/issues/1323
		a: object [b: "abc"]
		protect/deep 'a
		--assert is-protected-error? [insert a/b "x"]
		unprotect 'a
		a: object [b: "abc"]
		protect/deep/words 'a
		--assert is-protected-error? [insert a/b "x"]
		unprotect 'a
	--test-- "protect/hide inside an object"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1139
		o: make object! [f: 1 g: self h: does [f] protect/hide 'f]
		--assert is-invalid-path-error? [do in o [self/f]]
		--assert is-invalid-path-error? [do in o [self/f: 2]]
		--assert is-invalid-path-error? [do bind [self/f] o]

	--test-- "resolve"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1095
		obj: make object! [a: 1]
		protect/deep obj
		--assert is-protected-error? [resolve obj make object! [a: 99]]
		a: make object! [pass: "1234" getp: does [return pass] protect/hide 'pass]
		--assert is-invalid-path-error? [a/pass]
		--assert object? resolve a make object! [pass: 999]
		--assert "1234" = a/getp

===end-group===


===start-group=== "protect/hide"
	--test-- "trim object! with hidden fields"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1137
		--assert 0 = length? trim make object! [f: 1 protect/hide 'f]
		--assert 0 = length? trim make object! [f: none protect/hide 'f]
	--test-- "resolve/all hidden"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1769
		con1: context [a: 1 b: 2 c: "hidden" protect/hide 'c]
		con2: context [a: 9 b: 8 c: none]
		--assert object? resolve/all con2 con1
		--assert all [con2/a = 1 con2/b = 2]
		--assert none? con2/c ;- not holding "hidden"!

===end-group===

~~~end-file~~~