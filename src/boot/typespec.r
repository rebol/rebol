REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Datatype help spec"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		Provides useful information about datatypes.
		Can be expanded to include info like min-max ranges.
	}
]

action		["datatype native function (standard polymorphic)" function]
binary		["string series of bytes" string]
bitset		["set of bit flags" string]
block		["series of values" block]
char		["8bit and 16bit character" scalar]
closure		["function with persistent locals (indefinite extent)" function]
datatype	["type of datatype" symbol]
date		["day, month, year, time of day, and timezone" scalar]
decimal		["64bit floating point number (IEEE standard)" scalar]
email		["email address" string]
end			["internal marker for end of block" internal]
error		["errors and throws" object]
event		["user interface event (efficiently sized)" opt-object]
file		["file name or path" string]
frame		["internal context frame" internal]
function	["interpreted function (user-defined or mezzanine)" function]
get-path	["the value of a path" block]
get-word	["the value of a word (variable)" word]
gob			["graphical object" opt-object]
handle		["arbitrary internal object or value" internal]
image		["RGB image with alpha channel" vector]
integer		["64 bit integer" scalar]
issue		["identifying marker word" word]
library		["external library reference" internal]
lit-path	["literal path value" block]
lit-word	["literal word value" word]
logic		["boolean true or false" scalar]
map			["name-value pairs (hash associative)" block]
module		["loadable context of code and data" object]
money		["high precision decimals with denomination (opt)" scalar]
native		["direct CPU evaluated function" function]
none		["no value represented" scalar]
object		["context of names with values" object]
op			["infix operator (special evaluation exception)" function]
pair		["two dimensional point or size" scalar]
paren		["automatically evaluating block" block]
path		["refinements to functions, objects, files" block]
percent		["special form of decimals (used mainly for layout)" scalar]
port		["external series, an I/O channel" object]
rebcode		["virtual machine function" block]
refinement	["variation of meaning or location" word]
command		["special dispatch-based function" function]
set-path	["definition of a path's value" block]
set-word	["definition of a word's value" word]
string		["string series of characters" string]
struct		["native structure definition" block]
tag			["markup string (HTML or XML)" string]
task		["evaluation environment" object]
time		["time of day or duration" scalar]
tuple		["sequence of small integers (colors, versions, IP)" scalar]
typeset		["set of datatypes" opt-object]
unicode		["string of unicoded characters" string]
unset		["no value returned or set" internal]
url			["uniform resource locator or identifier" string]
utype		["user defined datatype" object]
vector		["high performance arrays (single datatype)" vector]
word		["word (symbol or variable)" word]

