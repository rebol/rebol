REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Date"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

to-itime: func [
	{Returns a standard internet time string (two digits for each segment)}
	time [time! number! block! none!]
][
	time: make time! time
	format/pad [-2 #":" -2 #":" -2] reduce [
		time/hour  time/minute  to integer! time/second
	] #"0"
]

to-idate: func [
	"Returns a standard Internet date string."
	date [date!]
	/gmt "Converts local time to GMT (Greenwich Mean Time)"
	/local zone
][
	either date/zone [
		either gmt [
			date/time: date/time - date/zone
			date/zone: none
			zone: "GMT"
		][
			zone: form date/zone
			remove find zone ":"
			if #"-" <> first zone [insert zone #"+"]
			if 4 >=  length? zone [insert next zone #"0"]
		]
	][	zone: "GMT" ]
	reform [
		pick ["Mon," "Tue," "Wed," "Thu," "Fri," "Sat," "Sun,"] date/weekday
		date/day
		pick ["Jan" "Feb" "Mar" "Apr" "May" "Jun" "Jul" "Aug" "Sep" "Oct" "Nov" "Dec"] date/month
		date/year
		to-itime any [date/time 0:00]
		zone
	]
]

to-date: wrap [
	digit: charset [#"0" - #"9"]
	alpha: charset [#"a" - #"z" #"A" - #"Z"]
	function [
		"Converts to date! value."
		value [any-type!] "May be also a standard Internet date string/binary"
		/utc "Returns the date with UTC zone"
	][
		if all [
			any [string? value binary? value]
			parse value [
				5 skip
				copy day:   1 2 digit space
				copy month:   3 alpha space
				copy year:  1 4 digit space
				copy time: to space space
				copy zone: to end
			]
		][
			if zone = "GMT" [zone: "+0"]
			value: to string! rejoin [day "-" month "-" year "/" time zone]
		]
		
		if all [value: to date! value  utc] [ value/timezone: 0 ]
		value
	]
]