REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Debug"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2022 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

dt: delta-time: function [
	{Delta-time - returns the time it takes to evaluate the block.}
	block [block!]
][
	; force GC, so there is less chance that it is fired in `do block`
	recycle
	start: stats/timer
	do block
	stats/timer - start
]

dp: delta-profile: func [
	{Delta-profile of running a specific block.}
	block [block!]
	/local start end adjust
][
	; first force GC
	recycle
	; than count adjustments for empty code
	adjust: copy end: stats/profile 
	do [] 
	stats/profile 
	foreach [key num] adjust [
		set key end/:key - num
	]
	adjust/timer: 0:0:0
	; and now count real stats
	start: copy end: stats/profile 
	do block 
	stats/profile 
	foreach [key num] start [
		set key end/:key - num - adjust/:key
	]
	start
]

speed?: function [
	"Returns approximate speed benchmarks [eval cpu memory file-io]."
	/no-io "Skip the I/O test"
	/times "Show time for each test"
][
	result: copy []
	foreach block [
		[
			loop 100'000 [
				; measure more than just loop func
				; typical load: 1 set, 2 data, 1 op, 4 trivial funcs
				x: 1 * index? back next "x"
				x: 1 * index? back next "x"
				x: 1 * index? back next "x"
				x: 1 * index? back next "x"
			]
			calc: [100'000 / secs / 100] ; arbitrary calc
		][
			tmp: make binary! 500'000
			insert/dup tmp "abcdefghij" 50000
			loop 10 [
				random tmp
				decompress compress tmp 'zlib 'zlib
			]
			calc: [(length? tmp) * 10 / secs / 1900]
		][
			repeat n 40 [
				change/dup tmp to-char n 500'000
			]
			calc: [(length? tmp) * 40 / secs / 1024 / 1024]
		][
			unless no-io [
				write file: %tmp-junk.txt "" ; force security request before timer
				tmp: make string! 32000 * 5
				insert/dup tmp "test^/" 32000
				loop 100 [
					write file tmp
					read file
				]
				delete file
				calc: [(length? tmp) * 100 * 2 / secs / 1024 / 1024]
			]
		]
	][
		recycle
		secs: now/precise
		calc: 0
		do block
		secs: to decimal! difference now/precise secs
		append result to integer! do calc
		if times [append result secs]
	]
	result
]

;@@ time formating and print table are probably originally made by https://github.com/toomasv

format-time: function [
	"Convert a time value to a human readable string"
	time [time!]
] [
	if time >= 0:00:01 [
		return form round/to time 0:0.001
	]
	units: ["ms" "μs" "ns"]
	foreach u units [
		time: time * 1000
		if time >= 0:00:01 [
			time: to integer! round time
			return append form time u
		]
	]
	"1ns" ; the lowest time in Rebol
]

print-table: function [
	"Print a block of blocks as an ASCII table"
	headers [block!]
	block [block!]
] [
	format: clear []
	header: clear []
	sep:    []
	i:      1
	unless parse headers [
		some [
			(text: width: fmt-func: none)
			set text string! any [set width integer! | set fmt-func word! | set fmt-func path!]
			(
				append header sep
				append header as-yellow either width [pad text width] [text]
				either width [
					either fmt-func [
						append format compose [(sep) pad (fmt-func) pick block (i) (width)]
					] [
						append format compose [(sep) pad pick block (i) (width)]
					]
				] [
					either fmt-func [
						append format compose [(sep) (fmt-func) pick block (i)]
					] [
						append format compose [(sep) pick block (i)]
					]
				]
				sep: "|"
				i:   i + 1
			)
		]
	] [
		cause-error "Invalid headers spec"
	]
	print header
	format: func [block] reduce ['print format]
	foreach row block [format row]
]

print-horizontal-line: does [
	;@@ quering window-cols width in CI under Windows now throws error: `Access error: protocol error: 6`
	;@@ it should return `none` like under Posix systems!
	loop -1 + any [attempt [query/mode system/ports/output 'window-cols] 76][ prin #"-" ] prin lf
]

;@@ profile idea is based on code from https://gist.github.com/giesse/1232d7f71a15a3a8417ec6f091398811

profile: function [
	"Profile code"
	blocks [block!]   "Block of code values (block, word, or function) to profile"
	/times            "Running the test code multiple times, results are average"
	 count [integer!] "Default value is 10, minimum is 2 and maximum 1000"
	/quiet "Returns [time evaluations series-made series-expanded memory source] results only"
][
	; limit the number of code runs. There is GC forced between each run, which may be time consuming!
	; As the output is counted as an average of each run result, it does not make sense to have too many of them.
	count: min max any [count 10] 2 1000
	unless quiet [
		print ["^/Running" as-green length? blocks "code blocks" as-green count "times."]
		print-horizontal-line
	]
	res: collect [
		foreach blk blocks [
			case [
				block? :blk [code: :blk]
				all [word? :blk block? code: get/any :blk][]
				code: to block! :blk
			]
			; to get the most precise results, make a deep copy for each test!
			; GC is done in `delta-profile` call.
			test: copy/deep :code
			data: try [delta-profile :test]
			if error? data [
				keep/only reduce [0:0:0 0 0 0 0 :blk]
				continue
			]
			loop count - 1 [
				test: copy/deep :code
				temp: delta-profile :test
				foreach [k v] data [ data/(k): v + temp/:k ]
			]
			keep/only reduce [
				data/timer / count
				to integer! data/evals / count
				to integer! round/ceiling data/series-made / count
				to integer! round/ceiling data/series-expanded / count
				to integer! data/series-bytes / count
				:blk
			]
		]
	]
	sort res                ; sort by time
	either quiet [
		new-line/all res on ; return formatted results
	][
		unless empty? res [
			reference: res/1/1
		]
		fmt-time: function [time] [
			if time < 0:00:00.000000001 [return "error"]
			rel: time / (max reference 0:00:00.000000001)
			ajoin [round/to rel 0.01 "x (" format-time time ")"]
		]
		print-table [
			"Time"   18 fmt-time
			"Evals"   6
			"S.made"  6
			"S.expa"  6
			"Memory" 11
			"Code" mold/flat
		] res
		print-horizontal-line
	]
]