Rebol [
	Title:   "Rebol module! test script"
	Author:  "Peter W A Wood, Oldes"
	File: 	 %module-test.r3
	Tabs:	 4
	Needs:   quick-test
]

~~~start-file~~~ "module!"

; extend module-paths with units/files/ directory
; so modules there can be located
supplement system/options/module-paths join what-dir %units/files/


===start-group=== "module keywords"
	--test-- "hidden"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1696
		--assert all [
			[lib-local a] = words-of m: module [] [a: 1]
			m/a = 1
		]
		--assert all [
			[lib-local b] = words-of m: module [] [hidden a: 1 b: does[a + 1]]
			error? try [m/a]
			m/b = 2
		]

	--test-- "export"
	;@@ https://github.com/Oldes/Rebol-issues/issues/689
	;@@ https://github.com/Oldes/Rebol-issues/issues/1446
		unset in system/contexts/user 'a
		unset in system/contexts/user 'b
		unset in system/contexts/user 'c
		m: module [][export a: 1 b: 2 export c: does [a + b]]
		--assert all [
			object? spec: spec-of m ;@@ https://github.com/Oldes/Rebol-issues/issues/1006
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
	--test-- "select"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1380
		m: module [] [a: 1]
		--assert 1 = select m 'a
		--assert none? select m 'b


===end-group===

===start-group=== "make module"
	--test-- "title-of"
	;@@ https://github.com/Oldes/Rebol-issues/issues/572
		test: module [Title: "Test Module"] []
		--assert "Test Module" = try [title-of test]
		
	--test-- "inlined module"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1628
		unset 'z1 z2: 2
		--assert all [
			error? e: try [do "rebol [type: module] attempt [z1: 12345] z1"]
			e/id = 'not-defined
			e/arg1 = 'z1
		]
		--assert all [
			error? e: try [do "rebol [type: module] attempt [z2: 12345] z2"]
			e/id = 'not-defined
			e/arg1 = 'z2
		]
		--assert all [
			error? e: try [do "rebol [] module [] [attempt [z1: 12345] z1]"]
			e/id = 'not-defined
			e/arg1 = 'z1
		]
		--assert all [
			error? e: try [do "rebol [] module [] [attempt [z2: 12345] z2]"]
			e/id = 'not-defined
			e/arg1 = 'z2
		]
	--test-- "module body modified on export"
		module [] body: [export 'b]
		--assert body = ['b]

	--test-- "make module type assertion"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1444
		--assert all [
			error? err: try [module [type: :print][]]
			err/id = 'wrong-type
		]
	--test-- "make module! integer!" ; not allowed
	;@@ https://github.com/Oldes/Rebol-issues/issues/1551
	;@@ https://github.com/Oldes/Rebol-issues/issues/926
		--assert all [
			error? err: try [make module! 10]
			err/id = 'bad-make-arg
		]
	--test-- "make module! map!" ; not allowed
	;@@ https://github.com/Oldes/Rebol-issues/issues/1551
		--assert all [
			error? err: try [make module! #(a: 1)]
			err/id = 'bad-make-arg
		]
	--test-- "make module! object!" ; not allowed
	;@@ https://github.com/Oldes/Rebol-issues/issues/1551
		--assert all [
			error? err: try [make module! context [a: 1]]
			err/id = 'bad-make-arg
		]
	--test-- "make module! empty block!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/898
		--assert all [
			error? err: try [to module! []]
			err/id = 'bad-make-arg
		]
===end-group===


===start-group=== "module import"
	--test-- "import"
	;@@ https://github.com/Oldes/Rebol-issues/issues/923
	write %mymodule.reb {
Rebol [
    type: 'module
    name: 'mymodule
    exports: [myfunc]
]
print "mymodule imported"
myfunc: func [arg [string!]][reverse arg]
}
	import 'mymodule
	--assert "cba" = myfunc "abc"
	import 'mymodule     ;-- this works... the file isn't reloaded... and indeed the console doesn't print another "mymodule imported"
	--assert "cba" = myfunc "abc"
	import %mymodule.reb ;-- no crash
	--assert "cba" = myfunc "abc"
	delete %mymodule.reb

;;; This test would fail as the module needs itself! It should be detected, but it isn't yet.
;;	write %mymodule2.reb {
;; Rebol [
;;     type: 'module
;;     name: 'mymodule2
;;     exports: [myfunc2]
;;     needs: [%mymodule2.reb]
;; ]
;; print "mymodule2 imported"
;; myfunc2: func [arg [string!]][reverse arg]
;; }
;; 	import 'mymodule2
;; 	import 'mymodule2
;; 	import %mymodule2.reb ;-- no crash
;; 	--assert "cba" = myfunc2 "abc"
;; 	delete %mymodule2.reb

	--test-- "import to user context"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1680
	;- using external script as tests are by default inside `wrap`
	;- and in such a case the words from module are not imported
	;- (I'm quite not sure yet, if it is good or bad)
	write %issue-1680.reb {
REBOL []
c: true ; should not be rewritten
m: module [][export a: 1 b: 2 export c: 3]
import m
;-assert-1
probe a = 1     ; exported
;-assert-2
probe unset? :b ; not exported
;-assert-3
probe logic? :c ; not rewritten

;-assert-4
unset [a b c]   ; reset all
import/no-user m
probe all [unset? :a unset? :b unset? :c]
}
	o: copy ""
	call/wait/shell/output reform [to-local-file system/options/boot %issue-1680.reb] o
	--assert "true^/true^/true^/true^/" = o
	delete %issue-1680.reb

	--test-- "import/no-lib"
	;- using external script again!
	write %no-lib-import.reb {
REBOL []
;-assert-1 = make sure, that a and b are not in lib
probe all [
	none? get in system/contexts/lib 'a
	none? get in system/contexts/lib 'b
]

m1: module [name: no-lib-test-a][export a: 1]
import m1

;-assert-2 = check existence of imported values
probe all [
	1 = a                      ; imported to user context
	1 = system/contexts/lib/a  ; imported also to lib
]

;-assert-3
unset 'a ; uset user's value
probe 1 = system/contexts/lib/a ; still available in lib

;-assert-4 = import using /no-lib
m2: module [name: no-lib-test-b][export b: 2]
import/no-lib m2
probe all [
	2 = b                      ; imported to user context
	none? get in system/contexts/lib 'b ; but not to lib
]}
	o: copy ""
	call/wait/shell/output reform [to-local-file system/options/boot %no-lib-import.reb] o
	--assert "true^/true^/true^/true^/" = o
	delete %no-lib-import.reb

	--test-- "import block"
	;- using external script again!
	write %m1.reb {Rebol [name: m1 type: module] export m1a: 1}
	write %m2.reb {Rebol [name: m2 type: module] export m2b: 2}
	write %block-import-1.reb {
Rebol []
probe all [
	block? b: import [m1 m2]
	module? b/1
	module? b/2
	m1a = 1
	m2b = 2
]}
	o: copy ""
	call/wait/shell/output reform [to-local-file system/options/boot %block-import-1.reb] o
	--assert "true^/" = o
	--test-- "import/no-lib block"
	write %block-import-2.reb {
Rebol []
probe all [
	block? b: import/no-lib [m1 m2]
	module? b/1
	module? b/2
	m1a = 1
	m2b = 2
	none? get in system/contexts/lib 'm1a
	none? get in system/contexts/lib 'm1a
]}
	o: copy ""
	call/wait/shell/output reform [to-local-file system/options/boot %block-import-2.reb] o
	--assert "true^/" = o
	delete %block-import-1.reb
	delete %block-import-2.reb
	delete %m1.reb
	delete %m2.reb

	--test-- "import/version"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1687
		m-1687: module [version: 1.0.0 name: 'm-1687][a: 1]
		--assert all [
			error? e: try [import/version m-1687 2.2.2]
			e/id = 'needs
		]
		--assert module? m: try [import/version m-1687 1.0.0]
		--assert all [
			;@@ https://github.com/Oldes/Rebol-wishes/issues/13
			object? m/lib-local
			empty?  m/lib-local ; because there was no import in this module
		]

	--test-- "import/check"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1686
		s: save/header none [f: func[arg][arg]] [type: module checksum: true]
		c: select first load/header s 'checksum
		--assert #{3556AE790FFAC48BBBBC1CF2BC5F8942782416F7} = c
		--assert block? m: sys/load-module/check s c
		--assert module? m/2
		--assert 22 = m/2/f 22
		--assert module? m: import/check s c
		--assert 33 = m/f 33
		; corrupt the source:
		change find s to-binary "f:" "g"
		--assert all [
			error? e: try [import/check s c]
			e/id = 'bad-checksum
		]


	--test-- "import string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1721
		unset in system/contexts/user 'issue-1721-a
		unset in system/contexts/user 'issue-1721-b
		import {rebol [type: 'module] issue-1721-a: 1 export issue-1721-b: 2}
		--assert unset? :system/contexts/user/issue-1721-a
		--assert system/contexts/user/issue-1721-b = 2

	;@@ https://github.com/Oldes/Rebol-issues/issues/1694
	--test-- "import needs word!"
		import 'test-needs-name
		--assert true? test-needs-name-result
	--test-- "import needs file!"
		;- at this moment files imported from other file
		;- does not use parent file's location!
		m: import %units/files/test-needs-file.reb
		--assert true? test-needs-file-result
		--assert all [
			;@@ https://github.com/Oldes/Rebol-wishes/issues/13
			object? m/lib-local
			m/lib-local/test-needs-file-value = 42 ; value imported from the inner module
		]

	--test-- "import needs url!"
		--assert not error? try [
			import https://github.com/Oldes/Rebol3/raw/master/src/tests/units/files/test-needs-url.reb
			--assert true? test-needs-url-result
		]

	--test-- "lib-local"
	;@@ https://github.com/Oldes/Rebol-wishes/issues/13
		--assert same? lib-local system/contexts/user

	--test-- "do module from file"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1203
		do %units/files/test-1203.reb
		--assert all [
			object? o: system/modules/test-1203/f
			o/title = "test-1203"
			system/script/title <> "test-1203"
		]

===end-group===

===start-group=== "module issues"
	--test-- "issue-1005"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1005
		m: module [] [a: 1 2]
		--assert [lib-local: #[object![]] a: 1] = body-of m
		--assert [lib-local a] = keys-of m
		--assert [#[object![]] 1] = values-of m
		m: module [exports: [a]] [a: 1 2]
		--assert [lib-local: #[object![]] a: 1] = body-of m
		--assert [lib-local a] = keys-of m
		--assert [#[object![]] 1] = values-of m

	--test-- "issue-1708"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1708
		--assert none? find system/modules module [] []

===end-group===

~~~end-file~~~

