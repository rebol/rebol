Rebol [
	Title:   "Rebol3 console port! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %port-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "port"

===start-group=== "console port"	
	--test-- "query input port"
		--assert  port? system/ports/input
		--assert  all [
			object?  console-info: query system/ports/input object!
			integer? console-info/window-cols
			integer? console-info/window-rows
			integer? console-info/buffer-cols
			integer? console-info/buffer-rows
			;?? console-info
		]
		--assert integer? query system/ports/input 'window-cols
		--assert integer? query system/ports/input 'window-rows
		--assert integer? query system/ports/input 'buffer-cols
		--assert integer? query system/ports/input 'buffer-rows
		--assert integer? query system/ports/input 'length
		--assert (words-of system/standard/console-info)
						= m: query system/ports/input none
		--assert block?   v: query system/ports/input m
		--assert 10 = length? v
	--test-- "Using just a name of the console scheme"
	;@@ https://github.com/Oldes/Rebol-issues/issues/826
		--assert all [
			port? try [p: open 'console]
			port? close p
		]
===end-group===


~~~end-file~~~