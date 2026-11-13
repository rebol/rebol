Rebol [
	Title:   "Rebol Console"
	Purpose: {Rebol Console with multiline input and TAB completion}
	Version: 0.4.0
	Date:    27-May-2026
	Needs:   3.21.18
	type:    module
	name:    rebol-console
	exports: [rebol-console]
]



rebol-console: function [
	"Start an interactive REPL using the line editor."
	/with "Customize the console by overriding line-editor! defaults."
	 spec [block!]
][
	unless tty? [exit] ;; start console only if input is available!
	editor: make line-editor! any [spec default-spec]
	editor/init
	forever [ editor/on-key read-key ]
]

acceptable-code: function/with [
	"Returns the currently open bracket if the code can be fixed with additional edits."
	;; If it has a missing (but balanced) closing parenthesis.
	code [string!]
][
	stack: clear ""
	all [
		parse code [any code-rule ]
		last stack
	]
][
	stack: ""
	raw: none
	code-char:    complement charset "[](){}^"%;^/"
	string1-char: complement charset {"^^^/}
	string2-char: complement charset "^^{}"
	code-rule: [
		some code-char
		| block-rule
		| paren-rule
		| string1-rule ;= single line
		| string2-rule ;= multiline
		| string3-rule ;= raw-string
		| comment-rule
		| lf | #"%"
	]
	block-rule: [
		 #"[" (append stack #"[") any code-rule
		[#"]" (take/last stack) | end]
	]
	paren-rule: [
		 #"(" (append stack #"(") any code-rule
		[#")" (take/last stack) | end]
	]
	string1-rule: [
		#"^"" (append stack #"^"") some [
			  #"^^" skip
			| #"^/" to end ;; failed!
			| any string1-char
		] #"^"" (take/last stack)
	]
	string2-rule: [
		#"{" (append stack #"{") some [
			  #"^^" skip
			| string2-rule
			| any string2-char
		]
		[#"}" (take/last stack) | end]
	]
	string3-rule: [
		copy raw: some #"%" (append stack #"{" insert raw "}")
		thru raw (take/last stack)
	]
	comment-rule: [#";" [to LF | to end] ]
]

default-spec: [
	;banner: :sys/boot-banner
	history: system/console/history
	prompt: function [][
		dir: what-dir
		parse dir [change system/options/home "~/" to end]
		ajoin [ansi/magenta dir "^[[1;31m>^[[m "]
	]
	completion: make completion! []
	console-ctx: completion/user-context

	on-edit-key:    :on-key
	on-edit-escape: :on-escape
	on-key: func[key][
		try/with [
			on-edit-key key
			unless find [tab backtab #"^-"] key [
				if status? == 'tab [
					hide-status
					either find [#" " #":" #"/" right] key [
						completion/accept
					][	completion/reset ]
				]
			]
		][
			prin next-line
			result: system/state/last-error
			on-result
		]
	]
	on-tab: does [
		;; If line is empty or contains only spaces, treat TAB as 2 spaces.
		either parse line [any SP end] [
			pos: insert pos "  "
			emit at pos -2
			col: col + 2
			if tail? pos [prev-col: col]
		][
			;; Completion only at line tail.
			if tail? pos [
				;; remove existing tab completion
				if completion/suffix [
					remove-back completion/suffix/length
					completion/suffix: _
					emit "^[[K"
				]
				completion/complete line
				if zero? completion/count [continue]
				;; TAB cycles forward, SHIFT+TAB (backtab) cycles backward
				completion/get-match (did any [current-key = 'backtab system/state/shift?])
				either completion/count == 1 [
					;; direct hit - append and forget
					append append pos completion/suffix SP
					completion/accept
				][	;; display multiple posibilities in the status line
					show-status 'tab completion/status-line
					append pos completion/suffix
				]
				emit pos
				skip-to-end
			]
		]
	]
	on-line: func[/local index] [
		completion/accept
		if status? [hide-status]
		either multiline [
			result: try [transcode code: ajoin [ajoin/with multiline LF LF line]]
		][	result: try [transcode code: line]]
		either error? result [
			if ml-type: acceptable-code code [
				unless multiline [
					multiline: clear []
					ml-prompt: :prompt  ;; store original prompt
					prompt: as-purple append/dup clear "" SP max 2 prompt-width
				]
				change back find/last prompt " "  ml-type
				append multiline copy line
				pos: clear line
				emit [LF prompt]
				exit
			]
			;; It's an error from transcode, no need to show the stack!
			unset in :result 'where
			prin next-line
			reset-multiline
		][
			prin next-line
			if multiline [ reset-multiline ]
			index: 1 + length? console-ctx        ;; optimization (only new words)
			code: bind/new result console-ctx     ;; extend current console's context with new words
			resolve/only console-ctx system/contexts/user index  ;; e.g. values from startup scripts
			resolve/only console-ctx lib index    ;; copy only the new values into the console's context

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
	on-escape: does [
		either status? = 'tab [
			;; Remove existing TAB completion suffix.
			if completion/suffix [
				remove-back completion/suffix/length
				completion/suffix: _
				skip-to col
				emit "^[[K"
			]
			;; And hide the completion status line.
			hide-status
			completion/reset
		][	;; Or call default escape handler.
			on-edit-escape
		]
	]
]