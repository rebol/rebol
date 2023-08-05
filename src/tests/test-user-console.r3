Rebol [
	title: "Custom console test"
	needs: 3.12.0
	purpose: {
		Console port in an async (char based) mode.
		Allows console input while having other async devices running.
	}
]

my-console: context [
	port: system/ports/input
	port/data: make string! 32
	prompt: "## "
	port/awake: function[event /local res][
		;probe event
		;probe event/offset
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
								prin as-green "== "
								probe res
							]
						]
					]
				][
					append event/port/data event/key
				]
				prin "^[[G^[[K"
				prin as-red prompt
				prin event/port/data
			]
			control	[
				prin "^[[G^[[K"
				print ["control:" event/key event/flags]
				prin as-red prompt
				prin event/port/data
				if event/key = 'escape [
					print "[ESC]"
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
				print ["^[[G^[[Ksize:" event/offset]
				prin as-red prompt
				prin event/port/data
			]
			interrupt [	return true ]
		]
		false
	]
	modify port 'line false
	prin as-red prompt
	wait [port]
	modify port 'line true
]