Rebol [
	Title: "Test async TCP port scanner"
	File:  %test-port-scanner.r3
	Date: 16-Feb-2021
]

scanner: import 'port-scanner
	
;- higher batch size means higher CPU use!
;scanner/batch-size: 10000
;- low timeout may result in not founding all listening ports!
;- if the scenner is used on non local ip, the timeout should
;- count with netword latency!
;scanner/timeout: 0:0:0.25

scan-ports/host 1 65535 127.0.0.1

if system/options/script [
	?? scanner/found
	ask "^/DONE"
]