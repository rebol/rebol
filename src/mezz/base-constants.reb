REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Base: Constants and Equates"
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

; NOTE: The system is not fully booted at this point, so only simple
; expressions are allowed. Anything else will crash the boot.

;-- Standard constants:
on:  true
off: false
yes: true
no:  false
zero: 0

;-- Special values:
REBOL: system
sys: system/contexts/sys
lib: system/contexts/lib

;-- Char constants:
null:      #"^@"
space:     #" "
sp:        space
backspace: #"^H"
bs:        backspace
tab:       #"^-"
newline:   #"^/"
newpage:   #"^l"
slash:     #"/"
backslash: #"\"
escape:    #"^["
cr:        #"^M"
lf:        newline
crlf:      "^M^J"
comma:     #","
dot:       #"."
dbl-quote: #"^""

;-- Bitset constants
whitespace: #(bitset! #{0064000080}) ;= charset [#" " #"^-" #"^/" #"^M"]

;-- Function synonyms:
q: :quit
!: :not
min: :minimum
max: :maximum
abs: :absolute
empty?: :tail?
---: :comment
;bind?: :bound? ;@@ https://github.com/Oldes/Rebol-issues/issues/2440

rebol.com: http://www.rebol.com
