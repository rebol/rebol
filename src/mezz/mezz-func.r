REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Mezzanine: Function Helpers"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
]

clos: func [
	{Defines a closure function.}
	spec [block!] {Help string (opt) followed by arg words (and opt type and string)}
	body [block!] {The body block of the function}
][
	make closure! copy/deep reduce [spec body]
]

closure: func [
	{Defines a closure function with all set-words as locals.}
	spec [block!] {Help string (opt) followed by arg words (and opt type and string)}
	body [block!] {The body block of the function}
	/with {Define or use a persistent object (self)}
	object [object! block! map!] {The object or spec}
	/extern words [block!] {These words are not local}
][
	; Copy the spec and add /local to the end if not found
	unless find spec: copy/deep spec /local [append spec [
		/local ; In a block so the generated source gets the newlines
	]]
	; Make a full copy of the body, to allow reuse of the original
	body: copy/deep body
	; Collect all set-words in the body as words to be used as locals, and add
	; them to the spec. Don't include the words already in the spec or object.
	insert find/tail spec /local collect-words/deep/set/ignore body either with [
		; Make our own local object if a premade one is not provided
		unless object? object [object: make object! object]
		bind body object  ; Bind any object words found in the body
		; Ignore the words in the spec and those in the object. The spec needs
		; to be copied since the object words shouldn't be added to the locals.
		append append append copy spec 'self words-of object words ; ignore 'self too
	][
		; Don't include the words in the spec, or any extern words.
		either extern [append copy spec words] [spec]
	]
	make closure! reduce [spec body]
]

has: func [
	{A shortcut to define a function that has local variables but no arguments.}
	vars [block!] {List of words that are local to the function}
	body [block!] {The body block of the function}
][
	make function! reduce [head insert copy/deep vars /local copy/deep body]
]

context: func [
	{Defines a unique object.}
	blk [block!] {Object words and values (modified)}
][
	make object! blk
]

map: func [
	{Make a map value (hashed associative block).}
	val
][
	make map! :val
]

task: func [
	{Creates a task.}
	spec [block!] {Name or spec block}
	body [block!] {The body block of the task}
][
	make task! copy/deep reduce [spec body]
]
