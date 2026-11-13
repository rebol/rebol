Rebol [
	Title:   "Rebol3 port! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %port-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "port-net"



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

===start-group=== "DNS"
;@@ https://github.com/Oldes/Rebol-issues/issues/1827
;@@ https://github.com/Oldes/Rebol-issues/issues/1860
;@@ https://github.com/Oldes/Rebol-issues/issues/1935
	--test-- "read dns://"
		--assert string? try [probe read dns://] ;- no crash!

	--test-- "Using just a name of the dns scheme"
	;@@ https://github.com/Oldes/Rebol-issues/issues/826
		--assert string? try [read 'dns]

	--test-- "read dns://8.8.8.8"
		--assert "dns.google" = try [probe read dns://8.8.8.8]
	--test-- "read dns://google.com"
		--assert tuple? try [read dns://google.com]

	--test-- "query dns://"
	;@@ https://github.com/Oldes/rebol-issues/issues/1826
		--assert all [error? e: try [query dns:// object!]  e/id = 'no-port-action]

	--test-- "read dns://not-exists"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2498
		--assert none? try [read dns://not-exists]
===end-group===


===start-group=== "TCP"
	--test-- "query net info"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1712
		port: open tcp://8.8.8.8:80
		--assert (words-of system/standard/net-info) = query port none
		--assert 0.0.0.0 = query port 'local-ip
		--assert       0 = query port 'local-port
		--assert not none? find [0.0.0.0 8.8.8.8] query port 'remote-ip ;; on posix there is sync lookup and so it reports 8.8.8.8 even without wait
		--assert      80 = query port 'remote-port
		--assert all [
			port? wait [port 1] ;= wait for lookup, so remote-ip is resolved
			8.8.8.8 = query port 'remote-ip
			[80 8.8.8.8] = query port [:remote-port :remote-ip]
			[local-ip: 0.0.0.0 local-port: 0] = query port [local-ip local-port]
		]
		try [close port]
===end-group===


~~~end-file~~~