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

===start-group=== "concat url/file using path notation"
--test-- "issue-1427"
;@@https://github.com/Oldes/Rebol-issues/issues/1427
	url: http://www.rebol.com page: %/test.html
	--assert url/test  = http://www.rebol.com/test
	--assert url/:page = http://www.rebol.com/test.html
--test-- "issue-2585"
;@@ https://github.com/Oldes/Rebol-issues/issues/2585
	url: http://www/
	dir: %path/
	file: next %a/bc ;== %/bc
	--assert dir/:file = %path/bc
	--assert url/:file = http://www/bc
	file: next file  ;== %bc
	--assert dir/:file = %path/bc
	--assert url/:file = http://www/bc
	file: next file  ;== %c
	--assert dir/:file = %path/c
	--assert url/:file = http://www/c
--test-- "concat with integer"
;@@ https://github.com/Oldes/Rebol-issues/issues/2306
	dir: %path
	--assert dir/1 = %path/1
	--assert #"p" = pick dir 1

===end-group===

===start-group=== "UN/DIRIZE"
	--test-- "dirize"
	--assert %file/ = dirize %file
	--test-- "undirize"
	;@@ https://github.com/Oldes/Rebol-issues/issues/619
	--assert %./file = undirize %./file/
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

if find [Linux macOS] system/platform [
--test-- "issue-2538"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2538
	cd (mkdir %issue-2538)
	call/shell/wait "touch a:0:0"
	--assert %a%3A0%3A0 == to-rebol-file "a:0:0" 
	--assert %a%3A0%3A0 == f: first read %.
	--assert %a%3A0%3A0 == second split-path query/mode f 'name
	delete f
	cd ..
	delete %issue-2538
]

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
	;@@ https://github.com/Oldes/Rebol-issues/issues/654
	--test-- "script? string!"
	--assert #{5245424F4C5B5D2031} = script? {REBOL[] 1}
	--assert #{5245424F4C5B5D2031} = script? {bla^/REBOL[] 1}
===end-group===

===start-group=== "file://"
	--test-- "issue-834"
	;@@ https://github.com/Oldes/Rebol-issues/issues/834
	write %temp.txt #{}
	--assert #{} = read %temp.txt
	--assert #{} = read file://temp.txt
	delete %temp.txt
===end-group===

===start-group=== "WILDCARD"
	--test-- "wildcard *"
		--assert all [
			block? files: wildcard %units/files/ %*.zip
			empty? find/last/tail files/1 %.zip
		]
		--assert empty? wildcard %units/files/ %*.something
	--test-- "wildcard ?"
		--assert all [
			block? files: wildcard %units/files/ %issue-2186-UTF??-BE.txt
			2 = length? files
		]
	--test-- "wildcard *?*"
		--assert all [
			block? files: wildcard %units/files/ %*2186-UTF??*.txt
			4 = length? files
		]
	--test-- "read wildcard"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2443
		--assert all [
			block? files: try [read %units/files/issue-2186-*.txt]
			4 = length? files
			to logic! find files %issue-2186-UTF16-LE.txt
		]
		--assert all [
			block? files: try [read %units/files/issue-2186-UTF??-BE.txt]
			2 = length? files
			to logic! find files %issue-2186-UTF32-BE.txt
		]
		--assert all [
			block? files: try [read %units/files/*2186-UTF??*.txt]
			4 = length? files
			to logic! find files %issue-2186-UTF16-BE.txt
		]

===end-group===


===start-group=== "ECHO"
	--test-- "echo test"
		echo %echo-result print 1 echo none
		--assert "1^/" = read/string %echo-result

	--test-- "echo failed"
	;@@ https://github.com/Oldes/Rebol-issues/issues/834
		--assert all [error? e: try [echo %not-existing-dir/foo] e/id = 'cannot-open]
===end-group===


~~~end-file~~~