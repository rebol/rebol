Rebol [
	Title:   "Rebol protect test script"
	Author:  "Oldes"
	File: 	 %protect-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Protect"

===start-group=== "Checks if protected data are really protected"
	data: #{cafe}
	protect data

	is-protected-error?: function[code][
		true? all [
			error? err: try code
			err/id = 'protected
		]
	]

	--test-- "clear"   --assert is-protected-error? [clear data]
	--test-- "append"  --assert is-protected-error? [append data #{0bad}]
	--test-- "insert"  --assert is-protected-error? [insert data #{0bad}]

	;@@ https://github.com/rebol/rebol-issues/issues/2321
	--test-- "encloak" --assert is-protected-error? [encloak data "key"]
	--test-- "decloak" --assert is-protected-error? [decloak data "key"]
	;@@ https://github.com/rebol/rebol-issues/issues/1780
	--test-- "remove-each" --assert is-protected-error? [remove-each a data [a < 3]]

===end-group===

~~~end-file~~~