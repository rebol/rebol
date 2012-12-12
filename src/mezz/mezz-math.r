REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Math"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

mod: func [
	"Compute a nonnegative remainder of A divided by B."
	; In fact the function tries to find the remainder,
	; that is "almost non-negative"
	; Example: 0.15 - 0.05 - 0.1 // 0.1 is negative,
	; but it is "almost" zero, i.e. "almost non-negative"
	[catch]
	a [number! money! time!]
	b [number! money! time!] "Must be nonzero."
	/local r
] [
	; Compute the smallest non-negative remainder
	all [negative? r: a // b   r: r + b]
	; Use abs a for comparisons
	a: abs a
	; If r is "almost" b (i.e. negligible compared to b), the
	; result will be r - b. Otherwise the result will be r
	either all [a + r = (a + b)  positive? r + r - b] [r - b] [r]
]

modulo: func [
	{Wrapper for MOD that handles errors like REMAINDER. Negligible values (compared to A and B) are rounded to zero.}
	;[catch]
	a [number! money! time!]
	b [number! money! time!] "Absolute value will be used"
	/local r
] [
	; Coerce B to a type compatible with A
	any [number? a  b: make a b]
	; Get the "accurate" MOD value
	r: mod a abs b
	; If the MOD result is "near zero", w.r.t. A and B,
	; return 0--the "expected" result, in human terms.
	; Otherwise, return the result we got from MOD.
	either any [a - r = a   r + b = b] [make r 0] [r]
]

sign?: func [
	"Returns sign of number as 1, 0, or -1 (to use as multiplier)."
	number [number! money! time!]
][
	case [
		positive? number [1]
		negative? number [-1]
		true [0]
	]
]

minimum-of: func [
	{Finds the smallest value in a series}
	series [series!] {Series to search}
	/skip {Treat the series as records of fixed size}
	size [integer!]
	/local spot
][
	size: any [size 1]
	if 1 > size [cause-error 'script 'out-of-range size]
	spot: series
	forskip series size [
		if lesser? first series first spot [spot: series]
	]
	spot
]

maximum-of: func [
	{Finds the largest value in a series}
	series [series!] {Series to search}
	/skip {Treat the series as records of fixed size}
	size [integer!]
	/local spot
][
	size: any [size 1]
	if 1 > size [cause-error 'script 'out-of-range size]
	spot: series
	forskip series size [
		if greater? first series first spot [spot: series]
	]
	spot
]
