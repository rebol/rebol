REBOL [
	Name:    csv
	Type:    module
	Options: [delay]
	Exports: [to-csv load-csv]
	Version: 1.2.0
	Title:   "Codec: CSV"
	Purpose: "Loads and formats CSV data, for enterprise or mezzanine use."
	Author: ["Brian Hawley" "Oldes"]
	File:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-csv.reb
	Date:    25-May-2022
	History: [
		1.0.0  5-Dec-2011 @BrianH "Initial public release"
		1.1.0  6-Dec-2011 @BrianH "Added LOAD-CSV /part option"
		1.1.1 13-Dec-2011 @BrianH "Added money! special case to TO-CSV"
		1.1.2 18-Dec-2011 @BrianH "Fixed TO-ISO-DATE for R2 with datetimes"
		1.1.3 19-Dec-2011 @BrianH "Speed up TO-ISO-DATE using method from Graham Chiu"
		1.1.4 20-Dec-2011 @BrianH "Added /with option to TO-CSV"
		1.1.5 20-Dec-2011 @BrianH "Fixed a bug in the R2 TO-CSV with the number 34"
		1.2.0 25-May-2022 @Oldes  "Removed Rebol2 compatibility part and converted to Rebol3 codec"
	]
	License: 'mit
	References: http://www.rebol.org/view-script.r?script=csv-tools.r
]

;; The standards implemented here are http://tools.ietf.org/html/rfc4180 for CSV
;; and http://en.wikipedia.org/wiki/ISO_8601 for date formatting, falling back to
;; Excel compatibility where the standards are ambiguous or underspecified, such
;; as for handling of malformed data. All standard platform newlines are handled
;; even if they are all used in the same file; the complexity of doing this is
;; why the newline delimiter is not an option at this time. Binary CSV works.
;; Passing a block of sources to LOAD-CSV loads them all into the same output
;; block, in the order specified.
;; 
;; There was no point in indluding a SAVE-CSV since it's pretty much a one-liner.
;; Just use WRITE/lines MAP-EACH x data [TO-CSV :x].
;; 
;; Warning: LOAD-CSV reads the entire source data into memory before parsing it.
;; You can use LOAD-CSV/part and then LOAD-CSV/into to do the parsing in parts.
;; An incremental reader is possible, but might be better done as a csv:// scheme.

to-iso-date: func [
	"Convert a date to ISO format (Excel-compatible subset)"
	date [date!] /utc "Convert zoned time to UTC time"
] [
	if utc [date: date/utc] ; Excel doesn't support the Z suffix
	either date/time [ajoin [
		next form 10000 + date/year   #"-"
		next form 100   + date/month  #"-"
		next form 100   + date/day    #" "  ; ... or T
		next form 100   + date/hour   #":"
		next form 100   + date/minute #":"
		next form 100   + date/second       ; ... or offsets
	]] [ajoin [
		next form 10000 + date/year   #"-"
		next form 100   + date/month  #"-"
		next form 100   + date/day
	]]
]

to-csv: function/with [
	"Convert block of value blocks to CSV or a block of values to a CSV-formatted line in a string."
	data [block!] "Block of values"
	/with "Specify field delimiter (preferably char, or length of 1)"
	delimiter [char! string! binary!] {Default #","}
	; Empty delimiter, " or CR or LF may lead to corrupt data
][
	delimiter: either with [to string! delimiter] [#","]
	either block? first data [
		output: make string! 1000
		forall data [
			append append output to-csv/with data/1 delimiter LF
		]
		output
	][
		output: make block! 2 * length? data
		unless empty? data [append output format-field first+ data]
		foreach x data [append append output delimiter format-field :x]
		to string! output
	]
][
	format-field: func [x [any-type!] /local qr] [
		; Parse rule to put double-quotes around a string, escaping any inside
		qr: [return [insert #"^"" any [change #"^"" {""} | skip] insert #"^""]]
		case [
			none?   :x [""]
			any-string? :x [parse copy x qr]
			:x =? #"^"" [{""""}]  ; =? is the most efficient equality in R3
			char?   :x [ajoin [#"^"" x #"^""]]
			money?  :x [find/tail form x #"$"]
			date?   :x [to-iso-date x]
			scalar? :x [form x]
			any [any-word? :x binary? :x any-path? :x] [parse to string! :x qr]
			'else [cause-error 'script 'expect-val reduce [
				[any-string! any-word! any-path! binary! scalar! date!] type? :x
			]]
		]
	]
]

load-csv: function [
	"Load and parse CSV-style delimited data. Returns a block of blocks."
	source [file! url! string! binary! block!] "File or url will be read"
	/binary "Don't convert the data to string (if it isn't already)"
	/with "Specify field delimiter (preferably char, or length of 1)"
	delimiter [char! string! binary!] {Default #","}
	/into "Insert into a given block, rather than make a new one"
	output [block!] "Block returned at position after the insert"
	/part "Get only part of the data, and set to the position afterwards"
	count [integer!] "Number of lines to return"
	after [any-word! any-path! none!] "Set to source after decoded"
][
	if block? source [ ; Many sources, load them all into the same block
		unless into [output: make block! length? source]
		unless with [delimiter: #","]
		foreach x source [
			assert/type [x [file! url! string! binary!]]
			output: load-csv/:binary/:with/:into x delimiter output
		]
		return either into [output] [head output]
	]
	; Read the source if necessary
	if any [file? source url? source] [
		source: either binary [read source] [read/string source]
		assert/type [source [string! binary!]] ; It could be something else
		; /string or not may not affect urls, but it's not this function's fault
	]
	; Use to-string if string conversion needed, pass-through function otherwise
	emit: func[x] either any [string? source binary][
		[append line :x]
	][	[append line to string! :x]]

	; Prep output and local vars
	unless into [output: make block! 8]
	line: [] val: make source 0
	; Parse rules
	if all [not char? delimiter: any [delimiter #","] empty? delimiter] [
		cause-error 'script 'invalid-arg delimiter
	]

	dq: #"^""
	valchars: either binary? source [
		[to [delimiter | #{0D0A} | cr | lf | end]]
	][	[to [delimiter |   crlf  | cr | lf | end]]]

	value: [
		; Value in quotes, with Excel-compatible handling of bad syntax
		dq (clear val) x: to [dq | end] y: (append/part val x y)
		any [dq x: dq to [dq | end] y: (append/part val x y)]
		[dq x: valchars y: (append/part val x y) | end]
		(emit copy val) |
		; Raw value
		copy x valchars (emit x)
	]
	if part [part: [if (positive? -- count)]] ; Test must succeed to continue
	parse/case source [
		opt #{EFBBBF} ;= UTF-8 BOM header
		any [
		not end part (line: make block! length? line)
		value any [delimiter value] [crlf | cr | lf | end]
		(output: insert/only output line) source:
	]]
	if after [set after source]
	either into [output] [head output]
]

register-codec [
	name:  'csv
	type:  'text
	title: "Comma-separated values"
	suffixes: [%.csv]

	encode: func [data [any-type!]] [
		to-csv/with data #","
	]

	decode: function [
		data [binary! string!] "Comma-separated data to parse"
	][
		load-csv data
	]
]
register-codec [
	name:  'tsv
	type:  'text
	title: "Tab-separated values"
	suffixes: [%.tsv]

	encode: func [data [any-type!]] [
		to-csv/with data TAB
	]

	decode: function [
		data [binary! string!] "Tab-separated data to parse"
	][
		load-csv/with data TAB
	]
]
