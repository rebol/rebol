REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL Mezzanine: Security"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

secure: function/with [
	"Set security policies (use SECURE help for more information)."
	'policy [word! lit-word! block! unset!] "Set single or multiple policies (or HELP)"
] append bind [

	"Two funcs bound to private system/state/policies with protect/hide after."
	set-policies: func [p] [set 'policies p]
	get-policies: func [] [copy/deep policies]

] system/state [

	if unset? :policy [policy: 'help]
	if policy = 'none [policy: 'allow] ; note: NONE is a word here (like R2)
	if policy = 'help [
		print " You can set policies for:^[[1;32m"
		foreach [target pol] pol-obj [print ["    " target]]
		print " ^[[mThese can be set to:"
		print [
			"^[[1;32m     allow ^[[m - no security^/"
			"^[[1;32m     ask   ^[[m - ask user for permission^/"
			"^[[1;32m     throw ^[[m - throw as an error^/"
			"^[[1;32m     quit  ^[[m - exit the program immediately^/"
			"^[[1;32m     file  ^[[m - a file path^/"
			"^[[1;32m     url   ^[[m - a file path^/"
			"^[[1;32m     other ^[[m - other value, such as integer"
		]
		print " Settings for read, write, and execute are also available."
		print "^/ ^[[1;35mNOTE: ^[[1;31mSecure is not fully implemented!^[[m^/"
		;print "Type: help/doc secure for detailed documentation and examples."
		exit
	]
	;; Get a deep copy of the current policy object
	pol-obj: get-policies

	if policy = 'query [
		out: make block! 2 * length? pol-obj
		foreach [target pol] pol-obj [
			lib/case [
				;; Don't display `eval` and `memory` as these are special
				find [eval memory] target [continue]
				; file 0.0.0 (policies)
				tuple? pol [repend out [target word-policy pol]]
				; file [allow read quit write]
				block? pol [
					foreach [item pol] pol [
						repend out [item word-policy pol]
					]
				]
			]
		]
		new-line/skip out on 2
		return out
	]

	;; Check if SECURE is secured:
	if all [check (n: pol-obj/secure/2) > 0 system/options/boot-level = 'full] [
		if all [n == 1 not confirm-policy 'secure 2 policy][
			either n == 2 [
				cause-error 'access 'security :policy
			][	quit/return 101 ]
		]
	]

	; Bulk-set all policies:
	if word? policy [
		n: make-policy 'all policy
		foreach word words-of pol-obj [set word n]
		set-policies pol-obj
		exit
	]

	; Set each policy target separately:
	foreach [target pol] policy [
		try/with [
			assert/type [target [word! file! url!] pol [block! word! integer!]]
		][	cause-error 'access 'security-error reduce [target pol] ]
		set-policy target make-policy target pol
	]

	;; Sort file path exceptions.
	foreach [target pol] pol-obj [
		if block? pol [
			;; First 2 values are policy defaults (global)
			pol: skip pol 2
			;; Sort from the longest path to the shortest
			sort/skip/compare pol 2 func[a b /local la lb] [
				la: length? a
				lb: length? b
				either la <> lb [la > lb] [a > b]
			]
			;; Global level reduction
			rule: pol/-1
			while [not tail? pol][
				;; remove all exceptions with same policy as the global one
				pol: either rule = pol/2 [remove/part pol 2][skip pol 2] 
			]
			if target = 'file [
				;; Directory policy level reduction 
				case: system/platform != 'Windows
				pol: reverse head pol                     ;; work from least to most specific
				while [series? pol/2][
					cur: pol: skip pol 2                  ;; advance to next exception pair
					unless dir? pol/-1 [continue]         ;; parent must be a directory!
					while [series? cur/2][                ;; skip global fallback (tuple, not block)
						cur: either all [
							cur/1 = pol/-2                ;; same policy as parent?
							find/match/:case cur/2 pol/-1 ;; path is under parent?
						][  remove/part cur 2             ;; redundant, remove
						][  skip cur 2 ]                  ;; keep, move on
					]
				]
				pol: reverse head pol                     ;; restore original order
			]
		]
	]
	set-policies pol-obj
	exit
][
	;; Permanent values and sub-functions of SECURE:
	pol-obj: _                   ;; shared policy object, valid only during `secure` evaluation
	acts: [allow ask throw quit] ;; policy levels, index-based (0=allow 1=ask 2=throw 3=quit)
	check: true ;; used to detect, if secure must be checked

	assert-policy: func [tst kind arg] [unless tst [cause-error 'access 'security-error reduce [kind arg]]]

	make-policy: func [
		; Build the policy tuple used by lower level code.
		target ; "For special cases: eval, memory"
		pol    ; word number or block
		/local n m key flags
	][
		; Special cases: [eval 100000]
		if find [eval memory] target [
			assert-policy number? pol target pol
			limit-usage target pol ; pol is a number here
			return 3.3.3 ; always quit
		]
		; The set all case: [file allow]
		if word? pol [
			n: find acts pol
			assert-policy n target pol
			return 1.1.1 * indexz? n
		]
		; Detailed case: [file [allow read throw write]]
		key: case [
			file? :target ['file]
			url?  :target ['net ]
			true [:target]
		]
		if block? flags: pol-obj/:key [flags: select flags key]

		assert-policy block? pol target pol
		foreach [act perm] pol [
			n: find acts act
			assert-policy n target act
			m: select [read 1 write 2 execute 3] perm
			assert-policy m target perm
			flags/:m: indexz? n
		]
		flags
	]

	set-policy: func [
		; Set the policy as tuple or block:
		target
		pol
		/local val old
	][
		case [
			file? target [
				;; convert to absolute file
				val: to-real-file target
				target: 'file
			]
			url? target [
				val: target
				target: 'net
			]
		]
		old: select pol-obj target
		assert-policy old target pol
		either val [
			; Convert tuple to block if needed:
			if tuple? old [old: reduce [target old]]
			remove/part find old val 2  ; can be in list only once
			append old reduce [val pol]
		][
			old: pol
		]
		pol-obj/:target: old
	]

	word-policy: func [pol /local blk n][
		; Convert lower-level policy tuples to words:
		if all [pol/1 = pol/2 pol/2 = pol/3][
			return pick acts 1 + pol/1
		]
		blk: make block! 4
		n: 1
		foreach act [read write execute] [
			repend blk [pick acts 1 + pol/:n act]
			++ n
		]
		blk
	]

	system/state/confirm-policy: confirm-policy: func [
		policy [word!]    ;; one of: [file net eval memory secure protect debug envr call browse extension]
		mode   [integer!] ;; 1=read 2=write 3=execute
		value ;; e.g. file path or URL
		/local lines mode-name label action response
	][
		unless tty? [return false] ;; Deny automatically if not TTY (no terminal input possible)
		lines: 4
		;print ["policy:" mold policy "mode:" mold mode "value:" mold value]

		print "^/^[[91m╔══ Security Request ════════════════════════════╴^[[m"
		if system/options/script [
			++ lines
			print ajoin ["^[[91m╟── Script :^[[m " system/options/script]
		]
		mode-name: pick ["READ" "WRITE" "EXECUTE"] mode

		label: if :value [switch policy [
			file
			extension ["Path   "]
			net       ["Host   "]
			protect
			envr      ["Target "]
			call      ["Command"]
			browse    ["URL    "]
			secure    ["Policy "]
		]]
		action: switch/default policy [
			file      [reform [mode-name either dir? :value ['directory]['file]]]
			extension ["IMPORT a native extension"]
			net       ["CONNECT"]
			envr      [reform [mode-name "environment variable"]]
			call      ["EXECUTE an external program"]
			browse    ["BROWSE open"]
			secure    ["MODIFY security policy"]
			protect   [pick [UNPROTECT PROTECT] mode]
		][  reform [mode-name policy]]
		print ajoin ["^[[91m╟── Action :^[[m " action]

		if label [
			++ lines
			print ajoin ["^[[91m╟── " label ":^[[m " either file? :value [to-local-file value][form value]]
		]
		prin  {^[[91m║^/╚══ Allow?  ^[[m No / Yes / Once / Quit (default: no)}
		until [
			find "ynqo^M^C" response: read-key
		]
		;; clear the confirmati^/on dialog after user's answer
		prin "^M^[[K" loop lines [prin "^[[1A^[[K"]
		;; resolve user's response
		switch response [
			#"y" [
				check: off
				case [
					all [policy = 'file file? value][
						secure (compose/deep [(value) [
							allow (pick [read write execute] mode)
						]])
					]
					all [policy = 'net url? value][
						secure (compose [(value) allow])
					]
				]
				check: on
				true
			]
			#"o" [ true ]
			#"n" #"^M" [false]
			#"q" #"^C" [quit/return 101]
		]
	]
]

unless system/options/flags/secure-min [
	; Remove all other access to the policies:
	protect/hide in system/state 'policies
	protect/hide in system/state 'confirm-policy
]

protect-system: func [
	"Protect the system object and selected sub-objects."
][
	protect/words system
	protect 'system ; to make sure, that lib/system is always available (user channot unset it)

	"full protection:"
	protect/words/deep [
		system/build
		system/catalog
		;system/standard
		;system/dialects
		;system/intrinsic
	]

	"mild protection:"
	protect/words [
		system/standard
		system/license
		system/contexts
		system/user
	]

	unprotect/values [
		system/options ; some are modified by scripts
		system/catalog/file-types
	]
	unprotect/words [
		system/script
		;system/schemes
		;system/ports   ; should not be modified, fix this
		;system/view    ; should not be modified!
	]
	;; remove itself
	unset in lib 'protect-system
	unset in lib 'limit-usage
]
