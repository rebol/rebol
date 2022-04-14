Rebol [
	Title:   "Rebol3 url! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %url-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "url"

===start-group=== "decode-url"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1644
	;@@ https://github.com/Oldes/Rebol-issues/issues/2014
	;@@ https://github.com/Oldes/Rebol-issues/issues/2380
	;@@ https://github.com/Oldes/Rebol-issues/issues/2381
	--test-- "decode-url-unicode"
		url: decode-url http://example.com/get?q=ščř#kovtička
		--assert url/scheme   = 'http
		--assert url/host     = "example.com"
		--assert url/path     = %/
		--assert url/target   = %get
		--assert url/query    = "q=ščř"
		--assert url/fragment = "kovtička"

	--test-- "decode-url-unicode"
		url: decode-url http://švéd:břéťa@example.com:8080/get?q=ščř#kovtička
		--assert url/scheme   = 'http
		--assert url/user     = "švéd"
		--assert url/pass     = "břéťa"
		--assert url/host     = "example.com"
		--assert url/port     = 8080
		--assert url/path     = %/
		--assert url/target   = %get
		--assert url/query    = "q=ščř"
		--assert url/fragment = "kovtička"

	--test-- "decode-url http://host?query"
		url: decode-url http://host?query
		--assert url/host  = "host"
		--assert url/query = "query"

	--test-- "query with encoded &"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2012
		u2: load molded: mold u1: to-url "http://a.b.c/d?e=f%26&g=h"
		--assert molded = "http://a.b.c/d?e=f%2526&g=h"
		--assert u1 = u2
		--assert (decode-url u1) = [scheme: 'http host: "a.b.c" path: %/ target: %d query: "e=f%26&g=h"]

	--test-- "query with @ and : in the target"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2207
		--assert (decode-url http://localhost:8008/@xxx:localhost)
				== [scheme: 'http host: "localhost" port: 8008 path: %/ target: %%40xxx%3Alocalhost]
		--assert (decode-url http://localhost:8008/_matrix/client/api/v1/join/#test2:localhost?access_token=shortened)
				== [scheme: 'http host: "localhost" port: 8008 path: %/_matrix/client/api/v1/join/ fragment: "test2:localhost?access_token=shortened"]

	--test-- "decode-url with space in the password part"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1333
		--assert (decode-url to-url "ftp://a:b c@d") = [scheme: 'ftp user: "a" pass: "b c" host: "d"]

	--test-- "decode-url with cetain chars in host/path"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1334
		--assert (decode-url to-url "ftp://a b") = [scheme: 'ftp host: "a b"]
		--assert (decode-url ftp://a!b) = [scheme: 'ftp host: "a!b"]
		;@@ https://github.com/Oldes/Rebol-issues/issues/475
		--assert (decode-url http://www.my-host.com/!photo)
				== [scheme: 'http host: "www.my-host.com" path: %/ target: %!photo]

	--test-- "decode-url tcp://:9000"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1275
		url: decode-url tcp://:9000
		--assert url/scheme = 'tcp
		--assert url/port   = 9000

	--test-- "decode-url with all ascii chars"
		for i 0 127 1 [
			--assert block? decode-url join http://test/q= to-char i
		]

	--test-- "decode-url urn"
		--assert (decode-url urn:example:animal:ferret:nose)
				== [scheme: 'urn path: "example" target: "animal:ferret:nose"]
		--assert (decode-url urn:isan:0000-0000-9E59-0000-O-0000-0000-2)
				== [scheme: 'urn path: "isan" target: "0000-0000-9E59-0000-O-0000-0000-2"]
		--assert (decode-url urn:isbn:0451450523)
				== [scheme: 'urn path: "isbn" target: "0451450523"]
		--assert (decode-url "urn:sici:1046-8188(199501)13:1%3C69:FTTHBI%3E2.0.TX;2-4")
				== [scheme: 'urn path: "sici" target: "1046-8188(199501)13:1%3C69:FTTHBI%3E2.0.TX;2-4"]

	--test-- "decode-url with @ in path"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2489
		--assert (decode-url https://medium.com/@karaiskc/understanding-apples-binary-property-list-format-281e6da00dbd)
				== [scheme: 'https host: "medium.com" path: %/@karaiskc/ target: %understanding-apples-binary-property-list-format-281e6da00dbd]
		--assert (decode-url xxx://u:p@host/path@/file?@query#anchor)
				== [scheme: 'xxx user: "u" pass: "p" host: "host" path: %/path@/ target: %file query: "@query" fragment: "anchor"]
		;@@ https://github.com/Oldes/Rebol-issues/issues/482
		--assert (decode-url file:///foo@.bar)
				== [scheme: 'file path: %/ target: %"foo@.bar"]

	--test-- "Gregg's tests"
		;@@ from https://gist.github.com/greggirwin/207149d46441cd48a1426e60926a7d25
		--assert (decode-url foo://)
				== [scheme: 'foo]
		--assert (decode-url foo:/a/b/c)
				== [scheme: 'foo path: %/a/b/ target: %c]
		--assert (decode-url foo://example.com:8042/over/there?name=ferret#nose)
				== [scheme: 'foo host: "example.com" port: 8042 path: %/over/ target: %there query: "name=ferret" fragment: "nose"]
		--assert (decode-url ftp://ftp.is.co.za/rfc/rfc1808.txt)
				== [scheme: 'ftp host: "ftp.is.co.za" path: %/rfc/ target: %rfc1808.txt]
		--assert (decode-url "ldap://[2001:db8::7]/c=GB?objectClass?one")
				== [scheme: 'ldap host: "[2001:db8::7]" path: %/ target: %c=GB query: "objectClass?one"]
		--assert (decode-url mailto:John.Doe@example.com)
				== [scheme: 'mailto target: "John.Doe@example.com"]
		--assert (decode-url tel:+1-816-555-1212)
				== [scheme: 'tel target: %+1-816-555-1212]

===end-group===


~~~end-file~~~