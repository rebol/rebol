REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Series Helpers"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2023 Rebol Open Source Contributors
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
		series [series! object! gob! port! bitset! typeset! map! none!]
	]
]

offset?: func [
	"Returns the offset between two series positions."
	series1 [series!]
	series2 [series!]
][
	subtract index? series2 index? series1
]

;@@ https://github.com/Oldes/Rebol-issues/issues/2053
;found?: func [
;	"Returns TRUE if value is not NONE."
;	value
;][
;	not none? :value
;]

last?: single?: func [
	"Returns TRUE if the series length is 1."
	series [series! port! map! tuple! bitset! object! gob! any-word!]
][
	1 = length? series
]

extend: func [
	"Extend an object, map, or block type with word and value pair."
	obj [object! map! block! paren!] {object to extend (modified)}
	word [any-word!]
	val
][
	if :val [put obj to-set-word word :val]
	:val
]

rejoin: func [
	"Reduces and joins a block of values."
	block [block!] "Values to reduce and join"
	;/with "separator"
][
	if empty? block: reduce block [return block]
	append either series? block/1 [copy block/1][form block/1] next block
]

remold: func [
	{Reduces and converts a value to a REBOL-readable string.}
	value {The value to reduce and mold}
	/only {For a block value, mold only its contents, no outer []}
	/all  {Mold in serialized format}
	/flat {No indentation}
	/part {Limit the length of the result}
	limit [integer!]
][
	mold/:only/:all/:flat/:part reduce :value limit
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
	/local block rest word
	/with tag indexes  ; Token to ensure internal use, and block of index expressions
][
	unless same? :tag 'tag [with: tag: indexes: none]  ; Enforced internal option
	if block? size [
		if all [not with any-function? :value] [  ; Make indexes to pass to function
			indexes: append/dup make block! 2 * length? size [index? block] length? size
		]
		if tail? rest: next size [rest: none]
		unless integer? set/any 'size first size [
			cause-error 'script 'expect-arg reduce ['array 'size type? :size]
		]
	]
	block: make block! size
	case [
		block? :rest [
			either any-function? :value [  ; Must construct params to pass to function
				word: in make object! copy [x: block] 'x  ; Make a persistent word for this level
				indexes: change next indexes word  ; Put that word in the params block
				loop size [  ; Pass indexes block to recursive call, at that level's position
					set word insert/only get word array/initial/with rest :value 'tag indexes
				]
				block: get word
			] [  ; Regular value, no parameter handling needed
				loop size [block: insert/only block array/initial rest :value]
			]
		]
		series? :value [
			loop size [block: insert/only block copy/deep value]
		]
		any-function? :value [
			unless indexes [indexes: [index? block]]  ; Single dimension, single index
			loop size [block: insert/only block apply :value head indexes]
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
			unless binary? :search [ ; Must be convertable
				if integer? :search [
					if any [:search > 255 :search < 0] [
						cause-error 'Script 'out-of-range :search
					]
					search: to char! :search
				]
				search: to binary! :search
			] 
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
	"Make a string or binary based on a template and substitution values."
	source [any-string! binary!] "Template series with escape sequences"
	values [map! object! block!] "Keyword literals and value expressions"
	/case "Characters are case-sensitive"  ;!!! Note CASE is redefined in here!
	/only "Use values as-is, do not reduce the block, insert block values"
	/escape "Choose your own escape char(s) or [begin end] delimiters"
	char [char! any-string! binary! block! none!] {Default "$"}
	/into "Insert into a buffer instead (returns position after insert)"
	output [any-string! binary!] "The buffer series (modified)"
	/local char-end vals word wtype cword out fout rule a b c w v
][
	assert/type [local none!]  ; Prevent locals injection
	unless into [output: make source length? source]
	; Determine the datatype to convert the keywords to internally
	; Case-sensitive map keys must be binary, tags are special-cased by parse
	wtype: lib/case [case binary! tag? source string! 'else type? source]
	; Determine the escape delimiter(s), if any
	lib/case/all [
		not escape [char: "$"]
		block? char [
			; Have to use parse here because ASSERT/type is broken
			rule: [char! | any-string! | binary!]
			unless parse c: char [set char rule set char-end opt rule] [
				cause-error 'script 'invalid-arg reduce [c]
			]
		]
		char? char [char: to wtype char]
		char? char-end [char-end: to wtype char-end]
	]
	lib/case [
		; Check whether values is a map of the kind we can use internally
		all [
			map? values      ; Must be a map to use series keys with no dups
			empty? char-end  ; If we have char-end, it gets appended to the keys
			foreach [w v] values [
				; Key types must match wtype and no unset values allowed
				if any [unset? :v wtype <> type? :w] [break/return false]
				true
			]
		] [vals: values]  ; Success, so use it
		; Otherwise, convert keywords to wtype and eliminate duplicates and empties
		; Last duplicate keyword wins; empty keywords, unset or none vals removed
		; Any trailing delimiter is added to the end of the key for convenience
		all [
			vals: make map! length? values  ; Make a new map internally
			not only block? values  ; Should we evaluate value expressions?
		] [
			while [not tail? values] [
				w: first+ values  ; Keywords are not evaluated
				set/any 'v do/next values 'values
				if any [set-word? :w lit-word? :w] [w: to word! :w]
				lib/case [
					wtype = type? :w none
					wtype <> binary! [w: to wtype :w]
					any-string? :w [w: to binary! :w]
					'else [w: to binary! to string! :w]
				]
				unless empty? w [
					unless empty? char-end [w: append copy w char-end]
					either unset? :v [ remove/key vals w ][ poke vals w :v ]
				]
			]
		]
		'else [  ; /only doesn't apply, just assign raw values
			foreach [w v] values [  ; foreach can be used on all values types
				if any [set-word? :w lit-word? :w] [w: to word! :w]
				lib/case [
					wtype = type? :w none
					wtype <> binary! [w: mold :w]
					any-string? :w [w: to binary! :w]
					'else [w: to binary! to string! :w]
				]
				unless empty? w [
					unless empty? char-end [w: append copy w char-end]
					either unset? :v [ remove/key vals w ][ poke vals w :v ]
				]
			]
		]
	]
	; Construct the reword rule
	word: make block! 2 * length? vals
	foreach w vals [word: reduce/into [w '|] word]
	word: head remove back word
	; Convert keyword if the type doesn't match
	cword: pick [(w: to wtype w)] wtype <> type? source
	set/any [out: fout:] pick [
		[   ; Convert to string if type combination needs it
			(output: insert output to string! copy/part a b)
			(output: insert output to string! a)
		][  ; ... otherwise just insert it directly
			(output: insert/part output a b)
			(output: insert output a)
		]
	] or~ tag? source and~ binary? source not binary? output
	escape: [
		copy w word cword out (
			output: insert output lib/case [
				block? v: select vals w [either only [v] :v]
				any-function? :v [apply :v [:b]]
				'else :v
			]
		) a:
	]
	rule: either empty? char [
		; No starting escape string, use TO multi
		[a: any [to word b: [escape | skip]] to end fout]
	][
		; Starting escape string defined, use regular TO
		if wtype <> type? char [char: to wtype char]
		[a: any [to char b: char [escape | none]] to end fout]
	]
	parse/:case source rule
	; Return end of output with /into, head otherwise
	either into [output] [head output]
]
; It's big, it's complex, but it works. Placeholder for a native.

move: func [
	"Move a value or span of values in a series."
	source [series! gob!] "Source series (modified)"
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

deduplicate: func [
    "Removes duplicates from the data set."
    set [block! string! binary!] "The data set (modified)"
    /case "Use case-sensitive comparison"
    /skip "Treat the series as records of fixed size"
    size [integer!]
] [
    head insert set also unique/:case/:skip :set :size clear set
]

alter: func [
	"Append value if not found, else remove it; returns true if added."
	series [series! port! bitset!] {(modified)}
	value
	/case "Case-sensitive comparison"
][
	if bitset? series [
		return either find series :value [
			remove/key series :value false
		][
			append series :value true
		]
	]
	to logic! unless remove (find/:case series :value) [append series :value]
]

supplement: func [
	"Append value if not found; returns series at same position."
	series [block!] {(modified)}
	value
	/case "Case-sensitive comparison"
	/local result
][
	result: series ; to return series at same position if value is found
	any[
		find/:case series :value
		append series :value
	]
	result
]

collect: func [
	"Evaluates a block, storing values via KEEP function, and returns block of collected values."
	body [block!] "Block to evaluate"
	/into "Insert into a buffer instead (returns position after insert)"
	output [series!] "The buffer series (modified)"
][
	unless output [output: make block! 16]
	do func [keep] body func [value [any-type!] /only] [
		output: insert/:only output :value
		:value
	]
	either into [output] [head output]
]

pad: func [
	"Pad a FORMed value on right side with spaces" 
    str "Value to pad, FORM it if not a string" 
    n [integer!] "Total size (in characters) of the new string (pad on left side if negative)" 
    /with "Pad with char" 
    c [char!] 
    return: [string!] "Modified input string at head"
][
    unless string? str [str: form str] 
    head insert/dup 
    any [all [n < 0 n: negate n str] tail str] 
    any [c #" "] 
    (n - length? str)
]

format: function [
	"Format a string according to the format dialect."
	rules {A block in the format dialect. E.g. [10 -10 #"-" 4 $32 "green" $0]}
	values
	/pad p {Pattern to use instead of spaces}
][
	p: any [p #" "]
	unless block? :rules  [rules:  reduce [:rules ]]
	unless block? :values [values: reduce [:values]]

	; Compute size of output (for better mem usage):
	val: 0
	foreach rule rules [
		if word? :rule [rule: get rule]
		val: val + switch/default type?/word :rule [
			integer! [abs rule]
			string!  [length? rule]
			char!    [1]
			money!   [2 + length? form rule]
			tag!     [length? rule] ;@@ does not handle variadic length results (for example month names)!
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
			money!   [out: change out replace ajoin ["^[[" next form rule #"m"] #"." #";"]
			tag! [
				out: change out switch/default type?/word val: first+ values [
					date! time! [
						format-date-time val rule
					]
					;TODO: other types formatting...
				][	
					; when there is not expected value, ignore it and output just the rule
					-- values
					form rule
				]
			]
		]
	]

	; Provided enough rules? If not, append rest:
	if not tail? values [append out values]
	head out
]

format-date-time: function/with [
	"replaces a subset of ISO 8601 abbreviations such as yyyy-MM-dd hh:mm:ss"
	value [date! time!]
	rule  [string! tag!]
][
	;-- inspired by https://github.com/greggirwin/red-formatting/blob/master/format-date-time.red
	tmp: to string! rule
	either time? value [
		d: now
		t: value
	][
		d: value
		t: any [d/time 0:0:0]
	]
	either parse/case tmp [
	any [
		  change "dddd" (pick system/locale/days d/weekday)
		| change "ddd"  (copy/part pick system/locale/days d/weekday 3)
		| change "dd"   (pad/with d/day -2 #"0")
		| change #"d"   (d/day)
		| change "mmmm" (pick system/locale/months d/month)
		| change "mmm"  (copy/part pick system/locale/months d/month 3)
		| change "mm"   (pad/with either as-time? [t/minute][d/month] -2 #"0") (as-time?: true)
		| change #"m"   (either as-time? [t/minute][d/month]) (as-time?: true)
		| change "yyyy" (pad/with d/year -4 #"0")
		| change "yy"   (skip tail form d/year -2)
		| change #"y"   (d/year)
		| change "hh"   (pad/with t/hour -2 #"0") (as-time?: true)
		| change #"h"   (t/hour) (as-time?: true)
		|
		[ change "ss"   (pad/with to integer! t/second -2 #"0")
		| change #"s"   (to integer! t/second)
		] opt [
			#"." s: some #"s" e: (
				n: (index? e) - (index? s)
				v: any [find/tail form t/second #"." ""]
				either n <= length? v [
					; trim result if it is too long
					clear skip v n
				][	; or pad it if too short
					v: pad/with v n #"0"
				]
				change/part s v e
			)
		]
		| change "MM"   (pad/with d/month -2 #"0") (as-time?: true)
		| change #"M"   (d/month) (as-time?: true)
		| change [opt #"±" "zz:zz"] (zone/with d/zone #":")
		| change [opt #"±" "zzzz" ] (zone d/zone)
		| change "unixepoch" (to integer! d)
		| skip ;@@ or better limit to just some delimiters?
		]
	][ tmp ][ form rule ]
][
	zone: function[z [time! none!] /with sep][
		z: any [z 0:0]
		ajoin [
			pick "-+" negative? z
			pad/with absolute z/hour -2 #"0"
			any [sep ""]
			pad/with z/minute 2 #"0"
		]
	]
]


printf: func [
	"Formatted print."
	fmt "Format"
	val "Value or block of values"
][
	print format :fmt :val
]

split: function [
	"Split a series into pieces; fixed or variable size, fixed number, or at delimiters"
	series [series!] "The series to split"
	;!! If we support /at, dlm could be any-value.
	dlm    ;[block! integer! char! bitset! any-string! any-function!] "Split size, delimiter(s), predicate, or rule(s)." 
	/parts "If dlm is an integer, split into n pieces, rather than pieces of length n."
	/at "Split into 2, at the index position if an integer or the first occurrence of the dlm"
][
	if any-function? :dlm [
		res: reduce [ copy [] copy [] ]
		foreach value series [
			append/only pick res make logic! dlm :value :value
		]
		return res
	]
	if at [
		unless integer? :dlm [
			return reduce either dlm: find series :dlm [
				dlm: index? dlm
				[
					copy/part   series dlm - 1 ; excluding the delimiter
					copy lib/at series dlm + 1
				]
			][
				[copy series]
			]
		]
		return reduce [
			copy/part   series dlm
			copy lib/at series dlm + 1
		]
	]
	;print ['split 'parts? parts mold series mold dlm]
	either all [block? dlm  parse dlm [some integer!]][
		map-each len dlm [
			either positive? len [
				copy/part series series: skip series len
			][
				series: skip series negate len
				()	;-- return unset so that nothing is added to output
			]
		]
	][
		size: dlm	;-- alias for readability
		res: collect [
			;print ['split 'parts? parts mold series mold dlm newline]
			parse series case [
				all [integer? dlm  parts][
					if size < 1 [cause-error 'Script 'invalid-arg size]
					count: size - 1
					piece-size: to integer! round/down divide length? series size
					if zero? piece-size [piece-size: 1]
					[
						count [copy series piece-size skip (keep/only series)]
						copy series to end (keep/only series)
					]
				]
				integer? dlm [
					if size < 1 [cause-error 'Script 'invalid-arg size]
					[any [copy series 1 size skip (keep/only series)]]
				]
				'else [	;-- = any [bitset? dlm  any-string? dlm  char? dlm]
					[any [mk1: some [mk2: dlm break | skip] (keep/only copy/part mk1 mk2)]]
				]
			]
		]
		;-- Special processing, to handle cases where they spec'd more items in
		;   /parts than the series contains (so we want to append empty items),
		;   or where the dlm was a char/string/charset and it was the last char
		;   (so we want to append an empty field that the above rule misses).
		fill-val: does [copy either any-block? series [ [] ][ "" ]]
		add-fill-val: does [append/only res fill-val]
		case [
			all [integer? size  parts][
				;-- If the result is too short, i.e., less items than 'size, add
				;   empty items to fill it to 'size.
				;   We loop here, because insert/dup doesn't copy the value inserted.
				if size > length? res [
					loop (size - length? res) [add-fill-val]
				]
			]
			;-- integer? size
			;	If they spec'd an integer size, but did not use /parts, there is
			;	no special filing to be done. The final element may be less than
			;	size, which is intentional.
			'else [ ;-- = any [bitset? dlm  any-string? dlm  char? dlm]
					;-- If the last thing in the series is a delimiter, there is an
					;   implied empty field after it, which we add here.
				case [
					bitset? dlm [
						;-- ATTEMPT is here because LAST will return NONE for an 
						;   empty series, and finding none in a bitest is not allowed.
						if attempt [find dlm last series][add-fill-val]
					]
					char? dlm [
						if dlm = last series [add-fill-val]
					]
					string? dlm [
						if all [
							find series dlm
							empty? find/last/tail series dlm
						][add-fill-val]
					]
				]
			]
		]

		res
	]
]

combine: func [
	"Combines a block of values with a possibility to ignore by its types. Content of parens is evaluated."
	data  [block!] "Input values"
	/with "Add delimiter between values"
	 delimiter 
	/into "Output results into a serie of required type"
	 out [series!]
	/ignore  "Fine tune, what value types will be ignored"
	 ignored [typeset!] "Default is: #[typeset! [none! unset! error! any-function!]]"
	/only "Insert a block as a single value"
	/local val rule append-del append-val block-rule
][
	out: any [out make string! 15]
	ignored: any [ignored make typeset! [none! unset! error! any-function!]]

	append-del: either/only delimiter [
		(unless empty? out [append out :delimiter])
	][]

	append-val: [
		opt [
			if (not find ignored type? :val)[
				append-del (append out :val)
			]
		]
	]

	block-rule: either/only only [
		set val block! append-del (
			if any-string? out [val: mold val]
			append/only out :val
		)
	][	ahead block! into rule]

	parse data rule: [
		any [
			block-rule
			|
			[
				  set val paren!    (set/any 'val try :val)
				| set val get-word! (set/any 'val get/any :val)
				| set val skip
			]
			append-val
		]
	]
	out
]

find-all: func [
	"Find all occurrences of a value within a series (allows modification)."
	'series [word!] "Variable for block, string, or other series"
	value
	body [block!] "Evaluated for each occurrence"
	/local orig result
][
	assert [series? orig: get series]
	set/any 'result while [set series find get series :value] [
		do body
		++ (series)
	]
	unless get series [set series orig]
	:result
]
