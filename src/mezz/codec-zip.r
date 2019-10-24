REBOL [
	title: "REBOL 3 codec for ZIP files"
	name: 'codec-zip
	author: rights: "Oldes"
	version: 0.0.2
	specification: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
	history: [
		24-Oct-2019 "Oldes" {
			* Refactored decoder so it is using central directory structure
			* Added decode/info refinement used to resolve just the info about files without decompressing
			* Added access to decompress-file function which may be used to decompress single file using info from the info records
		}
		15-Mar-2019 "Oldes" {Initial version of the ZIP decoder}]
]

register-codec [
	name:  'ZIP
	title: "ZIP File Format"
	suffixes: [%.zip %.aar %.jar %.apk %.zipx %.appx %.epub]

	decode: function [
		{Decompress all content of the ZIP file}
		zip-data  [binary! file! url!]
		/validate "Check if decompressed data has valid CRC"
		/only     "Extract only specified files if found in the achive"
			files [block!] "Block with file names to extract"
		/info              "Does not decode data. Instead of data there is reported uncompressed size."
		return:   [block!] "Result is in format: [NAME [MODIFIED CRC DATA] ...]"
	] [
		unless binary? zip-data [ zip-data: read zip-data ]
		if verbose [
			sys/log/info 'ZIP ["^[[1;32mDecode ZIP data^[[m (^[[1m" length? zip-data "^[[mbytes )"]
		]
		bin: binary zip-data

		data-pos: 0

		if only [ files-to-extract: length? files ]

		;-[ reading central directory end record ]-
		; it must be present in each zip file
		unless pos: find/last/tail bin/buffer #{504B0506} [	return copy [] ]

		bin/buffer: pos
		if verbose [ sys/log/debug 'ZIP "End of central directory record" ]
		data: binary/read bin [
			     UI16LE      ; number of this disk
			     UI16LE      ; number of the disk with the start of the central directory
			     UI16LE      ; total number of entries in the central directory on this disk
			     UI16LE      ; total number of entries in the central directory
			     UI32LE      ; size of the central directory
			pos: UI32LE      ; offset of start of central directory with respect to the starting disk number
			len: UI16LE      ; .ZIP file comment length
			com: BYTES :len  ; .ZIP file comment
		]
		if verbose [
			sys/log/debug 'ZIP mold data
			unless empty? com [
				sys/log/info 'ZIP ["Comment: ^[[33m" to-string com]
			]
		]
		
		unless all [zero? data/1 zero? data/2][
			sys/log/error 'ZIP "Splitted zip files not supported!"
			return none
		] 

		result: make block! 2 * data/4

		;-[ reading central directory records ]-
		bin/buffer: at head bin/buffer (pos + 1)

		while [
			33639248 = binary/read bin 'UI32LE ;#{02014B50}
		][
			cheader: binary/read bin [
				          UI16LE         ; version made by
				          UI16LE         ; version needed to extract
				          BITSET16       ; general purpose bit flag
				method:   UI16LE         ; compression method
				modified: MSDOS-DATETIME ; last modified
				crc:      SI32LE         ; crc-32
				cmp-size: UI32LE         ; compressed size
				unc-size: UI32LE         ; uncompressed size
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
			cheader/16: name: to file! name
			if verbose [
				sys/log/debug 'ZIP "Central directory structure"
				sys/log/debug 'ZIP mold cheader
			]
			if all [only not find files name][
				if verbose [ sys/log/more 'ZIP ["Not extracting: ^[[33m" name] ]
				continue
			]

			either info [
				repend result [name reduce [modified offset cmp-size unc-size method crc extr comm]]
			][
				if verbose [
					sys/log/info 'ZIP [
						"Extracting: ^[[33m" name
						" ^[[0mbytes:^[[33m" cmp-size "^[[0m->^[[33m" unc-size
					]
					unless empty? comm [sys/log/info 'ZIP ["Comment: ^[[33m" to-string comm "^[[0m" mold name]]
				]

				either zero? unc-size [
					data: none
				][
					data: decompress-file (at head bin/buffer (offset + 1)) reduce [method cmp-size unc-size]

					if all [
						data
						any [validate validate-crc?]
						crc <> crc2: checksum/method data 'crc32
					][
						sys/log/error 'ZIP ["CRC check failed!" crc "<>" crc2]
					]
				]
				repend result [name reduce [modified crc data]]
			]

			if only [
				-- files-to-extract
				if files-to-extract = 0 [
					if verbose [ sys/log/more 'ZIP "All files extracted" ]
					break
				]
			]
		]

		new-line/all result true
		result
	]

	;encode: function [data [binary!]][	]

	identify: function [data [binary!]][
		all [
			4 < length? data
			#"P" = data/1
			#"K" = data/2
		]
	]

	decompress-file: function [
		buffer [binary!] "Binary at position of the zip's local record"
		header [block!]  "[method cmp-size unc-size]"
	][
		bin: binary buffer
		unless 67324752 = binary/read bin 'UI32LE [
			sys/log/error 'ZIP {Offset is not pointing to the "Local file header"}
			return none
		]

		if verbose [sys/log/debug 'ZIP "Local file header"]
		local: binary/read bin [
			          UI16LE         ; version
			          BITSET16       ; flags
			          UI16LE         ; compression
			          MSDOS-DATETIME ; last modified
			          SI32LE         ; crc-32
			          UI32LE         ; compressed size
			          UI32LE         ; uncompressed size
			len-name: UI16LE
			len-extr: UI16LE
			name:     BYTES :len-name
			extr:     BYTES :len-extr
			data-pos: INDEX
		]

		;@@ O: does really have central record precedense as I use it now?
		;@@ O: should I compare cental record with the local one?

		method:   header/1
		cmp-size: header/2
		unc-size: header/3

		if verbose [sys/log/debug 'ZIP mold local]

		data: at head buffer :data-pos

		switch/default method [
			8 [ ;- deflate
				output: decompress/deflate/size data unc-size
			]
			14 [ ;- LZMA
				output: decompress/lzma/part/size (skip data 4) cmp-size unc-size
			]
			0  [ ;- store
				output: copy/part data cmp-size
			]
		][
			sys/log/error 'ZIP ["Unsupported compression method:^[[0;35m" method]
		]

		return output
	]

	validate-crc?: true
	verbose: true
	level: 9
]