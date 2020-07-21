Rebol [
	Title:   "Rebol OS test script"
	Author:  "Oldes"
	File: 	 %os-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "OS"

===start-group=== "set-env / get-env"
	--test-- "env-1"
	--assert "hello" = set-env 'test-temp "hello"
	--assert "hello" = get-env 'test-temp
	--test-- "env-2"
	--assert "" = set-env 'test-temp ""
	--assert "" = get-env 'test-temp
	--test-- "env-3"
	--assert none? set-env 'test-temp none
	--assert none? get-env 'test-temp

===end-group===


~~~end-file~~~
