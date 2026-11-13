Rebol [
	Title:   "Rebol extension test script"
	Author:  "Oldes"
	File: 	 %extension-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Extension"
if %test-x64.rebx [
	===start-group=== "Test Extension"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2455
	--test-- "Import using absolute path"
		--assert module? try [m1: import clean-path %test-x64.rebx]
		--assert 3 = try [addi 1 2]
		--assert 6 = try [m1/addi2 1 2]
		--assert all [
			error? e: try [m1/bad-args]
			e/id = 'command-fail
			command? :e/arg1
		]
		--assert all [
			error? e: try [m1/error]
			e/id = 'command-fail
			e/arg1 = "test error!"
		]

	--test-- "Import using file (no path)"
		--assert module? try [m2: import %test-x64.rebx]
		--assert 3 = try [addi 1 2]
		--assert 6 = try [m2/addi2 1 2]
		--assert all [
			error? e: try [m2/bad-args]
			e/id = 'command-fail
			command? :e/arg1
		]
		--assert all [
			error? e: try [m2/error]
			e/id = 'command-fail
			e/arg1 = "test error!"
		]
	===end-group===
]




~~~end-file~~~