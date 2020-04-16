Rebol [
	Title:   "Rebol loading test script"
	Author:  "Peter W A Wood, Oldes"
	File: 	 %load-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
	Rights:  "Copyright (C) 2011-2015 Red Foundation. All rights reserved."
	License: "BSD-3 - https://github.com/red/red/blob/origin/BSD-3-License.txt"
]

~~~start-file~~~ "load"

===start-group=== "Load/all"
	--test-- "issue-20"
	;@@ https://github.com/Oldes/Rebol-issues/issues/20
		v: mold/all next "123"
		--assert block? v: load/all v
		--assert  "23" = v/1
		--assert "123" = head v/1

===end-group===

===start-group=== "Load/header"
	--test-- "issue-663"
	;@@ https://github.com/Oldes/Rebol-issues/issues/663
		--assert block? b: load/header "1"
		--assert none? b/1
		--assert 1 = b/2

		--assert block? b: load/header "rebol [title: {foo}] 1"
		--assert object? b/1
		--assert "foo" = b/1/title

===end-group===

===start-group=== "Load issues/wishes"
	--test-- "Load of a block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/691
		--assert block? b: load ["print 'hello" "print 'there"]
		--assert [print 'hello] = b/1
		--assert [print 'there] = b/2
	--test-- "issue-858"
	;@@ https://github.com/Oldes/Rebol-issues/issues/858
		--assert [<] = load mold [ < ]
	--test-- "issue-658"
	;@@ https://github.com/Oldes/Rebol-issues/issues/658
		--assert [[print now]] = load [{rebol [] print now}]
	--test-- "issue-234"
	;@@ https://github.com/Oldes/Rebol-issues/issues/234
		--assert [] = load to-string []
===end-group===


===start-group=== "find-script native"
	--test-- "find-script"
	;@@ https://github.com/Oldes/Rebol-issues/issues/182
		--assert 1 = index? find-script to-binary {Rebol [] print now}
		--assert 2 = index? find-script to-binary { Rebol[] print now}
		--assert 2 = index? find-script to-binary { Rebol [] print now}
		--assert 2 = index? find-script to-binary {^/Rebol [] print now}
		--assert 2 = index? find-script to-binary {^/REBOL [] print now}
		--assert 4 = index? find-script to-binary {   Rebol [] print now}
		--assert 4 = index? find-script to-binary {   RebOL [] print now}
		--assert 6 = index? find-script to-binary {^/bla^/Rebol [] print now}
		--assert none? find-script to-binary {bla Rebol [] print now}
===end-group===


~~~end-file~~~

