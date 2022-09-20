REBOL [
    System: "REBOL [R3] Language Interpreter and Run-time Environment"
    Title: "Make sys-zlib.h and u-zlib.c"
    Rights: {
        Copyright 2012-2022 Ren-C Open Source Contributors
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
        updating this script accordingly.  It was last tested on 1.2.11
        (released 15-Jan-2017)
    }
    Note: {
        "This runs relative to ../tools directory."

        !!! TBD: The `register` keyword has been deprecated.  If zlib doesn't
        remove it itself, then on the next import the by-hand removals will
        have to be repeated -or- this script will need to be updated to get
        rid of them (note `register` is used in comments too):

        https://stackoverflow.com/a/30809775
    }
    Author: []
]

do %c-lexicals.reb

;
; Target paths+filenames for the generated include and source file
;
path-include: %../../src/include/
file-include: %sys-zlib.h
path-source: %../../src/core/
file-source: %u-zlib.c

_: #[none]

path-zlib: %/b\tree\utility\zlib\ ;https://raw.githubusercontent.com/madler/zlib/master/

;
; Disable #include "foo.h" style inclusions (but not #include <foo.h> style)
; Optionally will inline a list of files at the inclusion point
;
disable-user-includes: function [
    lines [block!] {Block of strings}
    /inline files [block!] {Block of filenames to inline if seen}
    /stdio {Disable stdio.h}
] [
    open-include:  charset {"<}
    close-include: charset {">}

    include-rule: compose [
        ((if stdio [
            [open-include copy name "stdio.h" close-include |]
        ]))
        {"} copy name to {"}
    ]

    forall lines [
        if parse line: lines/1 [
            any space {#}
            any space {include}
            some space include-rule to end
        ][
            either pos: find files (as file! name) [
                change/part lines (read/lines path-zlib/:name) 1 
                take pos
            ][
                insert line ajoin [{//} space]
                append line ajoin [
                    space {/* REBOL: see make-zlib.r */}
                ]
            ]
        ] 
    ]

    if all [inline not empty? files] [
        print as-red [
            {Not all headers inlined by make-zlib:} (mold headers) LF
            {If we inline a header, should happen once and only once for each}
        ]
        halt
    ]
]

;
; Stern warning not to edit the files
;

make-warning-lines: func [name [file!] title [string!]] [
    reduce [
        {//}
        {// Extraction of ZLIB compression and decompression routines} 
        {// for REBOL [R3] Language Interpreter and Run-time Environment}
        {// This is a code-generated file.}
        {//}
        {// ZLIB Copyright notice:}
        {//}
        {//   (C) 1995-2022 Jean-loup Gailly and Mark Adler}
        {//}
        {//   This software is provided 'as-is', without any express or implied}
        {//   warranty.  In no event will the authors be held liable for any damages}
        {//   arising from the use of this software.}
        {//}
        {//   Permission is granted to anyone to use this software for any purpose,}
        {//   including commercial applications, and to alter it and redistribute it}
        {//   freely, subject to the following restrictions:}
        {//}
        {//   1. The origin of this software must not be misrepresented; you must not}
        {//      claim that you wrote the original software. If you use this software}
        {//      in a product, an acknowledgment in the product documentation would be}
        {//      appreciated but is not required.}
        {//   2. Altered source versions must be plainly marked as such, and must not be}
        {//      misrepresented as being the original software.}
        {//   3. This notice may not be removed or altered from any source distribution.}
        {//}
        {//       Jean-loup Gailly        Mark Adler}
        {//       jloup@gzip.org          madler@alumni.caltech.edu}
        {//}
        {// REBOL is a trademark of REBOL Technologies}
        {// Licensed under the Apache License, Version 2.0}
        {//}
        {// **********************************************************************}
        {//}
        ajoin [{// Title: } title]
        ajoin [{// Date:  } now/date]
        ajoin [{// File:  } name]
        {//}
        {// AUTO-GENERATED FILE - Do not modify. (From: make-zlib.r)}
        {//}
    ]
]

fix-kr: function [
    "Fix K&R style C function definition"
    source
][
    single-param: bind [
        identifier ;(part of)type
        some [
            any white-space
            any [#"*" any white-space]

            ; It could get here even after last identifier, so this tmp-start
            ; is not the begining of the name, but the last one is...
            ;
            tmp-start: copy name:
            identifier (
                name-start: tmp-start
            )
            any white-space
            any [#"*" any white-space]
        ]
    ] c-lexical/grammar

    parse source bind [
        while [
            copy fn: identifier
            any white-space
            #"(" open-paren: to #")" close-paren: #")"
            param-ser: copy param-spec: [
                some [
                    some [any white-space any [#"*" any white-space]
                        identifier any white-space opt #","
                        any [#"*" any white-space]
                    ] #";"
                ]
                any white-space
            ]
            #"^{" check-point: (
                ;print ["func:" to string! fn]
                remove/part param-ser length? param-spec
                insert param-ser "^/"
                length-diff: 1 - (length? param-spec)

                param-len: (index? close-paren) - (index? open-paren)
                params: copy/part open-paren param-len
                remove/part open-paren param-len
                length-diff: length-diff - param-len


                param-block: make block! 8
                parse params [
                    any white-space
                    copy name: identifier (
                        append param-block reduce [name _]
                    )
                    any [
                        any white-space
                        #","
                        any white-space
                        copy name: identifier (
                            append param-block reduce [name _]
                        )
                    ]
                    end
                ]

                ;dump param-block

                ; a param spec could be in the form of:
                ; 1) "int i;" or
                ; 2) "int i, *j, **k;"
                ;dump param-spec
                parse param-spec [
                    any white-space
                    some [
                        (typed?: true)
                        single-param-start: single-param (
                            spec-type: (
                                copy/part single-param-start
                                    (index? name-start)
                                    - (index? single-param-start)
                            )
                           ;dump spec-type
                       )
                       any [
                           any white-space
                           param-end: #"," (
                                ; case 2)
                                ; spec-type should be "int "
                                ; name should be "i"
                                poke (find/skip param-block name 2) 2
                                    either typed? [
                                        (copy/part single-param-start
                                            (index? param-end)
                                            - (index? single-param-start)
                                        )
                                    ][
                                    ; handling "j" in case 2)
                                        ajoin [
                                            spec-type    ; "int "
                                            (copy/part single-param-start
                                                (index? param-end)
                                                - (index? single-param-start)
                                            ) ; " *j"
                                       ]
                                   ]
                                   typed?: false
                           )
                           single-param-start:
                           any white-space
                           any [#"*" any white-space]
                           copy name identifier
                       ]
                       any white-space
                       param-end: #";"
                       (
                           poke (find/skip param-block name 2) 2
                               either typed? [
                                   (copy/part single-param-start
                                        (index? param-end)
                                        - (index? single-param-start)
                                    )
                               ][
                                   ; handling "k" in case 2)
                                   ajoin [
                                       spec-type    ; "int "
                                       (copy/part single-param-start
                                            (index? param-end)
                                            - (index? single-param-start)
                                       ) ; " **k"
                                   ]
                               ]
                           )
                       any white-space
                    ]
                ]

                insert open-paren new-param: combine/with (
                    extract/index param-block 2 2
                ) ",^/    "
                insert open-paren "^/    "

                length-diff: length-diff + length? new-param

                check-point: skip check-point length-diff
            )
            :check-point
            | skip
        ]
        end
    ] c-lexical/grammar

    source
]

fix-const-char: func [
    source
][
    parse source bind [
        while [
            "strm" any white-space "->" any white-space
            "msg" any white-space "=" any white-space
            "(" any white-space change "char" "z_const char"
                any white-space "*" any white-space ")"
            | skip
        ]
        end
    ] c-lexical/grammar
    source
]

;do %common.r

;
; Generate %sys-zlib.h Aggregate Header File
;

header-lines: copy []

foreach h-file [
    %zconf.h
    %zutil.h
    %zlib.h
    %deflate.h
] [
    append header-lines read/lines path-zlib/:h-file
]

disable-user-includes header-lines

insert header-lines [
    {}
    {// Rebol}
    {#define NO_DUMMY_DECL 1}
    {#define Z_PREFIX 1}
    {#define ZLIB_CONST}
    {// **********************************************************************}
    {}
]

insert header-lines make-warning-lines file-include {ZLIB aggregated header}

write/lines path-include/:file-include header-lines



;
; Generate %u-zlib.c Aggregate Source File
;

source-lines: copy []

append source-lines read/lines path-zlib/crc32.c

; 
; Macros DO1 and DO8 are defined differently in crc32.c, and if you don't
; #undef them you'll get a redefinition warning.
;
append source-lines [
    {#undef DO1  /* REBOL: see make-zlib.r */}
    {#undef DO8  /* REBOL: see make-zlib.r */}
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
][
    append source-lines read/lines path-zlib/:c-file
]

disable-user-includes/stdio/inline source-lines copy [
    %trees.h
    %inffixed.h
    %crc32.h
]

insert source-lines [
    {}
    {#include "sys-zlib.h"  /* REBOL: see make-zlib.r */}
    {#define local static}
    {}
]

insert source-lines make-warning-lines file-source {ZLIB aggregated source}

;all-source: make string! 200'000
;forall source-lines [append append all-source source-lines/1 LF]
;;write rejoin [path-source file-source] fix-const-char fix-kr all-source
;write path-source/:file-source fix-const-char all-source

write/lines path-source/:file-source source-lines
