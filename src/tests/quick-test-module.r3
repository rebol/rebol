Rebol [
	Title:   "Rebol simple testing framework"
	Author:  ["Peter W A Wood" "Oldes"]
	File:    %quick-test-module.r3
	Version: 0.3.0
	Rights:  "Copyright (C) 2012-2018 Peter W A Wood, Oldes. All rights reserved."
	License: "BSD-3 - https://github.com/red/red/blob/master/BSD-3-License.txt"
	History: [
		0.3.0 "Oldes: modified to be as a Rebol3's module"
		0.2.0 "Peter's version, which he was using in Red development with some modifications by Oldes fo use with R3"
	]
	Name:    Quick-test
	Type:    module
	Exports: [
		***start-run***
		~~~start-file~~~
		===start-group===
		--test--
		--red--
		--assert
		--assertf~=
		===end-group===
		~~~end-file~~~
		***end-run***
	]
]

;- counters
qt-run-tests: 0 
qt-run-asserts: 0
qt-run-passes: 0
qt-run-failures: 0
qt-run-incompatible: 0
qt-file-tests: 0 
qt-file-asserts: 0 
qt-file-passes: 0 
qt-file-failures: 0
qt-file-incompatible: 0

;- names
qt-run-name:
qt-file-name:
qt-group-name:
qt-test-name: none

;- group switches
qt-group-name-not-printed: true
qt-group?: false
qt-red-only: false

;-state
was-quiet: false



qt-init-group: does [
	qt-group-name-not-printed: true
	qt-group?: false
	qt-group-name: ""
]

qt-init-run: does [
	qt-run-tests: 
	qt-run-asserts:
	qt-run-passes:
	qt-run-failures:
	qt-run-incompatible: 0
	qt-init-group
]

qt-init-file: does [
	qt-file-tests:
	qt-file-asserts:
	qt-file-passes:
	qt-file-failures:
	qt-file-incompatible: 0
	qt-init-group
]

start-run: func[
    title [string!]
][
	was-quiet: system/options/quiet
	system/options/quiet: true
	qt-init-run
	qt-run-name: title
	prin "^[[7m***Starting***^[[0m " 
	print title
]

start-file: func [
	title [string!]
][
	qt-init-file
	prin "^[[7m~~~started test~~~^[[0m "
	print title
	qt-file-name: title
	qt-group?: false
]

start-group: func [
 	title [string!]
][
	qt-group-name: title
	qt-group?: true
]

start-test: func [
	title [string!]
][
	qt-test-name: title
	qt-file-tests: qt-file-tests + 1
	qt-red-only: false
]

as-red-only: does [
	qt-red-only: true
]

assert: func [
	assertion [logic!]
][
	qt-file-asserts: qt-file-asserts + 1
  
	either assertion [
		qt-file-passes: qt-file-passes + 1
	][
		either qt-red-only [
			qt-file-incompatible: qt-file-incompatible + 1
		][	qt-file-failures: qt-file-failures + 1]
		if qt-group? [  
			if qt-group-name-not-printed [
				prin "===group=== "
				print qt-group-name
				qt-group-name-not-printed: false
			]
		]
		prin "--test-- " 
		prin qt-test-name
		print either qt-red-only [
			" not like Red********"
		][	" FAILED**************"]
	]
]

assertf~=: func[
	x         [decimal!]
	y         [decimal!]
	e         [decimal!]
	/local
		diff  [decimal!]
		e1    [decimal!]
		e2    [decimal!]
][
	;; calculate tolerance to use
	;;    as e * max (1, x, y)
	e1: either x > 0.0 [ x * e ][ e1: -1.0 * x * e ]
	e2: either y > 0.0 [ y * e ][ e2: -1.0 * y * e ]
	if e  > e1 [e1: e]
	if e1 > e2 [e2: e1]

	;; perform almost equal check
	diff: either x > y [ x - y ][ y - x ]
	assert diff < e2
]
 
end-group: does[
	qt-init-group
]

end-file: func [] [
	print ["^[[7m~~~finished test~~~^[[0m " qt-file-name]
	print-totals qt-file-tests
	             qt-file-asserts
	             qt-file-passes
	             qt-file-failures
	             qt-file-incompatible
	print ""

	;; update run totals
	qt-run-passes:   qt-run-passes + qt-file-passes
	qt-run-asserts:  qt-run-asserts + qt-file-asserts
	qt-run-failures: qt-run-failures + qt-file-failures
	qt-run-tests:    qt-run-tests + qt-file-tests
	qt-run-incompatible: qt-run-incompatible + qt-file-incompatible
]

end-run: func [][
	system/options/quiet: was-quiet

	prin "^[[7m***Finished***^[[0m "
	print qt-run-name
	print-totals qt-run-tests
	             qt-run-asserts
	             qt-run-passes
	             qt-run-failures
	             qt-run-incompatible
]

print-totals: func [
	tests     [integer!]
	asserts   [integer!]
	passes    [integer!]
	failures  [integer!]
	incompat  [integer!]
][
	prin  "  Number of Tests Performed:      " 
	print tests 
	prin  "  Number of Assertions Performed: "
	print asserts
	prin  "  Number of Assertions Passed:    "
	print passes
	prin  "  Number of Assertions Failed:    "
	print failures
	prin  "  Number of Assertions Red-diff:  "
	print incompat
	if failures <> 0 [
	print "****************TEST FAILURES****************"
	]
]

;- exported functions used as a "test dialect"
***start-run***:   :start-run
~~~start-file~~~:  :start-file
===start-group===: :start-group
--test--:          :start-test
--red--:           :as-red-only
--assert:          :assert
--assertf~=:       :assertf~=
===end-group===:   :end-group
~~~end-file~~~:    :end-file
***end-run***:     :end-run

