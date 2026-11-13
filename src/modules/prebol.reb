REBOL [
	Name:    prebol
	Type:    module
	Options: [delay]
	Exports: [process-source]
	Version: 1.1.6
	Title:   "Prebol - Official REBOL Preprocessor"
	File:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/modules/prebol.reb
	Author: ["Carl Sassenrath" "Holger Kruse" "Oldes"]
	Purpose: {
		The official REBOL preprocessor. Combines multiple
		source or data files into a single REBOL output file.
		Includes the full power of REBOL, allowing you to
		evaluate any REBOL expression during processing. Allows
		merging of code, data, images, sound and other types of
		files.  Supports conditional expressions, such as
		conditional including of files. The output is a reduced
		REBOL source file with extra spacing, comments, and
		include file headers removed. Processing is recursive,
		so included files can themselves contain commands to
		process.
	}
	Rights: {Users are allowed to modify this source, but must
	keep this header intact. Changes to the official version
	must be approved by REBOL Technologies www.REBOL.com. Send
	changes to http://www.rebol.com/feedback.html.}
	Usage: {
		From command line, shortcut, or alias:
			rebol -s prebol.r input.r output.r

		From REBOL console or script:
			do/args %prebol.r [%input.r %output.r]
	
		Supported preprocessor commands:
			#include %file.r | (file-expr)
			#include-files %path [%file1 %file2 %file3]
			#include-string %file.txt | (file-expr)
			#include-binary %file.bin | (file-expr)
			#include-block  %file.rb  | (file-expr)
			#do [expr]
			#if [expr] [then-block]
			#either [expr] [then-block] [else-block]

		An unlimited number of expressions and results can be
		processed and created by using the #do command. (So, if
		you add new commands, please make sure they can't first
		be done with #do. Each command affects use of #issue
		data within source code, e.g. this preprocessor cannot
		be preprocessed.)
	}
	History: [
		8-3-2004 "Oldes" [
			"Modified not to be evaluated on start"
			"Added #include-block command"
			"Includes are described in the result code using comment"
		]
		5-Mar-2021 "Oldes" [
			"Updated for use with current Rebol version"
		]
	]
]

error: func [msg] [
	if block? msg [msg: reform msg]
	sys/log/error 'PREBOL msg
	halt
]

include-source-comment?: true ;adds comments before included content
process-source: func [
	; Process REBOL source code block. Modifies the block. Returns size.
	blk [any-block!] "Block of source to process"
	size [integer!] "Starting size"
	/only "Don't use recursive processing"
	/local file data expr cmd else tmp path include-cmds header do-expr base
][
	base: system/options/binary-base
	system/options/binary-base: 64   ; output data in base-64
	do-expr: func [expr /local result] [
		; Evaluate expression and make sure it returns a result.
		set/any 'result try [do :expr]
		if function? :result [set/any 'result try [do :result]]
		case [
			unset? :result [ error [mold cmd "must return a value or none:" mold expr] ]
			error? :result [ error result ]
		]
		:result
	]

	include-cmds: [
		#include [ ; REBOL code or data or loaded images or sounds.
			data: load/header file
			if header: take data [
				if include-source-comment? [
					if :header/author [
						insert data compose [
							comment (join {## Author:  } mold :header/author)
						]
					]
					if :header/title [
						insert data compose [
							comment (join {## Title:   } mold :header/title)
						]
					]
					insert insert data compose [
						comment (join {## Include: } mold file)
					]
					insert tail data compose [
						comment (join {-- End of:  } mold file)
					]
				]
			]
			data ; return it
		]
		#include-string [ ; Raw text data string
			read/string file
		]
		#include-binary [ ; Raw binary data (unloaded images, sounds).
			read/binary file
		]
		#include-block  [ ; Includes enclosed rebol block
			data: load/all file
			if data/1 = 'rebol [remove/part data 2]
			head insert/only copy [] data ; return it
		]
		#include-eval  [ ; Includes result from evaluation
			set/any 'data do file ;file is not a file but a block to evaluate!!!
			head insert/only copy [] data ; return it
		]
	]

	while [not tail? blk][
		; Source pragmas begin with # (they are of the ISSUE datatype)
		; and are followed by a filename, a block, or a paren.
		either issue? blk/1 [

			cmd: blk/1

			; If it is an #INCLUDE of some type:
			either find include-cmds cmd [

				file: blk/2

				; Expression such as #include (join %file num: num + 1)
				if paren? :file [
					file: do-expr to block! :file
				]

				sys/log/info 'PREBOL ["Including file:" as-green file]

				; Include requires a file argument:
				if not file? file [error ["Invalid" mold cmd "file expression:" mold file]]

				; File must exist:
				if not exists? file [error [mold cmd "file not found:" mold file]]

				size: size + size? file ; for stats only

				; Execute the include:
				if error? data: try select include-cmds cmd [ error data ]
				; Replace include command with contents of file:
				remove/part blk 2
				insert blk data
			][
				; Process other types of commands.  If they return
				; a non-none result, insert the result into the output block.
				; Otherwise, do not insert a result into the output.
				switch/default cmd [
					#do [	; #do [expression]
						expr: blk/2
						remove/part blk 2
						if data: do-expr expr [insert blk :data]
					]
					#if [	; #if [condition] [then-expr]
						expr: blk/2
						data: blk/3
						remove/part blk 3
						if do-expr expr [insert blk :data]
					]
					#either [ ; #either [condition] [then-expr] [else-expr]
						expr: blk/2
						data: blk/3
						else: blk/4
						remove/part blk 4
						insert blk either do-expr expr [:data] [:else]
					]
					#include-files [ ; #include-files %path [file1 file2...]
						tmp: copy []
						path: dirize blk/2
						foreach file blk/3 [
							size: size + size? join path to-file file
							data: read/binary join path to-file file
							repend tmp [file data]
						]
						remove/part blk 3
						insert/only blk tmp
					]
					#comments  [ ; #comments true/false
						include-source-comment?: either find [false off] blk/2 [false][true]
						blk: skip blk 2
					]
				][
					; Ignore it (could be a valid #issue datatype)
					blk: next blk
				]
			]
		][
			blk: next blk
		]
	]
	unless only [
		foreach item head blk [
			if block? :item [size: process-source item size]
		]
	]
	system/options/binary-base: base ; restore the original base
	size
]
