REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Backwards ordinals for old versions"
	Rights: {
		Copyright 2014 Brian Hawley
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Brian Hawley"
]

unless any-function? get/any 'first-back [

	; Define backwards ordinals missing from old versions
	either 'pick-has-0 = pick tail [pick-has-0 pick-no-0] -1 [
		first-back: func ["Returns the first value back in a series." value] [pick :value 0]
		second-back: func ["Returns the second value back in a series." value] [pick :value -1]
		third-back: func ["Returns the third value back in a series." value] [pick :value -2]
		fourth-back: func ["Returns the fourth value back in a series." value] [pick :value -3]
		fifth-back: func ["Returns the fifth value back in a series." value] [pick :value -4]
		sixth-back: func ["Returns the sixth value back in a series." value] [pick :value -5]
		seventh-back: func ["Returns the seventh value back in a series." value] [pick :value -6]
		eighth-back: func ["Returns the eighth value back in a series." value] [pick :value -7]
		ninth-back: func ["Returns the ninth value back in a series." value] [pick :value -8]
		tenth-back: func ["Returns the tenth value back in a series." value] [pick :value -9]
	] [
		first-back: func ["Returns the first value back in a series." value] [pick :value -1]
		second-back: func ["Returns the second value back in a series." value] [pick :value -2]
		third-back: func ["Returns the third value back in a series." value] [pick :value -3]
		fourth-back: func ["Returns the fourth value back in a series." value] [pick :value -4]
		fifth-back: func ["Returns the fifth value back in a series." value] [pick :value -5]
		sixth-back: func ["Returns the sixth value back in a series." value] [pick :value -6]
		seventh-back: func ["Returns the seventh value back in a series." value] [pick :value -7]
		eighth-back: func ["Returns the eighth value back in a series." value] [pick :value -8]
		ninth-back: func ["Returns the ninth value back in a series." value] [pick :value -9]
		tenth-back: func ["Returns the tenth value back in a series." value] [pick :value -10]
	]

]
