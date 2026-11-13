Rebol [
	Title:		"Munge functions"
	Owner:		"Ashley G Truter"
	Version:	3.0.6
	Date:		9-Apr-2019
	Purpose:	"Extract and manipulate tabular values in blocks, delimited files and database tables."
	Licence:	"MIT. Free for both commercial and non-commercial use."
	Tested: {
		Windows
			REBOL/View		2.7.8		rebol.com
			R3/64-bit		3.0.99		atronixengineering.com/downloads
			RED/32-bit		0.6.4		red-lang.org
		macOS
			REBOL/View		2.7.8		rebol.com
			RED/32-bit		0.6.4		red-lang.org
	}
	Changes: {
		Removed:
			build, platform and target moved from global context to settings
			append-column
			ascii-file?
			average-of
			call-oledb
			delta
			load-excel (replaced with oledb)
			map-source
			oledb-file?
			order
			remove-column
			split-line
			sum-of
			to-rebol-date
			to-rebol-time
		Added:
			crc32
			excel?
			load-xml (uses unarchive)
			oledb (refactored from load-excel)
			settings
			literal column support to load-dsv, load-xml and munge (e.g. load-dsv/part file [1 "No"])
			named column support to load-dsv, load-xml and munge (e.g. munge/where next [["Name" "Age"]["Sam" 50]] [&Age > 30])
		Updated:
			munge/update refinement removed (use munge/where blk [row/1: 0])
			munge/delete is now the same as munge/delete/where
			R2 support in archive and unarchive (requires /View)
			Excel support (using unarchive) added to cols?, fields?, rows? and sheets?
			replace-deep replaces multiple values in a single pass
			to-string-date and to-string-time now support Excel XML date (e.g. "41000") and time (e.g. ("0.33") formats
			list uses settings/console (default TRUE) to control new-line behaviour
			load-dsv (plus oledb, sqlcmd and sqlite) and load-xml use settings/as-is (default TRUE) to control trim/lines behaviour
			read-string optimized
			read-binary lost its /seek refinement
			added load-dsv/csv
		Fixed:
			Several parse copy words that escaped to the global context
			Unarchive bug
			/group 'count bug
	}
	Usage: {
		archive				Compress block of file and data pairs.
		call-out			Call OS command returning STDOUT.
		check				Verify data structure.
		cols?				Number of columns in a delimited file or string.
		crc32				Returns a CRC32 checksum.
		delimiter?			Probable delimiter, with priority given to comma, tab, bar, tilde then semi-colon.
		dezero				Remove leading zeroes from string.
		digit				DIGIT is a bitset! value: make bitset! #{000000000000FFC0}
		digits?				Returns TRUE if data not empty and only contains digits.
		distinct			Remove duplicate and empty rows.
		enblock				Convert a block of values to a block of row blocks.
		enzero				Add leading zeroes to a string.
		excel?				Returns TRUE if file is Excel or worksheet is XML.
		export				Export words to global context.
		fields?				Column names in a delimited file or string.
		first-line			Returns the first non-empty line of a file.
		flatten				Flatten nested block(s).
		latin1-to-utf8		Latin1 binary to UTF-8 string conversion.
		letter				LETTER is a bitset! value: make bitset! #{00000000000000007FFFFFE07FFFFFE0}
		letters?			Returns TRUE if data only contains letters.
		like				Finds a value in a series, expanding * (any characters) and ? (any one character), and returns TRUE if found.
		list				Uses settings to optionally trim strings and set the new-line marker.
		load-dsv			Parses delimiter-separated values into row blocks.
		load-fixed			Loads fixed-width values from a file.
		load-xml			Loads an Office XML sheet.
		max-of				Returns the largest value in a series.
		merge				Join outer block to inner block on primary key.
		min-of				Returns the smallest value in a series.
		mixedcase			Converts string of characters to mixedcase.
		munge				Load and/or manipulate a block of tabular (column and row) values.
		oledb				Execute an OLEDB statement.
		parse-series		Parses a series according to grammar rules.
		read-binary			Read bytes from a file.
		read-string			Read string from a text file.
		replace-deep		Replaces all occurences of search values with new values in a block or nested block.
		rows?				Number of rows in a delimited file or string.
		second-last/penult	Returns the second last value of a series.
		sheets?				Excel sheet names.
		sqlcmd				Execute a SQL Server statement.
		sqlite				Execute a SQLite statement.
		to-column-alpha		Convert numeric column reference to an alpha column reference.
		to-column-number	Convert alpha column reference to a numeric column reference.
		to-string-date		Convert a string or Rebol date to a YYYY-MM-DD string.
		to-string-time		Convert a string or Rebol time to a HH:MM:SS string.
		unarchive			Decompresses archive (only works with compression methods 'store and 'deflate).
		write-dsv			Write block(s) of values to a delimited text file.
		write-excel			Write block(s) of values to an Excel file.
	}
	needs: 3.16.0 ;; new construction syntax
]


average: function [
	"Returns the average of all values in a block"
	block [block!]
][
	all [empty? block return none]
	divide sum block length? block
]

sum: function [
	"Returns the sum of all values in a block"
	values [block!]
] [
	result: 0
	foreach value values [result: result + value]
	result
]


ctx-munge: context [

	settings: context [

		build: 'r3
		os: system/platform
		target: either find form system/build/target "64" [64] [32]

		stack: copy []

		start-time: start-used: none

		called: function [
			name [word! none!]
			/file path [file! url! binary!]
		] [
			any [trace exit]
			either word? name [
				insert/dup message: reform ["Call" either all [file not binary? path] [reform [name "on" last split-path path]] [name]] "  " length? stack
				all [
					empty? stack
					recycle
					settings/start-time: now/precise
					settings/start-used: stats
				]
				append stack name
			] [
				insert/dup message: reform ["Exit" last stack] "  " -1 + length? stack
				take/last stack
			]
			print [next next next to-string-time/precise difference now/precise start-time head insert/dup s: form to integer! stats - start-used / 1048576 " " 4 - length? s message]
		]

		exited: function [] [
			any [empty? stack called none]
		]

		error: function [
			message [string!]
		] compose [
			clear stack
			(either build = 'red [[cause-error 'user 'message reduce [message]]] [[cause-error 'user 'message message]])
		]

		as-is: console: trace: true
	]

	archive: function [ ; https://en.wikipedia.org/wiki/Zip_(file_format) & http://www.rebol.org/view-script.r?script=rebzip.r
		"Compress block of file and data pairs"
		source [series!]
	] [
		encode 'ZIP source
	]

	call-out: function [
		"Call OS command returning STDOUT"
		cmd [string!]
	] [
		all [settings/console settings/called 'call-out]
		call/wait/output/error cmd stdout: make binary! 65536 stderr: make string! 1024
		any [empty? stderr settings/error trim/lines stderr]
		also read-string stdout all [settings/console settings/exited]
	]

	check: function [
		"Verify data structure"
		data [block!]
	] [
		unless empty? data [
			cols: length? data/1
			i: 1
			foreach row data [
				if message: case [
					not block? row			[reform ["expected block but found" type? row]]
					zero? length? row		["empty"]
					cols <> length? row		[reform ["expected" cols "column(s) but found" length? row]]
					not not find row none	["contains a none value"]
					block? row/1			["did not expect first column to be a block"]
				] [
					cause-error 'user 'message reduce [reform ["Row" i message]]
				]
				i: i + 1
			]
		]
		true
	]

	cols?: function [
		"Number of columns in a delimited file or string"
		data [file! url! binary! string!]
		/with
			delimiter [char!]
		/sheet
			number [integer!]
	] [
		all [settings/console settings/called 'cols?]
		also either excel? data [
			any [
				binary? data
				data: unarchive/only data rejoin [%xl/worksheets/sheet any [number 1] %.xml]
				settings/error reform [number "not a valid sheet number"]
			]
			dim: cols: 0
			all [
				find data #{3C64696D}	; <dim
				parse to string! copy/part find data #{3C64696D} 32 [
					thru {<dimension ref="} thru ":" copy dim some letter (dim: to-column-number dim) to end
				]
			]
			all [
				find data #{3C636F6C}	; <col
				parse to string! copy/part find/last data #{3C636F6C} find data #{3C2F636F6C733E} [
					thru {="} copy cols to {"} (cols: to integer! cols) to end
				]
			]
			max dim cols
		] [
			length? either with [fields?/with data delimiter] [fields? data]
		] all [settings/console settings/exited]
	]

	crc32: function [
		"Returns a CRC32 checksum"
		data [binary! string!]
	] [
		checksum data 'CRC32
	]

	delimiter?: function [
		"Probable delimiter, with priority given to comma, tab, bar, tilde then semi-colon"
		data [file! url! string!]
	] [
		data: first-line data
		counts: copy [0 0 0 0 0]
		foreach char data [
			switch char [
				#","	[counts/1: counts/1 + 1]
				#"^-"	[counts/2: counts/2 + 1]
				#"|"	[counts/3: counts/3 + 1]
				#"~"	[counts/4: counts/4 + 1]
				#";"	[counts/5: counts/5 + 1]
			]
		]
		pick [#"," #"^-" #"|" #"~" #";"] index? find counts max-of counts
	]

	dezero: function [
		"Remove leading zeroes from string"
		string [string!]
	] [
		parse string [remove some #"0"]
		string
	]

	digit: charset [#"0" - #"9"]

	digits?: function [
		"Returns TRUE if data not empty and only contains digits"
		data [string! binary!]
	] compose/deep [
		all [not empty? data not find data (complement digit)]
	]

	distinct: function [
		"Remove duplicate and empty rows"
		data [block!]
	] [
		all [settings/console settings/called 'distinct]
		old-row: none
		remove-each row sort data [
			any [
				all [
					find ["" #(none)] row/1
					1 = length? unique row
				]
				either row = old-row [true] [old-row: row false]
			]
		]
		all [settings/console settings/exited]
		data
	]

	enblock: function [
		"Convert a block of values to a block of row blocks"
		data [block!]
		cols [integer!]
	] [
		all [block? data/1 return data]
		any [integer? rows: divide length? data cols cause-error 'user 'message ["Cols not a multiple of length"]]
		repeat i rows [
			change/part/only at data i copy/part at data i cols cols
		]
		data
	]

	enzero: function [
		"Add leading zeroes to a string"
		string [string!]
		length [integer!]
	] [
		insert/dup string #"0" length - length? string
		string
	]

	excel?: function [
		"Returns TRUE if file is Excel or worksheet is XML"
		data [file! url! binary! string!]
	] [
		switch/default type?/word data [
			string!		[false]
			binary!		[not not find copy/part data 8 #{3C3F786D6C}]	; ignore UTF mark
		] [
			all [
				%.xls = copy/part suffix? data 4
				#{504B} = read-binary/part data 2	; PK
			]
		]
	]

	export: function [
		"Export words to global context"
		words [block!] "Words to export"
	] [
		foreach word words [
			do compose [(to-set-word word) (to-get-word in self word)]
		]
		words
	]

	fields?: function [
		"Column names in a delimited file"
		data [file! url! binary! string!]
		/with
			delimiter [char!]
		/sheet
			number [integer!]
	] [
		all [settings/console settings/called 'fields?]
		also either excel? data [
			load-xml/sheet/fields data any [number 1]
		] [
			data: first-line data
			case [
				empty? data [
					make block! 0
				]
				#"^"" = first data [
					flatten load-dsv/ignore/with/csv data any [delimiter delimiter? data]
				]
				true [
					flatten load-dsv/ignore/with data any [delimiter delimiter? data]
				]
			]
		] all [settings/console settings/exited]
	]

	first-line: function [
		"Returns the first non-empty line of a file"
		data [file! url! string!]
	] [
		data: deline/lines either file? data [
			latin1-to-utf8 read-binary/part data 4096
		] [
			copy/part data 4096
		]

		foreach line data [
			any [empty? line return line]
		]

		copy ""
	]

	flatten: function [ ; http://www.rebol.org/view-script.r?script=flatten.r
		"Flatten nested block(s)"
		data [block!]
	] [
		either empty? data [data] [
			result: copy []
			foreach row data [
				append result row
			]
		]
	]

	latin1-to-utf8: function [ ; http://stackoverflow.com/questions/21716201/perform-file-encoding-conversion-with-rebol-3
		"Latin1 binary to UTF-8 string conversion"
		data [binary!]
	] compose/deep [
		;	remove #"^@"
		trim/with data null
		;	remove #"^M" from split crlf
		either empty? data [make string! 0] [
			all [cr = last data take/last data]
			;	replace char 160 with space - http://www.adamkoch.com/2009/07/25/white-space-and-character-160/
			mark: data
			while [mark: find mark #{C2A0}] [
				change/part mark #{20} 2
			]
			;	replace latin1 with UTF
			(either settings/build = 'r2 [] [[
				mark: data
				while [mark: invalid-utf? mark] [
					change/part mark to char! mark/1 1
				]
			]])
			deline to string! data
		]
	]

	letter: charset [#"A" - #"Z" #"a" - #"z"]

	letters?: function [
		"Returns TRUE if data only contains letters"
		data [string! binary!]
	] compose [
		not find data (complement letter)
	]

	like: function [ ; http://stackoverflow.com/questions/31612164/does-anyone-have-an-efficient-r3-function-that-mimics-the-behaviour-of-find-any
		"Finds a value in a series, expanding * (any characters) and ? (any one character), and returns TRUE if found"
		series [any-string!] "Series to search"
		value [any-string!] "Value to find"
		/local part
	] compose [
		all [empty? series return none]
		literal: (complement charset "*?")
		value: collect [
			parse value [
				end (keep [return (none)]) |
				some #"*" end (keep [to end]) |
				some [
					#"?" (keep 'skip) |
					copy part some literal (keep part) |
					some #"*" any [#"?" (keep 'skip)] opt [copy part some literal (keep 'thru keep part)]
				]
			]
		]
		parse series [some [result: value (return true)]]
	]

	list: function [
		"Uses settings to optionally trim strings and set the new-line marker"
		data [block!]
	] [
		either settings/console [
			foreach row data [all [block? row new-line/all row false]]
			settings/exited
			new-line/all data true
		] [data]
	]

	load-dsv: function [ ; http://www.rebol.org/view-script.r?script=csv-tools.r
		"Parses delimiter-separated values into row blocks"
		source [file! url! binary! string!]
		/part "Offset position(s) to retrieve"
			columns [block! integer! word!]
		/where "Expression that can reference columns as row/1, row/2, etc"
			condition [block!]
		/with "Alternate delimiter (default is tab, bar then comma)"
			delimiter [char!]
		/ignore "Ignore truncated row errors"
		/csv "Parse as CSV even though not comma-delimited"
	] compose [
		all [settings/console settings/called 'load-dsv]

		source: either string? source [
			deline source
		] [
			all [
				file? source
				#{22} = read-binary/part source 1
				csv: true
			]
			read-string source
		]

		any [with delimiter: delimiter? source]

		value: either any [delimiter = #"," csv] [
			[
				any [#" "] {"} copy v to [{"} | end]
				any [{"} x: {"} to [{"} | end] y: (append/part v x y)]
				[{"} to [delimiter | lf | end]] (append row v)
				| any [#" "] copy v to [delimiter | lf | end] (append row trim/tail v)
			]
		] [
			[any [#" "] copy v to [delimiter | lf | end] (append row trim/tail v)]
		]
		rule: copy/deep [
			any [
				not end (row: make block! cols)
				value
				any [delimiter value] [lf | end] ()
			]
		]

		cols: either all [ignore not find source newline] [32] [length? fields: fields?/with source delimiter]

		;	Replace field references with row paths

		all [
			find reform [columns condition] "&"
			set [columns condition] munge/spec/part/where reduce [fields] columns condition
		]

		line: 0

		blk: copy []

		append last last rule compose/deep [
			line: line + 1
			(either settings/as-is [] [[foreach val row [trim/lines val]]])
			all [
				row <> [""]
				(either where [condition] [])
				(either ignore [] [compose/deep [any [(cols) = len: length? row settings/error reform ["Expected" (cols) "values but found" len "on line" line]]]])
				(either part [
					part: copy/deep [reduce []]
					foreach col columns: to block! columns [
						append part/2 either integer? col [
							all [not ignore any [col < 1 col > cols] settings/error reform ["invalid /part position:" col]]
							compose [(append to path! 'row col)]
						] [col]
					]
					compose [row: (part)]
				] [])
				(either settings/build = 'r2 [[row <> pick tail blk -1 append/only]] [[row <> last blk append/only]]) blk row
			]
		]

		parse-series source bind rule 'row

		list blk
	]

	load-fixed: function [
		"Loads fixed-width values from a file"
		file [file! url!]
		/spec
			widths [block!]
		/part
			columns [integer! block!]
	] [
		all [settings/console settings/called 'load-fixed]

		unless spec [
			widths: reduce [1 + length? line: first-line file]
			;	R2/Red index? fails on none
			while [all [s: find/last/tail line "  " i: index? s]] [
				insert widths i
				line: trim copy/part line i - 1
			]

			insert widths 1

			repeat i -1 + length? widths [
				poke widths i widths/(i + 1) - widths/:i
			]

			take/last widths
		]

		spec: copy []
		pos: 1

		either part [
			part: copy []
			foreach width widths [
				append/only part reduce [pos width]
				pos: pos + width
			]
			foreach col to block! columns [
				append spec compose [trim/lines copy/part at line (part/:col/1) (part/:col/2)]
			]
		] [
			foreach width widths [
				append spec compose [trim/lines copy/part at line (pos) (width)]
				pos: pos + width
			]
		]

		blk: copy []

		foreach line deline/lines read-string file compose/deep [
			all [line/1 = #"^L" remove line]
			any [
				empty? trim copy line
				append/only blk reduce [(spec)]
			]
		]

		list blk
	]

	load-xml: function [
		"Loads an Office XML sheet"
		file [file!]
		/part "Offset position(s) to retrieve"
			columns [block! integer! word!]
		/where "Expression that can reference columns as row/1, row/2, etc"
			condition [block!]
		/sheet number [integer!]
		/fields
		/local s v x col type val
	] compose [
		all [settings/console settings/called 'load-xml]

		any [
			sheet: unarchive/only file rejoin [%xl/worksheets/sheet number: any [number 1] %.xml]
			settings/error reform [number "not a valid sheet number"]
		]

		strings: make block! 65536

		parse-series read-string unarchive/only file %xl/sharedStrings.xml [
			any [
				thru "<si>"
				thru ">" any #" " copy s to "<" (
					append strings decode 'html-entities s
				)
			]
		]

		cols: cols? sheet ;- cols? may return 0 if there is no <dimension> or <cols> tags!

		rule: copy/deep [
			to "<row"
			any [
				opt [newline]
				opt ["<row" (append/dup row: make block! cols "" cols)]
				thru {<c r="} copy col to digit
				copy type thru ">"
				opt ["<v>" copy val to "</v></c>" (
					col: to-column-number col
					if col > n: length? row [append/dup row "" col - n] 
					poke row col either find type {t="s"} [pick strings 1 + to integer! val] [trim val]
				) "</v></c>"]
				opt [newline]
				opt ["</row>" ()]
			]
		]

		if any [fields find reform [columns condition] "&"] [
			parse-series read-string copy/part sheet find/tail sheet #{3C2F726F773E} rule
			all [fields return list row]
			set [columns condition] munge/spec/part/where reduce [row] columns condition
		]

		append last last last rule compose/deep [
			(either settings/as-is [] [[foreach val row [trim/lines val]]])
			all [
				(either where [condition] [])
				(either part [
					part: copy/deep [reduce []]
					foreach col columns: to block! columns [
						append part/2 either integer? col [
							all [any [col < 1 col > (cols)] settings/error reform ["invalid /part position:" col]]
							compose [(append to path! 'row col)]
						] [col]
					]
					compose [row: (part)]
				] [])
				row <> last blk append/only blk row
			]
		]

		blk: copy []

		parse-series read-string sheet bind rule 'row

		list blk
	]

	max-of: function [
		"Returns the largest value in a series"
		series [series!] "Series to search"
	] [
		all [empty? series return none]
		val: series/1
		foreach v series [val: max val v]
		val
	]

	merge: function [
		"Join outer block to inner block on primary key"
		outer [block!] "Outer block"
		key1 [integer!]
		inner [block!] "Inner block to index"
		key2 [integer!]
		columns [block!] "Offset position(s) to retrieve in merged block"
		/default "Use none on inner block misses"
	] [
		;	build rowid map of inner block
		map: make map! length? inner
		i: 0
		foreach row inner [
			put map row/:key2 i: i + 1
		]
		;	build column picker
		code: copy []
		foreach col columns [
			append code compose [(append to path! 'row col)]
		]
		;	iterate through outer block
		blk: make block! length? outer
		do compose/deep [
			either default [
				foreach row outer [
					all [
						i: (either settings/build = 'r2 [[select-map]] [[select]]) map row/:key1
						append row inner/:i
					]
					append/only blk reduce [(code)]
				]
			] [
				foreach row outer [
					all [
						i: (either settings/build = 'r2 [[select-map]] [[select]]) map row/:key1
						append row inner/:i
						append/only blk reduce [(code)]
					]
				]
			]
		]

		blk
	]

	min-of: function [
		"Returns the smallest value in a series"
		series [series!] "Series to search"
	] [
		all [empty? series return none]
		val: series/1
		foreach v series [val: min val v]
		val
	]

	mixedcase: function [
		"Converts string of characters to mixedcase"
		string [string!]
	] [
		uppercase/part lowercase string 1
		foreach char [#"'" #" " #"-" #"." #","] [
			all [find string char string: next find string char mixedcase string]
		]
		string: head string
	]

	munge: function [
		"Load and/or manipulate a block of tabular (column and row) values"
		data [block!]
		/delete "Delete matching rows (returns original block)"
			clause
		/part "Offset position(s) and/or values to retrieve"
			columns [block! integer! word! none!]
		/where "Expression that can reference columns as row/1, row/2, etc"
			condition
		/group "One of count, max, min or sum"
			having [word! block!] "Word or expression that can reference the initial result set column as count, max, etc"
		/spec "Return coulmns and condition with field substitutions"
	] [
		all [empty? data return data]

		all [delete where: true condition: clause]

		if all [where condition not block? condition] [ ; http://www.rebol.org/view-script.r?script=binary-search.r
			lo: 1
			hi: rows: length? data
			mid: to integer! hi + lo / 2
			while [hi >= lo] [
				if condition = key: first data/:mid [
					lo: hi: mid
					while [all [lo > 1 condition = first data/(lo - 1)]] [lo: lo - 1]
					while [all [hi < rows condition = first data/(hi + 1)]] [hi: hi + 1]
					break
				]
				either condition > key [lo: mid + 1] [hi: mid - 1]
				mid: to integer! hi + lo / 2
			]
			all [
				lo > hi
				return either delete [data] [make block! 0]
			]
			rows: hi - lo + 1
			either delete [
				return head remove/part at data lo rows
			] [
				data: copy/part at data lo rows
				where: condition: none
			]
		]

		all [settings/console settings/called 'munge]

		;	Replace field references with row paths

		if find reform [columns condition] "&" [
			repeat i length? fields: copy/deep first head data [
				poke fields i to word! join '& trim/with trim/all form fields/:i "@#$%^,"
			]

			if find form columns "&" [
				;	replace &Word with n
				number-map: make map! 32
				repeat i length? fields [
					put number-map fields/:i i
				]
				repeat i length? columns: to block! columns [
					all [
						word? columns/:i
						any [
							columns/:i: select number-map columns/:i
							settings/error "invalid /part position"
						]
					]
				]
			]

			if find form condition "&" [
				;	replace &Word with row/n
				path-map: make block! 32
				repeat i length? fields [
					append path-map reduce [
						fields/:i
						append to path! 'row i
					]
				]
				replace-deep condition: copy/deep condition path-map
				if find form condition "&" [
					;	replace &Set-Word: with row/n:
					repeat i length? path-map [
						path-map/:i: either word? path-map/:i [to set-word! path-map/:i] [to set-path! path-map/:i]
					]
					replace-deep condition path-map
				]
			]

			all [spec also return reduce [columns condition] all [settings/console settings/exited]]
		]

		case [
			delete [
				remove-each row data bind compose/only [all (condition)] 'row
				all [settings/console settings/exited]
				return data
			]
			any [part where] [
				columns: either part [
					part: copy/deep [reduce []]
					cols: length? data/1
					foreach col to block! columns [
						append part/2 either integer? col [
							all [any [col < 1 col > cols] settings/error reform ["invalid /part position:" col]]
							compose [(append to path! 'row col)]
						] [col]
					]
					part
				] ['row]
				blk: copy []
				foreach row data compose [
					(
						either where [
							either settings/build <> 'red [
								compose/deep [all [(condition) append/only blk (columns)]]
							] [
								bind compose/deep [all [(condition) append/only blk (columns)]] 'row
							]
						] [
							compose [append/only blk (columns)]
						]
					)
				]
				all [empty? blk also return blk all [settings/console settings/exited]]
				data: blk
			]
		]

		if group [
			words: unique flatten to block! having
			operation: case [
				find words 'avg		['average]
				find words 'count	['count]
				find words 'max		['max-of]
				find words 'min		['min-of]
				find words 'sum		['sum]
				true				[settings/error "Invalid group operation"]
			]
			case [
				operation = 'count [
					i: 0
					blk: copy []
					group: copy first sort data
					foreach row data [
						either group = row [i: i + 1] [
							append group i
							append/only blk group
							group: copy row
							i: 1
						]
					]
					append group i
					append/only blk group
				]
				1 = length? data/1 [
					all [settings/console settings/exited]
					return do compose [(operation) flatten data]
				]
				true [
					val: copy []
					blk: copy []
					group: copy/part first sort data len: -1 + length? data/1
					foreach row data compose/deep [
						either group = copy/part row (len) [append val last row] [
							append group (operation) val
							append/only blk group
							group: copy/part row (len)
							append val: copy [] last row
						]
					]
					append group do compose [(operation) val]
					append/only blk group
				]
			]
			data: blk

			if block? having [
				replace-deep having reduce [operation append to path! 'row length? data/1]
				all [settings/console settings/exited]
				return munge/where data having
			]
		]

		list data
	]

	oledb: if settings/os = 'Windows [
		function [
			"Execute an OLEDB statement"
			file [file! url!]
			statement [string!] "SQL statement in the form (Excel) 'SELECT F1 FROM SHEET1' or (Access) 'SELECT Column FROM Table'"
			/local sheet
		] compose/deep [
			any [exists? file cause-error 'access 'cannot-open reduce [file]]
			all [settings/console settings/called/file 'oledb file]
			statement: replace/all copy statement {'} {''}
			properties: either %.accdb = suffix? file [""] [
				parse statement [thru "FROM " copy sheet [to " " | to end]]
				replace statement reform ["FROM" sheet] rejoin ["FROM ['+$o.GetSchema('Tables').rows[" -1 + to integer! skip sheet 5 "].TABLE_NAME+']"]
				{;Extended Properties=''Excel 12.0 Xml;HDR=NO;IMEX=1;Mode=Read''}
			]
			also remove load-dsv/csv/with call-out rejoin [
				(either settings/target = 64 ["powershell "] ["C:\Windows\SysNative\WindowsPowerShell\v1.0\powershell.exe "])
				{-nologo -noprofile -command "}
					{$o=New-Object System.Data.OleDb.OleDbConnection('Provider=Microsoft.ACE.OLEDB.12.0;}
						{Data Source=\"} replace/all to-local-file clean-path file "'" "''" {\"} properties {');}
					{$o.Open();$s=New-Object System.Data.OleDb.OleDbCommand('} statement {');}
					{$s.Connection=$o;}
					{$t=New-Object System.Data.DataTable;}
					{$t.Load($s.ExecuteReader());}
					{$o.Close();}
					{$t|ConvertTo-CSV -Delimiter `t -NoTypeInformation}
				{"}
			] tab all [settings/console settings/exited]
		]
	]

	parse-series: function [
		"Parses a series according to grammar rules"
		series [series!]
		rules [block!]
	] [
		all [settings/console settings/called 'parse]
		also parse series rules all [settings/console settings/exited]
	]

	read-binary: function [
		"Read bytes from a file"
		source [file! url!]
		/part "Reads a specified number of bytes."
			length [integer!]
	] [
		all [settings/console settings/called/file 'read-binary source]
		also either part [
			read/part source length
		] [	read source	] all [settings/console settings/exited]
	]

	read-string: function [
		"Read string from a text file"
		source [file! url! binary!]
	] compose/deep [
		all [settings/console settings/called/file 'read-string source]
		any [binary? source source: read source]
		also to string! either invalid-utf? source [iconv/to source 'latin1 'utf8][source]
			all [settings/console settings/exited]
	]

	replace-deep: function [
		"Replaces all occurences of search values with new values in a block or nested block"
		data [block!] "Block to replace within (modified)"
		map [map! block!] "Map of values to replace"
	] compose/deep [
		repeat i length? data [
			either block? data/:i [replace-deep data/:i map] [
				all [
					not path? data/:i
					not set-path? data/:i
					val: (either settings/build = 'r2 [[select-map]] [[select]]) map data/:i
					any [
						equal? type? data/:i type? val
						all [word? data/:i path? val]
						all [set-word? data/:i set-path? val]
					]
					data/:i: val
				]
			]
		]
		data
	]

	rows?: function [
		"Number of rows in a delimited file or string"
		data [file! url! binary! string!]
		/sheet
			number [integer!]
		/local rows
	] [
		either excel? data [
			any [
				binary? data
				data: unarchive/only data rejoin [%xl/worksheets/sheet any [number 1] %.xml]
				cause-error 'user 'message reduce [reform [number "not a valid sheet number"]]
			]
			all [
				find data #{3C726F7720}
				parse to string! find/last data #{3C726F7720} [
					thru {"} copy rows to {"} (return to integer! rows)
				]
			]
			0
		] [
			either any [
				all [file? data zero? size? data]
				empty? data
			] [0] [
				i: 1
				parse either file? data [read-binary data] [data] [
					any [thru newline (i: i + 1)]
				]
				i
			]
		]
	]

	second-last: penult: function [
		"Returns the second last value of a series"
		string [series!]
	] [
		pick tail string -2
	]

	sheets?: function [
		"Excel sheet names"
		file [file! url!]
		/local name
	] [
		all [settings/console settings/called 'sheets?]
		blk: copy []
		parse to string! unarchive/only file %xl/workbook.xml [
			any [
				thru {<sheet } copy tmp to #">" (
					parse tmp [thru {name="} copy name to {"} (append blk trim name)]
				)
			]
		]
		all [settings/console settings/exited]
		blk
	]

	sqlcmd: if settings/os = 'Windows [
		function [
			"Execute a SQL Server statement"
			server [string!]
			database [string!]
			statement [string!]
			/key "Columns to convert to integer"
				columns [integer! block!]
			/headings "Keep column headings"
			/string
			/affected "Return rows affected instead of empty block"
		] [
			all [settings/console settings/called 'sqlcmd]

			stdout: call-out reform compose ["sqlcmd -X -S" server "-d" database "-I -Q" rejoin [{"} statement {"}] {-W -w 65535 -s"^-"} (either headings [] [{-h -1}])]

			case [
				string [
					all [settings/console settings/exited]
					stdout
				]
				stdout/1 = #"^/" [
					all [settings/console settings/exited]
					either affected [last deline/lines stdout] [make block! 0]
				]
				like stdout "Msg*,*Level*,*State*,*Server" [
					settings/error trim/lines find stdout "Line"
				]
				true [
					stdout: load-dsv/with copy/part stdout find stdout "^/^/(" #"^-"

					all [headings remove next stdout]

					foreach row stdout [
						foreach val row [
							all ["NULL" == val clear val]
						]
					]

					all [
						key
						foreach row either headings [next stdout] [stdout] [
							foreach i to block! columns [
								row/:i: to integer! row/:i
							]
						]
					]

					all [settings/console settings/exited]

					stdout
				]
			]
		]
	]

	sqlite: function [
		"Execute a SQLite statement"
		database [file! url!]
		statement [string!]
	] [
		load-dsv/with call-out rejoin [{sqlite3 -separator "^-" } to-local-file database { "} statement {"}] tab
	]

	to-column-alpha: function [
		"Convert numeric column reference to an alpha column reference"
		number [integer!] "Column number between 1 and 702"
	] [
		any [positive? number cause-error 'user 'message ["Positive number expected"]]
		any [number <= 702 cause-error 'user 'message ["Number cannot exceed 702"]]
		either number <= 26 [form #"@" + number] [
			rejoin [
				#"@" + to integer! number - 1 / 26
				either zero? r: mod number 26 ["Z"] [#"@" + r]
			]
		]
	]

	to-column-number: function [
		"Convert alpha column reference to a numeric column reference"
		alpha [word! string! char!]
	] [
		any [find [1 2] length? alpha: uppercase form alpha cause-error 'user 'message ["One or two letters expected"]]
		any [find letter last alpha cause-error 'user 'message ["Valid characters are A-Z"]]
		minor: subtract to integer! last alpha: uppercase form alpha 64
		either 1 = length? alpha [minor] [
			any [find letter alpha/1 cause-error 'user 'message ["Valid characters are A-Z"]]
			(26 * subtract to integer! alpha/1 64) + minor
		]
	]

	to-string-date: function [
		"Convert a string or Rebol date to a YYYY-MM-DD string"
		date [string! date!]
		/mdy "Month/Day/Year format"
		/ydm "Year/Day/Month format"
		/day "Day precedes date"
	] compose/deep [
		if string? date [
			string: date
			any [
				attempt [
					either all [digits? date 6 > length? date] [ ; Excel
						date: 30-Dec-1899 + to integer! date
						all [
							mdy
							day: date/3
							date/3: date/2
							date/2: day
						]
						date
					] [
						all [day date: copy date remove/part date next find date " "]
						date: either digits? date [
							;	YYYYDDMM
							reduce [copy/part date 4 copy/part skip date 4 2 copy/part skip date 6 2]
						] [
							(either settings/build = 'red [[split date make bitset! "/-"]] [[parse date "/- "]])
						]
						repeat i 3 [date/:i: to integer! date/:i]
						date: to date! case [
							mdy		[reduce [date/2 date/1 date/3]]
							ydm		[reduce [date/2 date/3 date/1]]
							true	[reduce [date/1 date/2 date/3]]
						]
						all [
							date/year < 100
							date/year: date/year + either date/year <= (now/year - 1950) [2000] [1900]
						]
						date
					]
				]
				cause-error 'user 'message reduce [reform [string "is not a valid date"]]
			]
		]
		rejoin [date/year "-" next form 100 + date/month "-" next form 100 + date/day]
	]

	to-string-time: function [
		"Convert a string or Rebol time to a HH:MM:SS string"
		time [string! date! time!]
		/precise
	] compose/deep [
		unless time? time [
			string: time
			any [
				attempt [
					time: case [
						date? time				[time/time]
						;	don't match "00:00:00.000"
						find/part time "." 2	[24:00:00 * to decimal! time]	; Excel
						digits? time			[to time! rejoin [copy/part time 2 ":" copy/part skip time 2 2 ":" copy/part skip time 4 2]]
						true [
							(either settings/build = 'red [[
								either find time "PM" [
									time: to time! time
									all [time/1 < 13 time/1: time/1 + 12]
									time
								] [to time! time]
							]] [[
								to time! trim/all copy time
							]])
						]
					]
				]
				cause-error 'user 'message reduce [reform [string "is not a valid time"]]
			]
		]
		rejoin [
			next form 100 + time/hour ":"
			next form 100 + time/minute ":"
			next form 100 + to integer! time/second
			either precise [copy/part find form time/second + .0001 "." 4] [""]
		]
	]

	unarchive: function [
		"Decompresses archive (only works with compression methods 'store and 'deflate)"
		source [file! url! binary!]
		/only file [file!]
		/info "File name/sizes only (size only for gzip)"
		/local method size crc
	] [ ; https://en.wikipedia.org/wiki/Zip_(file_format) & http://www.rebol.org/view-script.r?script=rebzip.r
		all [settings/console settings/called/file 'unarchive any [file source]]

		any [binary? source source: read-binary source]

		source: case [
			#{1F8B08} = copy/part source 3 [
				either info [
					to integer! reverse skip tail copy source -4
				] [
					decompress source 'gzip
				]
			]
			#{504B0304} <> copy/part source 4 [
				settings/error reform [source "is not a ZIP file"]
			]
			true [
				either only [
					tmp: codecs/zip/decode/only source to block! file
					tmp/2/2 ; retuns only decompressed data
				][	codecs/zip/decode source ]
			]
		]

		all [settings/console settings/exited]

		source
	]

	write-dsv: function [
		"Write block(s) of values to a delimited text file"
		file [file! url!] "csv or tab-delimited text file"
		data [block!]
	] [
		all [settings/console settings/called 'write-dsv]
		b: make block! length? data
		foreach row data compose/deep [
			s: copy ""
			foreach value row [
				append s (
					either %.csv = suffix? file [
						[rejoin [either any [find val: trim/with form value {"} "," find val lf] [rejoin [{"} val {"}]] [val] ","]]
					] [
						[rejoin [value "^-"]]
					]
				)
			]
			take/last s
			any [empty? s append b s]
		]
		also write/lines file b all [settings/console settings/exited]
	]

	write-excel: function [
		"Write block(s) of values to an Excel file"
		file [file! url!]
		data [block!] "Name [string!] Data [block!] Widths [block!] records"
		/filter "Add auto filter"
	] [ ; http://officeopenxml.com/anatomyofOOXML-xlsx.php
		any [%.xlsx = suffix? file cause-error 'user 'message ["not a valid .xlsx file extension"]]

		xml-content-types:	copy ""
		xml-workbook:		copy ""
		xml-workbook-rels:	copy ""
		xml-version:		{<?xml version="1.0" encoding="UTF-8" standalone="yes"?>}

		sheet-number:		1

		xml-archive:		copy []

		foreach [sheet-name block spec] data [
			unless empty? block [
				width: length? spec

				append xml-content-types rejoin [{<Override PartName="/xl/worksheets/sheet} sheet-number {.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>}]
				append xml-workbook rejoin [{<sheet name="} sheet-name {" sheetId="} sheet-number {" r:id="rId} sheet-number {"/>}]
				append xml-workbook-rels rejoin [{<Relationship Id="rId} sheet-number {" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet} sheet-number {.xml"/>}]

				;	%xl/worksheets/sheet<n>.xml

				blk: rejoin [
					xml-version
					{<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
						<cols>}
				]
				repeat i width [
					append blk rejoin [{<col min="} i {" max="} i {" width="} spec/:i {"/>}]
				]
				append blk "</cols><sheetData>"
				foreach row block [
					append blk "<row>"
					foreach value row [
						append blk case [
							number? value [
								rejoin ["<c><v>" value "</v></c>"]
							]
							"=" = copy/part value: form value 1 [
								rejoin ["<c><f>" next value "</f></c>"]
							]
							true [
								foreach [char code] [
									"&"		"&amp;"
									"<"		"&lt;"
									">"		"&gt;"
									{"}		"&quot;"
									{'}		"&apos;"
									"^/"	"&#10;"
								] [replace/all value char code]
								rejoin [{<c t="inlineStr"><is><t>} value "</t></is></c>"]
							]
						]
					]
					append blk "</row>"
				]
				append blk {</sheetData>}
				all [filter append blk rejoin [{<autoFilter ref="A1:} to-column-alpha width length? block {"/>}]]
				append blk {</worksheet>}
				append xml-archive reduce [rejoin [%xl/worksheets/sheet sheet-number %.xml] blk]

				sheet-number: sheet-number + 1
			]
		]

		insert xml-archive reduce [
			%"[Content_Types].xml" rejoin [
				xml-version
				{<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
					<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
					<Default Extension="xml" ContentType="application/xml"/>
					<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>}
					xml-content-types
				{</Types>}
			]
			%_rels/.rels rejoin [
				xml-version
				{<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
					<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
				</Relationships>}
			]
			%xl/workbook.xml rejoin [
				xml-version
				{<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" mc:Ignorable="x15" xmlns:x15="http://schemas.microsoft.com/office/spreadsheetml/2010/11/main">
					<workbookPr defaultThemeVersion="153222"/>
					<sheets>}
						xml-workbook
					{</sheets>
				</workbook>}
			]
			%xl/_rels/workbook.xml.rels rejoin [
				xml-version
				{<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">}
					xml-workbook-rels
				{</Relationships>}
			]
		]

		write file archive xml-archive

		file
	]
]