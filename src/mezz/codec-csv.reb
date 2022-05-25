;; ===================================================
;; Script: csv-tools.r
;; downloaded from: www.REBOL.org
;; on: 25-May-2022
;; at: 18:19:33.815213 UTC
;; owner: brianh [script library member who can update
;; this script]
;; ===================================================
REBOL [
	Title: "CSV Handling Tools"
	Author: "Brian Hawley"
	File: %csv-tools.r
	Date: 20-Dec-2011
	Version: 1.1.5
	Purpose: "Loads and formats CSV data, for enterprise or mezzanine use."
	Library: [
		level: 'intermediate
		platform: 'all
		type: [tool idiom]
		domain: [database text file]
		tested-under: [2.7.8.3.1 2.100.111.3.1]
		license: 'mit
	]
	History: [
		1.0.0 5-Dec-2011 "Initial public release"
		1.1.0 6-Dec-2011 "Added LOAD-CSV /part option"
		1.1.1 13-Dec-2011 "Added money! special case to TO-CSV"
		1.1.2 18-Dec-2011 "Fixed TO-ISO-DATE for R2 with datetimes"
		1.1.3 19-Dec-2011 "Sped up TO-ISO-DATE using method from Graham Chiu"
		1.1.4 20-Dec-2011 "Added /with option to TO-CSV"
		1.1.5 20-Dec-2011 "Fixed a bug in the R2 TO-CSV with the number 34"
	]
]

comment {
This script includes versions of these functions for both R2 and R3. The R2
versions require either 2.7.7+ or many functions from R2/Forward. The R3
functions work with any version since the PARSE revamp.

The behavior of the functions is very similar to that of the mezzanines of
recent releases of REBOL, with similar treatment of function options and
error handling, and demonstrates some more modern REBOL techniques. It may be
useful to compare the R2 and R3 versions of the functions, to see how the
changes between the two platforms affects how you would optimize code. The
LOAD-CSV functions both take into account the limitations of their respective
PARSE dialects when it comes to handling string and binary code, and PARSE
control flow behavior.

The standards implemented here are http://tools.ietf.org/html/rfc4180 for CSV
and http://en.wikipedia.org/wiki/ISO_8601 for date formatting, falling back to
Excel compatibility where the standards are ambiguous or underspecified, such
as for handling of malformed data. All standard platform newlines are handled
even if they are all used in the same file; the complexity of doing this is
why the newline delimiter is not an option at this time. Binary CSV works.
Passing a block of sources to LOAD-CSV loads them all into the same output
block, in the order specified.

There was no point in indluding a SAVE-CSV since it's pretty much a one-liner.
Just use WRITE/lines MAP-EACH x data [TO-CSV :x].

Warning: LOAD-CSV reads the entire source data into memory before parsing it.
You can use LOAD-CSV/part and then LOAD-CSV/into to do the parsing in parts.
An incremental reader is possible, but might be better done as a csv:// scheme.
}

either system/version > 2.100.0 [ ; R3

to-iso-date: func [
	"Convert a date to ISO format (Excel-compatible subset)"
	date [date!] /utc "Convert zoned time to UTC time"
] [
	if utc [date: date/utc] ; Excel doesn't support the Z suffix
	either date/time [ajoin [
		next form 10000 + date/year "-"
		next form 100 + date/month "-"
		next form 100 + date/day " "  ; ... or T
		next form 100 + date/hour ":"
		next form 100 + date/minute ":"
		next form 100 + date/second  ; ... or offsets
	]] [ajoin [
		next form 10000 + date/year "-"
		next form 100 + date/month "-"
		next form 100 + date/day
	]]
]

to-csv: funct/with [
	"Convert a block of values to a CSV-formatted line in a string."
	data [block!] "Block of values"
	/with "Specify field delimiter (preferably char, or length of 1)"
	delimiter [char! string! binary!] {Default ","}
	; Empty delimiter, " or CR or LF may lead to corrupt data
] [
	output: make block! 2 * length? data
	delimiter: either with [to-string delimiter] [","]
	unless empty? data [append output format-field first+ data]
	foreach x data [append append output delimiter format-field :x]
	to-string output
] [
	format-field: func [x [any-type!] /local qr] [
		; Parse rule to put double-quotes around a string, escaping any inside
		qr: [return [insert {"} any [change {"} {""} | skip] insert {"}]]
		case [
			none? :x [""]
			any-string? :x [parse copy x qr]
			:x =? #"^(22)" [{""""}]  ; =? is the most efficient equality in R3
			char? :x [ajoin [{"} x {"}]]
			money? :x [find/tail form x "$"]
			scalar? :x [form x]
			date? :x [to-iso-date x]
			any [any-word? :x binary? :x any-path? :x] [parse to-string :x qr]
			'else [cause-error 'script 'expect-set reduce [
				[any-string! any-word! any-path! binary! scalar! date!] type? :x
			]]
		]
	]
]

load-csv: funct [
	"Load and parse CSV-style delimited data. Returns a block of blocks."
	source [file! url! string! binary! block!] "File or url will be read"
	/binary "Don't convert the data to string (if it isn't already)"
	/with "Specify field delimiter (preferably char, or length of 1)"
	delimiter [char! string! binary!] {Default ","}
	/into "Insert into a given block, rather than make a new one"
	output [block!] "Block returned at position after the insert"
	/part "Get only part of the data, and set to the position afterwards"
	count [integer!] "Number of lines to return"
	after [any-word! any-path! none!] "Set to source after decoded"
] [
	if block? source [ ; Many sources, load them all into the same block
		unless into [output: make block! length? source]
		unless with [delimiter: #","]
		foreach x source [
			assert/type [x [file! url! string! binary!]]
			output: apply :load-csv [x binary true delimiter true output]
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
	emit: either any [string? source binary] [func [x] [:x]] [:to-string]
	; Prep output and local vars
	unless into [output: make block! 1]
	line: [] val: make source 0
	; Parse rules
	if all [not char? delimiter: any [delimiter ","] empty? delimiter] [
		cause-error 'script 'invalid-arg delimiter
	]
	either binary? source [ ; You need binary constants when binary parsing
		unless binary? delimiter [delimiter: to-binary delimiter]
		dq: #{22} valchars: [to [delimiter | #{0D0A} | #{0D} | #{0A} | end]]
	][ ; You need string or char constants when string parsing
		if binary? delimiter [delimiter: to-string delimiter]
		dq: {"} valchars: [to [delimiter | crlf | cr | lf | end]]
	]
	value: [
		; Value in quotes, with Excel-compatible handling of bad syntax
		dq (clear val) x: to [dq | end] y: (insert/part tail val x y)
		any [dq x: dq to [dq | end] y: (insert/part tail val x y)]
		[dq x: valchars y: (insert/part tail val x y) | end]
		(insert tail line emit copy val) |
		; Raw value
		copy x valchars (insert tail line emit x)
	]
	if part [part: [if (positive? -- count)]] ; Test must succeed to continue
	parse source [any [
		not end part (line: make block! length? line)
		value any [delimiter value] [crlf | cr | lf | end]
		(output: insert/only output line) source:
	]]
	if after [set after source]
	either into [output] [head output]
]

] [ ; else R2

to-iso-date: func [
	"Convert a date to ISO format (Excel-compatible subset)"
	date [date!] /utc "Convert zoned time to UTC time"
] [
	if utc [date: date + date/zone date/zone: none] ; Excel doesn't support the Z suffix
	either date/time [ajoin [
		next form 10000 + date/year "-"
		next form 100 + date/month "-"
		next form 100 + date/day " "  ; ... or T
		next form 100 + date/time/hour ":"
		next form 100 + date/time/minute ":"
		next form 100 + date/time/second  ; ... or offsets
	]] [ajoin [
		next form 10000 + date/year "-"
		next form 100 + date/month "-"
		next form 100 + date/day
	]]
]

to-csv: funct/with [
	"Convert a block of values to a CSV-formatted line in a string."
	[catch]
	data [block!] "Block of values"
	/with "Specify field delimiter (preferably char, or length of 1)"
	delimiter [char! string! binary!] {Default ","}
	; Empty delimiter, " or CR or LF may lead to corrupt data
] [
	output: make block! 2 * length? data
	delimiter: either with [to-string delimiter] [","]
	unless empty? data [insert tail output format-field first data data: next data]
	foreach x data [insert insert tail output delimiter format-field get/any 'x]
	to-string output
] [
	format-field: func [x [any-type!]] [case [
		any [not value? 'x error? get/any 'x] [throw-error 'script 'expect-set [
			[any-string! any-word! any-path! binary! scalar! date!] type? get/any 'x
		]]
		none? :x [""]
		any-string? :x [ajoin [{"} replace/all copy x {"} {""} {"}]]
		:x == #"^(22)" [{""""}]  ; Weirdly, = and =? return true when x is 34
		char? :x [ajoin [{"} x {"}]]
		money? :x [find/tail form x "$"]
		scalar? :x [form x]
		date? :x [to-iso-date x]
		any [any-word? :x binary? :x any-path? :x] [
			ajoin [{"} replace/all to-string :x {"} {""} {"}]
		]
		'else [throw-error 'script 'expect-set reduce [
			[any-string! any-word! any-path! binary! scalar! date!] type? :x
		]]
	]]
]

load-csv: funct [
	"Load and parse CSV-style delimited data. Returns a block of blocks."
	[catch]
	source [file! url! string! binary! block!] "File or url will be read"
	/binary "Don't convert the data to string (if it isn't already)"
	/with "Specify field delimiter (preferably char, or length of 1)"
	delimiter [char! string! binary!] {Default #","}
	/into "Insert into a given block, rather than make a new one"
	output [block! list!] "Block returned at position after the insert"
	/part "Get only part of the data, and set to the position afterwards"
	count [integer!] "Number of lines to return"
	after [any-word! none!] "Set to data at position after decoded part"
] [
	if block? source [ ; Many sources, load them all into the same output block
		unless into [output: make block! length? source]
		unless with [delimiter: ","]
		x: [file! url! string! binary!]
		foreach y source [
			unless find x type?/word y [
				cause-error 'script 'expect-set reduce [x type? :y]
			]
			either binary [
				output: load-csv/binary/with/into y delimiter output
			] [
				output: load-csv/with/into y delimiter output
			]
		]
		return either into [output] [head output]
	]
	; Read the source if necessary
	if any [file? source url? source] [throw-on-error [
		source: either binary [read/binary source] [read source]
	]]
	unless binary [source: as-string source] ; No line conversion
	; Use either a string or binary value emitter
	emit: either binary? source [:as-binary] [:as-string]
	; Prep output and local vars
	unless into [output: make block! 1]
	line: [] val: make string! 0
	; Parse rules
	valchars: remove/part charset [#"^(00)" - #"^(FF)"] crlf
	case [
		any [char? delimiter: any [delimiter ","] last? delimiter] [ ; One char
			valchars: compose [any (remove/part valchars delimiter)]
		]
		empty? delimiter [throw-error 'script 'invalid-arg delimiter]
		'else [ ; Multi-character delimiter needs special handling
			remove/part valchars copy/part as-string delimiter 1
			valchars: compose/deep [any [
				some (valchars) | y: delimiter :y break | (first as-string delimiter)
			]]
		]
	]
	value: [
		; Value in quotes, with Excel-compatible handling of bad syntax
		{"} (clear val) x: [to {"} | to end] y: (insert/part tail val x y)
		any [{"} x: {"} [to {"} | to end] y: (insert/part tail val x y)]
		[{"} x: valchars y: (insert/part tail val x y) | end]
		(insert tail line emit copy val) |
		; Raw value
		x: valchars y: (insert tail line emit copy/part x y)
	]
	part: pick [ ; Rule must fail and go to the alternate in order to continue
		[end skip]  ; Will always fail, so the break won't be reached
		[(cont: if positive? count [count: count - 1 [end skip]]) cont]
		; While count is positive, cont is set to [end skip], which will fail
		; and go the alternate. Otherwise, cont is set to none, which will
		; succeed, and then the subsequent break will stop the parsing.
		; Parsing control flow can get a little convoluted at times in R2.
	] not part
	; as-string because R2 doesn't parse binary that well
	parse/all as-string source [z: any [
		end break | part break |
		(line: make block! length? line)
		value any [delimiter value] [crlf | cr | lf | end]
		(output: insert/only output line)
	] z:]
	if after [set after either binary? source [as-binary z] [z]]
	also either into [output] [head output]
		(source: output: line: val: x: y: none) ; Free the locals
]

]
