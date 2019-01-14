Rebol [
	Title:   "Rebol image test script"
	Author:  "Oldes"
	File: 	 %image-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Image"

===start-group=== "RGB - HSV conversions"
;@@ https://github.com/rebol/rebol-issues/issues/2342

--test-- "RGB-TO-HSV"
	--assert 36.235.134     = rgb-to-hsv 134.116.10
	--assert 36.235.134.100 = rgb-to-hsv 134.116.10.100
	--assert 0.0.134        = rgb-to-hsv 134.134.134
	--assert 42.253.134     = rgb-to-hsv 134.134.1
--test-- "HSV-TO-RGB"	
	--assert 5.9.10     = hsv-to-rgb 134.116.10
	--assert 5.9.10.100 = hsv-to-rgb 134.116.10.100
	--assert 63.123.134 = hsv-to-rgb 134.134.134
	--assert 0.1.2      = hsv-to-rgb 134.134.2

===end-group===


~~~end-file~~~