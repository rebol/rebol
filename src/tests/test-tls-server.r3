Rebol [
	Title: "Test TLS Server"
	File:  %test-tls-server.r3
]

;do %..\mezz\prot-tls.reb

; maximum information printed!
system/schemes/tls/set-verbose 4

;; In case that script is run multiple times from console,
;; close the previous instance.
try [close server]

server: open [
	scheme: 'tls
	port:   8435
	config: [
		;; TLS server requires valid certificate with a public key
		;; and its private key!
		;; These 2 are generated using OpenSSL command:
		;- openssl req -new -newkey rsa:4096 -x509 -sha256 -days 365 -nodes -out MyCertificate.crt -keyout MyKey.key
		;; For public use you may need authorized certificate (https://letsencrypt.org/)
		certificates: %units/files/MyCertificate.crt
		private-key:  %units/files/MyKey.key
	]
]

;? server
;? server/state

sys/log/info 'REBOL ["Waiting for connections..." server/spec/ref]

wait [server]
