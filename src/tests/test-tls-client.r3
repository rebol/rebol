Rebol [
	Title:   "Test TLS Client"
	File:    %test-tls-client.r3
	Purpose: {Test connection with TLS server: %test-tls-server.r3}
]

system/schemes/tls/set-verbose 4
print read https://127.0.0.1:8435

if system/options/script [ask "DONE"]