Rebol [
	Title: "Test UDP client"
	File: %test-udp.r3
	Note: https://github.com/rebol/rebol/pull/218
]

; start test UDP server...
launch probe to-real-file %test-udp-server.r3

; wait some time for server to boot...
wait 0:0:0.1

; send some data...
port: open udp://127.0.0.1:1189 
; there seems to be a bug in current implementation
; where message is lost if there is no wait between each write!
write port "Hello" wait 0:0:0.1
write port "Rebol" wait 0:0:0.1
write port "quit"
print as-red "CLIENT DONE"
wait 0:0:1