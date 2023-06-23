REBOL [
	title: "Codec: ZIP"
	name: 'codec-zip
	author: rights: "Oldes"
	version: 0.0.4
	specification: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
	history: [
		30-Nov-2021 "Oldes" {
			* Access to comment and extra field of uncompressed data
			* Added support to include file comments, extras or insternal and external attributes
			* Added support to include uncompressed data (useful when making APK targeting version 30 and above)
		}
		24-Oct-2019 "Oldes" {
			* Refactored decoder so it is using central directory structure
			* Added decode/info refinement used to resolve just the info about files without decompressing
			* Added access to decompress-file function which may be used to decompress single file using info from the info records
			* Added basic extra field data decoder
		}
		15-Mar-2019 "Oldes" {Initial version of the ZIP decoder}]
]

register-codec [
	name:  'zip
	type:  'compression
	title: "ZIP File Format"
	suffixes: [%.zip %.aar %.jar %.apk %.zipx %.appx %.epub %.docx]

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
				flags:    BITSET16       ; general purpose bit flag
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
				de-extra-fields extr
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
						crc <> crc2: checksum data 'crc32
					][
						sys/log/error 'ZIP ["CRC check failed!" crc "<>" crc2]
					]
				]
				data: reduce [modified data crc]
				unless empty? extr [append append data quote   extra: extr]
				unless empty? comm [append append data quote comment: comm]
				if att-int <> 0    [append append data quote att-int: att-int]
				if att-ext <> 0    [append append data quote att-ext: att-ext]
				append/only append result name data
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

	encode: wrap [
		bin: dir: data: date: file: add-data: root: none
		compressed-data: method: att-ext: att-int:
		compressed-size: size: crc: entries: filename-length: offset: 0

		add-file: func[file [file!] /local dir spec][
			try/with [
				spec: query/mode file [type: date:]
				either spec [
					file-name: find/tail file root
					either spec/type = 'dir [
						dir: file
						add-data file-name spec
						foreach file read dir  [
							add-file dir/:file
						]
					][
						add-data file-name reduce [spec/date read file]
					]
				][
					; wildcard?
					dir: first split-path file
					foreach file read file [
						add-file dir/:file
					]
				]
			][
				sys/log/error 'ZIP ["Failed to add file:" as-green file]
			]
		]

		add-data: func[file spec /local no-compress? extra extra-length comm comm-length][
			if verbose [sys/log/info 'ZIP ["Adding:" as-green file]]

			any [file? file cause-error 'user 'message reduce [reform ["found" type? file "where file! expected"]]]
			data: date: none
			compressed-size: size: crc: filename-length: extra-length: comm-length: att-ext: att-int: 0
			any [
				all [
					block? spec 
					parse spec [any [
						spec:
						  date!   (date: spec/1)
						| string! (data: to binary! spec/1)
						| binary! (data: spec/1)
						| 'store  (no-compress?: true)
						| quote extra: set extra binary! (extra-length: length? extra)
						| quote comment: set comm [binary! | any-string!] (
							if string? comm [comm: to binary! comm]
							comm-length: length? comm
						)
						| quote att-int: set att-int integer!
						| quote att-ext: set att-ext integer!
						| 1 skip
					]]
				]
				all [binary? spec data: spec]
				all [string? spec data: to binary! spec]
				none? spec ; just a directory
				spec = 'none
				;else..
				all [
					sys/log/error 'ZIP ["Invalid zip file's data specification:" as-red mold/part spec 30]
					continue
				]
			]
			data: any [data #{}]
			crc: checksum data 'CRC32
			size: length? data
			method: either any [
				no-compress?
				zero? size
				lesser-or-equal? size length? compressed-data: compress data 'deflate
			][
				compressed-data: data
				0 ;store
			][
				8 ;deflate
			]

			either compressed-data [
				compressed-size: length? compressed-data
			][	compressed-data: #{}
				compressed-size: 0
			]

			if any [
				none? date
				"?date?" = form date ; temp fix for invalid date!
			][	date: now ]

			filename-length: length? file
			offset: indexZ? bin/buffer-write

			unless extra [extra: #{}]
			unless comm  [comm:  #{}]

			binary/write bin [
				#{504B0304 1400 0000} ;signature / version / flags
				UI16LE         :method
				MSDOS-DATETIME :date
				UI32LE         :crc
				UI32LE         :compressed-size
				UI32LE         :size
				UI16LE         :filename-length
				UI16LE         0 ; extra
				BYTES          :file
				BYTES          :compressed-data
			]
			binary/write dir [
				#{504B0102 1703 1400 0000} ; signature / version made / version needed / flags
				UI16LE         :method
				MSDOS-DATETIME :date
				UI32LE         :crc
				UI32LE         :compressed-size
				UI32LE         :size
				UI16LE         :filename-length
				UI16LE         :extra-length ; Extra field length
				UI16LE         :comm-length  ; File comment length
				UI16LE         0             ; Disk number where file starts
				UI16LE         :att-int      ; Internal file attributes
				UI32LE         :att-ext      ; External file attributes
				UI32LE         :offset       ; Relative offset of local file header
				BYTES          :file
				BYTES          :extra        ; Extra field
				BYTES          :comm         ; File comment
			]
			++ entries
		]

		;- ENCODE:
		func [
			"Compress given block of files."
			files [block! file!] "[file! binary! ..] or [file! [date! crc binary!] or [dir! none!] ..]"
		][
			bin: binary 10000
			dir: binary 1000
			entries: 0

			either file? files [
				root: first split-path files
				add-file files
			][
				foreach [file spec] files [
					add-data file spec
				]
			]

			dir-size: length? dir/buffer
			bin-size: length? bin/buffer

			binary/write bin [
				BYTES :dir/buffer
				#{504B0506}      ; End of central directory signature
				UI16LE 0         ; Number of this disk
				UI16LE 0         ; Disk where central directory starts
				UI16LE :entries  ; Number of central directory records on this disk
				UI16LE :entries  ; Total number of central directory records
				UI32LE :dir-size ; Size of central directory
				UI32LE :bin-size ; Offset of start of central directory
				UI16LE 0         ; Comment length
				;#{}             ; Comment
			]
			bin/buffer
		]
	]

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
		unless 67324752 = binary/read bin 'UI32LE [ ;#{504B0304}
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
			8  [ ;- deflate
				output: decompress/size data 'deflate unc-size
			]
			14 [ ;- LZMA
				output: decompress/part/size (skip data 4) 'lzma cmp-size unc-size
			]
			0  [ ;- store
				output: copy/part data cmp-size
			]
		][
			sys/log/error 'ZIP ["Unsupported compression method:^[[0;35m" method]
		]

		return output
	]

	de-extra-fields: function [
		"Decodes extra field data of the ZIP record"
		extra [binary!] "Extra field data"
	][
		sys/log/debug 'ZIP ["Decode extra fields:" mold extra]
		bin: binary extra
		fields: copy []
		while [not tail? bin/buffer][
			binary/read bin [id: UI16LE len: UI16LE data: BYTES :len]
			repend fields [id data]
		]
		sys/log/more 'ZIP ["Extra fields:" mold fields]
		fields
		
		;- Extra field info:
		; https://opensource.apple.com/source/zip/zip-6/unzip/unzip/proginfo/extra.fld

		;- List of extra field ids as defined in Info-ZIP utility:
		;#define EF_PKSZ64    0x0001    /* PKWARE's 64-bit filesize extensions */
		;#define EF_AV        0x0007    /* PKWARE's authenticity verification */
		;#define EF_EFS       0x0008    /* PKWARE's extended language encoding */
		;#define EF_OS2       0x0009    /* OS/2 extended attributes */
		;#define EF_PKW32     0x000a    /* PKWARE's Win95/98/WinNT filetimes */
		;#define EF_PKVMS     0x000c    /* PKWARE's VMS */
		;#define EF_PKUNIX    0x000d    /* PKWARE's Unix */
		;#define EF_PKFORK    0x000e    /* PKWARE's future stream/fork descriptors */
		;#define EF_PKPATCH   0x000f    /* PKWARE's patch descriptor */
		;#define EF_PKPKCS7   0x0014    /* PKWARE's PKCS#7 store for X.509 Certs */
		;#define EF_PKFX509   0x0015    /* PKWARE's file X.509 Cert&Signature ID */
		;#define EF_PKCX509   0x0016    /* PKWARE's central dir X.509 Cert ID */
		;#define EF_PKENCRHD  0x0017    /* PKWARE's Strong Encryption header */
		;#define EF_PKRMCTL   0x0018    /* PKWARE's Record Management Controls*/
		;#define EF_PKLSTCS7  0x0019    /* PKWARE's PKCS#7 Encr. Recipient Cert List */
		;#define EF_PKIBM     0x0065    /* PKWARE's IBM S/390 & AS/400 attributes */
		;#define EF_PKIBM2    0x0066    /* PKWARE's IBM S/390 & AS/400 compr. attribs */
		;#define EF_IZVMS     0x4d49    /* Info-ZIP's VMS ("IM") */
		;#define EF_IZUNIX    0x5855    /* Info-ZIP's first Unix[1] ("UX") */
		;#define EF_IZUNIX2   0x7855    /* Info-ZIP's second Unix[2] ("Ux") */
		;#define EF_IZUNIX3   0x7875    /* Info-ZIP's newest Unix[3] ("ux") */
		;#define EF_TIME      0x5455    /* universal timestamp ("UT") */
		;#define EF_UNIPATH   0x7075    /* Info-ZIP Unicode Path ("up") */
		;#define EF_UNICOMNT  0x6375    /* Info-ZIP Unicode Comment ("uc") */
		;#define EF_MAC3      0x334d    /* Info-ZIP's new Macintosh (= "M3") */
		;#define EF_JLMAC     0x07c8    /* Johnny Lee's old Macintosh (= 1992) */
		;#define EF_ZIPIT     0x2605    /* Thomas Brown's Macintosh (ZipIt) */
		;#define EF_ZIPIT2    0x2705    /* T. Brown's Mac (ZipIt) v 1.3.8 and newer ? */
		;#define EF_SMARTZIP  0x4d63    /* Mac SmartZip by Marco Bambini */
		;#define EF_VMCMS     0x4704    /* Info-ZIP's VM/CMS ("\004G") */
		;#define EF_MVS       0x470f    /* Info-ZIP's MVS ("\017G") */
		;#define EF_ACL       0x4c41    /* (OS/2) access control list ("AL") */
		;#define EF_NTSD      0x4453    /* NT security descriptor ("SD") */
		;#define EF_ATHEOS    0x7441    /* AtheOS ("At") */
		;#define EF_BEOS      0x6542    /* BeOS ("Be") */
		;#define EF_QDOS      0xfb4a    /* SMS/QDOS ("J\373") */
		;#define EF_AOSVS     0x5356    /* AOS/VS ("VS") */
		;#define EF_SPARK     0x4341    /* David Pilling's Acorn/SparkFS ("AC") */
		;#define EF_TANDEM    0x4154    /* Tandem NSK ("TA") */
		;#define EF_THEOS     0x6854    /* Jean-Michel Dubois' Theos "Th" */
		;#define EF_THEOSO    0x4854    /* old Theos port */
		;#define EF_MD5       0x4b46    /* Fred Kantor's MD5 ("FK") */
		;#define EF_ASIUNIX   0x756e    /* ASi's Unix ("nu") */ 
	]

	validate-crc?: true
	verbose: true
	level: 9
]