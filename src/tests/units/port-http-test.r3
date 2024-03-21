Rebol [
	Title:   "Rebol3 HTTP port! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %port-http-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "port-http"

;; it looks that httpbin.org has long response times:/
;; so let's give it more time...
system/schemes/http/spec/timeout: 30

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
		--assert #{47494638} = read/binary/part http://www.rebol.com/graphics/reb-logo.gif 4
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

	--test-- "read/seek/part"
		; first results without read/part
		--assert "<!doctype html>" = copy/part str: read/string http://example.com 15
		--assert "<!doctype html>" = copy/part skip str 0 15
		--assert   "doctype html>" = copy/part skip str 2 13
		; using read/part
		--assert "<!doctype html>" = read/part http://example.com 15
		--assert "<!doctype html>" = read/seek/part/string http://example.com 0 15
		--assert   "doctype html>" = read/seek/part/string http://example.com 2 13
		; when used /seek without /string, than result is always binary!
		--assert #{3C21646F63747970652068746D6C3E} = read/seek/part http://example.com 0 15
		--assert     #{646F63747970652068746D6C3E} = read/seek/part http://example.com 2 13

	--test-- "read/lines/seek/part"
		--assert ["<!doctype html>" "<html>" "<head>"] = read/lines/part http://example.com 3
		--assert ["<!doctype html>" "<html>" "<head>"] = read/lines/seek/part http://example.com 0 3
		--assert                   ["<html>" "<head>"] = read/lines/seek/part http://example.com 1 2
		; using /lines with /binary is not allowed
		--assert all [error? e: try [read/lines/binary http://example.com] e/id = 'bad-refine]
		--assert all [block? b: try [read/lines/seek http://example.com 2] b/1 = "<head>"]

	--test-- "read/string/binary"
		--assert all [error? e: try [read/string/binary http://example.com] e/id = 'bad-refines]

===end-group===

===start-group=== "HTTP scheme - Successful responses"
	--test-- "success http"
		--assert all [ ;= OK
			block? res: try [read/all http://httpbin.org/status/200]
			res/1 = 200
		]
		--assert all [ ;= Created
			block? res: try [read/all http://httpbin.org/status/201]
			res/1 = 201
		]
		--assert all [ ;= Accepted
			block? res: try [read/all http://httpbin.org/status/202]
			res/1 = 202
		]
		--assert all [ ;= Non-Authoritative Information
			block? res: try [read/all http://httpbin.org/status/203]
			res/1 = 203
		]
		--assert all [ ;= No Content
			block? res: try [read/all http://httpbin.org/status/204]
			res/1 = 204
			none? res/3
		]
		--assert all [ ;= Reset Content
			block? res: try [read/all http://httpbin.org/status/205]
			res/1 = 205
		]
		--assert all [ ;= Partial Content
			block? res: try [read/all http://httpbin.org/status/206]
			res/1 = 206
		]
	--test-- "success https"
		--assert all [ ;= OK
			block? res: try [read/all https://httpbin.org/status/200]
			res/1 = 200
		]
		--assert all [ ;= Created
			block? res: try [read/all https://httpbin.org/status/201]
			res/1 = 201
		]
		--assert all [ ;= Accepted
			block? res: try [read/all https://httpbin.org/status/202]
			res/1 = 202
		]
		--assert all [ ;= Non-Authoritative Information
			block? res: try [read/all https://httpbin.org/status/203]
			res/1 = 203
		]
		--assert all [ ;= No Content
			block? res: try [read/all https://httpbin.org/status/204]
			res/1 = 204
			none? res/3
		]
		--assert all [ ;= Reset Content
			block? res: try [read/all https://httpbin.org/status/205]
			res/1 = 205
		]
		--assert all [ ;= Partial Content
			block? res: try [read/all https://httpbin.org/status/206]
			res/1 = 206
		]
===end-group===

===start-group=== "HTTP scheme - Redirection messages"
	--test-- "redirect http"
		--assert all [ ;= Multiple Choices
			block? res: try [read/all http://httpbin.org/status/300]
			res/1 = 300
			empty? res/3
		]
		--assert all [ ;= Moved Permanently
			block? res: try [read/all http://httpbin.org/status/301]
			res/1 = 200
		]
		--assert all [ ;= Found
			block? res: try [read/all http://httpbin.org/status/302]
			res/1 = 200
		]
		--assert all [ ;= See Other
			block? res: try [read/all http://httpbin.org/status/303]
			res/1 = 200
		]
		--assert all [ ;= Not Modified
			block? res: try [read/all http://httpbin.org/status/304]
			res/1 = 304
			none? res/3
		]
		--assert all [ ;= Use Proxy
			block? res: try [read/all http://httpbin.org/status/305]
			res/1 = 200
		]
	--test-- "redirect https"
		--assert all [ ;= Multiple Choices
			block? res: try [read/all https://httpbin.org/status/300]
			res/1 = 300
			empty? res/3
		]
		--assert all [ ;= Moved Permanently
			block? res: try [read/all https://httpbin.org/status/301]
			res/1 = 200
		]
		--assert all [ ;= Found
			block? res: try [read/all https://httpbin.org/status/302]
			res/1 = 200
		]
		--assert all [ ;= See Other
			block? res: try [read/all https://httpbin.org/status/303]
			res/1 = 200
		]
		--assert all [ ;= Not Modified
			block? res: try [read/all https://httpbin.org/status/304]
			res/1 = 304
			none? res/3
		]
		--assert all [ ;= Use Proxy
			block? res: try [read/all https://httpbin.org/status/305]
			res/1 = 200
		]
===end-group===

===start-group=== "HTTP scheme - Client error responses"
	--test-- "client error http"
		--assert all [ ;= Bad Request
			block? res: try [read/all http://httpbin.org/status/400]
			res/1 = 400
			empty? res/3
		]
		--assert all [ ;= Unauthorized
			block? res: try [read/all http://httpbin.org/status/401]
			res/1 = 401
		]
		--assert all [ ;= Payment Required
			block? res: try [read/all http://httpbin.org/status/402]
			res/1 = 402
			"Fuck you, pay me!" = to string! res/3
		]
		--assert all [ ;= Forbidden
			block? res: try [read/all http://httpbin.org/status/403]
			res/1 = 403
		]
		--assert all [ ;= Not Found
			block? res: try [read/all http://httpbin.org/status/404]
			res/1 = 404
		]
		--assert all [ ;= Method Not Allowed
			block? res: try [read/all http://httpbin.org/status/405]
			res/1 = 405
		]
		--assert all [ ;= Not Acceptable
			block? res: try [read/all http://httpbin.org/status/406]
			res/1 = 406
		]
		--assert all [ ;= Proxy Authentication Required
			block? res: try [read/all http://httpbin.org/status/407]
			res/1 = 407
		]
		--assert all [ ;= Request Timeout
			block? res: try [read/all http://httpbin.org/status/408]
			res/1 = 408
		]
		--assert all [ ;= Conflict
			block? res: try [read/all http://httpbin.org/status/409]
			res/1 = 409
		]
		--assert all [ ;= Gone
			block? res: try [read/all http://httpbin.org/status/410]
			res/1 = 410
		]
		--assert all [ ;= Length Required
			block? res: try [read/all http://httpbin.org/status/411]
			res/1 = 411
		]
		--assert all [ ;= Precondition Failed
			block? res: try [read/all http://httpbin.org/status/412]
			res/1 = 412
		]
		--assert all [ ;= Payload Too Large
			block? res: try [read/all http://httpbin.org/status/413]
			res/1 = 413
		]
		--assert all [ ;= URI Too Long
			block? res: try [read/all http://httpbin.org/status/414]
			res/1 = 414
		]
		--assert all [ ;= Unsupported Media Type
			block? res: try [read/all http://httpbin.org/status/415]
			res/1 = 415
		]
		--assert all [ ;= Range Not Satisfiable
			block? res: try [read/all http://httpbin.org/status/416]
			res/1 = 416
		]
	--test-- "client error https"
		--assert all [ ;= Bad Request
			block? res: try [read/all https://httpbin.org/status/400]
			res/1 = 400
			empty? res/3
		]
		--assert all [ ;= Unauthorized
			block? res: try [read/all https://httpbin.org/status/401]
			res/1 = 401
		]
		--assert all [ ;= Payment Required
			block? res: try [read/all https://httpbin.org/status/402]
			res/1 = 402
			"Fuck you, pay me!" = to string! res/3
		]
		--assert all [ ;= Forbidden
			block? res: try [read/all https://httpbin.org/status/403]
			res/1 = 403
		]
		--assert all [ ;= Not Found
			block? res: try [read/all https://httpbin.org/status/404]
			res/1 = 404
		]
		--assert all [ ;= Method Not Allowed
			block? res: try [read/all https://httpbin.org/status/405]
			res/1 = 405
		]
		--assert all [ ;= Not Acceptable
			block? res: try [read/all https://httpbin.org/status/406]
			res/1 = 406
		]
		--assert all [ ;= Proxy Authentication Required
			block? res: try [read/all https://httpbin.org/status/407]
			res/1 = 407
		]
		--assert all [ ;= Request Timeout
			block? res: try [read/all https://httpbin.org/status/408]
			res/1 = 408
		]
		--assert all [ ;= Conflict
			block? res: try [read/all https://httpbin.org/status/409]
			res/1 = 409
		]
		--assert all [ ;= Gone
			block? res: try [read/all https://httpbin.org/status/410]
			res/1 = 410
		]
		--assert all [ ;= Length Required
			block? res: try [read/all https://httpbin.org/status/411]
			res/1 = 411
		]
		--assert all [ ;= Precondition Failed
			block? res: try [read/all https://httpbin.org/status/412]
			res/1 = 412
		]
		--assert all [ ;= Payload Too Large
			block? res: try [read/all https://httpbin.org/status/413]
			res/1 = 413
		]
		--assert all [ ;= URI Too Long
			block? res: try [read/all https://httpbin.org/status/414]
			res/1 = 414
		]
		--assert all [ ;= Unsupported Media Type
			block? res: try [read/all https://httpbin.org/status/415]
			res/1 = 415
		]
		--assert all [ ;= Range Not Satisfiable
			block? res: try [read/all https://httpbin.org/status/416]
			res/1 = 416
		]

===end-group===


===start-group=== "HTTP scheme - Server error responses"
	--test-- "server error http"
		--assert all [ ;= Internal Server Error
			block? res: try [read/all http://httpbin.org/status/500]
			res/1 = 500
			empty? res/3
		]
		--assert all [ ;= Not Implemented
			block? res: try [read/all http://httpbin.org/status/501]
			res/1 = 501
		]
		--assert all [ ;= Bad Gateway
			block? res: try [read/all http://httpbin.org/status/502]
			res/1 = 502
		]
		--assert all [ ;= Service Unavailable
			block? res: try [read/all http://httpbin.org/status/503]
			res/1 = 503
		]
		--assert all [ ;= Gateway Timeout
			block? res: try [read/all http://httpbin.org/status/504]
			res/1 = 504
		]
		--assert all [ ;= HTTP Version Not Supported
			block? res: try [read/all http://httpbin.org/status/505]
			res/1 = 505
		]
		--assert all [ ;= Variant Also Negotiates
			block? res: try [read/all http://httpbin.org/status/506]
			res/1 = 506
		]
		--assert all [ ;= Insufficient Storage
			block? res: try [read/all http://httpbin.org/status/507]
			res/1 = 507
		]
		--assert all [ ;= Loop Detected
			block? res: try [read/all http://httpbin.org/status/508]
			res/1 = 508
		]
		--assert all [ ;= Not Extended
			block? res: try [read/all http://httpbin.org/status/510]
			res/1 = 510
		]
		--assert all [ ;= Network Authentication Required
			block? res: try [read/all http://httpbin.org/status/511]
			res/1 = 511
		]
	--test-- "server error https"
		--assert all [ ;= Internal Server Error
			block? res: try [read/all http://httpbin.org/status/500]
			res/1 = 500
			empty? res/3
		]
		--assert all [ ;= Not Implemented
			block? res: try [read/all http://httpbin.org/status/501]
			res/1 = 501
		]
		--assert all [ ;= Bad Gateway
			block? res: try [read/all http://httpbin.org/status/502]
			res/1 = 502
		]
		--assert all [ ;= Service Unavailable
			block? res: try [read/all http://httpbin.org/status/503]
			res/1 = 503
		]
		--assert all [ ;= Gateway Timeout
			block? res: try [read/all http://httpbin.org/status/504]
			res/1 = 504
		]
		--assert all [ ;= HTTP Version Not Supported
			block? res: try [read/all http://httpbin.org/status/505]
			res/1 = 505
		]
		--assert all [ ;= Variant Also Negotiates
			block? res: try [read/all http://httpbin.org/status/506]
			res/1 = 506
		]
		--assert all [ ;= Insufficient Storage
			block? res: try [read/all http://httpbin.org/status/507]
			res/1 = 507
		]
		--assert all [ ;= Loop Detected
			block? res: try [read/all http://httpbin.org/status/508]
			res/1 = 508
		]
		--assert all [ ;= Not Extended
			block? res: try [read/all http://httpbin.org/status/510]
			res/1 = 510
		]
		--assert all [ ;= Network Authentication Required
			block? res: try [read/all http://httpbin.org/status/511]
			res/1 = 511
		]
===end-group===

~~~end-file~~~