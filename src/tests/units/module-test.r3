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
		unset in system/contexts/user 'a
		unset in system/contexts/user 'b
		unset in system/contexts/user 'c
		m: module [][export a: 1 b: 2 export c: does [a + b]]
		--assert all [
			object? spec: spec-of m
			[a c] = spec/exports
			m/c = 3
			unset? :system/contexts/user/a
			unset? :system/contexts/user/b
			unset? :system/contexts/user/c
			module? import m
			system/contexts/user/a = 1
			unset? :system/contexts/user/b
			system/contexts/user/c = 3
		]

		unset in system/contexts/user 'a
		unset in system/contexts/user 'b
		unset in system/contexts/user 'c
		m: module [][a: 1 b: 2  c: does [a + b] export [a c]]
		--assert all [
			object? spec: spec-of m
			[a c] = spec/exports
			m/c = 3
			unset? :system/contexts/user/a
			unset? :system/contexts/user/b
			unset? :system/contexts/user/c
			module? import m
			system/contexts/user/a = 1
			unset? :system/contexts/user/b
			system/contexts/user/c = 3
		]

		unset in system/contexts/user 'a
		unset in system/contexts/user 'b
		unset in system/contexts/user 'c
		m: module [][set export [a c] none]
		--assert all [
			object? spec: spec-of m
			[a c] = spec/exports
			none? m/c
			unset? :system/contexts/user/a
			unset? :system/contexts/user/b
			unset? :system/contexts/user/c
			module? import m
			none? system/contexts/user/a
			unset? :system/contexts/user/b
			none? system/contexts/user/c
		]


===end-group===

===start-group=== "module import"
	--test-- "import/version"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1687
		m: module [version: 1.0.0 name: 'Foo][a: 1]
		--assert all [
			error? e: try [import/version m 2.2.2]
			e/id = 'needs
		]
		--assert module? import/version m 1.0.0

===end-group===

===start-group=== "module issues"
	--test-- "issue-1005"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1005
		m: module [] [a: 1 2]
		--assert [ ] = body-of m
		--assert [a] = keys-of m
		--assert [1] = values-of m
		m: module [exports: [a]] [a: 1 2]
		--assert [a: 1] = body-of m
		--assert [a] = keys-of m
		--assert [1] = values-of m

	--test-- "issue-1708"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1708
		--assert none? find system/modules module [] []

===end-group===

~~~end-file~~~

