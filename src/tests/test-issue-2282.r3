Rebol [
	Title: "Test for issue #2282"
	Url: https://github.com/Oldes/Rebol-issues/issues/2282
]

file: %issue-2282
try [delete file]
print either all [
	not error? try [
		a: make binary! n: 268435391
		insert/dup a #"a" n
		loop 16 [write/append file a]

		b: make binary! n: 1039
		insert/dup b #"b" n
		write/append file b
	]
	(size? file) = to integer! #{FFFFFFFF}
]["OK"]["FAILED"]
wait 1
try [delete file]