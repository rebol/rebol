Rebol [
	Title:    "Test HTTP protocol"
	Date:     18-Mar-2025
	Author:   "Oldes"
	File:     %test-http.r3
	Version:  0.0.1
	Needs:    3.18.4 ;; tests features available since that version
	Note: {}
]

read-json: function[url][
	try [all [
		res: read/binary/all url
		res/1 == 200
		res/2/Content-Type == "application/json"
		decode 'json res/3
	]]
]

foreach [label test] [
	"Set first cookie..." [
		result: read-json https://httpbun.org/cookies/set/name/Rebol
		probe result
		result/cookies/name == "Rebol"
	]
	"Set another 2 cookies" [
		result: read-json https://httpbun.org/cookies/set?a=1&b=2%202
		probe result
		result/cookies == #[name: "Rebol" a: "1" b: "2 2"]
	]
	"Delete 2 cookies" [
		result: read-json https://httpbun.org/cookies/delete?a=&b=
		probe result
		result/cookies == #[name: "Rebol"]
	]
][
	print-hline
	print as-yellow label
	either true == try probe test [
		print as-green "OK"
	][	print as-purple "FAILED!"]
]
