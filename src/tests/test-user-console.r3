Rebol [
	title: "Custom console test"
	needs: 3.12.0
	purpose: {
		Console port in an async (char based) mode.
		Allows console input while having other async devices running.
	}
	issues: {
		* Using `wait` function inside another wait (like in this console) has strange results.
		* When using paste in this console, it processes all input as key presses, which is slow.
		* Does not allow changing the cursor position.
	}
]

my-console: context [
	port: system/ports/input
	port/data: make string! 32
	prompt: "## "
	port/awake: function/with [event /local res][
		;probe event
		;probe event/offset

		;; Using a buffer to print all output in one pass to avoid flickering.
		clear buffer
		switch event/type [
			key [
				;; debug:
				; print ["^[[G^[[K" mold event/key event/code]
				switch/default event/key [
					#"^~"
					#"^H" [
						take/last event/port/data
					]
					#"^M" [
						prin LF
						unless empty? event/port/data [
							set/any 'res try/all [do event/port/data]
							clear event/port/data
							unless unset? :res [
								emit as-green "== "
								emit mold :res
								print buffer
								clear buffer
							]
						]
					]
					#"^-" [ ; Tab Completion
						complete-input event/port/data
					]
				][
					append event/port/data event/key
				]
				emit "^[[G^[[K"
				emit as-red prompt
				emit event/port/data
			]
			control	[
				if find [shift control alt] event/key [ return false ]
				emit "^[[G^[[K"
				emit reform ["control:" event/key event/flags LF]
				emit as-red prompt
				emit event/port/data
				if event/key = 'escape [
					emit "[ESC]^/"
					return true
				]
			]
			;control-up [
			;	prin "^[[G^[[K"
			;	print ["control-up:" event/key]
			;	prin as-red prompt
			;	prin event/port/data
			;]
			resize    [
				;print ["^[[G^[[Ksize:" event/offset]
				emit "^[[G^[[K"
				emit as-red prompt
				emit event/port/data
			]
			interrupt [
				print "^/[INTERRUPT]^/"
				return true
			]
		]
		prin buffer
		false
	][
		buffer: make string! 1000
		emit: func[str][ append buffer str ]
	]


	complete-input: function[
		input-data [string!]
	][
		part: any [
			find/last/tail input-data SP
			input-data
		]
		case [
			part/1 == #"%" [ ; File completion
				part: as file! next part
				path-parts: split-path part
				files: sort read path-parts/1
				matching-part: none
				either perfect-match: find files part [
					append input-data SP
				][
					best-matches: clear []
					foreach file files [
						if parse file [part to end][
							append best-matches as string! file
						]
					]
					either single? best-matches [
						missing-part: skip best-matches/1 length? part
						append input-data join missing-part SP
					][
						print ["^[[G^[[K" mold best-matches]
						min-length: length? best-matches/1
						foreach match next best-matches [
							min-length: min min-length length? match
						]
						if match-count: catch [
							repeat char-count min-length [
								char: best-matches/1/:char-count
								foreach word best-matches [
									if char != word/:char-count [
										throw char-count - 1
									]
								]
							]
						][
							matching-part: skip copy/part best-matches/1 match-count length? part
						]
					]
				]
				if matching-part [
					append input-data matching-part
				]
			]
			not empty? part [ ; Word completion
				;@@ all-words should not be created on each completion call!
				all-words: sort union words-of system/contexts/lib words-of self
				forall all-words [all-words/1: to string! all-words/1]

				either perfect-match: find all-words part [
					append input-data SP
				][
					best-matches: clear []
					foreach word all-words [
						if parse word [ part to end ] [
							append best-matches word
						]
					]
					either single? best-matches [
						missing-part: skip to string! best-matches/1 length? part
						append append input-data missing-part SP
					] [
						print ["^[[G^[[K" mold best-matches]
					]
				]
			]
		]
	]

	modify port 'line false
	prin as-red prompt
	wait [port]
	modify port 'line true
]
