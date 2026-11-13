REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Amalgamate Rebol extension include files"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2023 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Oldes"
	Version: 1.0.0
	Needs:   3.5.0
	Purpose: {
		Collects Rebol extension related include files into a single one.
	}
]
context [
	dir: %../../src/include/
	space-or-tab: system/catalog/bitsets/space

	amalgamate: func[files /local result][
		result: make string! 100000
		included: copy []
		count: length? files
		start: stats/timer

		append result ajoin [
{////////////////////////////////////////////////////////////////////////
// File: rebol-extension.h
// Home: https://github.com/Oldes/Rebol3/
// Date: } now/utc {
// Note: This file is amalgamated from these sources:
//}]
		foreach file files [
			append result ajoin ["^///       " file]
		]
		append result {
//
////////////////////////////////////////////////////////////////////////
}
		keep-file: function [file][
			if find included file [continue]
			append included probe file
			append result ajoin ["^/// File: " file LF] 
			try/with [
				data: read/string dir/:file
			][
				append result "^/// *** Failed to include! ***//^/" 
				sys/log/error 'REBOL system/state/last-error
				exit
			]
			parse data [
				any [
					to "#include" [
						;; make sure, that the include is not commented out
						s: if (find crlf s/-1) [
							8 skip
							;; include only file includes
							some space-or-tab #"^"" copy name: to #"^"" thru lf e: (
								++ count
								name: as file! probe name
								unless find included name [ 
									insert e append read/string dir/:name LF
									append included name
								]
								insert s "//"
							)
							| 1 skip
						]
						;; in case of above failure, skip this include
						| 8 skip
					]
				]
			]
			append append result data LF
		]
		foreach file files [
			keep-file file
		]
		print [
			as-green "Amalgamated" 
			as-yellow count
			as-green "files into"
			as-yellow length? result
			as-green "bytes in time:"
			as-yellow stats/timer - start
		]
		result
	]

	make-dir %../../build/
	probe to-real-file write    %../../build/rebol-extension.h amalgamate [
		%reb-c.h
		%reb-args.h
		%reb-ext.h
		%reb-ext-handler.h
		%reb-device.h
		%reb-file.h
		%reb-filereq.h
		%reb-event.h
		%reb-evtypes.h
		%reb-lib.h
	]
]