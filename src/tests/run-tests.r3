Rebol [
	Title:   "Runs all available unit tests"
	Author:  ["Peter W A Wood" "Oldes"]
	File:    %run-tests.r3
	Needs:   [%quick-test-module.r3]
]

dt [ ;- delta time
	***start-run*** "All tests"
	;recycle/torture
    wrap load %units/bbcode-test.r3
    wrap load %units/bincode-test.r3
    wrap load %units/codecs-test.r3
    wrap load %units/compress-test.r3
    wrap load %units/conditional-test.r3
    wrap load %units/crash-test.r3
    wrap load %units/datatype-test.r3
    wrap load %units/date-test.r3
    wrap load %units/decimal-test.r3
    wrap load %units/delect-test.r3
    wrap load %units/enbase-test.r3
    wrap load %units/enum-test.r3
    wrap load %units/evaluation-test.r3
    wrap load %units/event-test.r3
    wrap load %units/gob-test.r3
    wrap load %units/file-test.r3
    wrap load %units/func-test.r3
    wrap load %units/image-test.r3
    wrap load %units/integer-test.r3
    wrap load %units/issue-test.r3
    wrap load %units/lexer-test.r3
    wrap load %units/load-test.r3
    wrap load %units/map-test.r3
    wrap load %units/mold-test.r3
    wrap load %units/money-test.r3
    wrap load %units/object-test.r3
    wrap load %units/pair-test.r3
    wrap load %units/parse-test.r3
    wrap load %units/percent-test.r3
    wrap load %units/port-test.r3
    wrap load %units/power-test.r3
    wrap load %units/protect-test.r3
    wrap load %units/series-test.r3
    wrap load %units/task-test.r3
    wrap load %units/time-test.r3
    wrap load %units/tuple-test.r3
    wrap load %units/typeset-test.r3
    wrap load %units/vector-test.r3
    wrap load %units/word-test.r3
    ;- cryptography tests:
    wrap load %units/aes-test.r3
    wrap load %units/chacha20-test.r3
    wrap load %units/checksum-test.r3
    wrap load %units/dh-test.r3
    wrap load %units/mezz-crypt-test.r3
    wrap load %units/poly1305-test.r3
    wrap load %units/rc4-test.r3
    wrap load %units/rsa-test.r3

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


]
