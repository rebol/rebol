Rebol [
	Title:   "Rebol loading test script"
	Author:  "Peter W A Wood, Oldes"
	File: 	 %load-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
	Rights:  "Copyright (C) 2011-2015 Red Foundation. All rights reserved."
	License: "BSD-3 - https://github.com/red/red/blob/origin/BSD-3-License.txt"
]

~~~start-file~~~ "load"

===start-group=== "Load/all"
	--test-- "issue-20"
	;@@ https://github.com/Oldes/Rebol-issues/issues/20
		v: mold/all next "123"
		--assert block? v: load/all v
		--assert  "23" = v/1
		--assert "123" = head v/1

===end-group===

~~~end-file~~~

