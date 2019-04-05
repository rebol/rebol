Rebol [
	Title: "Test HTTPD Scheme"
	Date: 1-Apr-2019
	Author: "Oldes"
	File: %test-httpd.r3
	Version: 0.4.0
	Note: {
		To test POST method from Rebol console, try this:
		```
		write http://localhost:8081 {msg=hello}
		write http://localhost:8081 [post [user-agent: "bla"] "hello"]

		```
	}
]

do %../modules/httpd.r3

system/options/log/httpd: 3 ; for verbose output

my-actor: object [
	On-Accept: func [info [object!]][
		; allow only connections from localhost
		; TRUE = accepted, FALSE = refuse
		find [ 127.0.0.1 ] info/remote-ip 
	]
	On-Header: func [ctx [object!]][
		switch ctx/inp/target/file [
			%form/     [
				; path rewrite...
				; http://localhost:8081/form/ is now same like http://localhost:8081/form.html
				ctx/inp/target/file: %form.html
				; request processing will continue
			]
			%form.htm
			%form.html [
				ctx/out/status: 301 ;= Moved Permanently
				ctx/out/header/Location: %/form/
				; request processing will stop with redirection response
			]
		]
	]
	On-Post-Received: func [ctx [object!]][
		ctx/out/header/Content-Type: "text/html; charset=UTF-8"
		ctx/out/content: rejoin either object? ctx/inp/content [
			[
				"URL-encoded data:<pre>" to-string ctx/inp/content/original </pre>
				"<br/>Parsed:<pre>" mold ctx/inp/content/values </pre>
				"<br/>Request header:<pre>" mold ctx/inp/header </pre>
			]
		][
			[	"Received " length? ctx/inp/content " bytes." ]
		]
	]

]

http-server/config/actor 8081 [
	root: %httpd-root/
	server-name: "nginx"  ;= it's possible to hide real server name
	keep-alive: [15 100] ;= [timeout max-requests] or FALSE to turn it off
] my-actor
