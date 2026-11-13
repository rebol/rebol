REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Base: ANSI light theme"
]

append system/options/ansi [
	black:          "^[[38;5;16m"
	blue:           "^[[38;5;32m"
	cyan:           "^[[38;5;30m"
	green:          "^[[38;5;35m"
	magenta:        "^[[38;5;55m"
	red:            "^[[38;5;167m"
	white:          "^[[38;5;102m"
	yellow:         "^[[38;5;178m"
	bright-black:   "^[[38;5;109m"
	bright-blue:    "^[[38;5;25m"
	bright-cyan:    "^[[38;5;31m"
	bright-green:   "^[[38;5;29m"
	bright-magenta: "^[[38;5;55m"
	bright-red:     "^[[38;5;124m"
	bright-white:   "^[[38;5;188m"
	bright-yellow:  "^[[38;5;136m"
	reset:  "^[[0m"
]