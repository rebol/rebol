Rebol [
	Title: "Rebol Console - Completion"
	Home:  https://github.com/Oldes/Rebol-Console
	Type:  module
	Name:  rebol-completion
	exports: [completion!]
	Note:  {This source must not use any `print` calls!}
]

completion!: context [
	;- public ---
	matches:      []  ;; current matches (using index as a position!)
	count:        0   ;; total number of matches
	status-line:  ""  ;; holds possible matches as a trimed line

	reset: does [
		count: 0 suffix: last-input: kind: _
		clear head matches
		clear status-line
	]

	accept: func [/local pos] [
		;; Move accepted match to tail so it's served first next time.
		if all [
			kind = 'word
			pos: find/last words matches/1
		][
			append words take pos
		]
		reset
	]
	
	complete: func [
		;; Input completion function.
		input     [string! ] ;; Current line to be completed
		/local files dir n
	][
		if last-input = input [exit]
		count: 0 suffix: _
		
		partial: any [
			find/last/tail input SP
			input
		]
		
		matches: clear head matches
		case [
			partial/1 == #"%" [ ;- File completion ----
				kind: 'file
				partial: next partial
				either empty? partial [
					files: read %.
					forall files [
						append matches as string! enhex files/1
					]
				][
					unless dir? dir: as file! partial [ dir: first split-path dir ]
					unless files: attempt [read dir][ exit ]
					foreach file files [
						file: dir/:file
						if apply :parse [
							file [opt [%./ file:] partial to end]
							system/platform != 'Windows ;; Case-sensitive on Posix!
						][
							append matches as string! enhex file
						]
					]
				]
			]
			find partial #"/" [ ;- Path completion ----
				kind: 'path
				append matches any [
					scan-context system/contexts/lib
					scan-context user-context
					[]
				]
			]
			not empty? partial [ ;- Word completion ----
				kind: 'word
				n: length? words
				if user-size < length? user-context [
					foreach word reverse skip words-of user-context user-size [
						append words form word
					]
					user-size: length? user-context
				]
				if lib-size < length? lib-context [
					foreach word reverse skip words-of lib-context lib-size [
						append words form word
					]
					lib-size: length? lib-context
				]
				if n < length? words [ words: unique words ]

				;; Collect from tail (new words will be served first)
				n: length? words
				while [n > 0] [
					if parse words/:n [ partial to end ][
						append matches words/:n
					]
					-- n
				]
			]
			'else [last-input: kind: none exit]
		]
		
		unless zero? count: length? matches [last-input: input]
		matches: tail matches ;; starting with position at tail (so first match will be from the head)
	]

	get-match: func[
		back? [logic!]
		/local mark start-mark index index-found len mlen match-visible term-width
	][
		if zero? count [return ""]
		;; rotate left/right
		either back? [
			matches: back either head? matches [tail matches][matches]
		][
			++ matches
			if tail? matches [matches: head matches]
		]
		;; prepare matches with highlighted current match
		term-width: query system/ports/input 'window-cols
		clear status-line
		index-found: index? matches 
		index: 1 len: 0 match-visible: _
		foreach match head matches [
			if kind = 'path [
				match: any [find/last match #"/" match]
			]
			mlen: match/width
			if (len + mlen + 1) >= term-width [
				if match-visible [ break ]
				clear status-line
				len: 0
			]
			append status-line ajoin either index == index-found [
				match-visible: true
				["^[[7m" match "^[[27m "]
			][	[match SP]]
			len: len + mlen + 1
			++ index
		]
		any [
			suffix: find/match/tail matches/1 partial
			all [empty? partial suffix: matches/1]
			;print ["^/partial:" mold partial matches/1]
		]
	]

	;- private --
	partial:    _   ;; the partial word being completed (the fragment after the last space)
	suffix:     _   ;; the currently inserted completion suffix (the part appended after partial)
	last-input: _   ;; used to detect whether the input has changed since the last TAB press
	kind:       _   ;; the completion type: word / path / file
	words: copy []  ;; collected words for possible completion
	lib-size:   0   ;; number of collected words from the lib context
	user-size:  0   ;; number of collected words from the user context
	lib-context: system/contexts/lib
	user-context: context []

	;; Object/function completion support

	form-all: func[blk [block!]][ forall blk [change blk form blk/1] blk]

	filter-matches: function [
		"From block of strings, return only those matching pattern"
		block   [block!]
		pattern [string!]
	][
		remove-each value block [ not find/match value pattern ]
	]

	scan-context: function [
		ctx [object!]
	][
		;; Working with local copy not to modify the original completion part!
		local-part: copy partial
		slash?: if #"/" = last local-part [ take/last local-part ]
		unless attempt [path: transcode/one local-part][ return none ]
		;; Casting to block to have propper formating with single segment path!
		path-start: either word? path [path][
			path: bind as block! path ctx
			path/1
		]
		foreach [key val] ctx [
			if equal? path-start key [
				case [
					any-function? :val [
						;; Collect all function's refinements..
						matches: parse spec-of :val [
							collect any [to refinement! set ref: skip keep (to word! ref)]
						]
						if block? path [
							;; Remove all refinements, which are already present.
							remove-each ref matches [find path ref]
							;; When there was not a slash at tail, user has partial refinement
							unless slash? [
								;; Remove all which does not start with the last path segment.
								filter-matches form-all matches form take/last path
							]
						]
						;; End the loop..
						break
					]			
					any-object? :val [
						matches: case [
							;; top level object
							word? path [
								form-all words-of :val
							]
							;; subobject
							slash? [
								result: get/any as path! path
								if any-object? result [ form-all words-of result ]
							]
							'else [
								unless attempt [ get/any as path! path ][ ;; fully resolved path, nothing to add
									;; partial word from subobject
									partial2: form take/last path
									path: either single? path [path/1][as path! path]
									if any-object? result: get/any path [
										filter-matches form-all words-of :result partial2
									]
								]
							]
						]
						;; End the loop..
						break
					]
				]
			]
		]
		either block? matches [
			if block? path [path: as path! path] ;; Cast back to path before converting to string
			prefix: dirize form path
			forall matches [matches/1: ajoin [prefix matches/1]]
			head matches
		][ none ]
	]
]