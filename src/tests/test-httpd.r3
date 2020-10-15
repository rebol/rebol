Rebol [
	Title: "Test HTTPD Scheme"
	Date: 02-Jul-2020
	Author: "Oldes"
	File: %test-httpd.r3
	Version: 0.6.0
	Note: {
		To test POST method from Rebol console, try this:
		```
		write http://localhost:8081 {msg=hello}
		write http://localhost:8081 [post [user-agent: "bla"] "hello"]

		```
	}
]
secure [%../modules/ allow]
do %../modules/httpd.reb

system/options/log/httpd: 3 ; for verbose output

; make sure that there is the directory for logs
make-dir/deep %httpd-root/logs/

http-server/config/actor 8081 [
	;- Main server configuration
	
	root: %httpd-root/
	server-name: "nginx"  ;= it's possible to hide real server name
	keep-alive: [15 100] ;= [timeout max-requests] or FALSE to turn it off
	log-access: %httpd-root/logs/test-access.log
	log-errors: %httpd-root/logs/test-errors.log
	list-dir?:  #[true]

] [
	;- Server's actor functions

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
			%plain/ [
				ctx/out/status: 200
				ctx/out/header/Content-Type: "text/plain; charset=UTF-8"
				ctx/out/content: "hello"
				; request processing will stop with response 200 serving the plain text content
			]
		]
	]
	On-Post-Received: func [ctx [object!]][
		ctx/out/content: ajoin [
			"<br/>Request header:<pre>" mold ctx/inp/header </pre>
			"Received <code>" ctx/inp/header/Content-Type/1 
			"</code> data:<pre>" mold ctx/inp/content </pre>
		]
	]
]
