REBOL [
	title:  "Codec: Unix time stamp"
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
		result: to date! epoch
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
		unix: to integer! date
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