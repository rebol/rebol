REBOL [
	Title:   "Codec: UTC time as used in ASN.1 data structures (BER/DER)"
	Author:  "Oldes"
	Rights:  "Copyright (C) 2018 Oldes. All rights reserved."
	License: "BSD-3"
	Test:    %tests/units/crypt-test.r3
]

register-codec [
	name: 'utc-time
	type: 'time
	title: "UTC time as used in ASN.1 data structures (BER/DER)"
	decode: function [
		"Converts DER/BER UTC-time data to Rebol date! value"
		utc [binary! string!]
	][
		ch_digits: charset [#"0" - #"9"]
		parse utc [
			insert "20"
			  2 ch_digits   insert #"-"
			  2 ch_digits   insert #"-"
			  2 ch_digits   insert #"/"
			  2 ch_digits   insert #":"
			  2 ch_digits   insert #":" 
			[ 2 ch_digits | insert #"0"] ;seconds
			[
				remove #"Z" end
				|
				[#"-" | #"+"]
				2 ch_digits insert #":" 
			]
		]
		try [load utc]
	]
]
