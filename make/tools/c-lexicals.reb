REBOL [
    Title: "C Programming Language Lexical Definitions"
    Rights: {
        Copyright 2015 Brett Handley
    }
    License: {
        Licensed under the Apache License, Version 2.0
        See: http://www.apache.org/licenses/LICENSE-2.0
    }
    Author: "Brett Handley"
    Purpose: {Parse C source text into preprocessing tokens.}
    Description: {
        Based upon N1570 Committee Draft - April 12, 2011 ISO/IEC 9899:201x

        Trigraphs are not implemented.

        Do not put any actions (e.g. executable GROUP!s in the PARSE rules)
        in this file.  To use these rules, copy them, call them from your
        own rules or use rule injection to dynamically add emit actions.
    }
]

c-lexical: context [

    grammar: [

        text: [some c-pp-token]

        c-pp-token: [

            white-space | preprocessing-token
        ]

        white-space: [
            nl
            | eol
            | wsp
            | span-comment
            | line-comment
        ]

        ;
        ; -- A.1.1 Lexical Elements
        ; Order is significant

        preprocessing-token: [

            pp-number
            | character-constant
            | identifier
            | string-literal
            | header-name
            | punctuator
            | other-pp-token
        ]

        other-pp-token: not-wsp

        ;
        ; -- A.1.3 Identifiers

        identifier: [id-nondigit any id-char]
        id-nondigit: [nondigit | universal-character-name]

        ;
        ; -- A.1.4 Universal character names

        universal-character-name: [{\U} 2 hex-quad | {\u} hex-quad]
        hex-quad: [4 hexadecimal-digit]

        ;
        ; -- A.1.5 Constants

        character-constant: [
            #"'" some c-char #"'"
            | {L'} some c-char #"'"
            | {u'} some c-char #"'"
            | {U'} some c-char #"'"
        ]

        escape-sequence: [
            simple-escape-sequence
            | octal-escape-sequence
            | hexadecimal-escape-sequence
            | universal-character-name
        ]

        simple-escape-sequence: [
            {\'} | {\"} | {\?} | {\\}
            | {\a} | {\b} | {\f} | {\n} | {\r} | {\t} | {\v}
        ]

        hexadecimal-escape-sequence: [
            {\x} hexadecimal-digit any hexadecimal-digit
        ]

        octal-escape-sequence: [#"\" 1 3 octal-digit]

        ;
        ; -- A.1.6 String literals

        string-literal: [
            opt encoding-prefix #"^"" any s-char #"^""
        ]
        encoding-prefix: [{u8} | #"L" | #"u" | #"U"]
        s-char: [s-char-cs | escape-sequence]

        ;
        ; -- A.1.7 Punctuators

        punctuator: [
            {->} | {++} | {--} | {<<} | {>>} | {<=} | {>=} | {==} | {!=}
            | {&&} | {||} | {...} | {*=} | {/=} | {%=} | {+=} | {<<=} | {>>=}
            | {&=} | {^^=} | {|=} | {##} | {<:} | {:>} | {<%} | {%>}
            | {%:%:} | {%:}
            | p-char
        ]

        ;
        ; -- A.1.8 Header names

        header-name: [#"<" some h-char #">" | #"^"" some q-char #"^""]

        ;
        ; -- A.1.9 Preprocessing numbers

        pp-number: [
            [digit | #"." digit]
            any [
                digit
                | id-nondigit
                | #"."
                | [#"e" | #"p" | #"E" | #"P"] sign
            ]
        ]

        ;
        ; -- Whitespace

        nl: {\^/} ; Line break in logical line.
        eol: newline ; End of logical line.
        wsp: [some ws-char]
        span-comment: [{/*} thru {*/}]
        line-comment: [{//} to newline]

    ]

    charsets: context [

        ; Header name
        h-char: complement charset {^/<}
        q-char: complement charset {^/"}

        ; Identifier
        nondigit: charset [#"_" #"a" - #"z" #"A" - #"Z"]
        digit: charset {0123456789}
        octal-digit: charset {01234567}
        id-char: union nondigit digit
        hexadecimal-digit: charset [#"0" - #"9" #"a" - #"f" #"A" - #"F"]

        ; pp-number
        sign: charset {+-}

        ; character-constant
        c-char: complement charset {'\^/}

        ; string-literal
        s-char-cs: complement charset {"\^/}

        ; punctuator
        p-char: charset "[](){}.&*+-~!/%<>^^|?:;=,#"

        ; whitespace
        ws-char: charset { ^-^M^/^K^L}
        not-wsp: complement ws-char
    ]

    grammar: context bind grammar charsets
    ; Grammar defined first in file.
]
