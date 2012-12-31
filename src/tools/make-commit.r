REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Pre-commit consistency checks."
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		Run pre-commit tests on the codebase for any violations of coding standards.
		This includes cosmetic issues like spaces instead of tabs.
	}
	Note: [
		"This runs relative to ../tools directory."
	]
]


; While the coding style and checkin rules should be documented at length in a wiki
; somewhere, the "success" message should evolve to bring to mind whatever mistakes
; turn out to be the most common that the tests don't catch

success-message: {
The source files passed what the pre-commit suite currently tests for!  But also be
sure that:

* A clean build (make clean, make make, make prep, make) succeeds with zero
  compiler warnings

* You tested your changes on your operating platform to make sure they do what you think

* You have considered the impact of your changes on platforms you have NOT tested them
  on.  If it seems likely that impact will be there, then make sure either you (or
  someone else) does those tests on your branch before you send a pull request to
  the master.
}


; The failure message shouldn't have to say much, because the alerts delivered by the
; check should speak for themselves...

failure-message: {
The source files did not pass what the pre-commit suite currently tests for.  Please
fix these issues before submitting a pull request for your commit!

(Alternatively: if you believe the tests *themselves* are in error, take a look in
%/src/tools/make-commit.r and present a suggested modification to the tests along
with your grievance.)
}


; make-make.r uses the information in file-base.r
; however this needs to check header files and other things not mentioned there
; so easiest for now is to just enumerate directories looking for [.c .r .h]

; BUT in case people have some miscellaneous directories not related to the
; current Rebol /src/ files, we only scan the "known" toplevel ones in the repository

toplevel-dirs: [%boot/ %core/ %include/ %mezz/ %os/ %tools/]


;
; Global variables
;

num-checked-files: 0
num-problem-files: 0


;
; Worker functions
;

check-basic-indent: func [lines [block!]] [
	errors: copy []
	line-number: 0

	foreach line lines [
		++ line-number

		if empty? line [continue]

		whitespace: charset reduce [space tab]
		non-whitespace: complement whitespace

		unless parse line [any tab non-whitespace to end] [
			append/only errors rejoin [
				line-number {:} space {Indentation of line not done with pure tabs.}
			]
		]
	]

	return errors
]

check-embedded-documentation: func [lines [block!]] [
	errors: copy []

	; just a stub for the moment...this has an indentation convention also
	; See: http://www.rebol.com/article/0515.html

	; (one could argue the embedded documentation script should be the place where bad
	; whitespace is caught.  But then again, complaining about the whitespace on every
	; edit (even possibly an intermediate one) could create a beaurocratic headache.
	; Such checks may be better here, although it creates more work to "find them twice")

	return errors
]

check-file: func [file [file!]] [
	unless parse file [
		thru [
			%.r (type: 'REBOL) |
			%.c (type: 'C-SOURCE) |
			%.h (type: 'C-HEADER)
		]
		end
	] [
		return
	]

	lines: read/lines file
	errors: copy []

	switch type [
		REBOL [
			append errors (check-basic-indent lines)
		]

		C-SOURCE [
			append errors (check-basic-indent lines)
			append errors (check-embedded-documentation lines)
		]

		C-HEADER [
			append errors (check-basic-indent lines)
			append errors (check-embedded-documentation lines)
		]
	]

	if not empty? errors [
		print rejoin [lf mold file { - } type]
		foreach error errors [
			print error
		]

		++ num-problem-files
	]


	++ num-checked-files
]

check-directory: func [dir [file!]] [
	foreach entry load dir [
		either dir? entry [
			; REVIEW: catch recursive symlinks, if someone happened to make them?
			check-directory rejoin [dir entry]
		] [
			check-file rejoin [dir entry]
		]
	]
]


;
; Entry point
;

print "Performing Pre-Commit Tests"

foreach dir toplevel-dirs [
	check-directory rejoin [%../ dir]
]

print rejoin [{Number of files checked: } num-checked-files]
print rejoin [{Number of files with problems: } num-problem-files]

print either 0 == num-problem-files [success-message] [failure-message]
