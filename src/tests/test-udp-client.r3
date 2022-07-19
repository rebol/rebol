Rebol [
	Title: "Test UDP client"
	File: %test-udp-client.r3
	Note: https://github.com/Oldes/Rebol-issues/issues/1803
]

; start test UDP server...
launch probe to-real-file %test-udp-server.r3

; wait some time for server to boot...
wait 0:0:0.1

stdout: system/ports/output

; open port for output...
port: open udp://127.0.0.1:1189
port/awake: func[event][
	print ["[UDP Client] event:" event/type]
	;; console output is buffered and so messages could come out of order;
	;; to prevent it, just use the `flush`! 
	flush stdout
	true
]
; wait for port to be opened (required on Windows!)...
wait port

write-udp:  func[msg [string!]][
	print ["[UDP Client] sending:" as-yellow mold msg]
	flush stdout ;= flush console buffer!
	write port msg
]
write-udp "Hello"
write-udp "Rebol"
wait 1 ;= pretending some work here...
write-udp "I'm done!"
write-udp "quit"

print as-red "CLIENT DONE"
wait 0.5
if system/options/script [
	ask "ENTER to quit"
]
