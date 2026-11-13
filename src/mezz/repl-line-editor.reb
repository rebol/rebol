Rebol [
	Title:   "Line editor context"
	Purpose: {Reusable line editor}
	Name:    line-editor
	Type:    module
	Version: 0.3.0
	Date:    6-May-2026
	Needs:   3.21.18
	exports: [line-editor!]
]

line-editor!: context [
	prompt: "^[[1;31m## ^[[1;33m"
	result-limit: 500 ;; max length of the molded result output
	buffer: copy ""
	line: pos: result: code: banner: parent-console: _
	time: 0:0
	prev-col: col: 0
	history: clear []
	current-key: _
	console-ctx: context []
	ansi: system/options/ansi

	init: func [][
		clear buffer
		line: pos: clear ""
		prev-col: col: 0
		parent-console: system/console/current
		if none? parent-console [
			try [append history load system/options/data/.repl-history]
		]
		system/console/current: context? 'parent-console
		if string? :banner [print banner]
		prin prompt
	]

	;-- Main callbacks ---
	on-key: func[key /local tmp][
		current-key: key
		prev-col: col
		clear buffer
		switch/default key [
			#"^M"         [ on-enter  ]
			#"^-" backtab [ on-tab    ]
			#"^[" escape  [ on-escape ]
			backspace #"^~" #"^H" #"^(7F)" [
				unless head? pos [
					either system/state/control? [
						;; delete to the previous delimiter
						tmp: pos
						skip-to-prev-delimiter
						remove/part pos tmp
					][	;; delete previous char
						col: col - pos/-1/width
						pos: remove back pos
					]
					skip-to col
					emit ["^[[K" pos]
					if tail? pos [prev-col: col]
				]
			]
			delete [
				unless tail? pos [
					either system/state/control? [
						tmp: pos prev-col: col
						skip-to-next-delimiter
						pos: remove/part tmp pos
						col: prev-col
					][	;; delete following char
						pos: remove pos
					]
					emit ["^[[K" pos]
					prev-col: none ;; force cursor position refresh
				]
			]
			#"^C" [
				print ajoin [clear-newline ansi/magenta "(CTRL+C)"]
				on-exit
				break
			]
			#"^U" [ ;= CTRL+U - clear line
				pos: clear line
				col: prev-col: 0
				emit [clear-line prompt]
			]
			#"^L" [ ;= CTRL-L - clear screen
				pos: clear line
				col: prev-col: 0
				emit [clear-screen clear-buffer prompt]
			]
			up [
				either tail? history [ emit beep ][
					emit [clear-line prompt ]
					append clear line history/1
					++ history
					emit line
					skip-to-end
					prev-col: col
				]
			]
			down [
				either head? history [ emit beep ][
					-- history
					emit [clear-line prompt ]
					append clear line history/1
					emit line
					skip-to-end
					prev-col: col
				]
			]
			left [
				unless head? pos [
					either system/state/control? [
						;; Skip all delimiters backwards.
						skip-to-prev-delimiter
					][	skip-back ]
				]
			]
			right [
				unless tail? pos [
					either system/state/control? [
						;; Skip all delimiters forward
						skip-to-next-delimiter
					][	skip-next ]
				]
			]
			home #"^A" [
				pos: head pos
				col: 0
			]
			end #"^E" [
				pos: tail pos
				col: line/width
			]
		][
			if all [char? key key > 0#1F][
				emit back pos: insert pos key
				col: col + key/width
				if tail? pos [prev-col: col]
			]
		]
		time: stats/timer
		flush
	]
	on-enter: does [
		if empty? line [
			prin ajoin [unless multiline [clear-line] clear-newline prompt]
			exit
		]
		if line != first history: head history [
			insert history copy line
		]
		on-line
	]
	on-line: does [
		result: try [transcode code: line]
		prin clear-newline
		either error? :result [
			;; It's an error from transcode, no need to show the stack!
			unset in :result 'where
		][
			code: bind result system/contexts/lib  ;; core values
			code: bind code system/contexts/user   ;; e.g. values from startup scripts
			code: bind/set code console-ctx        ;; per console session values
			;; Evaluate code with protection from all errors and quit.
			set/any 'result try/all [ catch/quit code ]
			if system/state/quit? [
				system/state/quit?: false ;; quit only from this console
				on-quit
				break
			]
		]
		on-result
	]
	on-result: func[/local molded] [
		pos: clear line
		col: prev-col: 0
		set/any 'system/state/last-result :result
		case [
			find system/options/result-types type? :result [
				molded: mold/part :result result-limit + 1
				if molded/length > result-limit [ append trim/tail molded "^[[m…" ]
				emit [ansi/green "== " ansi/bright-green molded LF LF]
			]
			error? :result [
				;; ignore stack values after first `catch`
				if block? select :result 'where [clear find result/where 'catch]
				;; output each line...
				foreach line split-lines form :result [
					emit [ansi/error line LF]
				]
				emit LF
			]
			unset? :result [prin LF] ; ignored
		]
		unset 'result
		emit [clear-line prompt]
		flush
	]
	on-escape: does [
		unless empty? line [
			pos: clear line
			col: prev-col: 0
			hide-status
			emit [clear-line prompt]
		]
	]
	on-tab: does [
		emit skip pos: insert/dup pos SP 4 -4
		col: col + 4
		if tail? pos [prev-col: col]
	]
	on-exit: does [
		system/console/current: parent-console
		;; save only root console's history
		if none? parent-console [ try [save-history] ]
		()
	]
	on-quit: does [
		emit [clear-line ansi/magenta  "(quit)" ansi/reset LF]
		flush
		on-exit
	]
	save-history: does [
		parse history [any ["q" | "quit"] history:] ;; don't include `quit` commands
		save system/options/data/.repl-history new-line/all history true
	]

	;-- Private editor functions ---

	emit: func[s][append buffer either block? s [ajoin s][s]]

	prompt-width: function/with [][
		either prev-prompt == prompt [ width ][
			tmp: sys/remove-ansi copy prev-prompt: prompt
			width: tmp/width ;; in columns
		]
	][  ;; cache previous prompt width
		prev-prompt: none width: 0
	]
	
	skip-to: func[col][emit ["^[[" prompt-width + col + 1 #"G"]]
	skip-to-end: does [ pos: tail line  col: line/width	]
	skip-to-prev-delimiter: does [
		;; skip any delimiters immediately to the left of `pos`
		while [ all [not head? pos find delimiters pos/-1 ]][ skip-back ]
		;; then keep going left until we hit the head or another delimiter
		unless head? pos [
			until [ skip-back any [head? pos  find delimiters pos/-1] ]
		]
	]
	skip-to-next-delimiter: does [
		;; skip any delimiters immediately to the right of `pos`
		while [ all [not tail? pos find delimiters pos/1 ]][ skip-next ]
		;; then keep going right until we hit the tail or another delimiter
		unless tail? pos [
			until [ skip-next any [tail? pos  find delimiters pos/1] ]
		]
	]
	skip-back: does [
		unless head? pos [
			pos: back pos
			col: col - pos/1/width
		]
	]
	skip-next: does [
		unless tail? pos [
			col: col + pos/1/width
			pos: next pos
		]
	]
	remove-back: func[n][
		loop n [
			if head? pos [break]
			col: col - pos/-1/width
			pos: remove back pos
		]
	]
	flush: does [
		;; Move cursor only if really changed its position.
		if prev-col != col [skip-to col]
		prin take/all buffer
	]		

	;---- Constants ----
	clear-line:      "^M^[[K^[[0m"       ;; go to line start, clear to its end, reset
	clear-newline:   "^/^[[K"            ;; go to new line and clear it (removes optional status line)
	;clear-next-line: "^[[1B^[[2K^[[1A"
	clear-down:      "^[[J"
	clear-to-pos:    "^[[1K"             ;; erase from start of line to cursor.
	clear-screen:    "^[[H^[[2J"
	clear-buffer:    "^[[3J"
	save-cur:        "^[[s"              ;= tui [save]
	restore-cur:     "^[[u"              ;= tui [restore]
	move-up:         "^[[1A"             ;= tui [up]
	move-down:       "^[[1B"             ;= tui [down]
	move-start:      "^M"                ;= tui [col 0]  
	highlight:       "^[[7m"             ;= tui [invert]
	reset-style:     "^[[0m"             ;= tui [reset]
	next-line:       "^/^[[J"
	beep:            #"^G"

	delimiters: charset { /%[({})];:"}

	;-- Multiline support ---
	multiline:  _        ;; block of lines
	ml-prompt:  _        ;; stored original prompt while inside multiline mode
	ml-type:    _        ;; current bracket type
	reset-multiline: does [
		if multiline [
			multiline: none
			prompt: ml-prompt
		]
	]

	;-- Status line ---
	status?: off
	show-status: func [type [word!] txt][
		status?: type
		;; limit output to max line width...
		;@@TODO: this code is not correct as it counts also ANSI codes!
		;max-cols: query system/ports/output 'window-cols 
		;unless string? txt [txt: reform txt]
		;if txt/width >= max-cols [ 
		;	clear skip txt max-cols - 2
		;	append txt "…^[[m"
		;]
		prin ajoin [
			LF clear-down ansi/gray txt ansi/reset move-up "^[[" (prompt-width + col + 1) #"G"
		]
	]

	hide-status: does [
		if status? [
			status?: off
			prin "^[[1B^[[1K^[[J^[[1A" ;= move-down, clear-to-pos, clear-down, move-up
		]
	]
]
