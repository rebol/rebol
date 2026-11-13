Rebol [
	Title:   "Rebol native test script"
	Author:  "Oldes"
	File: 	 %native-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "NATIVE"


===start-group=== "CLAMP"
	;-- integer!
	--test-- "clamp integer in range"
	--assert (clamp 5 0 10) == 5
	--test-- "clamp integer below minimum"
	--assert (clamp -3 0 10) == 0
	--test-- "clamp integer above maximum"
	--assert (clamp 15 0 10) == 10
	--test-- "clamp integer at minimum"
	--assert (clamp 0 0 10) == 0
	--test-- "clamp integer at maximum"
	--assert (clamp 10 0 10) == 10

	;-- decimal!
	--test-- "clamp decimal in range"
	--assert (clamp 0.5 0.0 1.0) == 0.5
	--test-- "clamp decimal below minimum"
	--assert (clamp -0.5 0.0 1.0) == 0.0
	--test-- "clamp decimal above maximum"
	--assert (clamp 1.5 0.0 1.0) == 1.0

	;-- percent!
	--test-- "clamp percent in range"
	--assert (clamp 50% 0% 100%) == 50%
	--test-- "clamp percent below minimum"
	--assert (clamp -10% 0% 100%) == 0%
	--test-- "clamp percent above maximum"
	--assert (clamp 150% 0% 100%) == 100%

;	;-- money!
;	--test-- "clamp money in range"
;	--assert (clamp $5.00 $0.00 $10.00) == $5.00
;	--test-- "clamp money below minimum"
;	--assert (clamp $-3.00 $0.00 $10.00) == $0.00
;	--test-- "clamp money above maximum"
;	--assert (clamp $15.00 $0.00 $10.00) == $10.00

	;-- pair!
	--test-- "clamp pair in range"
	--assert (clamp 5x5 0x0 10x10) == 5x5
	--test-- "clamp pair below minimum"
	--assert (clamp -1x-1 0x0 10x10) == 0x0
	--test-- "clamp pair above maximum"
	--assert (clamp 15x15 0x0 10x10) == 10x10
	--test-- "clamp pair mixed"
	--assert (clamp -1x15 0x0 10x10) == 0x10

	;-- tuple!
	--test-- "clamp tuple in range"
	--assert (clamp 128.128.128 0.0.0 255.255.255) == 128.128.128
	--test-- "clamp tuple below minimum"
	--assert (clamp 0.0.0 10.10.10 255.255.255) == 10.10.10
	--test-- "clamp tuple above maximum"
	--assert (clamp 255.255.255 0.0.0 200.200.200) == 200.200.200
	--test-- "clamp tuple mixed"
	--assert (clamp 5.128.250 10.10.10 200.200.200) == 10.128.200
	--test-- "clamp tuple longer vmin"
	--assert (clamp 5.128.250 10.10.10.10 200.200.200) == 10.128.200
	--assert (clamp 5.128.250.0 10.10.10.10 200.200.200) == 10.128.200.10
	--test-- "clamp tuple longer vmax"
	--assert (clamp 5.128.50 10.10.10 200.200.200.100) == 10.128.50
	--assert (clamp 5.128.50.200 10.10.10 200.200.200.100) == 10.128.50.100

	;-- type mismatch errors
	--test-- "clamp type mismatch value/minimum"
	--assert error? try [clamp 5 0.0 10]
	--test-- "clamp type mismatch value/maximum"
	--assert error? try [clamp 5 0 10.0]
	--test-- "clamp decimal vs integer"
	--assert error? try [clamp 0.5 0 1]
===end-group===


~~~end-file~~~
