REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Infix operator symbol definitions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		This table maps infix operator symbols to function names.
	}
]

+  add
-  subtract
*  multiply
/  divide
// remainder
** power
=  equal?
=? same?
== strict-equal?
!= not-equal?
<> not-equal?
!== strict-not-equal?
<  lesser?
<= lesser-or-equal?
>  greater?
>= greater-or-equal?
& and~
|  or~
and and~
or  or~
xor xor~
