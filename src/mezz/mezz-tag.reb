REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Tag"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

build-tag: func [
    "Generates a tag from a composed block."
    values [block!] "Block of parens to evaluate and other data."
    /local tag value-rule xml? name attribute value
][  ;@@ slightly modified version from Rebol2
    tag: make string! 7 * length? values
    value-rule: [
        set value issue! (value: mold value)
        | set value file! (value: replace/all copy value #" " "%20")
        | set value any-type! (value: replace/all form value #"'" {\'})
    ]
    xml?: false
    parse compose values [
        [
            set name ['?xml (xml?: true) | word! | url!] (append tag name)
            any [
                set attribute [word! | url!] value-rule (
                    append tag ajoin [#" " attribute {='} value #"'"]
                )
                | value-rule (append tag join #" " value)
            ]
            end (if xml? [append tag #"?"])
        ]
        |
        [set name refinement! to end (tag: mold name)]
    ]
    to tag! tag
]