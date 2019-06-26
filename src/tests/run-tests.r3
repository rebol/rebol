Rebol [
	Title:   "Runs all available unit tests"
	Author:  ["Peter W A Wood" "Oldes"]
	File:    %run-tests.r3
	Needs:   [%quick-test-module.r3]
]

dt [ ;- delta time
	***start-run*** "All tests"
	;recycle/torture
	wrap load %units/mold-test.r3
	wrap load %units/lexer-test.r3
	wrap load %units/evaluation-test.r3
	wrap load %units/enbase-test.r3
	wrap load %units/map-test.r3
	wrap load %units/integer-test.r3
	wrap load %units/decimal-test.r3
	wrap load %units/money-test.r3
	wrap load %units/power-test.r3
	wrap load %units/mezz-crypt-test.r3
	wrap load %units/rc4-test.r3
	wrap load %units/aes-test.r3
	wrap load %units/chacha20-test.r3
	wrap load %units/poly1305-test.r3
	wrap load %units/rsa-test.r3
	wrap load %units/dh-test.r3
	wrap load %units/port-test.r3
	wrap load %units/checksum-test.r3
	wrap load %units/enum-test.r3
	wrap load %units/conditional-test.r3
	wrap load %units/protect-test.r3
	wrap load %units/crash-test.r3
	wrap load %units/bincode-test.r3
	wrap load %units/codecs-test.r3
	wrap load %units/series-test.r3
	wrap load %units/vector-test.r3
	wrap load %units/object-test.r3
	wrap load %units/compress-test.r3
	wrap load %units/date-test.r3
	wrap load %units/image-test.r3
	wrap load %units/file-test.r3
	wrap load %units/func-test.r3

	recycle/torture
	recycle
	stats/show

	print "System profiler:"
	print dump-obj stats/profile

	print "------------------------------------------------------------"
	***end-run***
	recycle/on
]
