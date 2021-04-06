Rebol [
	Title:   "Rebol3 port! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %port-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "port"

===start-group=== "decode-url"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2380
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
	--test-- "decode-url http://host?query"
		url: decode-url http://host?query
		--assert url/host = "host"
		--assert url/path = "?query"

===end-group===

===start-group=== "directory port"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2320
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
	--test-- "query directory type"
		;@@ https://github.com/Oldes/Rebol-issues/issues/606
		make-dir %dir-606/
		--assert all [
			object? d: query %dir-606
			d/type = 'dir
			object? d: query %dir-606/
			d/type = 'dir
			d/size = none
		]
		delete %dir-606/

	--test-- "make-dir/delete/exists? with path without a slash"
		;@@ https://github.com/Oldes/Rebol-issues/issues/499
		--assert %dir-606/ = make-dir %dir-606
		--assert not error? try [delete %dir-606]
		--assert not exists? %dir-606

	--test-- "make-dir if file exists"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1777
		--assert not error? try [write %issue-1777.txt "test"]
		--assert error? er: try [make-dir %issue-1777.txt/]
		--assert er/id = 'no-create
		--assert not error? try [delete %issue-1777.txt]

	--test-- "open %."
		;@@ https://github.com/Oldes/Rebol-issues/issues/117
		--assert port? p: open %.
		--assert 'dir  = p/scheme/name
		--assert 'dir  = p/spec/scheme
		--assert  %./  = p/spec/ref
		--assert port? close p

	--test-- "open wildcard"
		;@@ https://github.com/Oldes/Rebol-issues/issues/158
		--assert all [
			port? p: try [open %*.r3]
			'dir  = p/scheme/name
			'dir  = p/spec/scheme
			%*.r3 = p/spec/ref
			port? close p
		]

	--test-- "DIR?"
		;@@ https://github.com/Oldes/Rebol-issues/issues/602
		; dir? only checks if the last char is / or \
		--assert dir? %doesnotexists/
		--assert not dir? %doesnotexists
		--assert dir? %./
		--assert not dir? %.
		; dir?/check
		--assert not dir?/check %doesnotexists
		--assert dir?/check %.
		--assert dir?/check %./

	--test-- "READ on existing dir-name"
		;@@ https://github.com/Oldes/Rebol-issues/issues/635
		;@@ https://github.com/Oldes/Rebol-issues/issues/1675
		;@@ https://github.com/Oldes/Rebol-issues/issues/2379
		--assert block? b1: read %.
		--assert block? b2: read %./
		--assert b1 = b2
		;@@ https://github.com/Oldes/Rebol-issues/issues/604
		--assert 'dir = exists? %.
		--assert 'dir = exists? %./

	--test-- "READ on non-existing dir-name"
		;@@ https://github.com/Oldes/Rebol-issues/issues/500
		--assert error? e: try [read %carl-for-president/]
		--assert e/id = 'cannot-open

	--test-- "READ wildcard"
		;@@ https://github.com/Oldes/Rebol-issues/issues/158
		--assert all [block? b: try [read %*.r3]             not empty? b]
		--assert all [block? b: try [read %run-tests.?3]     not empty? b]
		--assert all [block? b: try [read %units/files/*.r3] not empty? b]
		--assert all [block? b: try [read %*.xxx]                empty? b]

	--test-- "DELETE-DIR"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1545
		--assert all [
			not error? try [make-dir/deep %units/temp-dir/sub-dir/]
			not error? try [write %units/temp-dir/file "hello"]
			not error?      delete-dir %units/temp-dir/
			not exists? %units/temp-dir/
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2447
		--assert all [
			logic? v: try [delete %not-exists/]
			not v
		]
		--assert error? try [delete %/]

	--test-- "CHANGE-DIR"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2446
		--assert what-dir = change-dir %.
		--assert all [
			error? e: try [change-dir %issues/2446]
			e/id = 'cannot-open
			e/arg1 = join what-dir %issues/2446/
		]
		
if system/platform = 'Windows [
;@@ it looks that on Linux there is no lock on opened file
		--assert all [
			all [
				not error? try [make-dir/deep %units/temp-dir/]
				; open a file for writing in the directory
				p: open/write %units/temp-dir/file
				; and see that the directory cannot be deleted
				error? e: delete-dir %units/temp-dir/
				e/id = 'no-delete
				exists? %units/temp-dir/
			]
			all [
				; closing the file
				close p
				; and now the file and dir are both deleted
				not error? delete-dir %units/temp-dir/
			]
		]
]

	--test-- "RENAME dir"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1533
		--assert all [
			not error? try [make-dir %units/temp-dir/]
			not error? try [rename %units/temp-dir/ %units/new-dir/]
			exists? %units/new-dir/
			not error? try [delete-dir %units/new-dir/]
		]

if system/platform = 'Windows [
	--test-- "read %/"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2031
		--assert all [
			block? drives: read %/
			not empty? drives
			2 = length? drives/1
			#"/" = last drives/1
		]
]
	--test-- "exists? %/"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2317
		--assert 'dir = exists? %/       ;@@ https://github.com/Oldes/Rebol-issues/issues/612
		--assert object? info: query %/
		--assert info/name = %/
		--assert info/type = 'dir
		--assert none? info/size
		either system/platform = 'Windows [
			--assert none? info/date
		][
			; on linux %/ is just a normal directory root
			--assert date? info/date
		]
		

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

	--test-- "query file name"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2442
		file: %units/files/čeština.txt
		--assert not none? find (query/mode file 'name) file

	--test-- "query file info (port)"
		file: open %units/files/alice29.txt.gz
		--assert [name size date type] = query/mode file none
		--assert 'file = query/mode file 'type
		--assert date?   query/mode file 'date
		--assert 51732 = query/mode file 'size
		--assert [file 51732] = query/mode file [type size]
		--assert [type: file size: 51732] = query/mode file [type: size:]
		close file

	--test-- "write/lines"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2102
		write/lines %tmp.txt {a^/}
		--assert ["a" ""] = read/lines %tmp.txt
		;@@ https://github.com/Oldes/Rebol-issues/issues/2429
		--assert "a^/b^/" = read/string write/lines %tmp.txt ["a" "b"]
		;@@ https://github.com/Oldes/Rebol-issues/issues/612
		--assert 'file = exists? %tmp.txt
		delete %tmp.txt

	--test-- "read/lines - issue/1794"
		--assert 1 = length? read/lines %units/files/issue-1794.txt

	--test-- "read/string - issue/622"
		;@@ https://github.com/Oldes/Rebol-issues/issues/622
			; making file where is one CRLF and one LF
			write %units/files/issue-622.txt to-binary "a^M^/b^/c"
			--assert "a^M^/b^/c" = to string! read %units/files/issue-622.txt
			--assert "a^/b^/c" = deline to string! read %units/files/issue-622.txt
			--assert "a^/b^/c" = read/string %units/files/issue-622.txt
			delete %units/files/issue-622.txt


	--test-- "write file result - wish/2337"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2337
		--assert file? write %foo "hello"
		;@@ https://github.com/Oldes/Rebol-issues/issues/67
		--assert "test" = read/string write %foo "test"
		delete %foo

	--test-- "write non binary data to file"
		;@@ https://github.com/Oldes/Rebol3/issues/16
		--assert "make object! [^/    type: 'foo^/]" = read/string write %foo object [type: 'foo]
		--assert "[a: 1]" = read/string write %foo [a: 1]
		--assert "<foo>"  = read/string write %foo <foo>
		delete %foo

	--test-- "open/close file"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1456
		;@@ https://github.com/Oldes/Rebol-issues/issues/1453
		file: %tmp-1456
		write file "abc"
		--assert #{616263} = read file
		port: open file
		--assert #{616263} = read port
		--assert open? port
		--assert empty? read port ; because the port is still open, but we are at tail already
		close port
		--assert not open? port
		--assert #{616263} = read port ;port was not one, so it's opened for read action
		--assert not open? port ;but was closed again by read
		--assert #{616263} = read port ;so next read is again full
		delete %tmp-1456

	--test-- "write/part"
		;@@ https://github.com/Oldes/Rebol-issues/issues/445
		a: make binary! 100000
		insert/dup a #{00} 100000
		write %zeroes-445.txt a
		; write junk bug
		f: open %zeroes-445.txt
		write/part f #{1020304050} 100
		close f
		--assert 10873462 = checksum read %zeroes-445.txt 'crc24
		delete %zeroes-445.txt

	--test-- "write/append"
		;@@ https://github.com/Oldes/Rebol-issues/issues/443
		--assert all [
			not error? try [write %issue-443 "test"]
			not error? try [write/append %issue-443 "443"]
			"test443" = read/string %issue-443
			not error? try [delete %issue-443]
		]

	--test-- "write/seek"
		;@@ https://github.com/Oldes/Rebol-issues/issues/552
		--assert file? write %file-552 to-binary "Hello World!"
		--assert port? f: open/seek %file-552
		--assert "Hello World!" = to-string read/seek f 0
		--assert file? write/seek f to-binary "a" 4  ; In range
		--assert file? write/seek f to-binary " Goodbye World!" 12  ; Out of range
		--assert "Hella World! Goodbye World!" = to-string read/seek f 0
		--assert port? close f
		try [delete %file-552]

	--test-- "clear file port"
		;@@ https://github.com/Oldes/Rebol-issues/issues/812
		--assert file? write %file-812 to-binary "Hello World!"
		--assert port? f: open %file-812
		--assert "Hello World!" = to-string read f
		--assert port? clear f
		--assert 0 = length? f
		--assert port? close f
		try [delete %file-812]

	--test-- "RENAME file"
		;@@ https://github.com/Oldes/Rebol-issues/issues/446
		--assert all [
			not error? try [write %issue-446 "test"]
			not error? try [rename %issue-446 %issue-446.txt]
			"test" = read/string %issue-446.txt
			not error? try [delete %issue-446.txt]
		]
	--test-- "DELETE file"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2447
		--assert all [
			logic? v: try [delete %not-exists]
			not v
		]
		; create locked file...
		p: open %issue-2447

		either system/platform = 'Windows [
		; should not be possible to delete it on Windows..
			--assert error? try [delete %issue-2447]
		][
		; on Posix it can be deleted
			--assert not error? try [delete %issue-2447]
		]
		; close the file handle...
		close p
		if system/platform = 'Windows [
			; now it may be deleted..
			--assert  port? try [delete %issue-2447]
		]
		; validate...
		--assert not exists? %issue-2447

===end-group===

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
	===end-group===
]

===start-group=== "HTTP scheme"
	--test-- "read HTTP"
		--assert  string? try [read http://google.com]
	--test-- "read HTTPS"
		--assert  string? try [read https://www.google.com]
	--test-- "exists? url"
		;@@ https://github.com/Oldes/Rebol3/issues/14
		;@@ https://github.com/Oldes/Rebol-issues/issues/1613
		--assert 'url = exists? http://httpbin.org/  ;@@ https://github.com/Oldes/Rebol-issues/issues/612
		--assert not exists? http://httpbin.org/not-exists
	--test-- "read/part"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2434
		--assert "<!DOCTYPE" = read/part http://httpbin.org/ 9
		--assert #{89504E47} = read/binary/part http://avatars-04.gitter.im/gh/uv/4/oldes 4
	--test-- "read not existing url"
		;@@ https://github.com/Oldes/Rebol-issues/issues/470
		--assert all [
			error? e: try [read http://www.r]
			e/id = 'no-connect
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2441
		--assert string? try [read http://www.rebol.com]
	--test-- "query url"
		;@@ https://github.com/Oldes/Rebol-issues/issues/467
		--assert error? try [query https://www]
		--assert object? query https://www.google.com

===end-group===


===start-group=== "WHOIS scheme"
	--test-- "read WHOIS"
		--assert  string? probe try [read whois://google.com]
	--test-- "write WHOIS"
		--assert string? try [write whois://whois.nic.cz "seznam.cz"]
===end-group===


if all [
	"true" <> get-env "CONTINUOUS_INTEGRATION"
	"true" <> get-env "CI" ; for GitHub workflows
][
	;- don't do these tests on Travis CI
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


===start-group=== "DNS"
;@@ https://github.com/Oldes/Rebol-issues/issues/1827
;@@ https://github.com/Oldes/Rebol-issues/issues/1860
;@@ https://github.com/Oldes/Rebol-issues/issues/1935
	--test-- "read dns://"
		--assert string? try [probe read dns://] ;- no crash!
	--test-- "read dns://8.8.8.8"
		--assert "dns.google" = try [probe read dns://8.8.8.8]
	--test-- "read dns://google.com"
		--assert tuple? try [read dns://google.com]
===end-group===

~~~end-file~~~