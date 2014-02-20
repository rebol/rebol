REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Make sys-zlib.h and u-zlib.c"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		In order to limit build dependencies, Rebol makes a snapshot of a
		subset of certain libraries to include in the source distribution.
		This script will extract just the parts of ZLIB that Rebol needs
		to put into one .h file and one .c file.  It generates its
		snapshot from their official source repository:

		    https://github.com/madler/zlib

		Any significant reorganization of the ZLIB codebase would require
		updating this script accordingly.  It was last tested on 1.2.8
		(released 28-Apr-2013)
	}
	Note: [
		"This runs relative to ../tools directory."
	]
]


;
; Target paths+filenames for the generated include and source file
;
path-include: %../../src/include/
file-include: %sys-zlib.h
path-source: %../../src/core/
file-source: %u-zlib.c


;
; In theory this should be able to run against the network to get the
; source data from the Git raw format of the latest version.  But until
; https is in Rebol core, github is not an option; you have to clone
; locally and reset this path to wherever you put it.
; 
path-zlib: https://raw.github.com/madler/zlib/master/


;
; Disable #include "foo.h" style inclusions (but not #include <foo.h> style)
; Optionally will inline a list of files at the inclusion point
;
disable-user-includes: func [
	lines [block!] {Block of strings}
	/inline headers [block!] {Block of filenames to inline if seen}
	/local name line-iter line pos
] [
	line-iter: lines
	while [line-iter <> tail lines] [
		line: first line-iter
		if parse line [any space {#} any space {include} some space {"} copy name to {"} to end] [
			either all [
				inline
				pos: find headers to file! name
			] [
				change/part line-iter (read/lines rejoin [path-zlib name]) 1 
				take pos
			] [
				insert line rejoin [{//} space]
				append line rejoin [
					space {/* REBOL: see make-zlib.r */}
				]
			]
		] 
		line-iter: next line-iter
	]

	; If we inline a header, it should happen once and only once for each
	unless empty? headers [
		throw make error! rejoin [{Not all headers inlined by make-zlib:} space mold headers]  
	]
]


;
; Stern warning not to edit the files
;

make-warning-lines: func [name [file!] title [string!]] [
	reduce [
		{/***********************************************************************}
		{**}
		{**  Extraction of ZLIB compression and decompression routines} 
		{**  for REBOL [R3] Language Interpreter and Run-time Environment}
		{**  This is a code-generated file.}
		{**}
		{**  ZLIB Copyright notice:}
		{**}
		{**    (C) 1995-2013 Jean-loup Gailly and Mark Adler}
		{**}
		{**    This software is provided 'as-is', without any express or implied}
		{**    warranty.  In no event will the authors be held liable for any damages}
		{**    arising from the use of this software.}
		{**}
		{**    Permission is granted to anyone to use this software for any purpose,}
		{**    including commercial applications, and to alter it and redistribute it}
		{**    freely, subject to the following restrictions:}
		{**}
		{**    1. The origin of this software must not be misrepresented; you must not}
		{**       claim that you wrote the original software. If you use this software}
		{**       in a product, an acknowledgment in the product documentation would be}
		{**       appreciated but is not required.}
		{**    2. Altered source versions must be plainly marked as such, and must not be}
		{**       misrepresented as being the original software.}
		{**    3. This notice may not be removed or altered from any source distribution.}
		{**}
		{**        Jean-loup Gailly        Mark Adler}
		{**        jloup@gzip.org          madler@alumni.caltech.edu}
		{**}
		{**  REBOL is a trademark of REBOL Technologies}
		{**  Licensed under the Apache License, Version 2.0}
		{**}
		{************************************************************************}
		{**}
		rejoin [{**  Title: } title]
		{**  Build: A0}
		rejoin [{**  Date:  } now/date]
		rejoin [{**  File:  } to string! name]
		{**}
		{**  AUTO-GENERATED FILE - Do not modify. (From: make-zlib.r)}
		{**}
		{***********************************************************************/}
	]
]


;;
;; Generate %sys-zlib.h Aggregate Header File
;;

header-lines: copy []

foreach h-file [
	%zconf.h
	%zutil.h
	%zlib.h
	%deflate.h
] [
	append header-lines read/lines rejoin [path-zlib h-file]
]

disable-user-includes header-lines

insert header-lines [
	{}
	{#define NO_DUMMY_DECL 1}
	{#define Z_PREFIX 1}
	{}
]

insert header-lines make-warning-lines file-include {ZLIB aggregated header file}

write/lines rejoin [path-include file-include] header-lines



;;
;; Generate %u-zlib.c Aggregate Source File
;;

source-lines: copy []

append source-lines read/lines rejoin [path-zlib %crc32.c]

; 
; Macros DO1 and DO8 are defined differently in crc32.c, and if you don't #undef
; them you'll get a redefinition warning.
;
append source-lines [
	{#undef DO1 /* REBOL: see make-zlib.r */}
	{#undef DO8 /* REBOL: see make-zlib.r */}
]

foreach c-file [
	%adler32.c

	%deflate.c
	%zutil.c
	%compress.c
	%uncompr.c
	%trees.c

	%inftrees.h
	%inftrees.c
	%inffast.h
	%inflate.h
	%inffast.c
	%inflate.c
] [
	append source-lines read/lines rejoin [path-zlib c-file]
]

disable-user-includes/inline source-lines [%trees.h %inffixed.h %crc32.h]

insert source-lines [
	{}
	{#include "sys-zlib.h" /* REBOL: see make-zlib.r */}
	{}
]

insert source-lines make-warning-lines file-source {ZLIB aggregated source file}

write/lines rejoin [path-source file-source] source-lines
