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

===start-group=== "#[date! ...]"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1034
	--test-- "#[date!] valid"
		--assert 1-Feb-0003 = #[date! 1 2 3]
		--assert 1-Feb-0003/4:00 = #[date! 1 2 3 4:0]
		--assert 1-Feb-0003/4:00+5:00 = #[date! 1 2 3 4:0 5:0]
	--test-- "#[date!] invalid"
		--assert error? try [load {#[date!]}]
		--assert error? try [load {#[date! 1]}]
		--assert error? try [load {#[date! 1 2]}]
		--assert error? try [load {#[date! 1 2 3 x]}]
		--assert error? try [load {#[date! 1 2 3 4:0 x]}]
		--assert error? try [load {#[date! 1 2 3 4:0 5:0 3]}]

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

	--test-- "issue 2414"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2414
		d: 1-1-2000
		--assert none? d/zone
		--assert none? d/time
		--assert none? d/hour
		--assert none? d/minute
		--assert none? d/second
		--assert integer? d/year
		d/hour: 2
		--assert d/time = 2:00
		--assert d/date = 1-Jan-2000
		d: 1-1-2000 d/minute: 10
		--assert d/time = 0:10
		--assert d/date = 1-Jan-2000

	--test-- "issue 1375"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1375
		; none instead of error for an out-of-range ordinal selector
		d: now
		--assert none? try [d/0]
		--assert none? try [d/-1]
		--assert none? try [d/100]
		; consistent with time:
		t: now/time
		--assert none? try [t/0]
		--assert none? try [t/-1]
		--assert none? try [t/100]

	--test-- "issue-1412"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1412
		--assert date? try [load "3-Jan-2010/10:00+2:00"]
		--assert error? try [load "3-Jan-2010/10:00+2:00:"]
		--assert error? try [load "3-Jan-2010/10:00+2:00:56"]
		--assert error? try [load "3-Jan-2010/10:00+2:00:56.1234pm"]

	--test-- "issue-276"
		;@@ https://github.com/Oldes/Rebol-issues/issues/276
		--assert error? try [0 - 1-jan-0000] ;- no crash!

	--test-- "poke on date not supported"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1074
		d: 2009-jul-09
		--assert all [
			error? e: try [poke d 1 2000]
			e/id = 'expect-arg
		]

===end-group===

===start-group=== "date protect"
;	;@@ https://github.com/Oldes/Rebol-issues/issues/2416
;	--test-- "date protect 1"
;		d: now protect 'd
;		--assert error? e: try [d/year: 0]
;		--assert e/id = 'locked-word
;		unprotect 'd
;	--test-- "date protect 2"
;		protect/words o: object [d: now]
;		--assert error? e: try [o/d/year: 0]
;		--assert e/id = 'locked-word
;	--test-- "date protect 3"
;		o: object [d: 1 protect/words o: object [d: now]]
;		--assert protected? 'o/o/d
;		--assert protected? 'o/o/d/year        ;@@ <--- fails!
;		--assert error? e: try [o/o/d/year: 0] ;@@ <--- fails!
===end-group===

===start-group=== "DATE/TIMEZONE"
	--test-- "set timezone"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2370
		d: 1-Jan-2000
		d/zone: 2
		--assert d = 1-Jan-2000/0:00+2:00
		d/timezone: 2
		--assert d = 1-Jan-2000/0:00+2:00
		d/timezone: 4
		--assert d = 1-Jan-2000/2:00+4:00
		d/timezone: -7
		--assert d = 31-Dec-1999/15:00-7:00
		d/timezone: 0
		--assert d = 31-Dec-1999/22:00

		--assert error? try [d/timezone: -70]

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


===start-group=== "QUERY date"
	date: 8-Apr-2020/12:04:32+2:00
	--test-- "query date"
		--assert object? o: query date
		--assert o/date = 8-Apr-2020

	--test-- "query/mode datetime"
		all-date-words: words-of system/standard/date-info
		--assert all-date-words = query/mode date none
		--assert date/time = query/mode date 'time
		--assert [2020 4] = query/mode date [year month]
		--assert [month: 4 year: 2020] = query/mode date [month: year:]
		--assert equal? query/mode date all-date-words [2020 4 8 12:04:32 8-Apr-2020 2:00 12 4 32 3 99 2:00 8-Apr-2020/10:04:32 99]
	
	--test-- "query/mode date"
		date: 8-Apr-2020 ; no time!
		--assert equal? query/mode date all-date-words [2020 4 8 #[none] 2020-04-08 #[none] #[none] #[none] #[none] 3 99 #[none] 2020-04-08 99]

===end-group===



===start-group=== "NOW"
	--test-- "now"
		--assert integer? now/year
		--assert integer? now/month
		--assert integer? now/day
		--assert time?    now/time
		--assert time?    now/zone
		--assert date?    now/date
		--assert integer? now/weekday
		--assert integer? now/yearday
		--assert date?    now/utc
        
    --test-- "now/precise"
		--assert time?    now/time/precise
		--assert date?    now/utc/precise
		--assert date?    now/precise
		--assert integer? now/year/precise ; precise ignored

	--test-- "now with too many refines"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2286
		--assert error? try [now/time/day]
		--assert error? try [now/time/day/precise]
		--assert error? try [now/utc/month]

===end-group===

~~~end-file~~~
