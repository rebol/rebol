Rebol [
	Title:   "Rebol delect test script"
	Author:  "Oldes"
	File: 	 %delect-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "DELECT"

===start-group=== "Delect"
; test only if `delect` is available
if find lib 'delect [
	dialect: context [
		default: []
		cmd:     [any-string! integer!]
	]
	out: make block! 4
	--test-- "valid delect with typeset!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/51
		inp: [
			cmd "a" 1
			cmd <a> 2
			cmd 3 a@b
			cmd http://
			cmd 5
			cmd 60
		]
		inp: delect dialect inp out
		--assert out = [cmd "a" 1]
		inp: delect dialect inp out
		--assert out = [cmd <a> 2]
		inp: delect dialect inp out
		--assert out = [cmd a@b 3]
		inp: delect dialect inp out
		--assert out = [cmd http:// #(none)]
		inp: delect dialect inp out
		--assert out = [cmd #(none) 5]

		;-- should be extended with more tests...
]
===end-group===

~~~end-file~~~