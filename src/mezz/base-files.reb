REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Base: File Functions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Note: {
		This code is evaluated just after actions, natives, sysobj, and other lower
		levels definitions. This file intializes a minimal working environment
		that is used for the rest of the boot.
	}
]

exists?: func [
	{Determines if a file or URL exists.}
	target [file! url!]
][
	all [
		word? target: try [query/mode target 'type]
		target
	]
]

size?: func [
	{Returns the size of a file or vector (bits per value).}
	target [file! url! port! vector!]
][
	query/mode target 'size
]

modified?: func [
	{Returns the last modified date of a file.}
	target [file! url!]
][
	query/mode target 'date
]

suffix?: func [
	"Return the file suffix of a filename or url. Else, NONE."
	path [file! url! string!]
	/local end
][
	either all [
		url? path end: find path #(bitset! #{0000000010000001}) ; "?#"
	][
		all [
			path: find/reverse end #"."
			not find/part path #"/" end
			to file! copy/part path end
		]
	][
		all [
			path: find/last path #"."
			not find path #"/"
			to file! path
		]
	]
]

dirize: func [
	{Returns a copy (always) of the path as a directory (ending slash).}
	path [file! string! url!]
][
	path: copy path
	if slash <> last path [append path slash]
	path
]

make-dir: func [
	"Creates the specified directory. No error if already exists."
	path [file! url!]
	/deep "Create subdirectories too"
	/local dirs end created
][
	if empty? path [return path]
	if slash <> last path [path: dirize path]

	switch exists? path [
		dir  [return path]
		file [cause-error 'access 'cannot-open path]
	]

	if any [not deep url? path] [
		create path
		return path
	]

	; Scan reverse looking for first existing dir:
	path: copy path
	dirs: copy []
	while [
		all [
			not empty? path
			not exists? path
			remove back tail path ; trailing slash
		]
	][
		end: any [find/last/tail path slash path]
		insert dirs copy end
		clear end
	]

	; Create directories forward:
	created: copy []
	foreach dir dirs [
		path: either empty? path [dir][path/:dir]
		append path slash
		if error? try [make-dir path] [
			foreach dir created [attempt [delete dir]]
			cause-error 'access 'cannot-open path
		]
		insert created path
	]
	path
]

delete-dir: func [
	{Deletes a directory including all files and subdirectories.}
	path [file!]
	/local files
][
	try [
		if all [
			'dir = exists? path
			block? files: read path
		] [
			foreach file files [delete-dir path/:file]
		]
		delete path
	]
]

script?: func [
	{Checks file, url, or string for a valid script header.}
	source [file! url! binary! string!]
][
	switch type?/word source [
		file! url! [source: read source]
		string! [source: to binary! source] ; Remove this line if FIND-SCRIPT changed to accept string!
	]
	find-script source ; Returns binary!
]

file-type?: func [
	"Return the identifying word for a specific file type (or NONE)."
	file [file! url!]
][
	all [
		file: find find system/options/file-types suffix? file word!
		first file
	]
]

split-path: func [
	"Splits and returns directory path and file as a block."
	target [file! url! string!]
	/local dir pos
][
	parse target [
		[#"/" | 1 2 #"." opt #"/"] end (pos: %"" dir: dirize target) |
		pos: any [thru #"/" [end | pos:]] (
			all [empty? dir: copy/part target at head target index? pos dir: %./]
			all [find [%. %..] pos: to file! pos insert tail pos #"/"]
		)
	]
	reduce [dir pos]
]

intern: function [
	"Imports (internalize) words and their values from the lib into the user context."
	data [block! any-word!] "Word or block of words to be added (deeply)"
][
	index: 1 + length? usr: system/contexts/user ; optimization
	data: bind/new :data usr   ; Extend the user context with new words
	resolve/only usr lib index ; Copy only the new values into the user context
	:data
]

load: function [
	{Simple load of a file, URL, or string/binary - minimal boot version.}
	source [file! url! string! binary!]
	/header  {Includes REBOL header object if present}
	/all     {Load all values, including header (as block)}
	;/unbound {Do not bind the block}
][
	if string? data: case [
		file? source [read source]
		url? source [read source]
		'else [source]
	] [data: to binary! data]

	if binary? :data [
		data: transcode data
		hdr?: lib/all ['REBOL = first data block? second data]
		case [
			header [
				unless hdr? [cause-error 'syntax 'no-header source]
				remove data
				data/1: attempt [construct/with first data system/standard/header]
			]
			all none ; /header overrides /all
			hdr? [remove/part data 2]
		]
	;	unless unbound []
		data: intern data

		; If appropriate and possible, return singular data value
		unless any [
			all
			header
			1 <> length? data
		][data: first data]
	]

	:data
]

; Reserve these slots near LOAD:
save:
import:
	none
