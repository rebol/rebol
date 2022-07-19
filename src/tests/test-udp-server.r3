Rebol [
	Title: "Test UDP server"
	File: %test-udp-server.r3
	Note: https://github.com/Oldes/Rebol-issues/issues/1803
]

print [as-red "Opening UDP server listening on port" as-yellow 1189]

udp-server: try/except [open udp://:1189][
	print as-purple "Failed to listen on UDP port 1189!"
	quit
]
stdout: system/ports/output

udp-server/awake: func [event /local port str] [
	port: event/port
	print ["[UDP Server] event:" event/type]
	switch event/type [
		read [
			str: to string! port/data
			clear port/data
			print ["[UDP Server] received:" as-green mold str]
			if str = "quit" [port/state: 'quit]
		]
	]
	;; console output is buffered and so messages could come out of order;
	;; to prevent it, just use the `flush`! 
	flush stdout 
	true
]
forever [
	wait read udp-server
	if udp-server/state  = 'quit [
		close udp-server
		break
	]
]
 
print as-red "SERVER DONE"