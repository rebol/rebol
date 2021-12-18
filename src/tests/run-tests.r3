Rebol [
	Title:   "Runs all available unit tests"
	Author:  ["Peter W A Wood" "Oldes"]
	File:    %run-tests.r3
	Needs:   [%quick-test-module.r3]
]

;- check speed of available checksum methods
;do %test-checksums.r3

;- run unit tests..
dt [ ;- delta time
	***start-run*** "All tests"
	;recycle/torture
	failed-units: copy []
	foreach file [
		%units/bbcode-test.r3
		%units/bincode-test.r3
		%units/bitset-test.r3
		%units/call-test.r3
		%units/codecs-test.r3
		%units/codecs-test-pdf.r3
		%units/compare-test.r3
		%units/compress-test.r3
		%units/conditional-test.r3
		%units/copy-test.r3
		%units/crash-test.r3
		%units/datatype-test.r3
		%units/date-test.r3
		%units/decimal-test.r3
		%units/delect-test.r3
		%units/enbase-test.r3
		%units/enum-test.r3
		%units/error-test.r3
		%units/evaluation-test.r3
		%units/event-test.r3
		%units/gob-test.r3
		%units/handle-test.r3
		%units/file-test.r3
		%units/format-test.r3
		%units/func-test.r3
		%units/image-test.r3
		%units/integer-test.r3
		%units/issue-test.r3
		%units/lexer-test.r3
		%units/load-test.r3
		%units/make-test.r3
		%units/map-test.r3
		%units/module-test.r3
		%units/mold-test.r3
		%units/money-test.r3
		%units/object-test.r3
		%units/os-test.r3
		%units/pair-test.r3
		%units/parse-test.r3
		%units/percent-test.r3
		%units/port-test.r3
		%units/power-test.r3
		%units/protect-test.r3
		%units/series-test.r3
		%units/struct-test.r3
		%units/task-test.r3
		%units/time-test.r3
		%units/tuple-test.r3
		%units/typeset-test.r3
		%units/vector-test.r3
		%units/word-test.r3
		;- cryptography tests:
		%units/aes-test.r3
		%units/chacha20-test.r3
		%units/checksum-test.r3
		%units/dh-test.r3
		%units/crypt-test.r3
		%units/poly1305-test.r3
		%units/rc4-test.r3
		%units/rsa-test.r3
		;- known issues - waiting for fix
		;%units/_known-issues_.r3
		;%units/xx-test.r3
		;%units/_test.r3
	][
		try/except [wrap load file] func[error][
			repend failed-units [file error]
		]
	]


	recycle/torture
	recycle
	stats/show

	print "System profiler:"
	print dump-obj stats/profile

	print "------------------------------------------------------------"
	prin "Checking memory...   " evoke 2 print "OK"
	prin "Checking bindings... " evoke 3 print "OK"
	print "------------------------------------------------------------"
	
	***end-run***
	recycle/on

	unless empty? failed-units [
		print "------------------------------------------------------------"
		print "FAILED units files:"
		foreach [file error] failed-units [
			?? file
			?? error
		]
	]


]
