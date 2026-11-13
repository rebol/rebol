Rebol [
	title: "to-ascii"
	purpose: "Latin to ASCII characters transliteration"
	name:   to-ascii
	type:   module
	options: [delay]
	version: 0.1.2
	exports: [to-ascii]
	author: @Oldes
	file: %to-ascii.reb
	home: https://src.rebol.tech/modules/to-ascii.reb
]
to-ascii: function [
	"Return a string with characters transliterated to ASCII"
	text [any-string!] "String to be converted"
	/keep "Keep unhandled non-ASCII characters"
][
	out: make string! length? text
	parse text [any [
		  copy s: some ascii-chars (append out s)
		| set s: convert (
			append out pick replacements select/case table s
		)
		| set s: skip (if keep [append out s])
	]]
	as type? text out
]

ascii-chars: make bitset! #{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}
replacements: [
	#"a" #"A" "ae" "AE" #"c" #"C" #"D" #"d" #"E" #"e" #"f" #"G" #"g" #"H" #"h"
	#"i" #"I" "IJ" "ij" #"J" #"j" #"K" #"k" #"L" #"l" #"N" #"n" #"o" #"O" "oe"
	"OE" #"r" #"R" #"s" #"S" #"T" #"t" #"u" #"U" #"W" #"w" #"Y" #"y" #"Z" #"z"
] 
table: make map! [
	#"á" 1  ;= a
	#"â" 1
	#"ą" 1
	#"ă" 1
	#"ā" 1
	#"ǻ" 1
	#"ǎ" 1
	#"å" 1
	#"ä" 1
	#"ã" 1
	#"à" 1
	#"Ā" 2  ;= A
	#"Ä" 2
	#"Ã" 2
	#"Â" 2
	#"Á" 2
	#"À" 2
	#"Ǎ" 2
	#"Ǻ" 2
	#"Ą" 2
	#"Ă" 2
	#"Å" 2
	#"æ" 3  ;= ae
	#"ǽ" 3
	#"Æ" 4  ;= AE
	#"Ǽ" 4
	#"ć" 5  ;= c
	#"ç" 5
	#"ċ" 5
	#"č" 5
	#"ĉ" 5
	#"Č" 6  ;= C
	#"Ç" 6
	#"Ć" 6
	#"Ĉ" 6
	#"Ċ" 6
	#"Đ" 7  ;= D
	#"Ð" 7
	#"Ď" 7
	#"đ" 8  ;= d
	#"ð" 8
	#"ď" 8
	#"Ë" 9  ;= E
	#"Ě" 9
	#"Ē" 9
	#"È" 9
	#"É" 9
	#"Ê" 9
	#"Ę" 9
	#"Ė" 9
	#"Ĕ" 9
	#"Ȩ" 9
	#"ē" 10 ;= e
	#"ė" 10
	#"ë" 10
	#"ê" 10
	#"é" 10
	#"ę" 10
	#"è" 10
	#"ě" 10
	#"ĕ" 10
	#"ȩ" 10
	#"ƒ" 11 ;= f
	#"Ğ" 12 ;= G
	#"Ġ" 12
	#"Ģ" 12
	#"Ĝ" 12
	#"ģ" 13 ;= g
	#"ğ" 13
	#"ĝ" 13
	#"ġ" 13
	#"Ħ" 14 ;= H
	#"Ĥ" 14
	#"ĥ" 15 ;= h
	#"ħ" 15
	#"ī" 16 ;= i
	#"ǐ" 16
	#"ì" 16
	#"í" 16
	#"î" 16
	#"ï" 16
	#"ĩ" 16
	#"ĭ" 16
	#"į" 16
	#"ı" 16
	#"Í" 17 ;= I
	#"Ĩ" 17
	#"Ī" 17
	#"Ĭ" 17
	#"Į" 17
	#"Ǐ" 17
	#"İ" 17
	#"Ï" 17
	#"Î" 17
	#"Ì" 17
	#"Ĳ" 18 ;= IJ
	#"ĳ" 19 ;= ij
	#"Ĵ" 20 ;= J
	#"ĵ" 21 ;= j
	#"Ķ" 22 ;= K
	#"ķ" 23 ;= k
	#"Ŀ" 24 ;= L
	#"Ľ" 24
	#"Ļ" 24
	#"Ĺ" 24
	#"ł" 25 ;= l
	#"ĺ" 25
	#"ŀ" 25
	#"Ł" 25
	#"ļ" 25
	#"ľ" 25
	#"Ņ" 26 ;= N
	#"Ň" 26
	#"Ñ" 26
	#"Ń" 26
	#"ň" 27 ;= n
	#"ñ" 27
	#"ņ" 27
	#"ń" 27
	#"ŉ" 27
	#"ǿ" 28 ;= o
	#"ò" 28
	#"ó" 28
	#"ô" 28
	#"õ" 28
	#"ö" 28
	#"ø" 28
	#"ō" 28
	#"ŏ" 28
	#"ő" 28
	#"ơ" 28
	#"ǒ" 28
	#"Ō" 29 ;= O
	#"Ǿ" 29
	#"Õ" 29
	#"Ơ" 29
	#"Ŏ" 29
	#"Ô" 29
	#"Ő" 29
	#"Ó" 29
	#"Ò" 29
	#"Ǒ" 29
	#"Ö" 29
	#"Ø" 29
	#"œ" 30 ;= oe
	#"Œ" 31 ;= OE
	#"ŕ" 32 ;= r
	#"ŗ" 32
	#"ř" 32
	#"Ŕ" 33 ;= R
	#"Ŗ" 33
	#"Ř" 33
	#"ś" 34 ;= s
	#"ß" 34
	#"ſ" 34
	#"ş" 34
	#"š" 34
	#"ŝ" 34
	#"Ś" 35 ;= S
	#"Ş" 35
	#"Š" 35
	#"Ŝ" 35
	#"Ŧ" 36 ;= T
	#"Ť" 36
	#"Ţ" 36
	#"ŧ" 37 ;= t
	#"ţ" 37
	#"ť" 37
	#"ú" 38 ;= u
	#"ũ" 38
	#"ū" 38
	#"ŭ" 38
	#"ů" 38
	#"ű" 38
	#"ų" 38
	#"ư" 38
	#"ǔ" 38
	#"ǖ" 38
	#"ǘ" 38
	#"ǚ" 38
	#"ǜ" 38
	#"ü" 38
	#"û" 38
	#"ù" 38
	#"Ǘ" 39 ;= U
	#"Ų" 39
	#"Ǚ" 39
	#"Ü" 39
	#"Ű" 39
	#"Ů" 39
	#"Ư" 39
	#"Ǜ" 39
	#"Ǖ" 39
	#"Û" 39
	#"Ú" 39
	#"Ù" 39
	#"Ŭ" 39
	#"Ū" 39
	#"Ũ" 39
	#"Ǔ" 39
	#"Ŵ" 40 ;= W
	#"ŵ" 41 ;= w
	#"Ŷ" 42 ;= Y
	#"Ÿ" 42
	#"Ý" 42
	#"ý" 43 ;= y
	#"ÿ" 43
	#"ŷ" 43
	#"Ž" 44 ;= Z
	#"Ź" 44
	#"Ż" 44
	#"ź" 45 ;= z
	#"ž" 45
	#"ż" 45
]
convert: make bitset! keys-of table 