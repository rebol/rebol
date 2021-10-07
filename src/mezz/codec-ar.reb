REBOL [
	title: "REBOL 3 codec for AR files"
	name: 'codec-ar
	author: rights: "Oldes"
	version: 0.0.1
	specification: https://en.wikipedia.org/wiki/Ar_(Unix)
	history: [7-Oct-2021 "Oldes" {Initial version of the AR decoder}]
	todo: {AR encoder}
]

system/options/log/ar: 2
register-codec [
	name:  'ar
	type:  'compression
	title: "Unix archive file"
	suffixes: [%.ar %.a]

	decode: function [
		{Extract content of the AR file}
		data  [binary! file! url!]
		return: [block!] "Result is in format: [NAME [DATE UID GID MODE DATA] ...]"
	] [
		unless binary? data [data: read data]
		sys/log/info 'AR ["^[[1;32mDecode AR data^[[m (^[[1m" length? data "^[[mbytes )"]
		unless parse data ["!<arch>^/" data: to end][return none]
		bin: binary data
		out: make block! 32

		while [58 <= length? bin/buffer][
			binary/read bin [
				file: STRING-BYTES 16
				info: STRING-BYTES 42
				end:  UI16LE          ;#{600A}
			]
			unless all [
				try [info: load info]
				end = 2656
				info/1: to date! info/1
			][ return none]

			data: binary/read bin take/last info
			if parse file ["#1/" copy len to end ][
				; BSD variant
				len: to integer! to string! len
				file: take/part data len       ; real file name in front of the data section
				file: binary/read file 'string ; removes null padding
			]
			sys/log/info 'AR ["File:^[[33m" pad copy file 20 mold info]
			append info data
			append/only append out as file! file info
		]
		new-line/all out true
		out
	]

	;encode: function [data [binary!]][TODO]

	identify: function [data [binary!]][
		parse data ["!<arch>^/" to end]
	]
]