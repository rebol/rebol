REBOL [
	Title:  "Map and construction syntax literal syntax migration script"
	Author: "Oldes"
	File:   %map-conv.reb
	Assert: [
		probe "map: #[a: 1 b: #(none)]" == map-conv {map: #(a: 1 b: #[none])}
		probe  "blk: [#(none) #(true)]" == map-conv {blk: [#[none] #[true]]}
		probe  "vec: #(uint8! [1 2 3])" == map-conv {vec: #[uint8! [1 2 3]]}
		probe "m: #[a: #(none)]#[b: 1]" == map-conv {m: #(a: #[none])#(b: 1)}
		probe    "#[a: #[b: #(false)]]" == map-conv {#(a: #(b: #[false]))}
		probe           "#{} #[unset!]" == map-conv {#{} #(unset!)}
		probe  {#[unset] #"^{" #[true]} == map-conv {#(unset) #"^{" #(true)}
		probe  {#[unset]  "^{" #[true]} == map-conv {#(unset)  "^{" #(true)}
		probe  "#[unset] {^^{} #[true]" == map-conv "#(unset) {^^{} #(true)"
	]
	Note: "The parser is simplified! Content inside strings and comments is ignored!"
]

map-conv: function/with [
	"Swaps map! and construction syntax"
	data [binary! string! file!] "(modified)"
][
	either file? data [
		either dir? data [
			process-dir data
		][	process-file data ]
	][
		swap-map-cs data
	]
][
	pos: none
	ch_ignore: complement charset {[](){};#"}
	ch_ignore_str1: complement charset "^^{}"
	ch_ignore_str2: complement charset {^^"}
	rl_paren: [#"(" rl_swap #")"] 
	rl_block: [#"[" rl_swap #"]"]

	rl_in_string1: [any [some ch_ignore_str1 | #"^^" skip | rl_string1]]
	rl_in_string2: [any [some ch_ignore_str2 | #"^^" skip]]
	rl_string1: [#"{" rl_in_string1 #"}"]
	rl_string2: [#"^"" rl_in_string2 #"^""]
	rl_comment: [#";" [to LF | to end]]

	rl_swap: [
		any [
			some ch_ignore
			| rl_comment
			| rl_block
			| rl_paren
			| rl_string1
			| rl_string2
			| #"#" pos: [
				  ahead #"[" (append cons_s index? pos) rl_block pos: (append append cons take/last cons_s index? pos)
				| ahead #"(" (append maps_s index? pos) rl_paren pos: (append append maps take/last maps_s index? pos)
				| ahead #"{" thru #"}"                   ;; ignore binaries
				| #"^"" [#"^^" #"^"" #"^"" | thru #"^""] ;; ignore chars
				| skip
			]
			| end
		]
	]

	cons: make block! 1000
	maps: make block! 1000

	;; stacks..
	maps_s: make block! 1000
	cons_s: make block! 1000

	process-dir: function [path [file!]][
		foreach file read path [
			if dir? file [process-dir path/:file]
			all [
				find [%.reb %.r3] suffix? file
				process-file path/:file
			]
		]
	]
	process-file: function [file [file!]][
		try/with [
			print [as-green "File:" as-yellow file]
			data: swap-map-cs read file
			num-maps: (length? maps) / 2
			num-cons: (length? cons) / 2
			if any [num-maps > 0 num-cons > 0][
				print ["maps:" as-yellow num-maps "cons:" as-yellow num-cons]
				write file data
			]
		][
			print as-purple "File conversion failed!"
			print system/state/last-error
		]
	]

	swap-map-cs: func[data][
		clear cons
		clear maps
		clear cons_s
		clear maps_s
		if binary? data [data: to string! data]
		unless parse data rl_swap [
			print as-purple "File not fully parsed so not modified!"
			return data
		]
		;?? cons
		foreach [s e] cons [
			change at data s #"("
			change at data e - 1 #")"
		]
		;?? maps
		foreach [s e] maps [
			change at data s #"["
			change at data e - 1 #"]"
		]
		;print data
		data
	]
]


