REBOL [
    System: "REBOL [R3] Language Interpreter and Run-time Environment"
    Rights: {
        Copyright 2012 REBOL Technologies
        Copyright 2012-2025 Rebol Open Source Contributors
        REBOL is a trademark of REBOL Technologies
    }
    License: {
        Licensed under the Apache License, Version 2.0
        See: http://www.apache.org/licenses/LICENSE-2.0
    }
    Title:  "macOS AppleScript dialogs"
    Name:    osx-dialogs
    Version: 1.1.0
    type: module
    Exports: [request-dir request-file request-color]
]

;; Remove the native (not implemented) placeholders
do in lib [
    unset 'request-dir
    unset 'request-file
    unset 'request-color
]

applescript-ctx: make object! [
    ;; Local variables used by the function
    title:      ""   ;; Dialog window title
    defaultLoc: ""   ;; Default file location
    filters:    ""   ;; File type filters string
    out:        ""   ;; Command output buffer
    err:        ""   ;; Command error buffer
    last-dir:   _    ;; Previous directory path
    
    ;; AppleScript for multiple file selection
    script-multi:
    %%{osascript -e 'set AppleScript'\''s text item delimiters to linefeed' \
      -e 'set defaultLoc to POSIX file "$defaultLoc" as alias' \
      -e 'set sel to choose file with prompt "$title" $filters with multiple selections allowed default location defaultLoc' \
      -e 'set out to {}' \
      -e 'repeat with f in sel' \
      -e 'set end of out to POSIX path of f' \
      -e 'end repeat' \
      -e 'return out as text'
    }%%
    
    ;; AppleScript for single file selection
    script-single:
    %%{osascript -e 'tell application "System Events" to activate' \
      -e 'set defaultLoc to POSIX file "$defaultLoc" as alias' \
      -e 'return POSIX path of (choose file with prompt "$title" $filters default location defaultLoc)'
    }%%

    ;; AppleScript for single directory selection
    script-dir:
    %%{osascript -e 'tell application "System Events" to activate' \
      -e 'set defaultLoc to POSIX file "$defaultLoc" as alias' \
      -e 'return POSIX path of (choose folder with prompt "$title" default location defaultLoc)'
    }%%
]

double-escape: function/with [
    "Escape special characters in strings for shell/command usage"
    value [any-string!] "Input string or file to be escaped"
][
    ;; Convert file paths to local file format if needed
    if file? value [value: to-local-file value]
    ;; Check if escaping is needed - if string contains only normal chars, return as-is
    if parse value [any normal-chars end][ return value ]
    ;; Perform escaping process
    out: copy "" ;; Initialize output string
    ;; Parse input string and escape special characters
    parse value [
        collect into out  ;; Collect results into output string
        any [
            ;; Keep sequences of normal characters
            keep some normal-chars
            ;; Handle line feed character - convert to literal \n
            | lf keep ("\n")
            ;; Handle special characters - prefix with backslash
            | keep (#"\") keep skip
        ]
    ]
    out  ;; Return escaped string
][
    normal-chars: complement make bitset! {$`'"\^/}
]

request-file: function/with [
    "Asks user to select a file and returns full file path (or block of paths)."
     /save         "File save mode" ;@@ Not used on macOS!
     /multi        "Allows multiple file selection, returned as a block"
     /file         "Default file name or directory"
      name         [file!] 
     /title        "Window title"
      text         [string!] 
     /filter       "Block of filters"
      list         [block!]
][
    if text [text: double-escape text]
    ;; Choose appropriate script and title based on selection mode
    either multi [
        self/title: any [text "Select files"]    ;; Default title for multi-select
        script: script-multi                     ;; Use multi-selection AppleScript
    ][   
        self/title: any [text "Select a file"]   ;; Default title for single-select
        script: script-single                    ;; Use single-selection AppleScript
    ]
    
    ;; Set default location for file dialog
    self/defaultLoc: double-escape any [
        all [name to-real-file name]  ;; Use provided file/directory if given
        what-dir                      ;; Otherwise use current directory
    ]
    
    ;; Build filter string for AppleScript
    self/filters: clear ""
    if filter [
        append self/filters "of type {"
        ;; Process each filter pair (name, filter)
        foreach value list [
            if any-string? value [
                append self/filters ajoin [#"^"" double-escape value {",}]
            ]
        ]
        ;; Replace final comma with closing brace
        change skip tail self/filters -2 #"}"
    ]
    
    ;; Generate final AppleScript command by substituting variables
    cmd: reword script self
    
    ;; Clear output buffers
    clear out
    clear err
    
    ;; Execute AppleScript command and capture output/errors
    call/shell/output/error cmd :out :err 
    
    ;; Check for errors in execution
    unless empty? err [
        unless find err "(-128)" [ ;= User cancelled.
            sys/log/error 'REBOL err
        ]
        return none
    ]
    
    ;; Process and return results based on selection mode
    either multi [
        ;; Multi-select: split lines and convert each to Rebol file format
        out: split-lines out
        forall out [out/1: to-rebol-file out/1]
        out
    ][
        ;; Single select: trim whitespace and convert to Rebol file format
        to-rebol-file trim/tail out
    ]
] :applescript-ctx

request-dir: function/with [
    "Asks user to select a directory and returns full directory path (or block of paths)."
     /title        "Change heading on request"
      text         [string!] 
     /dir          "Set starting directory"
      name         [file!] 
     /keep         "Keep previous directory path"
][
    if text [text: double-escape text]
    self/title: any [text "Select a folder"]

    ;; Set default location for file dialog
    defaultLoc: double-escape any [
        all [name to-real-file name]  ;; Use provided file/directory if given
        all [keep last-dir]
        what-dir                      ;; Otherwise use current directory
    ]

    ;; Generate final AppleScript command by substituting variables
    cmd: reword script-dir self
    
    ;; Clear output buffers
    clear out
    clear err
    ;; Execute AppleScript command and capture output/errors
    call/shell/output/error cmd :out :err 
    
    dir: to-rebol-file trim/tail out
    if keep [self/last-dir: dir]
    dir

] :applescript-ctx


request-color: function/with [
    "Prompt for a color using the macOS color picker dialog"
    /default  "Default RGB color"
     color    [tuple!]
    /rgb16    "Return block with 16bit RGB values instead"
][
    defaultColor: any [
        all [default to-local-color color]
        defaultColor
    ]

    ;; Build the AppleScript command to prompt for a color
    cmd: reword script-color self
    ;; Initialize output and error buffers
    clear out
    clear err

    ;; Execute the AppleScript via shell, capturing standard output and error
    call/shell/output/error cmd :out :err

    ;; Convert the comma-separated RGB list into a Rebol block of integers
    out: trim/with out ",^/"
    if empty? out [return _]
    clr: transcode out
    either rgb16 [clr][to-rebol-color clr]
][
    ;; Buffers for shell execution results
    out: ""
    err: ""

    ;; Default RGB color as comma-separated string (red)
    defaultColor: "65535, 0, 0"

    ;; AppleScript to open the macOS color picker dialog
    script-color: 
    %%{osascript -e 'tell application "System Events" to activate' \
      -e 'set chosenColor to choose color default color {$defaultColor}' \
      -e 'return chosenColor'
    }%%

    ;; Convert a Rebol 8-bit color to macOS 16-bit AppleScript string
    to-local-color: func[c [tuple!]][
        ;; Scale channels from 0–255 to 0–65535
        rejoin [
            c/1 * 257 ", "
            c/2 * 257 ", "
            c/3 * 257
        ]
    ]
    to-rebol-color: func[c [block!]][
        ;; Scale down channels 0–65535 to 0–255
        as-color c/1 / 257 c/2 / 257 c/3 / 257
    ]
]
