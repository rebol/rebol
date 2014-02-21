REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Base: Function Constructors"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Note: {
		This code is evaluated just after actions, natives, sysobj, and other lower
		levels definitions. This file intializes a minimal working environment
		that is used for the rest of the boot.
	}
]

;-- Reserve FUNC word early in the system context, but make it error if you
;-- try to invoke the copying-body semantics until we run %mezz-tail.r
func: make function! [[spec body] [
	print "FUNC primitive used in Mezzanine, use FUNC-BOOT!"
	print mold spec
	print mold body
	exit
]]

func-boot: make function! [[
	;
	; The boot function generator does not copy the body.  This makes it not
	; suitable for situations like:
	;
	;     body: [
	;         value: {}
	;         append value {text}
	;         return value
	;     ]
	; 
	;     x: func-boot [] body
	;     insert body [print "Entering y function"]
	;     y: func-boot [] body
	;
	;     x
	;     y
	;
	; Since the func-boot generator does not copy the body, this would mean the
	; output would be:
	;
	;     Entering y function
	;     text
	;     Entering y function
	;     texttext
	;
	; The Mezzanine loads faster by using the non-copying version, since it
	; does not run into this situation.  We don't want to expose that to the
	; average user, however, so after the Mezzanine hits %mezz-tail.r we
	; define it to cause an error.
	;
	{Non-copying function constructor (optimized for boot).}
	spec [block!] {Help string (opt) followed by arg words (and opt type and string)}
	body [block!] {The body block of the function}
][
	make function! protect/deep reduce [spec body]
]]

does: func-boot [
	{A shortcut to define a function that has no arguments or locals.}
	body [block!] {The body block of the function}
][
	make function! copy/deep reduce [[] body]
]

use: func-boot [
	{Defines words local to a block.}
	vars [block! word!] {Local word(s) to the block}
	body [block!] {Block to evaluate}
][ ; !!Needs the R3 equivalent of the [throw] function attribute in the created closure!
	apply make closure! reduce [to block! vars copy/deep body] []
]

object: func-boot [
	{Defines a unique object.}
	blk [block!] {Object words and values (modified)}
][
	make object! append blk none
]

module: func-boot [
	"Creates a new module."
	spec [block!] "The header block of the module (modified)"
	body [block!] "The body block of the module (modified)"
	/mixin "Mix in words from other modules"
	words [object!] "Words collected into an object"
][
	make module! unbind/deep reduce pick [[spec body] [spec body words]] not mixin
]

cause-error: func-boot [
	"Causes an immediate error throw with the provided information."
	err-type [word!]
	err-id [word!]
	args
][
	; Make sure it's a block:
	args: compose [(:args)]
	; Filter out functional values:
	forall args [
		if any-function? first args [
			change/only args spec-of first args
		]
	]
	; Build and throw the error:
	do make error! [
		type: err-type
		id:   err-id
		arg1: first args
		arg2: second args
		arg3: third args
	]
]

default: func-boot [
	"Set a word to a default value if it hasn't been set yet."
	'word [word! set-word! lit-word!] "The word (use :var for word! values)"
	value "The value" ; unset! not allowed on purpose
][
	unless all [value? word not none? get word] [set word :value] :value
]

secure: func-boot ['d] [boot-print "SECURE is disabled"]

