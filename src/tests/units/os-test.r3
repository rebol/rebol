Rebol [
	Title:   "Rebol OS test script"
	Author:  "Oldes"
	File: 	 %os-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "OS"

===start-group=== "set-env / get-env"
;@@ https://github.com/Oldes/Rebol-issues/issues/533
;@@ https://github.com/Oldes/Rebol-issues/issues/1307
--test-- "env-1"
	--assert "hello" = set-env 'test-temp "hello"
	--assert "hello" = get-env 'test-temp
	--assert "hello" = set-env "test-temp" "hello"
	--assert "hello" = get-env "test-temp"
	--assert all [
		map? env: list-env
		"hello" = pick env "test-temp"
	]
--test-- "env-2"
	--assert "" = set-env 'test-temp ""
	--assert "" = get-env 'test-temp
--test-- "env-3"
	--assert none? set-env 'test-temp none
	--assert none? get-env 'test-temp
	--assert none? pick list-env "test-temp"

===end-group===

===start-group=== "others"
	--test-- "echo"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1224
		echo %temp-echo
		print 123
		--assert not error? try [echo off]
		--assert not error? try [echo none]
		--assert 123 = try [load %temp-echo]
		delete %temp-echo

===end-group===


~~~end-file~~~
