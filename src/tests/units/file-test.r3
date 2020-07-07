Rebol [
	Title:   "Rebol3 file test script"
	Author:  "Oldes"
	File: 	 %file-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

secure [%/ allow]

~~~start-file~~~ "FILE tests"

===start-group=== "cd"
--test-- "cd / cd ~"
	dir: what-dir
	--assert all [
		not error? try [cd /]
		not error? try [cd ~]
	]
	change-dir dir
===end-group===


===start-group=== "to-local-file"

--test-- "issue-2351"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2351
	f: first read what-dir
	--assert  (to-rebol-file to-local-file/full f) = join what-dir f

	d: what-dir
	cd %/ f: first read %/
	--assert  (to-rebol-file to-local-file/full f) = join what-dir f
	cd d

--test-- "issue-1115"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1115
	either system/platform = 'Windows [
		--assert "\\rodan\shareddocs"  = to-local-file %/rodan/shareddocs
		--assert "\\rodan\shareddocs\" = to-local-file %/rodan/shareddocs/
	][
		--assert "/rodan/shareddocs"  = to-local-file %/rodan/shareddocs
		--assert "/rodan/shareddocs/" = to-local-file %/rodan/shareddocs/
	]
	--assert %/rodan/shareddocs    = to-rebol-file "\\rodan\shareddocs"
	--assert %/rodan/shareddocs/   = to-rebol-file "\\rodan\shareddocs\"

===end-group===

===start-group=== "suffix?"
	--test-- "suffix? file!"
	--assert %.c = suffix? %b.c
	--assert %.c = suffix? %a/b.c

	--test-- "suffix? url!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2136
	--assert %.c = suffix? http://a/b.c
	--assert %.c = suffix? http://a/b.c?d.e
	--assert %.c = suffix? http://a/b.c#d.e
	--assert %.c = suffix? http://a/b.c?d#e.f
===end-group===

===start-group=== "script?"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1885
	--test-- "script? string!"
	--assert #{5245424F4C5B5D2031} = script? {REBOL[] 1}
	--assert #{5245424F4C5B5D2031} = script? {bla^/REBOL[] 1}

~~~end-file~~~