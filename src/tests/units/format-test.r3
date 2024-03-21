Rebol [
	Title:   "Rebol3 file test script"
	Author:  "Oldes"
	File: 	 %format-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

secure [%/ allow]

~~~start-file~~~ "FORMAT tests"

===start-group=== "FORMAT"

--test-- "FORMAT date-time"
	value: 3-Jun-2022/1:23:45.1234+2:00
	--assert (format <dd-mm-yy> :value) == "03-06-22"
	--assert (format <dd-mm-yyyy> :value) == "03-06-2022"
	--assert (format <dd/mm/yyyy> :value) == "03/06/2022"
	--assert (format <d.m.y> :value) == "3.6.2022"
	--assert (format <d.mmmm y> :value) == "3.June 2022"
	--assert (format <dddd> :value) == "Friday"
	--assert (format <dddd d.mmmm y> :value) == "Friday 3.June 2022"
	--assert (format <yyyymmddhhmmss±zzzz> :value) == "20220603012345+0200"
	--assert (format <yyyy/mm/dd hh:mm> :value) == "2022/06/03 01:23"
	--assert (format <h:m:s> :value) == "1:23:45"
	--assert (format <hh:mm:ss> :value) == "01:23:45"
	--assert (format <hh:mm:ss.s> :value) == "01:23:45.1"
	--assert (format <hh:mm:ss.ss> :value) == "01:23:45.12"
	--assert (format <hh:mm:ss.sss> :value) == "01:23:45.123"
	--assert (format <hh:mm:ss.sssssss> :value) == "01:23:45.45.1234000"
	--assert (format <unixepoch> :value) == "1654212225"
--test-- "FORMAT date"
	value: 3-Jun-2022
	--assert (format <dd-mm-yy> :value) == "03-06-22"
	--assert (format <dd-mm-yyyy> :value) == "03-06-2022"
	--assert (format <dd/mm/yyyy> :value) == "03/06/2022"
	--assert (format <d.m.y> :value) == "3.6.2022"
	--assert (format <d.mmmm y> :value) == "3.June 2022"
	--assert (format <dddd> :value) == "Friday"
	--assert (format <dddd d.mmmm y> :value) == "Friday 3.June 2022"
	--assert (format <yyyymmddhhmmss±zzzz> :value) == "20220603000000+0000"
	--assert (format <yyyy/mm/dd hh:mm> :value) == "2022/06/03 00:00"
	--assert (format <h:m:s> :value) == "0:0:0"
	--assert (format <hh:mm:ss> :value) == "00:00:00"
	--assert (format <hh:mm:ss.s> :value) == "00:00:00.0"
	--assert (format <hh:mm:ss.ss> :value) == "00:00:00.00"
	--assert (format <hh:mm:ss.sss> :value) == "00:00:00.000"
	--assert (format <hh:mm:ss.sssssss> :value) == "00:00:00.0000000"
	--assert (format <unixepoch> :value) == "1654214400"
--test-- "FORMAT time"
	value: 1:23:45.1234
	; would use current date for date parts if used.. excluded from testing!
	--assert (format <h:m:s> :value) == "1:23:45"
	--assert (format <hh:mm:ss> :value) == "01:23:45"
	--assert (format <hh:mm:ss.s> :value) == "01:23:45.1"
	--assert (format <hh:mm:ss.ss> :value) == "01:23:45.12"
	--assert (format <hh:mm:ss.sss> :value) == "01:23:45.123"
	--assert (format <hh:mm:ss.sssssss> :value) == "01:23:45.45.1234000"

	value: 12345:01
	--assert (format <h:m:s> :value) == "12345:1:0"
	--assert (format <hh:mm:ss> :value) == "12345:01:00"
	--assert (format <hh:mm:ss.s> :value) == "12345:01:00.0"
	--assert (format <hh:mm:ss.ss> :value) == "12345:01:00.00"
	--assert (format <hh:mm:ss.sss> :value) == "12345:01:00.000"
	--assert (format <hh:mm:ss.sssssss> :value) == "12345:01:00.0000000"

--test-- "issue-532"
	;@@ https://github.com/Oldes/Rebol-issues/issues/532
	str: format [8 -8] ["this will overrun" 123]
	--assert all [
		str = "this wil     123"
		16 = length? str
	]

===end-group===

~~~end-file~~~