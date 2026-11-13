Rebol [
	Title: "Test Ping Pong Server"
	File: %test-pingpong-server.r3
	Note: {
		Based on original Port examples which can be found at:
		https://web.archive.org/web/20131012055435/http://www.rebol.net:80/wiki/Port_Examples
	}
]

print "Ping pong server"

server: open tcp://:8080

server/awake: func [event /local port] [
    if event/type = 'accept [
        port: first event/port
        port/awake: func [event] [
            ;probe event/type
            switch event/type [
                read [
                    print ["Client said:" to-string event/port/data]
                    clear event/port/data
                    write event/port to-binary "pong!"
                ]
                wrote [
                    print "Server sent pong to client"
                    read event/port
                ]
                close [
                    close event/port
                    return true
                ]
            ]
            false
        ]
        read port
    ]
    false
]

wait [server 30]
close server