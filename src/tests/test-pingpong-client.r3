Rebol [
	Title: "Test Ping Pong Client"
	File: %test-pingpong-client.r3
	Note: {
		Based on original Port examples which can be found at:
		https://web.archive.org/web/20131012055435/http://www.rebol.net:80/wiki/Port_Examples
	}
]

print "Ping pong client"

ping-count: 0

client: open tcp://127.0.0.1:8080

client/awake: func [event] [
    ;probe event/type
    switch event/type [
        lookup [open event/port]
        connect [write event/port to-binary "ping!"]
        wrote [
            print "Client sent ping to server"
            read event/port
        ]
        read [
            print ["Server said:" to-string event/port/data]
            if (++ ping-count) > 50 [return true]
            clear event/port/data
            write event/port to-binary "ping!"
        ]
    ]
    false
]

wait [client 10] ; timeout after 10 seconds
close client
wait 2