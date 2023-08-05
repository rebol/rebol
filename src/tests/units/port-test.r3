Rebol [
	Title:   "Rebol3 port! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %port-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "port"


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
		;@@ https://github.com/Oldes/Rebol-issues/issues/1712
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

	--test-- "create dir"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2525
		--assert all [
			not error? try [create %issue-2525/]
			exists? %issue-2525/
			delete  %issue-2525/
			true
		]
		--assert all [
			not error? try [create %issue-2525-ěšč/]
			exists? %issue-2525-ěšč/
			delete  %issue-2525-ěšč/
			true
		]

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

	--test-- "READ dir with single file"
		;@@ https://github.com/Oldes/Rebol-issues/issues/611
		make-dir %dir-611/
		write %dir-611/foo "foo"
		--assert [%foo] = read  %dir-611/
		delete %dir-611/foo
		delete %dir-611/

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

	--test-- "CHANGE-DIR"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2446
		--assert what-dir = change-dir %.
		--assert all [
			error? e: try [change-dir %issues/2446]
			e/id = 'cannot-open
			e/arg1 = join what-dir %issues/2446/
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2448
		dir: pwd
		--assert pwd = to-rebol-file get-env "PWD"
		change-dir %../
		--assert pwd = to-rebol-file get-env "PWD"
		change-dir dir

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
		
	--test-- "unicode directory"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2555
		dir: what-dir
		subdir: %obrázky/
		full: dir/:subdir
		--assert try [
			full == make-dir subdir
			full == change-dir subdir
			full == what-dir
		]
		--assert dir = change-dir dir
		--assert not error? try [delete %obrázky/]

===end-group===

===start-group=== "file port"
	--test-- "query file info"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1712
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
		;@@ https://github.com/Oldes/Rebol-issues/issues/2008
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
		--assert all [
			port? p: open/new %file-552
			file? write p "a"
			0 = length? p
			1 = size? p
			tail? read p
			file? write p "b"
			0 = length? p
			2 = size? p
			tail? read p
			#{6162} = read/seek p 0
			tail? read p
			file? write back p "xy"
			#{617879} = read head p
			#{617879} = read/seek p 0
			0 = length? p
			3 = size? p
			port? close p
			not error? try [delete %file-552]
		]

	--test-- "read/part"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2505
		--assert all [
			file? write %12345 "12345"
			port? p: open/read/seek %12345
			#{31}   == read/part p 1 ; advances!
			#{3233} == read/part p 2
			#{3132} == read/part head p 2
			#{}     == read/part tail p 2
			#{35}   == read/part tail p -1 ;- no crash!
			#{3435} == read/part tail p -2
			all [error? e: try [read/part p -20]  e/id = 'out-of-range]
			port? close p
			not error? try [delete %12345]
		]
	--test-- "skip/at on file port"
		write %12345 "12345"
		p: open/read/seek %12345
		;@@ https://github.com/Oldes/Rebol-issues/issues/2506
		--assert all [
			1 = index? head p
			3 = index? skip p 2
			5 = index? skip p 2
			6 = index? tail p
			6 = index? skip p 20
			1 = index? skip head p -10
			1 = index? back head p  
			1 = index? back back head p
		]
		--assert all [
			6 = index? tail p
			6 = index? at p 20
			2 = index? at p 2
			2 = index? at p 2
			1 = index? at p 0
			1 = index? at p -10
		]
		--assert all [
			6 = index? tail p
			6 = index? atz p 20
			3 = index? atz p 2
			3 = index? atz p 2
			1 = index? atz p 0
			1 = index? atz p -10
		]
		--assert all [
			5 = indexz? tail p
			5 = indexz? atz p 20
			2 = indexz? atz p 2
			2 = indexz? atz p 2
			0 = indexz? atz p 0
			0 = indexz? atz p -10
		]
		close p
		delete %12345

	--test-- "CLEAR file port"
		;@@ https://github.com/Oldes/Rebol-issues/issues/812
		--assert all [
			file? write %file-812 "Hello World!"
			port? f: open %file-812
			"Hello World!" = read/string f
			13 = index? f
			port? clear f ; this actually does not clear the file as we are at the end of the stream
			0  = length? f
			12 =   size? f
			"Hello" = read/seek/string/part f 0 5
			6  =  index? f
			7  = length? f
			12 =   size? f
			port? clear f ; this should truncate the file
			0 = length? f
			5 =   size? f
			port? close f
			5 = length? f ; because there is still "Hello" left and we are counting from head again (port is closed)
			all [         ; it is not allowed to clear not opened port
				error? e: try [clear f]
				e/id = 'not-open
			]
			port? f: open %file-812
			port? clear f ; this should clear the file completely, because the position is at its head
			0 = size? f
			port? close f
			0 = size? %file-812
			not error? try [delete %file-812]
		]
		--assert all [
			file? write %file-812-b "Hello World!"
			port? f: skip open %file-812-b 5
			6 = index? f
			port? clear f ; this should truncate the file
			0 = length? f
			5 = size? f   ; becase that is number of all bytes in the file
			port? close f
			5 = size? %file-812-b
			not error? try [delete %file-812-b]
		]


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

	--test-- "WRITE/APPEND file-port"
		--assert all [
			not error? try [
				p: open/new %issue-1894
				write/append p "Hello"
				write/append p newline
				close p
				p: open %issue-1894
				write/append p #{5265626F6C}
				close p
			]
			"Hello^/Rebol" = read/string %issue-1894
		]

	--test-- "APPEND file-port"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1894
		--assert all [
			not error? try [
				p: open/new %issue-1894
				append p "Hello"
				append p newline
				close p
				p: open %issue-1894
				append p #{5265626F6C}
				close p
			]
			"Hello^/Rebol" = read/string %issue-1894
		]
		--assert all [error? e: try [append/dup p LF 10]  e/id = 'bad-refines]
		--assert all [error? e: try [append/only p "aa"]  e/id = 'bad-refines]
		try [delete %issue-1894]

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
		--test-- "issue-2486"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2486
			foreach ch [#"a" #"^(7F)" #"^(80)" #"^(A0)"][
				write clipboard:// append copy "" ch
				--assert (to binary! ch) = to binary! read clipboard://
			]
			
	===end-group===
]

;- "HTTP scheme" moved to %port-http-test.r3


===start-group=== "WHOIS scheme"
	--test-- "read WHOIS"
		--assert  string? probe try [read whois://google.com]
	--test-- "write WHOIS"
		--assert string? try [write whois://whois.nic.cz "seznam.cz"]
===end-group===


;- not using this test, because the serives limits number of requests from the IP
;- and on CI it may return "Access denied -- too many requests"
;import 'daytime
;if find system/schemes 'daytime [
;===start-group=== "DAYTIME scheme"
;	--test-- "read DAYTIME"
;		--assert  all [
;			block? res: try [read daytime://]
;			res/2/date = now/date
;		]
;
;===end-group===
;]

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

	--test-- "query dns://"
	;@@ https://github.com/Oldes/rebol-issues/issues/1826
		--assert all [error? e: try [query dns://]  e/id = 'no-port-action]
===end-group===


===start-group=== "TCP"
	--test-- "query net info"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1712
		port: open tcp://8.8.8.8:80
		--assert [local-ip local-port remote-ip remote-port] = query/mode port none
		--assert 0.0.0.0 = query/mode port 'local-ip
		--assert       0 = query/mode port 'local-port
		--assert not none? find [0.0.0.0 8.8.8.8] query/mode port 'remote-ip ;; on posix there is sync lookup and so it reports 8.8.8.8 even without wait
		--assert      80 = query/mode port 'remote-port
		--assert all [
			port? wait [port 1] ;= wait for lookup, so remote-ip is resolved
			8.8.8.8 = query/mode port 'remote-ip
			[80 8.8.8.8] = query/mode port [remote-port remote-ip]
			[local-ip: 0.0.0.0 local-port: 0] = query/mode port [local-ip: local-port:]
		]
		try [close port]
===end-group===


===start-group=== "SYSTEM"
	--test-- "query system://"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1373
		--assert all [error? e: try [query system://]  e/id = 'no-port-action]
===end-group===

~~~end-file~~~