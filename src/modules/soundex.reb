REBOL [
	Title: "Soundex"
	Date: 16-Jul-2024
	File: %soundex.reb
	Author: "Allen Kamp, Oldes"
	Purpose: {Soundex Encoding returns similar codes for similar sounding words or names. eg Stephens, Stevens are both S315, Smith and Smythe are both S53. Useful for adding Sounds-like searching to databases}
	Comment: {
		This is the basic Soundex algorithm: https://en.wikipedia.org/wiki/Soundex

		1. Remove vowels, H, W and Y
		2. Encode each char with its code value
		3. Remove adjacent duplicate numbers

		4. Return First letter, followed by the next 3 letter's code
		   numbers, if they exist.

		TODO: Other algorithms: Extended Soundex, Metaphone and the LC Cutter table
	}
	Version: 2.0.0
	Type: module
	Name: soundex
	Exports: [soundex]
	Needs: 3.0.0
	History: [
		17-Jul-1999 @Allen "Initial version"
		16-Jul-2024 @Oldes "Ported to Rebol3"

	]
]

soundex: function/with [
	{Returns the Census Soundex Code for the given string}
	string [any-string!] "String to Encode"
][
	code: make string! 4
	prev: none

	if empty? string [return "0000"]

	foreach letter string [
		either val: mapping/:letter [ 
			if val != prev [append code val prev: val]
		][
			if find "aeiouhwy" letter [prev: #" "]
			if empty? code [append code #"0"]
		]
		if 4 = length? code [break] ;maximum length for code is 4
	]
	change code uppercase first string
	pad/with code 4 #"0"
	code
][
	code: val: prev: none 
	mapping: make map! [
		;Set1
		#"B" #"1"
		#"F" #"1"
		#"P" #"1"
		#"V" #"1"
		;Set2
		#"C" #"2"
		#"G" #"2"
		#"J" #"2"
		#"K" #"2"
		#"Q" #"2"
		#"S" #"2"
		#"X" #"2"
		#"Z" #"2"
		;Set3
		#"D" #"3"
		#"T" #"3"
		;Set4
		#"L" #"4"
		;Set5
		#"M" #"5"
		#"N" #"5"
		;Set6
		#"R" #"6"
	]
]

