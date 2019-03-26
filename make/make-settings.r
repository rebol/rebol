REBOL [
	purpose: {User defined building properties}
]

TOOLS-Win32: "x:/MingW/mingw32/bin/"
TOOLS-Win64: "x:/MingW/mingw64/bin/"

;@@ If you add or remove some of these defines bellow, you should `make clean`
;@@ to make sure that all affected files will be recompiled!
Defines: [
	USE_BMP_CODEC
	USE_PNG_CODEC
	USE_GIF_CODEC
	USE_JPG_CODEC
	;USE_WAV_CODEC   ;-- deprecated; using Rebol codec instead
	;USE_NO_INFINITY ;-- use when you don't want to support IEEE infinity
	USE_LZMA         ;-- adds support for LZMA [de]compression
	USE_MIDI_DEVICE  ;-- includes MIDI device when possible (Windows & macOS)

	;@@ optional fine tuning:
	;DO_NOT_NORMALIZE_MAP_KEYS
	; with above define you would get:
	;	[a b:]     = words-of make map! [a 1 b: 2]
	;	[a 1 b: 2] = body-of  make map! [a 1 b: 2]
	;
	; else:
	;	[a b]       = words-of make map! [a 1 b: 2]
	;	[a: 1 b: 2] = body-of  make map! [a 1 b: 2]

	;USE_EMPTY_HASH_AS_NONE ;-- A single # means NONE, else error; Used in l-scan.c file
	;FORCE_ANSI_ESC_EMULATION_ON_WINDOWS ;-- would not try to use MS' built-in VIRTUAL_TERMINAL_PROCESSING
	;EXCLUDE_VECTOR_MATH ;-- don't include vector math support (like: 3 * #[vector! integer! 8 3 [1 2 3]])

	;DEBUG_MIDI ;-- prints some of internal traces from MIDI device handler

]
