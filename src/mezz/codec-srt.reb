REBOL [
	Title:  "Codec: SRT (SubRip Subtitle)"
	Name:    srt
	Type:    module
	Options: [delay]
	Version: 0.0.1
	Author: "Oldes"
	History: [23-Jan-2025 "Oldes" {Initial version}]
	Usage: [
		;- Example how to modify subtitle timestamps
		srt: load %example.srt
		forall srt [
			srt: change srt srt/1 - 0:0:16.4
			srt: change srt srt/1 - 0:0:16.4
		]
		save %example-fixed.srt srt
	]
]

invalid-data: "Data not in a correct format!"

register-codec [
	name: 'srt
	type: 'application
	title: "SubRip Subtitle"
	suffixes: [%.srt]

	decode: function [
		"Converts SRT file data to Rebol block: [time1 time2 subtitle ...]"
		srt [file! url! binary! string!]
	][
		case [
			any [file? srt url? srt] [ srt: read/string srt ]
			binary? srt [ srt: deline to string! srt ]
		]
		ch_digits: system/catalog/bitsets/numeric
		result: make block! 1000
		unless attempt [
			parse srt [
				any [SP | LF]
				any [
					some ch_digits LF
					copy time1: to SP " --> " copy time2: to LF skip
					copy text: to "^/^/" 2 skip
					(repend result [transcode/one time1 transcode/one time2 text])
				]
			]
		][
			log-error 'SRT invalid-data
			return none
		]
		new-line/skip result true 3
	]

	encode: function [
		"Converts block of subtitles to a SRT output"
		data [block!]
	][
		result: make string! 50000
		count: 0
		unless parse data [
			any [
				set time1: time!
				set time2: time!
				set text: string!
				(
					++ count
					append result ajoin [
						count LF
						time1 " --> " time2 LF
						text LF LF
					]
				)
			]
		][
			log-error 'SRT invalid-data
			return none
		]
		result
	]
]
