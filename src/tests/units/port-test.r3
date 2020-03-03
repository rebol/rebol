Rebol [
	Title:   "Rebol3 port! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %port-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "port"

===start-group=== "decode-url"
	;@@ https://github.com/rebol/rebol-issues/issues/2380
	--test-- "decode-url-unicode"
		url: decode-url http://example.com/get?q=ščř#kovtička
		--assert url/scheme = 'http
		--assert url/host   = "example.com"
		--assert url/path   = "/get?q=ščř"
		--assert url/tag    = "kovtička"
	--test-- "decode-url-unicode"
		url: decode-url http://švéd:břéťa@example.com:8080/get?q=ščř#kovtička
		--assert url/scheme = 'http
		--assert url/user   = "švéd"
		--assert url/pass   = "břéťa"
		--assert url/host   = "example.com"
		--assert url/port-id = 8080
		--assert url/path   = "/get?q=ščř"
		--assert url/tag    = "kovtička"

===end-group===

===start-group=== "directory port"
	;@@ https://github.com/rebol/rebol-issues/issues/2320
	--test-- "port-issue-2320"
		--assert  %port-issue-2320/ = make-dir %port-issue-2320/
		--assert  port? p: open %port-issue-2320/
		--assert  empty? p
		--assert  not open? close p
		--assert  error? try [empty? p] ;@@ or should it reopen it instead?
		--assert  not empty? open %./
		--assert  not error? [delete %port-issue-2320/]
	--test-- "query directory info"
		--assert [name size date type] = query/mode %. none
		--assert 'dir     = query/mode %. 'type
		--assert date?      query/mode %. 'date
		--assert what-dir = query/mode %. 'name
		;@@ https://github.com/Oldes/Rebol-issues/issues/2305
		--assert      none? query/mode %. 'size
	--test-- "READ on non-existing dir-name"
	;@@ https://github.com/Oldes/Rebol-issues/issues/500
		--assert error? e: try [read %carl-for-president/]
		--assert e/id = 'cannot-open
===end-group===

===start-group=== "file port"
	--test-- "query file info"
		file: %units/files/alice29.txt.gz
		--assert [name size date type] = query/mode file none
		--assert 'file = query/mode file 'type
		--assert date?   query/mode file 'date
		--assert 51732 = query/mode file 'size
		--assert [file 51732] = query/mode file [type size]
		--assert [type: file size: 51732] = query/mode file [type: size:]

	--test-- "query file info (port)"
		file: open %units/files/alice29.txt.gz
		--assert [name size date type] = query/mode file none
		--assert 'file = query/mode file 'type
		--assert date?   query/mode file 'date
		--assert 51732 = query/mode file 'size
		--assert [file 51732] = query/mode file [type size]
		--assert [type: file size: 51732] = query/mode file [type: size:]
		close file

	--test-- "write/lines - issue/2102"
		;@@ https://github.com/rebol/rebol-issues/issues/2102
		write/lines %tmp.txt {a^/}
		--assert ["a" ""] = read/lines %tmp.txt
		delete %tmp.txt

	--test-- "write file result - wish/2337"
		;@@ https://github.com/rebol/rebol-issues/issues/2337
		--assert file? write %foo "hello"
		--assert "hello" = read/string write %foo "hello"
		delete %foo

===end-group===

===start-group=== "HTTP scheme"
	--test-- "read HTTP"
		--assert  string? try [read http://google.com]
	--test-- "read HTTPS"
		--assert  string? try [read https://www.google.com]
	--test-- "exists? url"
		;@@ https://github.com/Oldes/Rebol3/issues/14
		--assert     exists? http://httpbin.org/
		--assert not exists? http://httpbin.org/not-exists
===end-group===


if "true" <> get-env "CONTINUOUS_INTEGRATION" [
	;- don't do these tests on Travis CI
	===start-group=== "WHOIS scheme"
		--test-- "read WHOIS"
			--assert  string? try [read whois://google.com]
		--test-- "write WHOIS"
			--assert string? try [write whois://whois.nic.cz "seznam.cz"]
	===end-group===

	===start-group=== "console port"	
		--test-- "query input port"
			--assert  port? system/ports/input
			--assert  all [
				object?  console-info: query system/ports/input
				integer? console-info/window-cols
				integer? console-info/window-rows
				integer? console-info/buffer-cols
				integer? console-info/buffer-rows
				;?? console-info
			]
			--assert integer? query/mode system/ports/input 'window-cols
			--assert integer? query/mode system/ports/input 'window-rows
			--assert integer? query/mode system/ports/input 'buffer-cols
			--assert integer? query/mode system/ports/input 'buffer-rows
			--assert [buffer-cols buffer-rows window-cols window-rows]
							= m: query/mode system/ports/input none
			--assert block?   v: query/mode system/ports/input m
			--assert 4 = length? v
	===end-group===
]

~~~end-file~~~