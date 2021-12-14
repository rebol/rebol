Rebol [
	Title: "Test UDP server"
	File: %test-udp-server.r3
	Note: https://github.com/rebol/rebol/pull/218
]

print [as-red "Opening UDP server listening on port" as-yellow 1189]

udp-server: try/except [open udp://:1189][
	print as-purple "Failed to listen on UDP port 1189!"
	quit
]

udp-server/awake: func [event] [
	print ["UDP event:" as-yellow event/type]
	true
]

forever [
	wait read udp-server
	str: to string! udp-server/data
	print ["UDP input:" as-green mold str]
	if str = "quit" [close udp-server break]
	clear udp-server/data
]
print as-red "SERVER DONE"
wait 0:0:1