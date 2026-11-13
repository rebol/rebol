Rebol [
	Title:   "ANSI escape sequences support"
	File:    %mezz-ansi.reb
	Version: 1.4.0
	Date:    8-May-2026
	Purpose: "Decorate any value with basic ANSI color sequences"
]

as-gray:   function ["Decorates a value with gray ANSI escape codes"   value return: [string!]] bind [ajoin [ansi/gray           value ansi/reset]] :system/options
as-red:    function ["Decorates a value with red ANSI escape codes"    value return: [string!]] bind [ajoin [ansi/bright-red     value ansi/reset]] :system/options
as-green:  function ["Decorates a value with green ANSI escape codes"  value return: [string!]] bind [ajoin [ansi/bright-green   value ansi/reset]] :system/options
as-yellow: function ["Decorates a value with yellow ANSI escape codes" value return: [string!]] bind [ajoin [ansi/bright-yellow  value ansi/reset]] :system/options
as-blue:   function ["Decorates a value with blue ANSI escape codes"   value return: [string!]] bind [ajoin [ansi/bright-blue    value ansi/reset]] :system/options
as-purple: function ["Decorates a value with purple ANSI escape codes" value return: [string!]] bind [ajoin [ansi/bright-magenta value ansi/reset]] :system/options
as-cyan:   function ["Decorates a value with cyan ANSI escape codes"   value return: [string!]] bind [ajoin [ansi/bright-cyan    value ansi/reset]] :system/options
as-white:  function ["Decorates a value with white ANSI escape codes"  value return: [string!]] bind [ajoin [ansi/bright-white   value ansi/reset]] :system/options

ansi-colorize: function/with [
	"Apply ANSI color and style markup to a string using a lightweight inline dialect." {
	Returns the string unchanged if color output is disabled.

	Markup syntax:
	```
	 `text`   - monospace/code style (bright yellow); nestable with other markup
	 _text_   - underline style (bright cyan)
	 ```      - code block (default gray)
	 ^^`       - literal backtick (escape sequence)
	 ^^_       - literal underscore (escape sequence)
	```
	The base color defaults to the foreground color unless overridden with /init.
	Markup resets back to the enclosing style when closed, so partial nesting works:
	
	 "normal _underline `code` still-underline_ normal"
	}
	text [string!]
	/init style "Base ANSI color/style to use instead of the default foreground color"
][
	if system/options/no-color [return text]
	clear stack
	clear out
	if init [append stack style]
	underline?: off
	code?: off
	parse text [
		any [
			copy str: to delimiter (emit str) [
				s:
				"```" 4 skip to "^/```" e: 4 skip (
					emit-code skip copy/part s e 4
				)
				|
				#"`" (
					emit either/only code? [
						pop-pen
					][  push-pen a/bright-yellow ]
					code?: not code?
				)
				|
				#"^"" thru #"^"" e: (
					emit either/only code? [
						copy/part s e
					][
						push-pen a/green
						copy/part s e
						pop-pen
					]
				)
				|
				#"_" (
					emit either/only underline? [
						a/underline-off
						pop-pen
					][  a/underline push-pen a/bright-cyan]
					underline?: not underline?
				)
				|
				#"^^" (emit copy/part s 1) skip
				|
				#"^/" (emit LF)
			]
		]
		opt [copy str: to end (emit str)]
	]
	if underline? [emit a/underline-off]
	unless empty? stack [emit stack/1]
	copy out
][
	a: system/options/ansi
	stack: [] out: ""
	push-pen: func[clr][
		if empty? stack [append stack a/foreground]
		append stack clr
		clr
	]
	pop-pen: does[
		take/last stack
		any [last stack ""]
	]
	emit: func[str][
		append out either block? str [ajoin str][str]
	]

	emit-code: func[text /local str][
		emit [LF push-pen a/gray]
		parse text [
			any [
				copy str some not-comm (emit str)
				| #";" copy str [to LF | to end] (
					emit [push-pen a/foreground #";" str pop-pen]
				) 
			]
		]
		emit [LF pop-pen]
	]
	not-comm: complement make bitset! ";"  
	delimiter: make bitset! {^/_`^^"}
]
