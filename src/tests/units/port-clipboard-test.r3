Rebol [
	Title:   "Rebol3 clipboard port! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %port-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "port"

if system/platform = 'Windows [
	===start-group=== "CLIPBOARD"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1968
		--test-- "Clipboard port test"
			c: "Clipboard port test"
			--assert all [
				port? p: try [open clipboard://]
				not error? try [write p c]
				strict-equal? c try [read p]
			]
			close p
		--test-- "Clipboard scheme test"
			c: "Clipboard scheme test"
			; this tests now seems to be failing when done from a run-tests script
			; but is ok when done in console :-/
			--assert all [
				not error? try [write clipboard:// c]
				strict-equal? c try [read clipboard://]
			]
		--test-- "issue-2486"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2486
			foreach ch [#"a" #"^(7F)" #"^(80)" #"^(A0)"][
				write clipboard:// append copy "" ch
				--assert (to binary! ch) = to binary! read clipboard://
			]
		--test-- "Using just a name of the scheme"
		;@@ https://github.com/Oldes/Rebol-issues/issues/826
			txt: "hello"
			--assert all [
				port? try [write 'clipboard txt]
				txt = try [read 'clipboard]
				txt = try [read open 'clipboard] 
			]
			
	===end-group===
]


~~~end-file~~~