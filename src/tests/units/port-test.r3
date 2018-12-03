Rebol [
	Title:   "Rebol3 port! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %port-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "port"

===start-group=== "directory port"
	;@@ https://github.com/rebol/rebol-issues/issues/2320
	--test-- "port-issue-2320"
		--assert  %port-issue-2320/ = make-dir %port-issue-2320/
		--assert  port? p: open %port-issue-2320/
		--assert  empty? p
		--assert  not open? close p
		--assert  error? try [empty? p] ;@@ or should it reopen it instead?
		--assert  not empty? open %./
		--assert  not error? [delete %port-issue-2320/]
	--test-- "query directory info"
		--assert [name size date type] = query/mode %. none
		--assert 'dir     = query/mode %. 'type
		--assert date?      query/mode %. 'date
		--assert what-dir = query/mode %. 'name
===end-group===

===start-group=== "file port"
	--test-- "query file info"
		file: %units/files/alice29.txt.gz
		--assert [name size date type] = query/mode file none
		--assert 'file = query/mode file 'type
		--assert date?   query/mode file 'date
		--assert 51732 = query/mode file 'size

	--test-- "query file info (port)"
		file: open %units/files/alice29.txt.gz
		--assert [name size date type] = query/mode file none
		--assert 'file = query/mode file 'type
		--assert date?   query/mode file 'date
		--assert 51732 = query/mode file 'size
		close file
===end-group===

===start-group=== "console port"
	--test-- "query input port"
		--assert  port? system/ports/input
		--assert  all [
			object?  console-info: query system/ports/input
			integer? console-info/window-cols
			integer? console-info/window-rows
			integer? console-info/buffer-cols
			integer? console-info/buffer-rows
			;?? console-info
		]
		--assert integer? query/mode system/ports/input 'window-cols
		--assert integer? query/mode system/ports/input 'window-rows
		--assert integer? query/mode system/ports/input 'buffer-cols
		--assert integer? query/mode system/ports/input 'buffer-rows
		--assert [buffer-cols buffer-rows window-cols window-rows]
						= query/mode system/ports/input none
===end-group===

~~~end-file~~~