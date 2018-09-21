Rebol [
	Title:   "Runs all available unit tests"
	Author:  ["Peter W A Wood" "Oldes"]
	File:    %run-tests.r3
	Needs:   [%quick-test-module.r3]
]

***start-run*** "All tests"

wrap load %units/lexer-test.r3
wrap load %units/enbase-test.r3
wrap load %units/map-test.r3
;wrap load %units/integer-test.r3
wrap load %units/power-test.r3
wrap load %units/mezz-crypt-test.r3
wrap load %units/rsa-test.r3
wrap load %units/dh-test.r3
wrap load %units/port-test.r3

wrap load %units/crash-test.r3

***end-run***
