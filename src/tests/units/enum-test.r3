Rebol [
	Title:   "Rebol enum test script"
	Author:  "Oldes"
	File: 	 %enum-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Enum"

===start-group=== "Basic enumeration"
--test-- "enum with basic specification"
	*FX-DX8: enum [
		CHORUS
		COMPRESSOR
		DISTORTION
		ECHO
		FLANGER
		GARGLE
		I3DL2REVERB
		PARAMEQ
		REVERB
	] "DX8 effect ID"

	--assert object? *FX-DX8
	--assert 0 = *FX-DX8/CHORUS
	--assert 8 = *FX-DX8/REVERB
	--assert found? find [ECHO FLANGER] *FX-DX8/name 4


--test-- "enum with mixed specification"
	*family: enum [
		Alice: 1
		Boban
		Bolek
		Lolek:  #{FF}
		Brian
	] 'Just-Some-Names

	--assert object?  *family
	--assert 'Boban = *family/name 2
	--assert 'Lolek = *family/name 255
	--assert none? *family/name 13
	--assert true? *family/assert 1
	--assert 256 = *family/Brian
	--assert error? err: try [*family/assert 13]
	--assert err/arg1 = 13 
	--assert err/arg2 = 'Just-Some-Names

===end-group===

~~~end-file~~~