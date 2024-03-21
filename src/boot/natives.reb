REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Native function specs"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2024 Rebol Open Source Developers
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Note: [
		"Used to generates C enums and tables"
		"Boot bind attributes are SET and not DEEP"
		"Todo: before beta release remove extra/unused refinements"
	]
]

;-- Control Natives - nat_control.c

ajoin: native [
	{Reduces and joins a block of values into a new string. Ignores none and unset values.}
	block [block!]
	/with delimiter [any-type!]
	/all "Do not ignore none and unset values"
]

also: native [
	{Returns the first value, but also evaluates the second.}
	value1 [any-type!]
	value2 [any-type!]
]

all: native [
	{Shortcut AND. Evaluates and returns at the first FALSE or NONE.}
	block [block!] {Block of expressions}
]

any: native [
	{Shortcut OR. Evaluates and returns the first value that is not FALSE or NONE.}
	block [block!] {Block of expressions}
]

apply: native [
	{Apply a function to a reduced block of arguments.}
	func [any-function!] "Function value to apply"
	block [block!] "Block of args, reduced first (unless /only)"
	/only "Use arg values as-is, do not reduce the block"
]

assert: native [
	"Assert that condition is true, else cause an assertion error."
	conditions [block!]
	/type "Safely check datatypes of variables (words and paths)"
]

attempt: native [
	"Tries to evaluate a block and returns result or NONE on error."
	block [block!]
	/safer "Capture all possible errors and exceptions"
]

break: native [
	{Breaks out of a loop, while, until, repeat, foreach, etc.}
	/return {Forces the loop function to return a value}
	value [any-type!]
]

case: native [
	{Evaluates each condition, and when true, evaluates what follows it.}
	block [block!] {Block of cases (conditions followed by values)}
	/all {Evaluate all cases (do not stop at first true case)}
]

catch: native [
	{Catches a throw from a block and returns its value.}
	block [block!] {Block to evaluate}
	/name {Catches a named throw}
	word [word! block!] {One or more names}
	/all  {Catches all throws, named and unnamed}
	/quit {Special catch for QUIT native}
	/with callback [block! function!] "Code to be evaluated on a catch"
]

;cause: native [
;	{Force error processing on an error value.}
;	error [error!]
;]

comment: native [
	{Ignores the argument value and returns nothing.}
	value {A string, block, file, etc.}
]

compose: native [
	{Evaluates a block of expressions, only evaluating parens, and returns a block.}
	value "Block to compose"
	/deep "Compose nested blocks"
	/only {Insert a block as a single value (not the contents of the block)}
	/into {Output results into a block with no intermediate storage}
	out [any-block!]
]

object: native [
	{Creates an object.}
	spec  [block!]
	/only {Do not bind nested blocks}
]

continue: native [
	{Throws control back to top of loop.}
]

;disarm: native [
;	{(Deprecated - not needed) Converts error to an object. Other types not modified.}
;	error [any-type!]
;]

do: native [
	{Evaluates a block, file, URL, function, word, or any other value.}
	value [any-type!] "Normally a file name, URL, or block"
	/args "If value is a script, this will set its system/script/args"
	arg   "Args passed to a script (normally a string)"
	/next "Do next expression only, return it, update block variable"
	var [word!] "Variable updated with new block position"
]

;eval: native [
;	{Evaluates a block, file, URL, function, word, or any other value.}
;	value "Normally a file name, URL, or block"
;]

either: native [
	{If TRUE condition return first arg, else second; evaluate blocks by default.}
	condition [any-type!]
	true-branch
	false-branch
	/only "Suppress evaluation of block args."
]

exit: native [
	{Exits a function, returning no value.}
]

find-script: native [
	{Find a script header within a binary string. Returns starting position.}
	script [binary!]
]

for: native [
	{Evaluate a block over a range of values. (See also: REPEAT)}
	'word [word!] "Variable to hold current value"
	start [series! number! pair!] "Starting value"
	end   [series! number! pair!] "Ending value"
	bump  [number! pair!] "Amount to skip each time"
	body  [block!] "Block to evaluate"
]

forall: native [
	"Evaluates a block for every value in a series."
	'word [word!] {Word that refers to the series, set to each position in series}
	body [block!] "Block to evaluate each time"
]

forever: native [
	{Evaluates a block endlessly.}
	body [block!] {Block to evaluate each time}
]

foreach: native [
	{Evaluates a block for each value(s) in a series.}
	'word [word! block!] {Word or block of words to set each time (local)}
	data [series! any-object! map! none!] {The series to traverse}
	body [block!] {Block to evaluate each time}
]

forskip: native [
	"Evaluates a block for periodic values in a series."
	'word [word!] {Word that refers to the series, set to each position in series}
	size [integer! decimal!] "Number of positions to skip each time"
	body [block!] "Block to evaluate each time"
	/local orig result
]

halt: native [
	{Stops evaluation and returns to the input prompt.}
]

if: native [
	{If TRUE condition, return arg; evaluate blocks by default.}
	condition [any-type!]
	true-branch
	/only "Return block arg instead of evaluating it."
]

loop: native [
	{Evaluates a block a specified number of times.}
	count [number!] {Number of repetitions}
	block [block!] {Block to evaluate}
]

map-each: native [
	{Evaluates a block for each value(s) in a series and returns them as a block.}
	'word [word! block!] {Word or block of words to set each time (local)}
	data [block! vector!] {The series to traverse}
	body [block!] {Block to evaluate each time}
]

;replace-all: native [
;	"Search and replace multiple values with a series; returns a new series."
;	target [block! string! binary!]
;	values [block!] "A block of [old new] search/replace pairs"
;]

quit: native [
	{Stops evaluation and exits the interpreter.}
	/return {Returns a value (to prior script or command shell)}
	value {Note: use integers for command shell}
	/now {Quit immediately}
]

protect: native [
	"Protect a series or a variable from being modified."
	value [word! series! bitset! map! object! module!]
	/deep "Protect all sub-series/objects as well"
	/words  "Process list as words (and path words)"
	/values "Process list of values (implied GET)"
	/hide "Hide variables (avoid binding and lookup)"
	/lock "Protect permanently (unprotect will fail)"
]

unprotect: native [
	"Unprotect a series or a variable (it can again be modified)."
	value [word! series! bitset! map! object! module!]
	/deep "Protect all sub-series as well"
	/words "Block is a list of words"
	/values "Process list of values (implied GET)"
]

protected?: native [
	"Return true if immediate argument is protected from modification."
	value [word! series! bitset! map! object! module!]
]

recycle: native [
	{Recycles unused memory.}
	/off {Disable auto-recycling}
	/on {Enable auto-recycling}
	/ballast {Trigger for auto-recycle (memory used)}
	size [integer!]
	/torture {Constant recycle (for internal debugging)}
]

release: native [
	"Release internal resources of the handle. Returns true on success."
	handle [handle!]
]

reduce: native [
	{Evaluates expressions and returns multiple results.}
	value
	/no-set {Keep set-words as-is. Do not set them.}
	/only {Only evaluate words and paths, not functions}
	words [block! none!] {Optional words that are not evaluated (keywords)}
	/into {Output results into a block with no intermediate storage}
	out [any-block!]
]

repeat: native [
	{Evaluates a block a number of times or over a series.}
	'word [word!] {Word to set each time}
	value [number! series! pair! none!] {Maximum number or series to traverse}
	body [block!] {Block to evaluate each time}
]

remove-each: native [
	{Removes values for each block that returns truthy value.}
	'word [word! block!] {Word or block of words to set each time (local)}
	data [series! map!] {The series to traverse (modified)}
	body [block!] {Block to evaluate (return TRUE to remove)}
	/count {Returns removal count}
]

return: native [
	{Returns a value from a function.}
	value [any-type!]
]

switch: native [
	"Selects a choice and evaluates the block that follows it."
	value "Target value"
	cases [block!] "Block of cases to check"
	/default  def "Default case if no others found"
	/all "Evaluate all matches (not just first one)"
	/case "Perform a case-sensitive comparison"
]

throw: native [
	{Throws control back to a previous catch.}
	value [any-type!] {Value returned from catch}
	/name {Throws to a named catch}
	word [word!]
]

trace: native [
	{Enables and disables evaluation tracing and backtrace.}
	mode [integer! logic!]
	/back {Set mode ON to enable or integer for lines to display}
	/function {Traces functions only (less output)}
;	/stack {Show stack index}
]

try: native [
	{Tries to DO a block and returns its value or an error!.}
	block [block! paren!]
	/all    "Catch also BREAK, CONTINUE, RETURN, EXIT and THROW exceptions."
	/with   "On error, evaluate the handler and return its result"
	handler [block! any-function!]
	/except "** DEPRERCATED **"
	code [block! any-function!]
]

unless: native [
	{If FALSE condition, return arg; evaluate blocks by default.}
	condition [any-type!]
	false-branch
	/only "Return block arg instead of evaluating it."
]

until: native [
	{Evaluates a block until it is TRUE. }
	block [block!]
]

while: native [
	{While a condition block is TRUE, evaluates another block.}
	cond-block [block!]
	body-block [block!]
]

;-- Data Natives - nat_data.c

;alias: native [
;	See CC#1835
;	{Creates an alternate spelling for a word.}
;	word [word!] {Word to alias}
;	name [string!] {Name of alias}
;]

;as-binary: native [
;	{Coerces any type of string into a binary! datatype without copying it.}
;	string [any-string!]
;]
;
;as-string: native [
;	{Coerces any type of string into a string! datatype without copying it.}
;	string [any-string!]
;]

as: native [
	{Coerce a series into a compatible datatype without copying it.}
	type [any-block! any-string! datatype!] "The datatype or example value"
	spec [any-block! any-string!] "The series to coerce"
]

bind: native [
	{Binds words to the specified context.}
	word [block! any-word!] {A word or block (modified) (returned)}
	context [any-word! object! module! port!] {A reference to the target context}
	/copy {Bind and return a deep copy of a block, don't modify original}
	/only {Bind only first block (not deep)}
	/new {Add to context any new words found}
	/set {Add to context any new set-words found}
]

unbind: native [
	{Unbinds words from context.}
	word [block! any-word!] {A word or block (modified) (returned)}
	/deep "Process nested blocks"
]

context?: native [ ; was originally named `bound?`
	{Returns the context in which a word is bound.}
	word [any-word!] "Word to check."
	;return: [object! function! none!]
]

;collect-words: native [] ;defined in %n-data.c

;checksum:   native [] ; defined in %n-string.c
;compress:   native [] ; defined in %n-string.c
;decompress: native [] ; defined in %n-string.c

construct: native [
	{Creates an object with scant (safe) evaluation.}
	block [block! string! binary!] "Specification (modified)"
	/with "Default object" object [object!]
	/only "Values are kept as-is"
]

debase: native [
	{Decodes binary-coded string to binary value.}
	value [binary! any-string!] {The string to decode}
	base  [integer!] {Binary base to use: 85, 64, 36, 16, or 2}
	/url  {Base 64 Decoding with URL and Filename Safe Alphabet}
	/part {Limit the length of the input}
	limit [integer! binary! any-string!]
]

enbase: native [
	{Encodes a string into a binary-coded string.}
	value [binary! any-string!] {If string, will be UTF8 encoded}
	base  [integer!] {Binary base to use: 85, 64, 36, 16, or 2}
	/url  {Base 64 Encoding with URL and Filename Safe Alphabet}
	/part {Limit the length of the input}
	limit [integer! binary! any-string!]
	/flat {No line breaks}
]

decloak: native [
	{Decodes a binary string scrambled previously by encloak.}
	data [binary!] "Binary series to descramble (modified)"
	key [string! binary! integer!] "Encryption key or pass phrase"
	/with "Use a string! key as-is (do not generate hash)"
]

encloak: native [
	{Scrambles a binary string based on a key.}
	data [binary!] "Binary series to scramble (modified)"
	key [string! binary! integer!] "Encryption key or pass phrase"
	/with "Use a string! key as-is (do not generate hash)"
]

deline: native [
	"Converts string terminators to standard format, e.g. CRLF to LF."
	string [any-string!] {(modified)}
	/lines "Return block of lines (works for LF, CR, CR-LF endings) (no modify)"
]

enline: native [
	"Converts string terminators to native OS format, e.g. LF to CRLF."
	series [any-string! block!] {(modified)}
]

detab: native [
	"Converts tabs to spaces (default tab size is 4)."
	string [any-string!] {(modified)}
	/size  "Specifies the number of spaces per tab"
	number [integer!]
]

entab: native [
	"Converts spaces to tabs (default tab size is 4)."
	string [any-string!] {(modified)}
	/size "Specifies the number of spaces per tab"
	number [integer!]
]

;delect: ;@@ defined in u-dialec.c

difference: native [
	{Returns the special difference of two values.}
	set1 [block! string! bitset! date! typeset! map!] "First data set"
	set2 [block! string! bitset! date! typeset! map!] "Second data set"
	/case {Uses case-sensitive comparison}
	/skip {Treat the series as records of fixed size}
	size [integer!]
]

exclude: native [
	{Returns the first data set less the second data set.}
	set1 [block! string! bitset! typeset! map!] "First data set"
	set2 [block! string! bitset! typeset! map!] "Second data set"
	/case {Uses case-sensitive comparison}
	/skip {Treat the series as records of fixed size}
	size [integer!]
]

intersect: native [
	{Returns the intersection of two data sets.}
	set1 [block! string! bitset! typeset! map!] "first set"
	set2 [block! string! bitset! typeset! map!] "second set"
	/case {Uses case-sensitive comparison}
	/skip {Treat the series as records of fixed size}
	size [integer!]
]

union: native [
	{Returns the union of two data sets.}
	set1 [block! string! bitset! typeset! map!] "first set"
	set2 [block! string! bitset! typeset! map!] "second set"
	/case {Use case-sensitive comparison}
	/skip {Treat the series as records of fixed size}
	size [integer!]
]

unique: native [
	{Returns the data set with duplicates removed.}
	set1 [block! string! bitset! typeset! map!]
	/case  {Use case-sensitive comparison (except bitsets)}
	/skip {Treat the series as records of fixed size}
	size [integer!]
]

lowercase: native [
	"Converts string of characters to lowercase."
	string [any-string! char!] {(modified if series)}
	/part {Limits to a given length or position}
	length [number! any-string!]
]

uppercase: native [
	"Converts string of characters to uppercase."
	string [any-string! char!] {(modified if series)}
	/part {Limits to a given length or position}
	length [number! any-string!]
]

dehex: native [
	{Converts URL-style hex encoded (%xx) strings. If input is UTF-8 encode, you should first convert it to binary!}
	value [any-string! binary!] {The string to dehex}
	/escape char [char!] {Can be used to change the default escape char #"%"}
	/uri {Decode space from a special char (#"+" by default or #"_" when escape char is #"=")}
]

enhex: native [
	{Converts string into URL-style hex encodeding (%xx) when needed.}
	value [any-string! binary!] {The string to encode}
	/escape {Can be used to change the default escape char #"%"}
	 char [char!] 
	/except {Can be used to specify, which chars can be left unescaped}
	 unescaped [bitset!] {By default it is URI bitset when value is file or url, else URI-Component}
	/uri {Encode space using a special char (#"+" by default or #"_" when escape char is #"=")}
]

get: native [
	{Gets the value of a word or path, or values of an object.}
	word {Word, path, object to get}
	/any {Allows word to have no value (allows unset)}
]

in: native [
	{Returns the word or block in the object's context.}
	object [any-object! block!]
	word [any-word! block! paren!]  {(modified if series)}
]

parse: native [
	{Parses a string or block series according to grammar rules.}
	input [series!] {Input series to parse}
	rules [block!] {Rules to parse}
	;/all {For simple rules (not blocks) parse all chars including whitespace}
	/case {Uses case-sensitive comparison}
]

set: native [
	{Sets a word, path, block of words, or object to specified value(s).}
	word [word! lit-word! any-path! block! object!] {Word, block of words, path, or object to be set (modified)}
	value [any-type!] {Value or block of values}
	/any {Allows setting words to any value, including unset}
	/only {Block or object value argument is set as a single value}
	/some {None values in a block or object value argument, are not set}
]

to-hex: native [
	{Converts numeric value to a hex issue! datatype (with leading # and 0's).}
	value [integer! tuple!] {Value to be converted}
	/size {Specify number of hex digits in result}
	len [integer!]
]

type?: native [
	{Returns the datatype of a value.}
	value [any-type!]
	/word {Returns the datatype as a word}
]

unset: native [
	{Unsets the value of a word (in its current context.)}
	word [word! block! none!] {Word or block of words}
]

utf?: native [
	{Returns UTF BOM (byte order marker) encoding; + for BE, - for LE.}
	data [binary!]
]

invalid-utf?: native [
	{Checks UTF encoding; if correct, returns none else position of error.}
	data [binary!]
	/utf "Check encodings other than UTF-8"
	num [integer!] "Bit size - positive for BE negative for LE"
]

value?: native [
	{Returns TRUE if the word has a value.}
	value [word!]
]

to-value: native [
	{Returns the value if it is a value, NONE if unset.}
	value [any-type!]
]

split-lines: native [
	{Given a string series, split lines on CR-LF.}
	value [string!]
]

;-- IO Natives - nat_io.c

print: native [
	{Outputs a value followed by a line break.}
	value [any-type!] {The value to print}
]

prin: native [
	{Outputs a value with no line break.}
	value [any-type!]
]

mold: native [
	{Converts a value to a REBOL-readable string.}
	value [any-type!] {The value to mold}
	/only {For a block value, mold only its contents, no outer []}
	/all  {Use construction syntax}
	/flat {No indentation}
	/part {Limit the length of the result}
	limit [integer!]
]

form: native [
	{Converts a value to a human-readable string.}
	value [any-type!] {The value to form}
]

new-line: native [
	{Sets or clears the new-line marker within a block or paren.}
	position [block! paren!] {Position to change marker (modified)}
	value {Set TRUE for newline}
	/all {Set/clear marker to end of series}
	/skip {Set/clear marker periodically to the end of the series}
	size [integer!]
]

new-line?: native [
	{Returns the state of the new-line marker within a block or paren.}
	position [block! paren!] {Position to check marker}
]

to-local-file: native [
	{Converts a REBOL file path to the local system file path.}
	path [file! string!]
	/full "Prepends current dir for full path (for relative paths only)"
]

to-rebol-file: native [
	{Converts a local system file path to a REBOL file path.}
	path [file! string!]
]

transcode: native [
	{Translates UTF-8 binary source to values. Returns one or several values in a block.}
	source [binary! string!] "UTF-8 input buffer; string argument will be UTF-8 encoded"
	/next "Translate next complete value (blocks as single value)"
	/one  "Translate next complete value (returns the value only)"
	/only "Translate only a single value (blocks dissected)"
	/error "Do not cause errors - return error object as value in place"
	/line  "Return also information about number of lines scaned"
	 count [integer!] "Initial line number"
]

echo: native [
    {Copies console output to a file.}
    target [file! none! logic!]
]

now: native [
	{Returns date and time.}
	/year {Returns year only}
	/month {Returns month only}
	/day {Returns day of the month only}
	/time {Returns time only}
	/zone {Returns time zone offset from UCT (GMT) only}
	/date {Returns date only}
	/weekday {Returns day of the week as integer (Monday is day 1)}
	/yearday {Returns day of the year (Julian)}
	/precise {High precision time}
	/utc {Universal time (no zone)}
]

wait: native [
	{Waits for a duration, port, or both.}
	value [number! time! port! block! none!]
	/all {Returns all in a block}
	/only {only check for ports given in the block to this function}
]

wake-up: native [
	{Awake and update a port with event.}
	port [port!]
	event [event!]
]

what-dir: native ["Returns the current directory path."]

change-dir: native [
	"Changes the current directory path."
	path [file!]
]

;dir?: in %n-io.c
;wildcard?: in %n-io.c

;-- Series Natives

first: native [
	{Returns the first value of a series.}
	value
]

second: native [
	{Returns the second value of a series.}
	value
]

third: native [
	{Returns the third value of a series.}
	value
]

fourth: native [
	{Returns the fourth value of a series.}
	value
]

fifth: native [
	{Returns the fifth value of a series.}
	value
]

sixth: native [
	{Returns the sixth value of a series.}
	value
]

seventh: native [
	{Returns the seventh value of a series.}
	value
]

eighth: native [
	{Returns the eighth value of a series.}
	value
]

ninth: native [
	{Returns the ninth value of a series.}
	value
]

tenth: native [
	{Returns the tenth value of a series.}
	value
]

last: native [
	{Returns the last value of a series.}
	value [series! tuple! gob!]
]

;-- Math Natives - nat_math.c

cosine: native [
	{Returns the trigonometric cosine.}
	value [number!] {In degrees by default}
	/radians {Value is specified in radians}
]

sine: native [
	{Returns the trigonometric sine.}
	value [number!] {In degrees by default}
	/radians {Value is specified in radians}
]

tangent: native [
	{Returns the trigonometric tangent.}
	value [number!] {In degrees by default}
	/radians {Value is specified in radians}
]

arccosine: native [
	{Returns the trigonometric arccosine (in degrees by default).}
	value [number!]
	/radians {Returns result in radians}
]

arcsine: native [
	{Returns the trigonometric arcsine (in degrees by default).}
	value [number!]
	/radians {Returns result in radians}
]

arctangent: native [
	{Returns the trigonometric arctangent (in degrees by default).}
	value [number!]
	/radians {Returns result in radians}
]

exp: native [
	{Raises E (the base of natural logarithm) to the power specified}
	power [number!]
]

log-10: native [
	{Returns the base-10 logarithm.}
	value [number!]
]

log-2: native [
	{Return the base-2 logarithm.}
	value [number!]
]

log-e: native [
	{Returns the natural (base-E) logarithm of the given value} 
	value [number!]
]

not: native [
	{Returns the logic complement.}
	value [any-type!] {(Only FALSE and NONE return TRUE)}
]

square-root: native [
	{Returns the square root of a number.}
	value [number!]
]

shift: native [
	{Shifts an integer left or right by a number of bits.}
	value [integer!]
	bits [integer!] "Positive for left shift, negative for right shift"
	/logical "Logical shift (sign bit ignored)"
]

;-- New, hackish stuff:

;shift-left:  ;@@ defined in n-math.c
;shift-right: ;@@ defined in n-math.c

++: native [
	{Increment an integer or series index. Return its prior value.}
	'word [word!] "Integer or series variable"
]

--: native [
	{Decrement an integer or series index. Return its prior value.}
	'word [word!] "Integer or series variable"
]

first+: native [
	{Return the FIRST of a series then increment the series index.}
	'word [word!] "Word must refer to a series"
]

stack: native [
	{Returns stack backtrace or other values.}
	offset [integer!] "Relative backward offset"
	/block "Block evaluation position"
	/word "Function or object name, if known"
	/func "Function value"
	/args "Block of args (may be modified)"
	/size "Current stack size (in value units)"
	/depth "Stack depth (frames)"
	/limit "Stack bounds (auto expanding)"
]

resolve: native [
	{Copy context by setting values in the target from those in the source.}
	target [any-object!] {(modified)}
	source [any-object!]
	/only from [block! integer!] "Only specific words (exports) or new words in target (index to tail)"
	/all "Set all words, even those in the target that already have a value"
	/extend "Add source words to the target if necessary"
]

;in-context: native [
;	{Set the default context for global words.}
;	context [object!]
;]

get-env: native [
	{Returns the value of an OS environment variable (for current process).}
	var [any-string! any-word!]
]

set-env: native [
	{Sets the value of an OS environment variable (for current process).} 
	var [any-string! any-word!] "Variable to set" 
	value [string!  none!] "Value to set, or NONE to unset it"
]

list-env: native [
	{Returns a map of OS environment variables (for current process).}
]

call: native [
	{Run another program; return immediately.}
	command [any-string! block! file!] "An OS-local command line (quoted as necessary), a block with arguments, or an executable file"
	/wait "Wait for command to terminate before returning"
	/console "Runs command with I/O redirected to console"
	/shell "Forces command to be run from shell"
	/info "Returns process information object"
	/input in [string! binary! file! none!] "Redirects stdin to in"
	/output out [string! binary! file! none!] "Redirects stdout to out"
	/error err [string! binary! file! none!] "Redirects stderr to err"
]


browse: native [
	{Open web browser to a URL or local file.}
	url [url! file! none!]
]

evoke: native [
	{Special guru meditations. (Not for beginners.)}
	chant [word! block! integer!] "Single or block of words ('? to list)"
]

request-file: native [
	{Asks user to select a file and returns full file path (or block of paths).}
	/save  "File save mode"
	/multi "Allows multiple file selection, returned as a block"
	/file  "Default file name or directory"
	 name  [file!]
	/title "Window title"
	 text  [string!]
	/filter "Block of filters (filter-name filter)"
	 list  [block!]
]

request-dir: native [
	{Asks user to select a directory and returns full directory path (or block of paths).}
	/title "Change heading on request"
	 text  [string!]
	/dir   "Set starting directory" 
	 name  [file!]
	/keep  "Keep previous directory path"
]

request-password: native [
	{Asks user for input without echoing, and the entered password is not stored in the command history.}
]

ascii?: native [
	{Returns TRUE if value or string is in ASCII character range (below 128).}
	value [any-string! char! integer!]
]

latin1?: native [
	{Returns TRUE if value or string is in Latin-1 character range (below 256).}
	value [any-string! char! integer!]
]

; Temps...

stats: native [
	{Provides status and statistics information about the interpreter.}
	/show {Print formatted results to console}
	/profile {Returns profiler object}
	/timer {High resolution time difference from start}
	/evals {Number of values evaluated by interpreter}
	/dump-series pool-id [integer!] {Dump all series in pool pool-id, -1 for all pools}
]

do-codec: native [
	{Evaluate a CODEC function to encode or decode media types.}
	handle [handle!] "Internal link to codec"
	action [word!] "Decode, encode, identify"
	data [binary! image! string!]
]

set-scheme: native [
	"Low-level port scheme actor initialization."
	scheme [object!]
]

load-extension: native [
	"Low level extension module loader (for DLLs)."
	name [file! binary!] "DLL file or UTF-8 source"
	/dispatch "Specify native command dispatch (from hosted extensions)"
	function [handle!] "Command dispatcher (native)"
]

do-commands: native [
	"Evaluate a block of extension module command functions (special evaluation rules.)"
	commands [block!] "Series of commands and their arguments"
]

ds: native ["Temporary stack debug"]
dump: native ["Temporary debug dump" v /fmt "only series format"]
check: native ["Temporary series debug check" val [series!]]

do-callback: native [
	"Internal function to process callback events."
	event [event!] "Callback event"
]


limit-usage: native [
	"Set a usage limit only once (used for SECURE)."
	field [word!] "eval (count) or memory (bytes)"
	limit [number!]
]

selfless?: native [
    "Returns true if the context doesn't bind 'self."
    context [any-word! any-object!] "A reference to the target context"
]

map-event: native [
	"Returns event with inner-most graphical object and coordinate."
	event [event!]
]

map-gob-offset: native [
	"Translates a gob and offset to the deepest gob and offset in it, returned as a block."
	gob [gob!] "Starting object"
	xy [pair!] "Staring offset"
	/reverse "Translate from deeper gob to top gob."
]

as-pair: native [
	"Combine X and Y values into a pair."
	x [number!]
	y [number!]
]

;read-file: native [f [file!]]

equal?: native [
	{Returns TRUE if the values are equal.}
	value1 [any-type!]
	value2 [any-type!]
]

not-equal?: native [
	{Returns TRUE if the values are not equal.}
	value1 [any-type!]
	value2 [any-type!]
]

equiv?: native [
	{Returns TRUE if the values are equivalent.}
	value1 [any-type!]
	value2 [any-type!]
]

not-equiv?: native [
	{Returns TRUE if the values are not equivalent.}
	value1 [any-type!]
	value2 [any-type!]
]

strict-equal?: native [
	{Returns TRUE if the values are strictly equal.}
	value1 [any-type!]
	value2 [any-type!]
]

strict-not-equal?: native [
	{Returns TRUE if the values are not strictly equal.}
	value1 [any-type!]
	value2 [any-type!]
]

same?: native [
	{Returns TRUE if the values are identical.}
	value1 [any-type!]
	value2 [any-type!]
]

greater?: native [ ; Note: some datatypes expect >, <, >=, <= to be in this order.
	{Returns TRUE if the first value is greater than the second value.}
	value1 value2
]

greater-or-equal?: native [
	{Returns TRUE if the first value is greater than or equal to the second value.}
	value1 value2
]

lesser?: native [
	{Returns TRUE if the first value is less than the second value.}
	value1 value2
]

lesser-or-equal?: native [
	{Returns TRUE if the first value is less than or equal to the second value.}
	value1 value2
]

minimum: native [
	{Returns the lesser of the two values.}
	value1 [scalar! date! series!]
	value2 [scalar! date! series!]
]

maximum: native [ ; Note: Some datatypes expect all binary ops to be <= this
	{Returns the greater of the two values.}
	value1 [scalar! date! series!]
	value2 [scalar! date! series!]
]

negative?: native [
	{Returns TRUE if the number is negative.}
	number [number! money! time! pair!]
]

positive?: native [
	{Returns TRUE if the value is positive.}
	number [number! money! time! pair!]
]

zero?: native [
	{Returns TRUE if the value is zero (for its datatype).}
	value
]
