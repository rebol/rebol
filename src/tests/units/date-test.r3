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
	--test-- "make date!"
		--assert 1-Feb-0003 = make date! [1 2 3]
		--assert 1-Feb-0003/4:00 = make date! [1 2 3 4:0]
		--assert 1-Feb-0003/4:00+5:00 = make date! [1 2 3 4:0 5:0]
		;@@ https://github.com/Oldes/Rebol-wishes/issues/1
		--assert 1-Jan-2000 = make date! [1-1-2000]
		--assert 1-Jan-2000/10:00 = make date! [1-1-2000 10:0]
		--assert 1-Jan-2000/10:00+2:00 = make date! [1-1-2000 10:0 2:0]
		--assert 5-Jan-2000/4:00 = make date! [1-1-2000 100:0]
		
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
		;@@ https://github.com/Oldes/Rebol-wishes/issues/1
		--assert 1-Jan-2000 = #[date! 1-1-2000]
		--assert 1-Jan-2000/10:00 = #[date! 1-1-2000 10:0]
		--assert 1-Jan-2000/10:00+2:00 = #[date! 1-1-2000 10:0 2:0]
		--assert 5-Jan-2000/4:00 = #[date! 1-1-2000 100:0]

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

	--test-- "negative year"
		;@@ https://github.com/Oldes/Rebol-issues/issues/548
		;@@ https://github.com/Oldes/Rebol-issues/issues/1250
		--assert all [error? e: try [load {1/11/-0}]  e/id = 'invalid]
		--assert all [error? e: try [load {1/11/-1}]  e/id = 'invalid]
		--assert all [error? e: try [load {1/11/-00}] e/id = 'invalid]
		--assert 1-Nov-2000 = try [load {1/11/0}] ; this is error in Red!

	--test-- "invalid time in date"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1413
		--assert error? try [load "3-Jan-2010/30:00"]
		--assert error? try [load "3-Jan-2010/-30:00"]
		--assert error? try [load "3-Jan-2010/-10:00"]
		--assert error? try [load "3-Jan-2010/-30:00+1:0"]

	--test-- "timezones"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1608
		--assert all [date?  d: try [load {27-Jan-2009/13:50+5}] d/zone = 0:0]
		--assert all [date?  d: try [load {27-Jan-2009/13:50+20}] d/zone = 0:15]
		--assert all [date?  d: try [load {27-Jan-2009/13:50+29}] d/zone = 0:15]
		--assert all [date?  d: try [load {27-Jan-2009/13:50+30}] d/zone = 0:30]
		--assert all [date?  d: try [load {27-Jan-2009/13:50+200}] d/zone = 2:0]
		--assert all [date?  d: try [load {27-Jan-2009/13:50+220}] d/zone = 2:15]
		--assert all [date?  d: try [load {27-Jan-2009/13:50+1000}] d/zone = 10:0]
		--assert all [date?  d: try [load {27-Jan-2009/13:50+1500}] d/zone = 15:0]
		--assert all [error? e: try [load {27-Jan-2009/15:05+1501}] e/id = 'invalid]
		;@@ https://github.com/Oldes/Rebol-issues/issues/570
		--assert all [date?  d: try [load {27-Jan-2009/13:50+5:45}] d/zone = 5:45]
		--assert all [error? e: try [load {27-Jan-2009/15:05+24:00}]  e/id = 'invalid]
		--assert all [error? e: try [load {27-Jan-2009/15:05-+26:00}] e/id = 'invalid]

	--test-- "poke on date not supported"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1074
		d: 2009-jul-09
		--assert all [
			error? e: try [poke d 1 2000]
			e/id = 'expect-arg
		]
	--test-- "error date set messages"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1385
		d: 1-1-2000
		--assert all [
			error? err: try [d/date: 1]
			err/id = 'bad-field-set
		]
		--assert all [
			error? err: try [d/foo: 1]
			err/id = 'invalid-path
		]
		--assert all [
			error? err: try [d/utc: 1] ; error, because it requires date
			err/id = 'bad-field-set
		]
		--assert all [
			not error? try [d/time: 1.2] ; setting time to decimal is now implemented
			d = 1-Jan-2000/0:00:01.2
		]
		d/time: none
		--assert all [
			;@@ https://github.com/Oldes/Rebol-wishes/issues/18
			not error? try [d/yearday: 17]
			d = 17-1-2000
			60 = d/yearday: 60
			d = 29-Feb-2000 ; was leak year
			d/year: 2001 d/yearday: 60
			d = 1-Mar-2001  ; this one not
			d/yearday: 0    ; last day of previous year
			d = 31-Dec-2000
			d/yearday: -1
			d = 30-Dec-1999
		]
		; it's now possible to set /utc (if used date or date-time)
		n: 27-Nov-2020/18:15:57+1:00 ; date-time with timezone
		--assert n = (d/utc: n) ; result is passed thru
		--assert d = 27-Nov-2020/17:15:57 ; but d is now adjusted


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
