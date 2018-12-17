Rebol [
	Title:   "Rebol3 date test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %date-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "date"

===start-group=== "ISO88601 subset"
	--test-- "ISO88601 basic load"
		--assert 8-Nov-2013/17:01 = load "2013-11-08T17:01"
		--assert 8-Nov-2013/17:01 = load "2013-11-08T17:01Z"
		--assert 8-Nov-2013/17:01+1:00 = load "2013-11-08T17:01+0100"
		--assert 8-Nov-2013/17:01-1:00 = load "2013-11-08T17:01-0100"
		--assert 8-Nov-2013/17:01+1:00 = load "2013-11-08T17:01+01:00"
	--test-- "basic load of not fully standard ISO88601"
		--assert 8-Nov-2013/17:01 = load "2013/11/08T17:01"
		--assert 8-Nov-2013/17:01 = load "2013/11/08T17:01Z"
		--assert 8-Nov-2013/17:01+1:00 = load "2013/11/08T17:01+0100"
		--assert 8-Nov-2013/17:01+1:00 = load "2013/11/08T17:01+01:00"
	--test-- "Invalid ISO88601 dates"
		--assert error? try [load "2013-11-08T17:01Z0100"]
		--assert error? try [load "2013/11/08T17:01Z0100"]

	--test-- "Using ISO88601 datetime in a path"
		;@@ https://github.com/rebol/rebol-issues/issues/2089
		b: [8-Nov-2013/17:01 "foo"]
		--assert "foo" = b/2013-11-08T17:01

===end-group===


===end-group===
	
~~~end-file~~~
