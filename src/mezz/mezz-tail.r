REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: End of Mezz"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

;-- Remove FUNC-BOOT from client use.
;-- (Note: experts who need this kind of control with their own function 
;-- generators can use MAKE FUNCTION!, which does not copy.)
func-boot: func [
	{Internal function generator (not for client use)}
	spec body
] [
    print "FUNC-BOOT should only be used in Mezzanine code"
    probe spec
    probe body
    exit
]


; MOVE THIS INTERNAL FUNCTION!
dump-obj: func [
	"Returns a block of information about an object or port."
	obj [object! port!]
	/match "Include only those that match a string or datatype" pat
	/local str val out 
][
	clip-str: func-boot [str] [
		; Keep string to one line.
		trim/lines str
		if (length? str) > 45 [str: append copy/part str 45 "..."]
		str
	]

	form-val: func-boot [val] [
		; Form a limited string from the value provided.
		if any-block? :val [return reform ["length:" length? val]]
		if image? :val [return reform ["size:" val/size]]
		if datatype? :val [return get in spec-of val 'title]
		if any-function? :val [
			return clip-str any [title-of :val mold spec-of :val]
		]
		if object? :val [val: words-of val]
		if typeset? :val [val: to-block val]
		if port? :val [val: reduce [val/spec/title val/spec/ref]]
		if gob? :val [return reform ["offset:" val/offset "size:" val/size]]
		clip-str mold :val
	]

	form-pad: func-boot [val size] [
		; Form a value with fixed size (space padding follows).
		val: form val
		insert/dup tail val #" " size - length? val
		val
	]

	; Search for matching strings:
	out: copy []
	wild: all [string? pat  find pat "*"]

	foreach [word val] obj [
		type: type?/word :val
		str: either find [function! closure! native! action! op! object!] type [
			reform [word mold spec-of :val words-of :val]
		][
			form word
		]
		if any [
			not match
			all [
				not unset? :val
				either string? :pat [
					either wild [
						tail? any [find/any/match str pat pat]
					][
						find str pat
					]
				][
					all [
						datatype? get :pat
						type = :pat
					]
				]
			]
		][
			str: form-pad word 15
			append str #" "
			append str form-pad type 10 - ((length? str) - 15)
			append out reform [
				"  " str
				if type <> 'unset! [form-val :val]
				newline
			]
		]
	]
	out
]


; Quick test runner (temporary):
t: does [do %test.r]

