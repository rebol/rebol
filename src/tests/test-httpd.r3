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
system/options/quiet: false

; make sure that there is the directory for logs
make-dir/deep %httpd-root/logs/

humans.txt: {
       __
      (  )
       ||
       ||
   ___|""|__.._
  /____________\
  \____________/~~~> http://github.com/oldes/
}

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
	On-Header: func [ctx [object!] /local path][
		path: ctx/inp/target/file
		;- detect some of common hacking attempts...
		if parse path [
			some [
				; common scripts, which we don't use
				  #"." [
				  	  %php
				  	| %aspx
				  	| %cgi
				][end | #"?" | #"#"] break
				; common hacking attempts to root folders...
				| #"/" [
					  %ecp/      ; we are not an exchange server
					| %mifs/     ; either not MobileIron (https://stackoverflow.com/questions/67901776/what-does-the-line-mifs-services-logservice-mean)
					| %GponForm/ ; nor Gpon router (https://www.vpnmentor.com/blog/critical-vulnerability-gpon-router/)
					| %.env end  ; https://stackoverflow.com/questions/64109005/do-these-env-get-requests-from-localhost-indicate-an-attack
				] break
				| end reject ; nothing above was detected so return false
				| 1 skip
			] to end
		][
			ctx/out/status: 418 ;= I'm a teapot
			ctx/out/header/Content-Type: "text/plain; charset=UTF-8"
			ctx/out/content: "Your silly hacking attempt was detected!"
			exit
		]
		;- serve valid content...
		switch path [
			%/form/     [
				; path rewrite...
				; http://localhost:8081/form/ is now same like http://localhost:8081/form.html
				ctx/inp/target/file: %/form.html
				; request processing will continue
			]
			%/form.htm
			%/form.html [
				ctx/out/status: 301 ;= Moved Permanently
				ctx/out/header/Location: %/form/
				; request processing will stop with redirection response
			]
			%/plain/ [
				ctx/out/status: 200
				ctx/out/header/Content-Type: "text/plain; charset=UTF-8"
				ctx/out/content: "hello"
				; request processing will stop with response 200 serving the plain text content
			]
			%/humans.txt [
				;@@ https://codeburst.io/all-about-humans-humans-txt-actually-f571d37f92d2
				;-- serving the content directly from the memory
				ctx/out/status: 200
				ctx/out/header/Content-Type: "text/plain; charset=UTF-8"
				ctx/out/content: humans.txt
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
