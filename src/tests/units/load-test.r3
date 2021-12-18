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

	--test-- "issue-655"
	;@@ https://github.com/Oldes/Rebol-issues/issues/655
		o: first load/header "rebol [a: true b: yes c: no d: false e: none f: foo] print 'hello"
		--assert all [logic? o/a logic? o/b logic? o/c logic? o/d none? o/e word? o/f]

	--test--"Script checksum verification"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1685
		s: save/header none [print "Hello REBOL!"] [checksum: true]
		--assert [print "Hello REBOL!"] = try [load s]
		; corrupt the script...
		clear at s 88
		--assert all [
			error? e: try [load s]
			e/id = 'bad-checksum
		]
	--test-- "Save/length"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1684
		s: sys/load-header save/length none [print "Hello REBOL!"]
		--assert s/1/length = 21
		--assert s/2 = #{7072696E74202248656C6C6F205245424F4C21220A}

===end-group===

===start-group=== "Load issues/wishes"
	--test-- "load UTF8 string from binary with BOM"
	;@@ https://github.com/red/red/issues/5000
		--assert "3" = load #{223322}
		--assert "3" = load #{EFBBBF223322}

	--test-- "Load of a block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/691
		--assert block? b: load ["print 'hello" "print 'there"]
		--assert [print 'hello] = b/1
		--assert [print 'there] = b/2

	--test-- "Length-specified script embedding"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1941
		--assert [print "hello"] = load {rebol [length: 14] print "hello" other stuff}
		--assert 1 = try [do {rebol [length: 2] 1 other stuff}]
		--assert [lib-local a] = words-of import {rebol [length: 5] a: 1 b: 2 print "evil code"}
		--assert [lib-local a] = words-of import/check {rebol [length: 5 checksum: #{E9A16FDEC8FF093599E2AA10C30D2D98D1C541C5}] a: 1 b: 2 print "evil code"} #{E9A16FDEC8FF093599E2AA10C30D2D98D1C541C5}

	--test-- "issue-858"
	;@@ https://github.com/Oldes/Rebol-issues/issues/858
		--assert [<] = load mold [ < ]
	--test-- "issue-658"
	;@@ https://github.com/Oldes/Rebol-issues/issues/658
		--assert [[print now]] = load [{rebol [] print now}]
	--test-- "issue-234"
	;@@ https://github.com/Oldes/Rebol-issues/issues/234
		--assert [] = load to-string []
	--test-- "issue-2435"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2435
		--assert all [
			error? e: try [load #{789DE3}] ;- no crash!
			e/id = 'invalid-chars
		]

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


===start-group=== "transcode"
	--test-- "issue-1594"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1594
		--assert error? try [transcode to-binary {["test^/]}]
		--assert all [
			block? e: transcode/error to-binary {"test^/}
			error? e/1
			e/2 = #{0A}
		]
		--assert all [
			block? e: transcode/error to-binary {["test^/]}
			error? e/1
			e/2 = #{0A5D}
		]
===end-group===


===start-group=== "SAVE"
	data: [1 1.2 10:20 "test" user@example.com [sub block]]
	--test-- "save to none"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1464
		--assert #{310A} = save none 1
		--assert #{0A} = save none [] ;@@ or better #{} ?
		bin: save none data
		--assert bin = #{
3120312E322031303A3230202274657374222075736572406578616D706C652E
636F6D205B73756220626C6F636B5D0A}
		--assert data = load bin
	--test-- "save to binary"
		b: #{}
		--assert #{310A} = save b 1
		--assert #{310A320A} = save b 2
		--assert #{310A320A237B30337D0A} = save b #{03}

	--test-- "save/header"
		bin: save/header none data [title: "my code"]
		--assert bin = #{
5245424F4C205B0A202020207469746C653A20226D7920636F6465220A5D0A31
20312E322031303A3230202274657374222075736572406578616D706C652E63
6F6D205B73756220626C6F636B5D0A} 
		--assert data = load bin

		;@@ https://github.com/Oldes/Rebol-issues/issues/1465
		bin: save/header none [1] true
		--assert bin = #{5245424F4C205B5D0A310A}
		--assert 1 = load bin
===end-group===


~~~end-file~~~

