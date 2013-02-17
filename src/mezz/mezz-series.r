REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Series Helpers"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

empty?: make :tail? [
	[
		{Returns TRUE if empty or NONE, or for series if index is at or beyond its tail.}
		series [series! object! gob! port! bitset! map! none!]
	]
]

offset?: func [
	"Returns the offset between two series positions."
	series1 [series!]
	series2 [series!]
][
	subtract index? series2 index? series1
]

found?: func [
	"Returns TRUE if value is not NONE."
	value
][
	not none? :value
]

last?: single?: func [
	"Returns TRUE if the series length is 1."
	series [series! port! map! tuple! bitset! object! gob! any-word!]
][
	1 = length? series
]

extend: func [
	"Extend an object, map, or block type with word and value pair."
	obj [object! map! block! paren!]
	word [any-word!]
	val
][
	if :val [append obj reduce [to-set-word word :val]]
	:val
]

rejoin: func [
	"Reduces and joins a block of values."
	block [block!] "Values to reduce and join"
	;/with "separator"
][
	if empty? block: reduce block [return block]
	append either series? first block [copy first block][
		form first block
	] next block
]

remold: func [
	{Reduces and converts a value to a REBOL-readable string.}
	value {The value to reduce and mold}
	/only {For a block value, mold only its contents, no outer []}
	/all  {Mold in serialized format}
	/flat {No indentation}
][
	apply :mold [reduce :value only all flat]
]

charset: func [
	"Makes a bitset of chars for the parse function."
	chars [string! block! binary! char! integer!]
	/length "Preallocate this many bits"
	len [integer!] "Must be > 0"
][
	either length [append make bitset! len chars] [make bitset! chars]
]

array: func [
	"Makes and initializes a series of a given size."
	size [integer! block!] "Size or block of sizes for each dimension"
	/initial "Specify an initial value for all elements"
	value "Initial value (will be called each time if a function)"
	/local block rest
][
	if block? size [
		if tail? rest: next size [rest: none]
		unless integer? set/any 'size first size [
			cause-error 'script 'expect-arg reduce ['array 'size type? :size]
		]
	]
	block: make block! size
	case [
		block? rest [
			loop size [block: insert/only block array/initial rest :value]
		]
		series? :value [
			loop size [block: insert/only block copy/deep value]
		]
		any-function? :value [ ; So value can be a thunk :)
			loop size [block: insert/only block value] ; Called every time
		]
		insert/dup block value size
	]
	head block
]

replace: func [
	"Replaces a search value with the replace value within the target series."
	target  [series!] "Series to replace within (modified)"
	search  "Value to be replaced (converted if necessary)"
	replace "Value to replace with (called each time if a function)"
	/all "Replace all occurrences"  ;!!! Note ALL is redefined in here!
	/case "Case-sensitive replacement"  ;!!! Note CASE is redefined in here!
	/tail "Return target after the last replacement position"  ;!!! Note TAIL is redefined in here!
	/local save-target len value pos do-break
][
	save-target: target
	; If target is a string but search is not, make search a string (except for bitset).
	; If target is a binary but search is not, make search a binary (except for bitset).
	; If target is a bitset, or a block and search is not a block, len = 1
	len: lib/case [
		bitset? :search  1
		any-string? target [
			if any [not any-string? :search tag? :search] [search: form :search]
			length? :search
		]
		binary? target [
			unless binary? :search [search: to-binary :search] ; Must be convertable
			length? :search
		]
		any-block? :search [length? :search]
		true  1
	]
	; /all and /case checked before the while, /tail after
	do-break: unless all [:break] ; Will be none if not /all, a noop
	while pick [
		[pos: find target :search]
		[pos: find/case target :search]
	] not case [
		(value: replace pos) ; The replace argument can be a function
		target: change/part pos :value len
		do-break
	]
	either tail [target] [save-target]
]
; We need to consider adding an /any refinement to use find/any, once that works.

reword: func [
	"Substitutes values into a template string, returning a new string."
	source [any-string!] "Template series (or string with escape sequences)"
	values [map! object! block!] "Pairs of values and replacements (will be called if functions)"
	/escape "Choose your own escape char (no escape for block templates)"
	char [char! any-string!] "Use this escape char (default $)"
	/into "Insert into a buffer instead (returns position after insert)"
	output [any-string!] "The buffer series (modified)"
	/local vals word a b c d
][
	output: any [output make source length? source]
	; Convert to strings and eliminate duplicates and empties (case insensitive)
	vals: make map! length? values
	foreach [w v] values [
		; Last duplicate word wins; empty words, unset or none vals removed
		unless string? :w [w: to string! :w]
		unless empty? w [poke vals w unless unset? :v [:v]]
	]
	; Construct the reword rule
	word: make block! 2 * length? vals
	foreach w vals [word: reduce/into [w '|] word]
	word: head remove back word
	escape: [c: word d: (
		; Copy the skipped part, then the result of evaluating the value
		output: insert insert/part output a b vals/(copy/part c d) :b
		; Value selected with path so functions evaluate with start pos arg
	) a:]
	; Determine the escape string, if any
	char: to string! any [char "$"]
	either empty? char [
		; No escape string, check everything (until PARSE is fixed)
		parse/all source [
			a: any [b: [escape | skip]]
			to end (output: insert output a)
		]  ; !! We need PARSE TO bitset! badly
	] [
		; Escape string defined, use the optimized method
		parse/all source [
			a: any [to char b: char [escape | none]]
			to end (output: insert output a)
		]
	]
	; Return end of output with /into, head otherwise
	either into [output] [head output]
]

move: func [
	"Move a value or span of values in a series."
	source [series!] "Source series"
	offset [integer!] "Offset to move by, or index to move to"
	/part "Move part of a series"
	length [integer!] "The length of the part to move"
	/skip "Treat the series as records of fixed size" ;; SKIP redefined
	size [integer!] "Size of each record"
	/to "Move to an index relative to the head of the series" ;; TO redefined
][
	unless length [length: 1]
	if skip [
		if 1 > size [cause-error 'script 'out-of-range size]
		offset: either to [offset - 1 * size + 1] [offset * size]
		length: length * size
	]
	part: take/part source length
	insert either to [at head source offset] [
		lib/skip source offset
	] part
]

extract: func [
	"Extracts a value from a series at regular intervals."
	series [series!]
	width [integer!] "Size of each entry (the skip)"
	/index "Extract from an offset position"
	pos "The position(s)" [number! logic! block!]
	/default "Use a default value instead of none"
	value "The value to use (will be called each time if a function)"
	/into "Insert into a buffer instead (returns position after insert)"
	output [series!] "The buffer series (modified)"
	/local len val
][  ; Default value is "" for any-string! output
	if zero? width [return any [output make series 0]]  ; To avoid an infinite loop
	len: either positive? width [  ; Length to preallocate
		divide length? series width  ; Forward loop, use length
	][
		divide index? series negate width  ; Backward loop, use position
	]
	unless index [pos: 1]
	either block? pos [
		unless parse pos [some [number! | logic!]] [cause-error 'Script 'invalid-arg reduce [pos]]
		unless output [output: make series len * length? pos]
		if all [not default any-string? output] [value: copy ""]
		forskip series width [forall pos [
			if none? set/any 'val pick series pos/1 [set/any 'val value]
			output: insert/only output :val
		]]
	][
		unless output [output: make series len]
		if all [not default any-string? output] [value: copy ""]
		forskip series width [
			if none? set/any 'val pick series pos [set/any 'val value]
			output: insert/only output :val
		]
	]
	either into [output] [head output]
]

alter: func [
	"Append value if not found, else remove it; returns true if added."
	series [series! port! bitset!] {(modified)}
	value
	/case "Case-sensitive comparison"
][
	if bitset? series [
		return either find series :value [
			remove/part series :value false
		][
			append series :value true
		]
	]
	found? unless remove (
		either case [find/case series :value] [find series :value]
	) [append series :value]
]

collect: func [
	"Evaluates a block, storing values via KEEP function, and returns block of collected values."
	body [block!] "Block to evaluate"
	/into "Insert into a buffer instead (returns position after insert)"
	output [series!] "The buffer series (modified)"
][
	unless output [output: make block! 16]
	do func [keep] body func [value [any-type!] /only] [
		output: apply :insert [output :value none none only]
		:value
	]
	either into [output] [head output]
]

format: funct [
	"Format a string according to the format dialect."
	rules {A block in the format dialect. E.g. [10 -10 #"-" 4]}
	values
	/pad p
][
	p: any [p #" "]
	unless block? :rules [rules: reduce [:rules]]
	unless block? :values [values: reduce [:values]]

	; Compute size of output (for better mem usage):
	val: 0
	foreach rule rules [
		if word? :rule [rule: get rule]
		val: val + switch/default type?/word :rule [
			integer! [abs rule]
			string! [length? rule]
			char!    [1]
		][0]
	]

	out: make string! val
	insert/dup out p val

	; Process each rule:
	foreach rule rules [
		if word? :rule [rule: get rule]
		switch type?/word :rule [
			integer! [
				pad: rule
				val: form first+ values
				clear at val 1 + abs rule
				if negative? rule [
					pad: rule + length? val
					if negative? pad [out: skip out negate pad]
					pad: length? val
				]
				change out :val
				out: skip out pad ; spacing (remainder)
			]
			string!  [out: change out rule]
			char!    [out: change out rule]
		]
	]

	; Provided enough rules? If not, append rest:
	if not tail? values [append out values]
	head out
]

printf: func [
	"Formatted print."
	fmt "Format"
	val "Value or block of values"
][
	print format :fmt :val
]

split: func [	; Gabriele, Gregg
	"Split a series into pieces; fixed or variable size, fixed number, or at delimiters"
	series	[series!] "The series to split"
	dlm		[block! integer! char! bitset! any-string!] "Split size, delimiter(s), or rule(s)."
	/into	"If dlm is an integer, split into n pieces, rather than pieces of length n."
	/local size count mk1 mk2
][
	either all [block? dlm  parse dlm [some integer!]] [
		map-each len dlm [
			either positive? len [
				copy/part series series: skip series len
			] [
				series: skip series negate len
				; return unset so that nothing is added to output
				()
			]
		]
	][
		size: dlm	; alias for readability
		collect [
			parse/all series case [
				all [integer? size into] [
					if size < 1 [cause-error 'Script 'invalid-arg size]
					count: size - 1
					size: round/down divide length? series size
					[
						count [copy series size skip (keep/only series)]
						copy series to end (keep/only series)
					]
				]
				integer? dlm [
					if size < 1 [cause-error 'Script 'invalid-arg size]
					[any [copy series 1 size skip (keep/only series)]]
				]
				'else [ ; = any [bitset? dlm  any-string? dlm  char? dlm]
					[any [mk1: [to dlm mk2: dlm | to end mk2:] (keep copy/part mk1 mk2)]]
				]
			]
		]
	]
]

find-all: funct [
    "Find all occurences of the value within the series (allows modification)."
    'series [word!] "Variable for block, string, or other series"
    value
    body [block!] "Evaluated for each occurence"
][
    assert [series? orig: get series]
    while [any [set series find get series :value (set series orig false)]] [
        do body
        ++ (series)
    ]
]
