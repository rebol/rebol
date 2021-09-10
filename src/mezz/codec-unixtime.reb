REBOL [
	title:  "REBOL 3 codec for Unix time stamp"
	name:   'codec-unixtime
	author: "Oldes"
	version: 0.1.0
	date:    17-Sep-2019
	history: [
		0.1.0 17-Sep-2019 "Oldes" {Initial version}
	]
	urls: [
		https://en.wikipedia.org/wiki/Unix_time
		https://www.unixtimestamp.com
	]
]

register-codec [
	name:  'unixtime
	type:  'time
	title: "Unix time stamp converter"
	suffixes: []

	decode: function [
		"Return date from unix time format"
		epoch [number! string! binary!] "Date in unix time format (string is uspposed to be in base-16 format)"
		/utc {Will not add time zone}
	][
		if string? epoch [ epoch: debase epoch 16 ]
		if binary? epoch [ epoch: to integer! epoch ]
		days: to integer! tmp: epoch / 86400
		hours: to integer! time: (tmp - days) * 24
		minutes: to integer! tmp: (time - hours) * 60
		seconds: to integer! 0.5 + ((tmp - minutes) * 60)
		time: to time! ((((hours * 60) + minutes) * 60) + seconds)
		result: 1-Jan-1970 + days + time
		unless utc [
			result: result + now/zone
			result/zone: now/zone
		]
		result
	]

	encode: function [
		"Encode unix (epoch) time"
		date [date!]
		/as type [word! datatype!] {one of: [string! binary! integer!]}
	][
		time: any [date/time 0:0:0]
		unix: ((date - 1-1-1970) * 86400)
			+ (time/hour * 3600)
			+ (time/minute * 60)
			+  time/second
			- to integer! (any [date/zone 0])
		if as [
			type: to word! type
			binary/write bin: #{} [ui32 :unix]
			switch type [
				binary!  [ return bin ]
				string!  [ return enbase bin 16]
				integer! [ return unix ] ; just for consistency
			]
			cause-error 'script 'invalid-arg type
		]
		unix
	]
]