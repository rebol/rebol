REBOL [
	title: "REBOL 3 codec for ZIP files"
	name: 'codec-zip
	author: rights: "Oldes"
	version: 0.0.1
	specification: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
	history: [15-Mar-2019 "Oldes" {Initial version of the ZIP decoder}]
]

register-codec [
	name:  'ZIP
	title: "ZIP File Format"
	suffixes: [%.zip %.aar %.jar %.apk %.zipx %.appx]

	decode: wrap [
		;- privates
		name:   none
		method: none
		crc:
		cmp-size:
		unc-size: 0

		decompress*: func [
			data      [binary! ]
			validate  [logic!]
			/local output crc2
		][
			if verbose > 0 [
				sys/log/info 'ZIP [
					"Extracting: ^[[33m" name
					" ^[[0mbytes:^[[33m" cmp-size "^[[0m->^[[33m" unc-size
				]
			]
			switch/default method [
				8 [ ;- deflate
					output: decompress/deflate/size data unc-size
				]
				14 [ ;- LZMA
					output: decompress/lzma/part/size skip data 4  cmp-size  unc-size
				]
				0  [ ;- store
					output: copy/part data cmp-size
				]
			][
				print [" [ZIP] ^[[1;35mUnsupported compression method:^[[0;35m" method "^[[0m"]
			]
			if all [
				validate
				output
				crc <> crc2: checksum/method output 'crc32
			][
				print [" [ZIP] ^[[1;35mCRC check failed!" crc "<>" crc2 "for file:^[[0;35m" name "^[[0m"]
			]
			output
		]

		;- decode
		function/extern [
			{Decompress all content of the ZIP file}
			zip-data  [binary! file! url!]
			/validate "Check if decompressed data has valid CRC"
			/only     "Extract only specified files if found in the achive"
				files [block!] "Block with file names to extract"
			return: [block!] "Result is in format: [NAME [MODIFIED CRC DATA] ...]"
		] [
			unless binary? zip-data [
				zip-data: read zip-data
			]
			if verbose > 0 [
				sys/log/info 'ZIP ["^[[1;32mDecode ZIP data^[[m (^[[1m" length? zip-data "^[[mbytes )"]
			]
			bin: binary zip-data

			data-pos: 0

			if only [ files-to-extract: length? files ]

			result: make block! 32

			while [pos: find bin/buffer #{504b}][
				pos: index? pos
				binary/read bin [AT :pos type: UI32LE]
				switch/default type [
					134695760 [ ;#{08074B50} 
						if verbose > 1 [sys/log/more 'ZIP "Data Descriptor"]
						binary/read bin [
							crc:      SI32LE
							cmp-size: UI32LE   ; compressed size
							unc-size: UI32LE   ; uncompressed size
						]

						if all [only not find files name][
							if verbose > 1 [sys/log/debug 'ZIP "not extracting"]
							continue
						]

						either all [name data-pos > 0] [
							data: decompress* at zip-data :data-pos any [validate validate-crc?]
							repend result [name reduce [modified crc data]]
						][
							if verbose > 0 [sys/log/info 'ZIP ["Extracting: ^[[33m" name]]
							repend result [name none]
						]

						if only [
							-- files-to-extract
							if files-to-extract = 0 [break]
						]
					]
					67324752 [ ;#{04034B50}
						if verbose > 1 [sys/log/more 'ZIP "Local file header"]
						header: binary/read bin [
									  UI16LE         ; version
							flags:    BITSET16       ; flags
							method:   UI16LE         ; compression
							modified: MSDOS-DATETIME ; last modified
							crc:      SI32LE         ; crc-32
							cmp-size: UI32LE         ; compressed size
							unc-size: UI32LE         ; uncompressed size
							len-name: UI16LE
							len-extr: UI16LE
							name:     BYTES :len-name
							extr:     BYTES :len-extr
							data-pos: INDEX
						]
						if verbose > 2 [sys/log/debug 'ZIP mold header]
						name: to file! name
						if all [
							flags/12 ; bit 3
							crc      = 0
							cmp-size = 0
							unc-size = 0
						][
							; The correct values are put in the data descriptor
							; immediately following the compressed data.
							if verbose > 1 [sys/log/debug 'ZIP "waiting for Data Descriptor"]
							continue
						]

						if all [only not find files name][
							if verbose > 1 [sys/log/debug 'ZIP "not extracting"]
							continue
						]

						either all [unc-size > 0] [
							data: decompress* bin/buffer any [validate validate-crc?]
							repend result [name reduce [modified crc data]]
						][
							if verbose > 0 [sys/log/info 'ZIP ["Extracting: ^[[33m" name]]
							repend result [name none]
						]
						if only [
							-- files-to-extract
							if files-to-extract = 0 [break]
						]
					]
					33639248 [ ;#{02014B50}
						if verbose > 1 [sys/log/more 'ZIP "Central directory structure"]
						cheader: binary/read bin [
							          UI16LE         ; version made by
							          UI16LE         ; version needed to extract
							          BITSET16       ; general purpose bit flag
							          UI16LE         ; compression method
							modified: MSDOS-DATETIME ; last modified
							          SI32LE         ; crc-32
							          UI32LE         ; compressed size
							          UI32LE         ; uncompressed size
							len-name: UI16LE         ; file name length
							len-extr: UI16LE         ; extra field length
							len-comm: UI16LE         ; file comment length
							disk-num: UI16LE         ; disk number start
							att-int:  UI16LE         ; internal file attributes
							att-ext:  UI32LE         ; external file attributes
							offset:   UI32LE         ; relative offset of local header
							name:     BYTES :len-name
							extr:     BYTES :len-extr
							comm:     BYTES :len-comm
						]
						if verbose > 2 [sys/log/debug 'ZIP mold cheader]
						unless empty? comm [sys/log/info 'ZIP ["Comment: ^[[33m" to-string comm "^[[0m" mold to file! name]]
					]
					101010256 [ ;#{06054B50}
						if verbose > 1 [sys/log/more 'ZIP "End of central directory record"]
						data: binary/read bin [
							UI16LE      ; number of this disk
							UI16LE      ; number of the disk with the start of the central directory
							UI16LE      ; total number of entries in the central directory on this disk
							UI16LE      ; total number of entries in the central directory
							UI32LE      ; size of the central directory
							UI32LE      ; offset of start of central directory with respect to the starting disk number
							len: UI16LE ; .ZIP file comment length
							BYTES :len  ; .ZIP file comment
						]
						if verbose > 2 [sys/log/debug 'ZIP mold data]
					]
					101075792 [ ;#{06064b50}
						if verbose > 1 [sys/log/more 'ZIP "Zip64 end of central directory record"]
						data: binary/read bin [
							UI64LE ; directory record
							UI16LE ; version made by
							UI16LE ; version needed to extract
							UI32LE ; number of this disk
							UI32LE ; number of the disk with the start of the central directory
							UI64LE ; total number of entries in the central directory on this disk
							UI64LE ; total number of entries in the central directory
							UI64LE ; size of the central directory
							UI64LE ; offset of start of central directory with respect to the starting disk number
							;@@BYTES ?? ; zip64 extensible data sector    (variable size)
						]
						if verbose > 2 [sys/log/debug 'ZIP mold data]
					]
				][
					if verbose > 1 [sys/log/more 'ZIP ["Unknown ZIP signature:" mold skip to-binary type 4]]
				]
			]
			new-line/skip result true 4
			result
		][
			; external `decode` variables
			name method crc cmp-size unc-size
		]
	]

	;encode: function [data [binary!]][	]

	identify: function [data [binary!]][
		all [
			4 < length? data
			#"P" = data/1
			#"K" = data/2
		]
	]
	validate-crc?: true
	verbose: 3
	level: 9
]