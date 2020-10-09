Rebol [
	Title:   "Rebol module! test script"
	Author:  "Peter W A Wood, Oldes"
	File: 	 %module-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "module!"

===start-group=== "module keywords"
	--test-- "hidden"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1696
		--assert all [
			[a] = words-of m: module [] [a: 1]
			m/a = 1
		]
		--assert all [
			[b] = words-of m: module [] [hidden a: 1 b: does[a + 1]]
			error? try [m/a]
			m/b = 2
		]
	--test-- "export"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1446
		unset [a b c]
		m: module [][export a: 1 b: 2 export c: does [a + b]]
		--assert all [
			object? spec: spec-of m
			[a c] = spec/exports
			m/c = 3
			unset? :a
			unset? :b
			unset? :c
			module? import m
			a = 1
			unset? :b
			c = 3
		]

		unset [a b c]
		m: module [][a: 1 b: 2  c: does [a + b] export [a c]]
		--assert all [
			object? spec: spec-of m
			[a c] = spec/exports
			m/c = 3
			unset? :a
			unset? :b
			unset? :c
			module? import m
			a = 1
			unset? :b
			c = 3
		]

		unset [a b c]
		m: module [][set export [a c] none]
		--assert all [
			object? spec: spec-of m
			[a c] = spec/exports
			none? m/c
			unset? :a
			unset? :b
			unset? :c
			module? import m
			none? a
			unset? :b
			none? c
		]



===end-group===

===start-group=== "module issues"
	--test-- "issue-1708"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1708
		--assert none? find system/modules module [] []


===end-group===

~~~end-file~~~

