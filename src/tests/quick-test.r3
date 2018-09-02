Rebol [
  title:   "Rebol simple testing framework"
  Author:  ["Peter W A Wood" "Oldes"]
  File:    %quick-test.red
  Version: "0.2.0"
  Rights:  "Copyright (C) 2012-2015 Peter W A Wood. All rights reserved."
  License: "BSD-3 - https://github.com/red/red/blob/master/BSD-3-License.txt"
  Comment: "This script is ported Peter's Red script for use with Rebol3 project too"
]

;; counters
if not value? 'qt-run-tests [
  qt-run-tests: 0 
  qt-run-asserts: 0
  qt-run-passes: 0
  qt-run-failures: 0
  qt-file-tests: 0 
  qt-file-asserts: 0 
  qt-file-passes: 0 
  qt-file-failures: 0
  qt-file-incompatible: 0
]

qt-file-name: none

;; group switches
qt-group-name-not-printed: true
qt-group?: false
qt-red-only: false

_qt-init-group: func [] [
  qt-group-name-not-printed: true
  qt-group?: false
  qt-group-name: ""
]

qt-init-run: func [] [
  qt-run-tests: 0 
  qt-run-asserts: 0
  qt-run-passes: 0
  qt-run-failures: 0
  qt-run-incompatible: 0
  _qt-init-group
]

qt-init-file: func [] [
  qt-file-tests: 0 
  qt-file-asserts: 0 
  qt-file-passes: 0 
  qt-file-failures: 0
  qt-file-incompatible: 0
  _qt-init-group
]

***start-run***: func[
    title [string!]
][
  qt-init-run
  qt-run-name: title
  prin "***Starting*** " 
  print title
]

~~~start-file~~~: func [
  title [string!]
][
  qt-init-file
  prin "~~~started test~~~ "
  print title
  qt-file-name: title
  qt-group?: false
]

===start-group===: func [
  title [string!]
][
  qt-group-name: title
  qt-group?: true
]

--test--: func [
  title [string!]
][
  qt-test-name: title
  qt-file-tests: qt-file-tests + 1
  qt-red-only: false
]

--red--: does [
  qt-red-only: true
]

--assert: func [
  assertion [logic!]
][

  qt-file-asserts: qt-file-asserts + 1
  
  either assertion [
     qt-file-passes: qt-file-passes + 1
  ][
    either qt-red-only [
      qt-file-incompatible: qt-file-incompatible + 1
    ][qt-file-failures: qt-file-failures + 1]
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
    ][ " FAILED**************"]
  ]
]

--assertf~=: func[
  x           [decimal!]
  y           [decimal!]
  e           [decimal!]
  /local
    diff      [decimal!]
    e1        [decimal!]
    e2        [decimal!]
][
  ;; calculate tolerance to use
  ;;    as e * max (1, x, y)
  either x > 0.0 [
    e1: x * e
  ][
    e1: -1.0 * x * e
  ]
  if e > e1 [e1: e]
  either y > 0.0 [
    e2: y * e
  ][
    e2: -1.0 * y * e
  ]
  if e1 > e2 [e2: e1]

  ;; perform almost equal check
  either x > y [
    diff: x - y
  ][
    diff: y - x
  ]
  either diff > e2 [
    --assert false
  ][
    --assert true
  ]
]
 
===end-group===: func [] [
  _qt-init-group
]

qt-print-totals: func [
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

~~~end-file~~~: func [] [
  print ["~~~finished test~~~ " qt-file-name]
  qt-print-totals qt-file-tests qt-file-asserts qt-file-passes qt-file-failures qt-file-incompatible
  print ""
  
  ;; update run totals
  qt-run-passes: qt-run-passes + qt-file-passes
  qt-run-asserts: qt-run-asserts + qt-file-asserts
  qt-run-failures: qt-run-failures + qt-file-failures
  qt-run-tests: qt-run-tests + qt-file-tests
]

***end-run***: func [][
  prin "***Finished*** "
  print qt-run-name
  qt-print-totals qt-run-tests
                  qt-run-asserts
                  qt-run-passes
                  qt-run-failures
]
