Rebol [
	Title:   "Rebol3 file test script"
	Author:  "Oldes"
	File: 	 %file-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "FILE tests"

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

~~~end-file~~~