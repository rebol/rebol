Rebol [
	Title: "Test raw TCP read"
	File: %test-raw-tcp-read.r3
	Note: {
		Based on original Carl's example which can be found at:
		http://web.archive.org/web/20130720122919/http://www.rebol.net/wiki/TCP_Port_Open_Issue
	}
]
http-request: {GET /robots.txt HTTP/1.1
Host: www.rebol.com

}

wp: make port! tcp://www.rebol.com

wp/awake: func [event /local port] [
    port: event/port
    print [as-green "==TCP-event:" as-red event/type]
    switch/default event/type [
        read [print ["^/read:" length? port/data] read port]
        wrote [read port]
        lookup [
        	print query port [remote-ip remote-port]
        	open port
        ]
        connect [write port http-request]
    ][true]
]
try [
	open wp
	either port? wait [wp 15][
		print to string! wp/data
	][
		print as-red "Timeout!"
	]
	close wp
]
print "DONE"
wait 0:0:5