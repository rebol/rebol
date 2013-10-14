REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Workarounds to allow old Rebol builds to bootstrap"
	Rights: {
		Copyright 2013 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		It is desirable to keep it possible to run the MAKE MAKE and
		MAKE PREP of Rebol using older interpreters than the latest.
		This file bridges across possible changes to natives and
		mezzanines so those steps can keep running despite the 
		code in the %tools/ and %boot/ directoires being written
		against the latest assumptions.
	}
]


;; Newer Rebol interpreters believe JOIN is equivalent to what
;; older interpreters would have called REJOIN.  We can bridge
;; past the incompatibility by checking to see if JOIN takes
;; one or two parameters...and alias it to REJOIN if it takes two

if 2 == length? words-of :join [join: :rejoin]


;; Older Rebol interpreters are not aware that the former 3-argument
;; FUNCTION creator has been deprecated.  Now FUNCTION is the
;; 2-argument locals gathering mezzanine formerly called FUNCT.

if 3 == length? words-of :function [function: :funct]