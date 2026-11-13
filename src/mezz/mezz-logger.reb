REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2025 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Title:  "Log-related functions"
	Name:    logger
	Version: 1.0.0
	Exports: [log-error log-warn log-info log-debug log-trace]
	type: module
]

timestamp: none ;or e.g.: does[ajoin ["^[[90m" pad/with to decimal! now/precise 15 #"0"]]
verbosity: 3 ;; default verbosity level (when not set in the system/options/log)
log-levels: system/options/log
emit: :print

ansi: system/options/ansi

log-error: function[
	"Outputs critical issues and error messages (always visible)"
	'id [any-word!]	message
][
	message: trim/head/tail form either block? message [reduce message][message]
	foreach line split-lines message [
		emit ajoin [
			timestamp
			SP ansi/magenta #"[" id "] " ansi/bright-magenta
			either line/1 = #"*" []["** Error: "]
			copy/part line 200 ;@@ I am not sure with this line length limit
			ansi/reset
		]
	]
]
log-warn: function[
	"Outputs potential problem messages (always visible unless the quiet option is set)"
	'id [any-word!] message
][
	if system/options/quiet [exit]
	emit ajoin [
		SP ansi/bright-yellow #"[" id "] " ansi/red
		either block? message [reduce :message][message]
		ansi/reset
	]
]
log-info: function[
	"Outputs general information messages (visible for verbosity level 1 and higher)"
	'id [any-word!] message
][
	if any [
		system/options/quiet
		1 > any [select log-levels id verbosity]
	] [ exit ]
	emit ajoin [
		SP ansi/bright-yellow #"[" id "] " ansi/bright-cyan
		either block? message [reduce :message][message]
		ansi/reset
	]
]
log-debug: function[
	"Outputs low-level debugging messages (visible for verbosity level 2 and higher)"
	'id [any-word!] message
][
	if any [
		system/options/quiet
		2 > any [select log-levels id verbosity]
	] [ exit ]
	emit ajoin [
		SP ansi/yellow #"[" id "] " ansi/cyan
		either block? message [reduce :message][message]
		ansi/reset
	]
]
log-trace: function[
	"Outputs extremely granular debugging messages (visible for verbosity level 3 and higher)"
	'id [any-word!] message
][
	if any [
		system/options/quiet
		3 > any [select log-levels id verbosity]
	] [ exit ]
	emit ajoin [
		SP ansi/yellow #"[" id "] " ansi/green
		either block? message [reduce :message][message]
		ansi/reset
	]
]

;; This function replaces the one from the system level, which may be used on boot!
sys/log: func [
	"Outputs a debug message (backward compatibility version)"
	"Use one of log-* functions instead!"
	'id [any-word!] message
	/error "critical issues and error messages"	
	/info  "general information messages"
	/more  "low-level debugging messages"
	/debug "extremely granular debugging messages"
][
	case [
		error [ log-error (id) :message ]
		info  [ log-info  (id) :message ]
		more  [ log-debug (id) :message ]
		debug [ log-trace (id) :message ]
		true  [ log-warn  (id) :message ]
	]
]

protect/words/lock 'log-levels

