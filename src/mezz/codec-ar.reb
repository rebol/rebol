REBOL [
	title: "REBOL 3 codec for AR files"
	name: 'codec-ar
	author: rights: "Oldes"
	version: 0.0.2
	specification: https://en.wikipedia.org/wiki/Ar_(Unix)
	history: [7-Oct-2021 "Oldes" {Initial version of the AR decoder}]
	todo: {AR encoder}
]

system/options/log/ar: 2
register-codec [
	name:  'ar
	type:  'compression
	title: "Unix archive file"
	suffixes: [%.ar %.a %.lib %.deb]

	decode: function [
		{Extract content of the AR/LIB file}
		data  [binary! file! url!]
		return: [block!] "Result is in format: [NAME [DATE UID GID MODE DATA] ...]"
	] [
		unless binary? data [data: read data]
		sys/log/info 'AR ["^[[1;32mDecode AR/LIB data^[[m (^[[1m" length? data "^[[mbytes )"]
		unless parse data ["!<arch>^/" data: to end][return none]
		bin: binary data
		out: make block! 32

		long-names: none
		numbers: system/catalog/bitsets/numeric

		while [58 <= length? bin/buffer][
			info: binary/read bin [
				STRING-BYTES 16 ; File identifier
				STRING-BYTES 12 ; File modification timestamp
				STRING-BYTES 6  ; Owner ID
				STRING-BYTES 6  ; Group ID
				STRING-BYTES 8  ; File mode (type and permission)
				STRING-BYTES 10 ; File size in bytes
			]
			if 2656 <> binary/read bin 'UI16LE [ ;= #{600A}
				sys/log/error 'AR "Invalid file header!"
				return none
			]
			file: trim/tail take info
			real: none
			forall info [
				; it may be an empty string in Windows' lib file!
				try/except [info/1: to integer! info/1][info/1: 0]
			]
			; convert timestamp to Rebol value (it may be -1 in *.lib files)
			info/1: either info/1 <= 0 [none][to date! info/1]
			size: take/last info
			data: binary/read bin size
			if odd? size [
				if 10 <> binary/read bin 'UI8 [
					sys/log/error 'AR "Invalid padding!"
				]
			]
			if parse file [opt ["#1" (bsd?: true)] #"/" copy len some numbers to end ][
				len: to integer! to string! len
				case [
					bsd? [
						;- BSD variant
						real: take/part data len            ; real file name in front of the data section
						real: binary/read real-name 'string ; removes null padding
					]
					long-names [
						;- System V (or GNU) variant
						binary/read long-names [ATZ :len real: STRING]
					]
				]
				if real [append info as file! real]
			]
			sys/log/info 'AR ["File:^[[33m" pad copy file 20 mold info]
			append info data
			append/only append out as file! file info

			if file = "//" [
				long-names: data
			]
		]
		new-line/all out true
		out
	]

	;encode: function [data [binary!]][TODO]

	identify: function [data [binary!]][
		parse data ["!<arch>^/" to end]
	]
]