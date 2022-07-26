REBOL [
	title: "Codec: TAR"
	name: 'codec-tar
	author: rights: "Oldes"
	version: 0.0.1
	specification: https://en.wikipedia.org/wiki/Tar_%28computing%29
	history: [20-Mar-2019 "Oldes" {Initial version of the TAR decoder}]
	todo: {
		* TAR encoder
		* prety print content of the TAR (list)
		* convert unixtime from the header to Rebol date?
		* be able to use wildcards to specify, what to extract
		* implement checksum check?
		* use streaming when decoding file or url directly
		* maybe provide just important fields of the headers
	}
]

register-codec [
	name:  'tar
	type:  'compression
	title: "TAR File Format"
	suffixes: [%.tar]

	decode: function [
		{Extract content of the TAR file}
		tar-data  [binary! file! url!]
		;/validate "Check if decompressed data has valid CRC"
		/only     "Extract only specified files if found in the achive"
			files [block! file!] "Block with file names to extract"
		return: [block!] "Result is in format: [NAME [DATA HDR1 HDR2] ...]"
	] [
		unless binary? tar-data [
			tar-data: read tar-data
		]
		if verbose > 0 [
			sys/log/info 'TAR ["^[[1;32mDecode TAR data^[[m (^[[1m" length? tar-data "^[[mbytes )"]
		]
		bin: binary tar-data

		if only [
			unless block? files [files: reduce [files]]
			files-to-extract: length? files
		]

		result: make block! 32

		while [not tail? bin/buffer][
			pos: index? bin/buffer

			;- Pre-POSIX.1-1988 format
			hdr1: binary/read bin [
				name: STRING-BYTES 100 ; file name
				mode: STRING-BYTES   8 ; permissions
				uid:  OCTAL-BYTES    8 ; user id
				gid:  OCTAL-BYTES    8 ; group id
				size: OCTAL-BYTES   12 ; size
				date: OCTAL-BYTES   12 ; modification time
				      OCTAL-BYTES    8 ; checksum for header
				type: UI8              ; link indicator
				link: STRING-BYTES 100 ; name of linked file
			]
			name: to file! name

			;- UStar format (POSIX IEEE P1003.1)
			if #{757374617200} = copy/part bin/buffer 6 [
				hdr2: binary/read bin [
					        STRING-BYTES   6 ; "ustar"
					        STRING-BYTES   2 ; ustar version
					uname:  STRING-BYTES  32 ; user name
					gname:  STRING-BYTES  32 ; group name
					dev1:   STRING-BYTES   8 ; device major number
					dev2:   STRING-BYTES   8 ; device minor number
					prefix: STRING-BYTES 155 ; file name prefix (for names longer than 100)
				]
				insert name prefix
			]

			if all [
				;@@ is this correct detection?
				empty? name
				empty? link
				   0 = size
				   0 = date
				   0 = type
			][	break ] ;- end of TAR

			pos: pos + 512 ; headers are padded to 512 boundary (at least with ustar header)

			binary/read/with bin 'AT :pos

			either any [none? only  find files name][
				;- store data
				data: binary/read/with bin 'BYTES :size
				append result name
				repend/only result [data hdr1 hdr2]
				if verbose > 0 [
					sys/log/info 'TAR ["Extracting:^[[33m" name]
				] 
				if only [
					-- files-to-extract
					if files-to-extract = 0 [break]
				]
			][
				if verbose > 1 [sys/log/debug 'TAR "not extracting"]
			]
			if size > 0 [
				; skip to end of the last data block
				pos: pos + size
				if 0 < r: pos // 512 [ pos: pos + 513 - r ]
				binary/read/with bin 'AT :pos
			]
		]
		new-line/all result true
		result
	]

	;encode: function [data [binary!]][	]

	identify: function [data [binary!]][
		;no "magic number" for identification
		none
	]
	;validate-crc?: true
	verbose: 1
]