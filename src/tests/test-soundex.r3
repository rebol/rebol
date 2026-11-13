Rebol [
	Title:    "Test Soundex function"
	Date:     16-Jul-2024
	Author:   "Oldes"
	File:     %test-soundex.r3
	Version:  1.0.0
]
import 'soundex
use [tmp][
	tmp: none
	foreach [code name] [
		"R163" "Robert"
		"R163" "Rupert"
		"R150" "Rubin"
		"A226" "Ashcraft"
		"A226" "Ashcroft"
		"T522" "Tymczak" ;; the chars 'z' and 'k' in the name are coded as 2 twice since a vowel lies in between them
		"P236" "Pfister"
		"H555" "Honeyman"
	][
		printf [5 9 5] [code name tmp: soundex name code == tmp]
	]
]