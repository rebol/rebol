Rebol [
	Title:   "Rebol3 word test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %thru-cache-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "THRU-CACHE"

if module? try [import 'thru-cache][
;@@ https://github.com/Oldes/Rebol-issues/issues/2554
===start-group=== "thru cache functions"
	--test-- "read-thru"
	url: https://raw.githubusercontent.com/Oldes/Rebol3/master/src/tests/units/files/%C4%8De%C5%A1tina.txt
	--assert try [all [
		binary? read-thru url
		'file = exists? path-thru url
		string? str1: read-thru/string url
		string? str2: read/string path-thru url
		equal? str1 str2
		equal? str1 read/string url
	]]
	
	--test-- "read-thru status <> 200"
	url: http://httpbin.org/status/400
	--assert try [all [
		binary? read-thru url ;; returned, but not stored!
		not exists-thru? url
	]]

	--test-- "load-thru"
	url: https://raw.githubusercontent.com/Oldes/Rebol3/master/src/tests/units/files/print-args.r3
	--assert try [all [
		block? blk1: load-thru url
		block? blk2: load url
		equal? blk1 blk2
	]]

	--test-- "exists-thru?"
	--assert 'file = exists-thru? url
	--assert   none? exists-thru? http://not-exists
	
	--test-- "do-thru"
	--assert not error? try [do-thru url] ;; evaluates the previously downloaded and cached script

	--test-- "clear-thru"
		clear-thru/test ;; just prints all cached files
		--assert 'file = exists? path-thru url
		clear-thru/only "*.txt" ;; removes all *.txt files from the cache
		--assert 'file = exists? path-thru url
		either "true" == get-env "CI" [
		;@@ don't do it at home or you will loose your data like me now!
			clear-thru ;; removes everything !!!
		][	clear-thru/only "*/files/print-args.r3" ]
		--assert none? exists? path-thru url

	--test-- "delete-thru"
	--assert all [
		binary? read-thru url
		'file = exists-thru? url
		port? delete-thru url
		not exists-thru? url
	]

===end-group===
]

~~~end-file~~~