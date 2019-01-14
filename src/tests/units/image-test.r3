Rebol [
	Title:   "Rebol image test script"
	Author:  "Oldes"
	File: 	 %image-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Image"

===start-group=== "make image"
	--test-- "make image using pair"
		--assert image? img: make image! 2x2
	--test-- "make image using pair and RGB tuple"
		--assert image? img: make image! [2x2 255.0.0]
		--assert 255.0.0.255 = img/1
	--test-- "make image using pair and RGBA tuple"
		--assert image? img: make image! [2x2 255.0.0.10]
		--assert 255.0.0.10 = img/1
===end-group===

===start-group=== "image range"
	img: make image! 2x2
	--test-- "image range (integer index)"
		--assert none = img/0
		--assert error? try [img/0: 1.2.3]
		--assert img/1 = 255.255.255.255
		--assert 1.2.3 = img/1: 1.2.3
		--assert img/2 = 255.255.255.255
		--assert 1.2.3 = img/2: 1.2.3
		--assert img/3 = 255.255.255.255
		--assert 1.2.3 = img/3: 1.2.3
		--assert img/4 = 255.255.255.255
		--assert 1.2.3 = img/4: 1.2.3
		--assert none = img/5
		--assert error? try [img/5: 1.2.3]
	img: make image! 2x2
	--test-- "image range (pair index)"
		--assert none = img/(0x0)
		--assert error? try [img/(0x0): 1.2.3]
		--assert none = img/(0x1)
		--assert error? try [img/(0x1): 1.2.3]
		--assert none = img/(1x0)
		--assert error? try [img/(1x0): 1.2.3]
		--assert img/(1x1) = 255.255.255.255
		--assert 1.2.3 = img/(1x1): 1.2.3
		--assert img/(1x2) = 255.255.255.255
		--assert 1.2.3 = img/(1x2): 1.2.3
		--assert img/(2x1) = 255.255.255.255
		--assert 1.2.3 = img/(2x1): 1.2.3
		--assert img/(2x2) = 255.255.255.255
		--assert 1.2.3 = img/(2x2): 1.2.3
		--assert none = img/(1x3)
		--assert error? try [img/(1x3): 1.2.3]
		;--assert none = img/(3x1)
		;--assert error? try [img/(3x1): 1.2.3]
		--assert none = img/(2x3)
		--assert error? try [img/(2x3): 1.2.3]
		--assert none = img/(3x2)
		--assert error? try [img/(3x2): 1.2.3]
===end-group===

===start-group=== "image pixel assignment validity"
	--test-- "image pixel 3-tuple assignment"
		--assert 255.255.255 = img/1: 255.255.255
	--test-- "image pixel 4-tuple assignment"
		--assert 255.255.255.255 = img/2: 255.255.255.255
	--test-- "image pixel 5-tuple assignment"
		--assert 1.2.3.4.5 = img/3: 1.2.3.4.5
		--assert 1.2.3.4 = img/3
	--test-- "image pixel junk assignment"
		--assert error? try [img/1: "junk"]
		--assert error? try [img/1: []]
		--assert error? try [img/1: 3.14]
	--test-- "image pixel unaffected by junk assignments?"
		--assert 255.255.255.255 = img/1
===end-group===

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

===start-group=== "Save/load image"
	if find words-of system/codecs 'png [
		--test-- "save/load PNG"
			img1: make image! [2x2 255.0.0.10]
			save %units/files/test.png img1
			img2: load %units/files/test.png
			--assert #{FF00000AFF00000AFF00000AFF00000A} = to binary! img2
	]
	if find words-of system/codecs 'bmp [
		--test-- "save/load BMP"
			img1: make image! [2x2 255.0.0.10]
			;@@ Current BMP save discards the transparency info!
			save %units/files/test.bmp img1
			img2: load %units/files/test.bmp
			--assert #{FF0000FFFF0000FFFF0000FFFF0000FF} = to binary! img2
	]

~~~end-file~~~

;clean up
error? try [
	delete %units/files/test.png
	delete %units/files/test.bmp
]