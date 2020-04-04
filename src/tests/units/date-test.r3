Rebol [
	Title:   "Rebol3 date test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %date-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "date"

===start-group=== "ISO8601 subset"
	--test-- "ISO8601 basic load"
		--assert 8-Nov-2013/17:01 = load "2013-11-08T17:01"
		--assert 8-Nov-2013/17:01 = load "2013-11-08T17:01Z"
		--assert 8-Nov-2013/17:01+1:00 = load "2013-11-08T17:01+0100"
		--assert 8-Nov-2013/17:01-1:00 = load "2013-11-08T17:01-0100"
		--assert 8-Nov-2013/17:01+1:00 = load "2013-11-08T17:01+01:00"
	--test-- "basic load of not fully standard ISO8601"
		--assert 8-Nov-2013/17:01 = load "2013/11/08T17:01"
		--assert 8-Nov-2013/17:01 = load "2013/11/08T17:01Z"
		--assert 8-Nov-2013/17:01+1:00 = load "2013/11/08T17:01+0100"
		--assert 8-Nov-2013/17:01+1:00 = load "2013/11/08T17:01+01:00"
	--test-- "Invalid ISO8601 dates"
		--assert error? try [load "2013-11-08T17:01Z0100"]
		--assert error? try [load "2013/11/08T17:01Z0100"]

	--test-- "Using ISO88601 datetime in a path"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2089
		b: [8-Nov-2013/17:01 "foo"]
		--assert "foo" = b/2013-11-08T17:01

===end-group===

===start-group=== "MOLD/ALL on date"
;-- producing ISO8601 valid result (https://tools.ietf.org/html/rfc3339)
	--test-- "MOLD/ALL on date"
		--assert "2000-01-01T01:02:03" = mold/all 1-1-2000/1:2:3
		--assert "2000-01-01T10:20:03" = mold/all 1-1-2000/10:20:3
		--assert "0200-01-01T01:02:03" = mold/all 1-1-200/1:2:3
		--assert "0200-01-01T01:02:03+02:00" = mold/all 1-1-200/1:2:3+2:0
		--assert "0200-01-01T01:02:03+10:00" = mold/all 1-1-200/1:2:3+10:0

===end-group===

===start-group=== "TO DATE!"
	--test-- "invalid input"
		;@@ https://github.com/Oldes/Rebol-issues/issues/878
		--assert error? try [to date! "31-2-2009"]
		--assert error? try [to date! [31 2 2009]]

===end-group===

===start-group=== "Date math"
	--test-- "adding by integer"
		;@@ https://github.com/Oldes/Rebol-issues/issues/213
		n: now
		--assert not error? try [now/date + 1]
		--assert not error? try [d1: n + 1]
		--assert not error? try [d2: n/date + 1]
		--assert d1/date = d2/date

===end-group===

===start-group=== "Various date issues"
	--test-- "issue 1637"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1637
		d: now/date
		--assert none? d/time
		--assert none? d/zone
		d: make date! [23 7 2010]
		--assert none? d/time
		d: now/date
		--assert none? d/time
		d: d/date
		--assert none? d/time

	--test-- "issue 1308"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1308
		d: 28-Oct-2009/10:09:38-7:00
		--assert 28-Oct-2009/17:09:38 = d/utc
		--assert 10 = d/hour

	--test-- "issue 2369"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2369
		d: 28-Mar-2019
		--assert not error? try [d/zone: 2]
		--assert "28-Mar-2019/0:00+2:00" = mold d
		d: 28-Oct-2009/10:09:38-7:00
		--assert not error? try [d/zone: 2]
		--assert "28-Oct-2009/10:09:38+2:00" = mold d

	--test-- "issue 1145"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1145
		test-issue-1145: does [for n 1 100 1 [if error? try [random/secure now][ return false ]] true]
		--assert test-issue-1145

===end-group===

===start-group=== "DATE/TIMEZONE"
	--test-- "set timezone"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2370
		d: 1-Jan-2000
		d/zone: 2
		--assert "1-Jan-2000/0:00+2:00" = mold d
		d/timezone: 2
		--assert "1-Jan-2000/0:00+2:00" = mold d
		d/timezone: 4
		--assert "1-Jan-2000/2:00+4:00" = mold d
		d/timezone: -7
		--assert "31-Dec-1999/15:00-7:00" = mold d
		d/timezone: -70
		--assert "29-Dec-1999/0:00-6:00" = mold d

===end-group===

===start-group=== "Internet date"
	--test-- "TO-ITIME (Normalized time as used in TO-IDATE"
		--assert "09:04:05" = to-itime 9:4:5
		--assert "13:24:05" = to-itime 13:24:5.21

	--test-- "TO-IDATE (Date to Internet date)"
		--assert "Thu, 28 Mar 2019 20:00:59 +0100" = to-idate 28-Mar-2019/20:00:59+1:00
		--assert "Thu, 28 Mar 2019 19:00:59 GMT" = to-idate/gmt 28-Mar-2019/20:00:59+1:00
		--assert "Thu, 28 Mar 2019 00:00:00 GMT" = to-idate 28-Mar-2019

	--test-- "TO-DATE (from Internet date)"
		--assert 28-Mar-2019/20:00:59+1:00 = to-date "Thu, 28 Mar 2019 20:00:59 +0100"
		--assert 28-Mar-2019/19:00:59 = to-date/utc "Thu, 28 Mar 2019 20:00:59 +0100"
		--assert 28-Mar-2019/0:00 = to-date "Thu, 28 Mar 2019 00:00:00 +0000"
		--assert 28-Mar-2019/0:00 = to-date to-binary "Thu, 28 Mar 2019 00:00:00 +0000"
		--assert 4-Apr-2019/19:41:46 = to-date "Thu, 04 Apr 2019 19:41:46 GMT"
		--assert 1-Apr-2019/21:50:04 = to-date "Mon, 1 Apr 2019 21:50:04 GMT"


===end-group===

~~~end-file~~~
