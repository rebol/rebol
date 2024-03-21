REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Datatype action definitions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Note: [
		"This list is order dependent!"
		"Used to generate C enums and tables"
		"Boot bind attributes are SET and not DEEP"
		"Todo: before beta release remove extra/unused refinements"
	]
]

;-- Binary Math & Logic

add: action [
	{Returns the addition of two values.}
	value1 [scalar! date! vector!]
	value2 [scalar! date! vector!]
]

subtract: action [
	{Returns the second value subtracted from the first.}
	value1 [scalar! date! vector!]
	value2 [scalar! date! vector!]
]

multiply: action [
	{Returns the first value multiplied by the second.}
	value1 [scalar! vector!]
	value2 [scalar! vector!]
]

divide: action [
	{Returns the first value divided by the second.}
	value1 [scalar! vector!]
	value2 [scalar! vector!]
]

remainder: action [
	{Returns the remainder of first value divided by second.}
	value1 [scalar!]
	value2 [scalar!]
]

power: action [
	{Returns the first number raised to the second number.}
	number [number!]
	exponent [number!]
]

and~: action [
	{Returns the first value ANDed with the second.}
	value1 [logic! integer! char! tuple! binary! bitset! typeset! datatype!]
	value2 [logic! integer! char! tuple! binary! bitset! typeset! datatype!]
]

or~: action [
	{Returns the first value ORed with the second.}
	value1 [logic! integer! char! tuple! binary! bitset! typeset! datatype!]
	value2 [logic! integer! char! tuple! binary! bitset! typeset! datatype!]
]

xor~: action [
	{Returns the first value exclusive ORed with the second.}
	value1 [logic! integer! char! tuple! binary! bitset! typeset! datatype!]
	value2 [logic! integer! char! tuple! binary! bitset! typeset! datatype!]
]

;-- Unary

negate: action [
	{Changes the sign of a number.}
	number [number! pair! money! time! bitset!]
]

complement: action [
	{Returns the one's complement value.}
	value [logic! integer! tuple! binary! bitset! typeset! image!]
]

absolute: action [
	{Returns the absolute value.}
	value [number! pair! money! time!]
]

round: action [
	{Rounds a numeric value; halves round up (away from zero) by default.}
	value [number! pair! money! time!] "The value to round"
	/to "Return the nearest multiple of the scale parameter"
	scale [number! money! time!] "Must be a non-zero value (result will be of this type)"
	/even      "Halves round toward even results"
	/down      "Round toward zero, ignoring discarded digits. (truncate)"
	/half-down "Halves round toward zero"
	/floor     "Round in negative direction"
	/ceiling   "Round in positive direction"
	/half-ceiling "Halves round in positive direction"
]

random: action [
	{Returns a random value of the same datatype; or shuffles series.}
	value   {Maximum value of result (modified when series)}
	/seed   {Restart or randomize}
	/secure {Returns a cryptographically secure random number}
	/only   {Pick a random value from a series}
]

odd?: action [
	{Returns TRUE if the number is odd.}
	number [number! char! date! money! time! pair!]
]

even?: action [
	{Returns TRUE if the number is even.}
	number [number! char! date! money! time! pair!]
]

;-- Series Navigation

head: action [
	{Returns the series at its beginning.}
	series [series! gob! port!]
]

tail: action [
	{Returns the series just past its end.}
	series [series! gob! port!]
]

head?: action [
	{Returns TRUE if a series is at its beginning.}
	series [series! gob! port!]
]

tail?: action [
	{Returns TRUE if series is at or past its end; or empty for other types.}
	series [series! gob! port! bitset! typeset! map!]
]

past?: action [
	{Returns TRUE if series is past its end.}
	series [series! gob! port!]
]

next: action [
	{Returns the series at its next position.}
	series [series! gob! port!]
]

back: action [
	{Returns the series at its previous position.}
	series [series! gob! port!]
]

skip: action [
	{Returns the series forward or backward from the current position.}
	series [series! gob! port!]
	offset [number! logic! pair!]
]

at: action [
	{Returns the series at the specified index, relative to the current position.}
	series [series! gob! port!]
	index [number! logic! pair!]
]

atz: action [
	{Returns the series at the specified 0-based index, relative to the current position.}
	series [series! gob! port!]
	index [number! logic! pair!]
]

index?: action [
	{Returns the current position (index) of the series.}
	series [series! gob! port! none!]
	/xy {Returns index as an XY pair offset}
]

indexz?: action [
	{Returns the current 0-based position (index) of the series.}
	series [series! gob! port! none!]
	/xy {Returns index as an XY pair offset}
]

length?: action [
	{Returns the length (from the current position for series.)}
	series [series! port! map! tuple! bitset! object! gob! struct! any-word! none!]
]

;-- Series Extraction

pick: action [
	{Returns the value at the specified position.}
	aggregate [series! map! gob! pair! date! time! tuple! bitset! port!]
	index {Index offset, symbol, or other value to use as index}
]

;-- Series Search

find: action [
	{Searches for a value; for series returns where found, else none.}
	series [series! gob! port! bitset! typeset! object! map! none!]
	value [any-type!]
	/part {Limits the search to a given length or position}
	range [number! series! pair!]
	/only {Treats a series value as only a single value}
	/case {Characters are case-sensitive}
	/same {Use "same?" as comparator}
	/any  {Enables the * and ? wildcards}
	/with {Allows custom wildcards}
	wild [string!] "Specifies alternates for * and ?"
	/skip {Treat the series as records of fixed size}
	size [integer!]
	/last {Backwards from end of series}
	/reverse {Backwards from the current position}
	/tail {Returns the end of the series}
	/match {Performs comparison and returns the head of the match (not imply /tail)}
]

select: action [
	{Searches for a value; returns the value that follows, else none.}
	series [series! port! map! object! module! none!]
	value [any-type!]
	/part {Limits the search to a given length or position}
	range [number! series! pair!]
	/only {Treats a series value as only a single value}
	/case {Characters are case-sensitive}
	/same {Use "same?" as comparator}
	/any  {Enables the * and ? wildcards}
	/with {Allows custom wildcards}
	wild [string!] "Specifies alternates for * and ?"
	/skip {Treat the series as records of fixed size}
	size [integer!]
	/last {Backwards from end of series}
	/reverse {Backwards from the current position}
]

;;;;!!! MATCH

reflect: action [
	{Returns specific details about a datatype.}
	value [any-type!]
	field [word!] "Such as: spec, body, words, values, title"
]

;-- Making, copying, modifying

make: action [
	{Constructs or allocates the specified datatype.}
	type [any-type!] {The datatype or an example value}
	spec [any-type!] {Attributes or size of the new value (modified)}
]

to: action [
	{Converts to a specified datatype.}
	type [any-type!] {The datatype or example value}
	value [any-type!] {The attributes of the new value}
]

copy: action [
	{Copies a series, object, or other value.}
	value [series! port! map! object! bitset! any-function! error!] {At position}
	/part {Limits to a given length or end position}
	range [number! series! pair!]
	/deep {Also copies series values within the block}
	/types {What datatypes to copy}
	kinds [typeset! datatype!]
]

take: action [
	{Removes and returns one or more elements.}
	series [series! port! gob! none!] {At position (modified)}
	/part {Specifies a length or end position}
	range [number! series! pair!]
	/deep {Also copies series values within the block}
	/last {Take it from the tail end}
]

put: action [
	{Replaces the value following a key, and returns the new value.}
	series [any-block! map! port! object!] {(modified)}
	key    [any-type!] 
	value  [any-type!] {The new value (returned)}
	/case  {Perform a case-sensitive search}
]

insert: action [
	{Inserts element(s); for series, returns just past the insert.}
	series [series! port! map! gob! object! bitset!] {At position (modified)}
	value [any-type!] {The value to insert}
	/part {Limits to a given length or position}
	range [number! series! pair!]
	/only {Only insert a block as a single value (not the contents of the block)}
	/dup {Duplicates the insert a specified number of times}
	count [number! pair!]
]

append: action [
	{Inserts element(s) at tail; for series, returns head.}
	series [series! port! map! gob! object! bitset!] {Any position (modified)}
	value [any-type!] {The value to insert}
	/part {Limits to a given length or position}
	range [number! series! pair!]
	/only {Only insert a block as a single value (not the contents of the block)}
	/dup {Duplicates the insert a specified number of times}
	count [number! pair!]
]

remove: action [
	{Removes element(s); returns same position.}
	series [series! gob! port! bitset! none! map!] {At position (modified)}
	/part {Removes multiple elements or to a given position}
	range [number! series! pair! char!]
	/key {Removes a key from map.}
	key-arg [any-type!]
]

change: action [
	{Replaces element(s); returns just past the change.}
	series [series! gob! port! struct!]{At position (modified)}
	value [any-type!] {The new value}
	/part {Limits the amount to change to a given length or position}
	range [number! series! pair!]
	/only {Only change a block as a single value (not the contents of the block)}
	/dup {Duplicates the change a specified number of times}
	count [number! pair!]
]

poke: action [
	{Replaces an element at a given position.}
	series [series! port! map! gob! bitset!] {(modified)}
	index {Index offset, symbol, or other value to use as index}
	value [any-type!] {The new value (returned)}
]

clear: action [
	{Removes elements from current position to tail; returns at new tail.}
	series [series! port! map! gob! bitset! none!] {At position, if ordered collection (modified)}
]

trim: action [
	{Removes spaces from strings, nulls from binary, nones from blocks or objects.}
	series [series! object! error! module!] {Series (modified) or object (made)}
	/head {Removes only from the head}
	/tail {Removes only from the tail}
	/auto {Auto indents lines relative to first line}
	/lines {Removes all line breaks and extra spaces}
	/all  {Removes all whitespace}
	/with str [char! string! binary! integer!] {Same as /all, but removes characters in 'str'}
]

swap: action [
	{Swaps elements between two series or the same series.}
	series1 [series! gob!] {At position (modified)}
	series2 [series! gob!] {At position (modified)}
]

reverse: action [
	{Reverses the order of elements; returns at same position.}
	series [series! gob! tuple! pair!] {At position (modified)}
	/part {Limits to a given length or position}
	range [number! series!]
]

sort: action [
	{Sorts a series; default sort order is ascending.}
	series [series!] {At position (modified)}
	/case {Case sensitive sort}
	/skip {Treat the series as records of fixed size}
	size [integer!] {Size of each record}
	/compare  {Comparator offset, block or function}
	comparator [integer! block! any-function!]
	/part {Limits the sorting to a given length or position}
	range [number! series!]
	/all {Compare all fields}
	/reverse {Reverse sort order}
]

;-- Port actions:

create: action [
	{Send port a create request.}
	port [port! file! url! block!]
]

delete: action [
	{Send port a delete request.}
	port [port! file! url! block!]
]

open: action [
	{Opens a port; makes a new port from a specification if necessary.}
	spec [port! file! url! block!]
	/new   {Create new file - if it exists, reset it (truncate)}
	/read  {Open for read access}
	/write {Open for write access}
	/seek  {Optimize for random access}
	/allow {Specifies protection attributes}
		access [block!]
]

close: action [
	{Closes a port.}
	port [port!]
]

read: action [
	{Read from a file, URL, or other port.}
	source [port! file! url! block!]
	/part {Partial read a given number of units (source relative)}
		length [number!]
	/seek {Read from a specific position (source relative)}
		index [number!]
	/string {Convert UTF and line terminators to standard text string}
	/binary {Preserves contents exactly}
	/lines  {Convert to block of strings (implies /string)}
	/all    {Response may include additional information (source relative)}
;	/as {Convert to string using a specified encoding}
;		encoding [none! number!] {UTF number (0 8 16 -16)}
]

write: action [
	{Writes to a file, URL, or port - auto-converts text strings.}
	destination [port! file! url! block!]
	data  {Data to write (non-binary converts to UTF-8)}
	/part {Partial write a given number of units}
		length [number!]
	/seek {Write at a specific position}
		index [number!]
	/append {Write data at end of file}
	/allow  {Specifies protection attributes}
		access [block!]
	/lines  {Write each value in a block as a separate line}
	/binary {Preserves contents exactly}
	/all    {Response may include additional information (source relative)}
;	/as {Convert string to a specified encoding}
;		encoding [none! number!] {UTF number (0 8 16 -16)}
]

open?: action [
	{Returns TRUE if port is open.}
	port [port!]
]

query: action [
	{Returns information about target if possible.}
	target [port! file! url! block! vector! date! handle!]
	/mode "Get mode information"
	field [word! block! none!] "NONE will return valid modes for target type"
]

modify: action [
	{Change mode or control for port or file.}
	target [port! file!]
	field [word! none!]
	value
]

update: action [
	{Updates external and internal states (normally after read/write).}
	port [port!]
]

rename: action [
	{Rename a file.}
	from [port! file! url! block!]
	to [port! file! url! block!]
]

flush: action [
	{Flush output stream buffer.}
	port [port!]
]

