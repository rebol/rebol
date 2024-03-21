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

funco: :func ; save it for expert usage

; Final FUNC definition:
func: funco [
	{Defines a user function with given spec and body.}
	spec [block!] {Help string (opt) followed by arg words (and opt type and string)}
	body [block!] {The body block of the function}
][
	make function! copy/deep reduce [spec body] ; (now it deep copies)
]

;- some shortcuts (aliases)
~: system/options/home
codecs: :system/codecs 
keys-of: :words-of ; as it sounds better when used with some objects

;; boot (path to the exe) may be none if not resolved!
if system/options/boot [
   system/options/boot: clean-path system/options/boot
]

;protect system/standard
protect-system-object
